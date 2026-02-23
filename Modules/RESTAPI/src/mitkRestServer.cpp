/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestServer.h>
#include "mitkDataStorageBridge.h"
#include "mitkHealthController.h"
#include "mitkDataStorageController.h"
#include "mitkSwaggerController.h"
#include "mitkErrorResponse.h"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <httplib.h>
#include <mitkIOUtil.h>
#include <mitkLog.h>
#include <mitkFileSystem.h>

#include <openssl/crypto.h>

#include <algorithm>

namespace mitk
{

namespace
{
  /** Number of rate-limit checks between stale-entry cleanup sweeps. */
  static constexpr uint64_t kRateLimitCleanupInterval = 100;

  /**
   * @brief Sanitize a string for safe inclusion in log messages.
   *
   * Replaces control characters (newlines, tabs, etc.) with their escaped
   * representations to prevent log injection attacks.
   *
   * @param input The raw string from external input.
   * @return Sanitized string safe for logging.
   */
  std::string SanitizeForLog(const std::string& input)
  {
    std::string result;
    result.reserve(input.size());

    for (const char c : input)
    {
      if (c == '\n')
        result += "\\n";
      else if (c == '\r')
        result += "\\r";
      else if (c == '\t')
        result += "\\t";
      else if (c >= 0 && c < 0x20 && c != ' ')
        result += "\\x" + std::string(1, "0123456789abcdef"[(c >> 4) & 0xF]) +
                  std::string(1, "0123456789abcdef"[c & 0xF]);
      else
        result += c;
    }

    return result;
  }

  /**
   * @brief Check if an IP address is a localhost address.
   */
  bool IsLocalhostIP(const std::string& ip)
  {
    return ip == "127.0.0.1" || ip == "::1" || ip == "::ffff:127.0.0.1";
  }

  /**
   * @brief Constant-time string comparison to prevent timing attacks.
   *
   * Uses OpenSSL's CRYPTO_memcmp for guaranteed constant-time behavior.
   */
  bool ConstantTimeCompare(const std::string& a, const std::string& b)
  {
    if (a.size() != b.size())
    {
      return false;
    }

    return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
  }

  /**
   * @brief Check if a request path is exempt from authentication.
   *
   * Health and info endpoints are always accessible without auth.
   */
  bool IsAuthExemptPath(const std::string& path)
  {
    return path == "/api/v1/health" ||
           path == "/api/v1/info" ||
           path == "/api/v1/";
  }

  /**
   * @brief Check if a request path is exempt from rate limiting.
   */
  bool IsRateLimitExemptPath(const std::string& path)
  {
    return path == "/api/v1/health" ||
           path == "/api/v1/info";
  }
}

RestServer::RestServer()
  : m_Bridge(std::make_unique<DataStorageBridge>())
{
}

RestServer::~RestServer()
{
  this->Stop();
}

bool RestServer::Start()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Running)
  {
    return true;  // Already running
  }

  m_RequestLog.clear();
  ++m_RequestLogVersion;
  m_ClientIPs.clear();
  m_RateLimitMap.clear();
  m_RateLimitCheckCount = 0;

  if (!m_PendingConfig.enabled)
  {
    m_LastError = "Server is disabled in configuration";
    MITK_ERROR << *m_LastError;
    return false;
  }

  // Validate HTTPS configuration if enabled
  if (m_PendingConfig.httpsEnabled)
  {
    if (m_PendingConfig.sslCertPath.empty())
    {
      m_LastError = "HTTPS enabled but no certificate path configured";
      MITK_ERROR << *m_LastError;
      return false;
    }
    if (m_PendingConfig.sslKeyPath.empty())
    {
      m_LastError = "HTTPS enabled but no private key path configured";
      MITK_ERROR << *m_LastError;
      return false;
    }
    if (!fs::exists(m_PendingConfig.sslCertPath))
    {
      m_LastError = "SSL certificate file not found: " + SanitizeForLog(m_PendingConfig.sslCertPath);
      MITK_ERROR << *m_LastError;
      return false;
    }
    if (!fs::exists(m_PendingConfig.sslKeyPath))
    {
      m_LastError = "SSL private key file not found: " + SanitizeForLog(m_PendingConfig.sslKeyPath);
      MITK_ERROR << *m_LastError;
      return false;
    }
  }

  try
  {
    // Create appropriate server (HTTPS or HTTP)
    if (m_PendingConfig.httpsEnabled)
    {
      m_Server = std::make_unique<httplib::SSLServer>(
        m_PendingConfig.sslCertPath.c_str(),
        m_PendingConfig.sslKeyPath.c_str());
    }
    else
    {
      m_Server = std::make_unique<httplib::Server>();
    }

    m_Server->set_read_timeout(m_PendingConfig.readTimeoutSeconds);
    m_Server->set_write_timeout(m_PendingConfig.writeTimeoutSeconds);

    // Apply payload size limit
    m_Server->set_payload_max_length(
      static_cast<size_t>(m_PendingConfig.maxPayloadSizeMB) * 1024 * 1024);

    // Setup temp directory for data serialization
    if (!this->SetupTempDirectory())
    {
      m_LastError = "Failed to create temporary directory for data operations";
      MITK_ERROR << *m_LastError;
      return false;
    }

    // Create controllers
    m_HealthController = std::make_unique<HealthController>(*m_Bridge);
    m_DataStorageController = std::make_unique<DataStorageController>(*m_Bridge);
    m_DataStorageController->SetTempDirectory(m_TempDirectory);
    m_SwaggerController = std::make_unique<SwaggerController>();

    // Pass file access config to controllers
    m_DataStorageController->SetFileAccessConfig(
      m_PendingConfig.fileAccessMode, m_PendingConfig.allowedFileDirectories, m_TempDirectory);
    m_DataStorageController->SetMaxActiveTempDirsPerIp(m_PendingConfig.maxActiveTempDirsPerIp);
    m_HealthController->SetFileAccessConfig(
      m_PendingConfig.fileAccessMode, m_PendingConfig.allowedFileDirectories);
    m_HealthController->SetMaxActiveTempDirsPerIp(m_PendingConfig.maxActiveTempDirsPerIp);

    // Connect uptime callback to HealthController
    m_HealthController->SetUptimeCallback([this]() -> std::optional<int64_t> {
      return this->GetUptimeSeconds();
    });

    // Register routes
    this->RegisterRoutes();

    // Record start time for uptime tracking
    m_StartTime = std::chrono::steady_clock::now();

    // Copy pending config to running config before starting
    m_RunningConfig = m_PendingConfig;

    // Start server in separate thread
    m_Running = true;
    m_ServerThread = std::make_unique<std::thread>(&RestServer::ServerThreadFunc, this);

    const std::string protocol = m_PendingConfig.httpsEnabled ? "HTTPS" : "HTTP";
    MITK_INFO << "REST API server starting (" << protocol << ") on "
              << SanitizeForLog(m_PendingConfig.host) << ":" << m_PendingConfig.port;

    return true;
  }
  catch (const std::exception& e)
  {
    m_LastError = std::string("Failed to start server: ") + e.what();
    m_Running = false;
    m_RunningConfig = std::nullopt;
    MITK_ERROR << *m_LastError;
    return false;
  }
}

void RestServer::Stop()
{
  std::unique_ptr<std::thread> threadToJoin;
  bool wasRunning = false;
  std::string tempDirToCleanup;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    wasRunning = m_Running;

    // Signal server to stop accepting connections (idempotent, safe to call always)
    // Note: httplib::Server::stop() is safe to call and does not throw.
    // It sets an internal flag that causes listen() to return.
    if (m_Server)
    {
      m_Server->stop();
    }

    // Move thread out so we can join outside the lock
    threadToJoin = std::move(m_ServerThread);
  }

  // Join thread outside of lock to avoid deadlock
  // The server thread will exit after listen() returns
  if (threadToJoin && threadToJoin->joinable())
  {
    threadToJoin->join();
  }

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    // Clean up server resources
    m_Running = false;
    m_RunningConfig = std::nullopt;
    m_StartTime = std::nullopt;
    m_Server.reset();
    m_HealthController.reset();
    m_DataStorageController.reset();
    m_SwaggerController.reset();

    // Clear request tracking, log, and rate limit data
    m_ClientIPs.clear();
    m_RequestLog.clear();
    ++m_RequestLogVersion;
    m_RateLimitMap.clear();
    m_RateLimitCheckCount = 0;

    // Copy and clear temp directory path while holding the lock
    // to prevent race condition if Start() is called concurrently
    tempDirToCleanup = std::move(m_TempDirectory);
    m_TempDirectory.clear();
  }

  // Clean up temp directory outside lock (may take time)
  this->CleanupTempDirectory(tempDirToCleanup);

  if (wasRunning)
  {
    MITK_INFO << "REST API server stopped";
  }
}

bool RestServer::IsRunning() const
{
  return m_Running;
}

void RestServer::SetConfig(const RestServerConfig& config)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_PendingConfig = config;
}

RestServerConfig RestServer::GetPendingConfig() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_PendingConfig;
}

std::optional<RestServerConfig> RestServer::GetRunningConfig() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_RunningConfig;
}

void RestServer::SetDataStorage(DataStorage* dataStorage)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Bridge->SetDataStorage(dataStorage);
}

void RestServer::SetDispatcher(StorageThreadDispatcherBase* dispatcher)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Bridge->SetDispatcher(dispatcher);
}

DataStorage::Pointer RestServer::GetDataStorage() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Bridge->GetDataStorage();
}

std::optional<std::string> RestServer::GetServerUrl() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (!m_Running || !m_RunningConfig)
  {
    return std::nullopt;
  }

  const std::string protocol = m_RunningConfig->httpsEnabled ? "https" : "http";
  return protocol + "://" + m_RunningConfig->host + ":" + std::to_string(m_RunningConfig->port);
}

std::optional<std::string> RestServer::GetLastError() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_LastError;
}

std::vector<std::string> RestServer::GetClientIPs() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return std::vector<std::string>(m_ClientIPs.begin(), m_ClientIPs.end());
}

std::optional<RequestInfo> RestServer::GetLastRequestInfo() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_RequestLog.empty())
  {
    return std::nullopt;
  }

  return m_RequestLog.back();
}

void RestServer::SetLogLimit(std::optional<unsigned int> limit)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_LogLimit = limit;

  if (m_LogLimit.has_value() && m_RequestLog.size() > m_LogLimit.value())
  {
    while (m_RequestLog.size() > m_LogLimit.value())
    {
      m_RequestLog.pop_front();
    }
    ++m_RequestLogVersion;
  }
}

std::optional<unsigned int> RestServer::GetLogLimit() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_LogLimit;
}

std::vector<RequestInfo> RestServer::GetRequestLog() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return std::vector<RequestInfo>(m_RequestLog.begin(), m_RequestLog.end());
}

void RestServer::ClearRequestLog()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_RequestLog.clear();
  ++m_RequestLogVersion;
}

uint64_t RestServer::GetRequestLogVersion() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_RequestLogVersion;
}

std::optional<int64_t> RestServer::GetUptimeSeconds() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (!m_Running || !m_StartTime.has_value())
  {
    return std::nullopt;
  }

  auto now = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_StartTime.value());
  return duration.count();
}

void RestServer::RecordRequest(const std::string& endpoint, const std::string& method,
                               int responseCode, const std::string& clientIP)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  const std::string sanitizedEndpoint = SanitizeForLog(endpoint);
  const std::string sanitizedClientIP = SanitizeForLog(clientIP);

  m_ClientIPs.insert(sanitizedClientIP);

  RequestInfo info{sanitizedEndpoint, method, responseCode, sanitizedClientIP};

  m_RequestLog.push_back(info);

  // Trim log if limit is set and exceeded
  while (m_LogLimit.has_value() && m_RequestLog.size() > m_LogLimit.value())
  {
    m_RequestLog.pop_front();
  }

  ++m_RequestLogVersion;
}

// --- Security middleware implementations ---

bool RestServer::CheckClientAccess(const httplib::Request& req, httplib::Response& res)
{
  // Access m_RunningConfig without lock - called from httplib worker thread.
  // m_RunningConfig is set before the server starts accepting connections
  // and cleared after the server stops, so it is safe to read here.
  const auto& config = *m_RunningConfig;

  switch (config.clientAccessMode)
  {
    case ClientAccessMode::AllowAll:
      return true;

    case ClientAccessMode::LocalhostOnly:
      if (IsLocalhostIP(req.remote_addr))
      {
        return true;
      }
      break;

    case ClientAccessMode::Whitelist:
    {
      const auto& allowedIPs = config.allowedClientIPs;
      if (std::find(allowedIPs.begin(), allowedIPs.end(), req.remote_addr) != allowedIPs.end())
      {
        return true;
      }
      break;
    }
  }

  // Denied
  auto error = ErrorResponse::AccessDenied(
    "Client IP " + SanitizeForLog(req.remote_addr) + " is not allowed", req.path);
  res.status = 403;
  res.set_content(error.dump(), "application/json");

  this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
  return false;
}

bool RestServer::CheckRateLimit(const httplib::Request& req, httplib::Response& res)
{
  const auto& config = *m_RunningConfig;

  if (!config.rateLimitEnabled)
  {
    return true;
  }

  // Exempt health/info endpoints from rate limiting
  if (IsRateLimitExemptPath(req.path))
  {
    return true;
  }

  const auto now = std::chrono::steady_clock::now();
  const auto windowDuration = std::chrono::seconds(60);
  const auto windowStart = now - windowDuration;

  std::lock_guard<std::mutex> lock(m_Mutex);

  auto& timestamps = m_RateLimitMap[req.remote_addr];

  // Remove entries older than the window
  while (!timestamps.empty() && timestamps.front() < windowStart)
  {
    timestamps.pop_front();
  }

  // Check if limit exceeded
  if (static_cast<int>(timestamps.size()) >= config.rateLimitPerMinute)
  {
    // Calculate retry-after as seconds until the oldest entry expires
    const auto oldestExpiry = timestamps.front() + windowDuration;
    const auto retryAfter = std::chrono::duration_cast<std::chrono::seconds>(oldestExpiry - now).count();
    const int retryAfterSeconds = std::max(1, static_cast<int>(retryAfter));

    auto error = ErrorResponse::RateLimitExceeded(retryAfterSeconds, req.path);
    res.status = 429;
    res.set_header("Retry-After", std::to_string(retryAfterSeconds));
    res.set_content(error.dump(), "application/json");

    this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    return false;
  }

  // Record this request's timestamp
  timestamps.push_back(now);

  // Periodically clean up stale IP entries to prevent memory growth
  ++m_RateLimitCheckCount;
  if (m_RateLimitCheckCount % kRateLimitCleanupInterval == 0)
  {
    for (auto it = m_RateLimitMap.begin(); it != m_RateLimitMap.end(); )
    {
      // Remove expired entries from this IP
      while (!it->second.empty() && it->second.front() < windowStart)
      {
        it->second.pop_front();
      }

      // Remove IP entry if no recent requests
      if (it->second.empty())
      {
        it = m_RateLimitMap.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  return true;
}

bool RestServer::CheckAuthentication(const httplib::Request& req, httplib::Response& res)
{
  const auto& config = *m_RunningConfig;

  if (!config.requireAuth)
  {
    return true;
  }

  // Exempt health and info endpoints
  if (IsAuthExemptPath(req.path))
  {
    return true;
  }

  // Check Authorization header
  if (!req.has_header("Authorization"))
  {
    auto error = ErrorResponse::Unauthorized("Authentication required. Provide Authorization: Bearer <token> header.", req.path);
    res.status = 401;
    res.set_header("WWW-Authenticate", "Bearer");
    res.set_content(error.dump(), "application/json");

    this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    return false;
  }

  const std::string authHeader = req.get_header_value("Authorization");

  // Validate Bearer token format
  const std::string bearerPrefix = "Bearer ";
  if (authHeader.size() <= bearerPrefix.size() ||
      authHeader.substr(0, bearerPrefix.size()) != bearerPrefix)
  {
    auto error = ErrorResponse::Unauthorized("Invalid authorization format. Expected: Bearer <token>", req.path);
    res.status = 401;
    res.set_header("WWW-Authenticate", "Bearer");
    res.set_content(error.dump(), "application/json");

    this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    return false;
  }

  const std::string providedToken = authHeader.substr(bearerPrefix.size());

  // Constant-time comparison to prevent timing attacks
  if (!ConstantTimeCompare(providedToken, config.apiToken))
  {
    auto error = ErrorResponse::Unauthorized("Invalid API token", req.path);
    res.status = 401;
    res.set_header("WWW-Authenticate", "Bearer");
    res.set_content(error.dump(), "application/json");

    this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    return false;
  }

  return true;
}

void RestServer::RegisterRoutes()
{
  const std::string apiBase = "/api/v1";

  // Enable CORS so browser-based clients (Swagger UI, web apps) can reach the API
  // regardless of which hostname/port they were loaded from.
  m_Server->set_default_headers({
    {"Access-Control-Allow-Origin", "*"},
    {"Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type, Accept, Authorization, X-MITK-Transfer-Mode"}
  });

  // Handle preflight OPTIONS requests for any route
  m_Server->Options(".*", [](const httplib::Request& /*req*/, httplib::Response& res) {
    res.status = 204;
  });

  // Install security middleware as pre-routing handler.
  // This runs before any route handler and can short-circuit the request.
  m_Server->set_pre_routing_handler(
    [this](const httplib::Request& req, httplib::Response& res) -> httplib::Server::HandlerResponse {
      // 1. Client IP access check
      if (!this->CheckClientAccess(req, res))
      {
        return httplib::Server::HandlerResponse::Handled;
      }

      // 2. Rate limiting check
      if (!this->CheckRateLimit(req, res))
      {
        return httplib::Server::HandlerResponse::Handled;
      }

      // 3. Authentication check
      if (!this->CheckAuthentication(req, res))
      {
        return httplib::Server::HandlerResponse::Handled;
      }

      // All checks passed - continue to route matching
      return httplib::Server::HandlerResponse::Unhandled;
    });

  // Health endpoints
  m_Server->Get(apiBase + "/health",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_health(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Get(apiBase + "/info",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_info(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // API root info
  m_Server->Get(apiBase + "/",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_info(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // File access config discovery endpoint
  m_Server->Get(apiBase + "/config/file-access",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_config_file_access(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // Node endpoints
  m_Server->Get(apiBase + "/datastorage/nodes",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Post(apiBase + "/datastorage/nodes",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePOST_nodes(req, res);
      this->RecordRequest(req.path, "POST", res.status, req.remote_addr);
    });

  m_Server->Get(apiBase + "/datastorage/nodes/:uid",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Patch(apiBase + "/datastorage/nodes/:uid",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePATCH_nodes_uid(req, res);
      this->RecordRequest(req.path, "PATCH", res.status, req.remote_addr);
    });

  m_Server->Delete(apiBase + "/datastorage/nodes/:uid",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleDELETE_nodes_uid(req, res);
      this->RecordRequest(req.path, "DELETE", res.status, req.remote_addr);
    });

  // Children endpoints
  m_Server->Get(apiBase + "/datastorage/nodes/:uid/children",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_children(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Post(apiBase + "/datastorage/nodes/:uid/children",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePOST_nodes_uid_children(req, res);
      this->RecordRequest(req.path, "POST", res.status, req.remote_addr);
    });

  // Data endpoints
  m_Server->Get(apiBase + "/datastorage/nodes/:uid/data",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_data(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Put(apiBase + "/datastorage/nodes/:uid/data",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePUT_nodes_uid_data(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  // Property endpoints
  m_Server->Get(apiBase + "/datastorage/nodes/:uid/properties",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_properties(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Get(apiBase + "/datastorage/nodes/:uid/properties/:key",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_properties_key(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  m_Server->Put(apiBase + "/datastorage/nodes/:uid/properties/:key",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePUT_nodes_uid_properties_key(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  m_Server->Delete(apiBase + "/datastorage/nodes/:uid/properties/:key",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleDELETE_nodes_uid_properties_key(req, res);
      this->RecordRequest(req.path, "DELETE", res.status, req.remote_addr);
    });

  m_Server->Put(apiBase + "/datastorage/nodes/:uid/properties",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePUT_nodes_uid_properties(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  m_Server->Patch(apiBase + "/datastorage/nodes/:uid/properties",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePATCH_nodes_uid_properties(req, res);
      this->RecordRequest(req.path, "PATCH", res.status, req.remote_addr);
    });

  // Documentation endpoints (Swagger UI and OpenAPI spec)
  m_Server->Get(apiBase + "/docs",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_docs(req, res);
    });

  m_Server->Get(apiBase + "/docs/swagger-ui.css",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_docs_css(req, res);
    });

  m_Server->Get(apiBase + "/docs/swagger-ui-bundle.js",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_docs_js(req, res);
    });

  m_Server->Get(apiBase + "/openapi.json",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_openapi(req, res);
    });
}

void RestServer::ServerThreadFunc()
{
  std::string host;
  int port;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    // m_RunningConfig is guaranteed to have a value here because Start() sets it
    // before launching this thread
    host = m_RunningConfig->host;
    port = m_RunningConfig->port;
  }

  // This blocks until server is stopped (either via stop() or error)
  bool result = m_Server->listen(host, port);

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check if this was an unexpected failure (not triggered by Stop())
    // m_Running being true here means Stop() hasn't been called yet
    if (!result && m_Running)
    {
      // listen() failed to start or returned unexpectedly
      m_LastError = "Server failed to listen on " + SanitizeForLog(host) + ":" + std::to_string(port);
      MITK_ERROR << *m_LastError;
    }

    // Server is no longer running regardless of how listen() returned
    // This ensures IsRunning() returns the correct state
    m_Running = false;
    m_RunningConfig = std::nullopt;
  }
}

bool RestServer::SetupTempDirectory()
{
  try
  {
    // Workaround: IOUtil::CreateTemporaryDirectory produces double separators
    // when GetTempPath() returns a path with trailing separator (T-XXXXX).
    // TODO: Fix in IOUtil::CreateTemporaryDirectory and CreateTemporaryFile.
    m_TempDirectory = fs::path(IOUtil::CreateTemporaryDirectory("mitk-rest-XXXXXX"))
                        .lexically_normal().string();
    MITK_DEBUG << "Created REST API temp directory: " << m_TempDirectory;
    return true;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Failed to create temp directory: " << e.what();
    return false;
  }
}

void RestServer::CleanupTempDirectory(const std::string& tempDir)
{
  if (tempDir.empty())
  {
    return;
  }

  try
  {
    fs::path tempPath(tempDir);
    if (fs::exists(tempPath))
    {
      // Remove all contents recursively
      fs::remove_all(tempPath);
      MITK_DEBUG << "Cleaned up REST API temp directory: " << tempDir;
    }
  }
  catch (const std::exception& e)
  {
    MITK_WARN << "Failed to cleanup temp directory: " << e.what();
  }
}

}
