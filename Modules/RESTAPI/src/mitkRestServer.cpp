/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestServer.h>
#include <mitkActionOriginScope.h>
#include <mitkDataStorageBridge.h>
#include <mitkHealthController.h>
#include <mitkDataStorageController.h>
#include <mitkSwaggerController.h>
#include <mitkRenderingController.h>
#include <mitkRenderWindowBridge.h>
#include <mitkErrorResponse.h>
#include "mitkRestRouter.h"
#include "mitkRestStopGate.h"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <httplib.h>
#include <mitkExceptionMacro.h>
#include <mitkIOUtil.h>
#include <mitkLog.h>
#include <mitkFileSystem.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <itksys/SystemInformation.hxx>

#include <openssl/crypto.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <cctype>

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

    for (const unsigned char c : input)
    {
      if (c == '\n')
        result += "\\n";
      else if (c == '\r')
        result += "\\r";
      else if (c == '\t')
        result += "\\t";
      else if (c < 0x20)
        result += "\\x" + std::string(1, "0123456789abcdef"[(c >> 4) & 0xF]) +
                  std::string(1, "0123456789abcdef"[c & 0xF]);
      else
        result += static_cast<char>(c);
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
    // Compare in constant time to avoid leaking token length through timing.
    // XOR of sizes is non-zero if they differ; CRYPTO_memcmp compares up to
    // min(len) bytes. The combined result is zero only when both size and
    // content match exactly.
    volatile size_t result = a.size() ^ b.size();
    const size_t len = std::min(a.size(), b.size());
    result |= static_cast<size_t>(CRYPTO_memcmp(a.data(), b.data(), len));
    return result == 0;
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
           path == "/api/v1/docs" ||
           path == "/api/v1/docs/" ||
           path == "/api/v1/docs/swagger-ui.css" ||
           path == "/api/v1/docs/swagger-ui-bundle.js" ||
           path == "/api/v1/openapi.json";
  }

  /**
   * @brief Check if a request path is exempt from rate limiting.
   */
  bool IsRateLimitExemptPath(const std::string& path)
  {
    return path == "/api/v1/health" ||
           path == "/api/v1/info";
  }

  const std::string ApiV1 = "/api/v1";

  std::string ToLower(std::string text)
  {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });

    return text;
  }

  std::optional<std::string> GetBearerToken(const httplib::Request& req)
  {
    // Authentication schemes are case-insensitive (RFC 9110).
    static const std::string Prefix = "bearer ";
    const auto header = req.get_header_value("Authorization");

    if (header.size() <= Prefix.size() || ToLower(header.substr(0, Prefix.size())) != Prefix)
      return std::nullopt;

    return header.substr(Prefix.size());
  }

  /** The host of the value of a Host header, without the port. */
  std::string GetHostOfHostHeader(const std::string& value)
  {
    if (!value.empty() && value.front() == '[')
    {
      const auto end = value.find(']');
      return end == std::string::npos ? value : value.substr(1, end - 1);
    }

    // More than one colon is an IPv6 literal without brackets and without a port.
    if (std::count(value.begin(), value.end(), ':') != 1)
      return value;

    return value.substr(0, value.find(':'));
  }

  bool IsIpLiteral(const std::string& host)
  {
    in_addr address4;
    in6_addr address6;

    return inet_pton(AF_INET, host.c_str(), &address4) == 1 || inet_pton(AF_INET6, host.c_str(), &address6) == 1;
  }

  const std::string& GetOwnHostName()
  {
    static const std::string Name = []() {
      itksys::SystemInformation information;
      const char* const name = information.GetHostname();
      return ToLower(name != nullptr ? name : "");
    }();

    return Name;
  }

  /**
   * Why a browser could have sent the request, or an empty string if it
   * could not. Besides the Origin header, which browsers add to requests
   * that can change state, this covers requests for a host name that a web
   * page can only reach through DNS rebinding, and requests that the browser
   * marks as coming from another site.
   */
  std::string GetBrowserRefusalReason(const httplib::Request& req, const std::string& serverHost)
  {
    if (req.has_header("Origin"))
      return "This endpoint does not accept requests from web pages.";

    if (req.has_header("Sec-Fetch-Site"))
    {
      const auto site = ToLower(req.get_header_value("Sec-Fetch-Site"));

      if (site != "none" && site != "same-origin")
        return "This endpoint does not accept requests from web pages.";
    }

    if (req.has_header("Host"))
    {
      const auto host = ToLower(GetHostOfHostHeader(req.get_header_value("Host")));

      const bool isServerHost = !serverHost.empty() && host == ToLower(serverHost);
      const bool isOwnHostName = !GetOwnHostName().empty() && host == GetOwnHostName();

      if (host != "localhost" && !IsIpLiteral(host) && !isServerHost && !isOwnHostName)
      {
        return "This endpoint does not accept requests for the host name \"" + host + "\".";
      }
    }

    return std::string();
  }

  /** Router patterns name parameters ":uid", OpenAPI paths "{uid}". */
  std::string ToOpenApiPath(const std::string& pattern)
  {
    std::string path;
    std::size_t start = 0;

    while (start < pattern.size())
    {
      auto end = pattern.find('/', start + 1);

      if (end == std::string::npos)
        end = pattern.size();

      auto segment = pattern.substr(start, end - start);

      if (segment.size() > 2 && segment[1] == ':')
        segment = "/{" + segment.substr(2) + "}";

      path += segment;
      start = end;
    }

    return path;
  }

  nlohmann::json LoadOpenApi()
  {
    auto* context = us::GetModuleContext();

    if (context == nullptr)
      return nlohmann::json();

    auto resource = context->GetModule()->GetResource("openapi.json");

    if (!resource.IsValid())
      return nlohmann::json();

    us::ModuleResourceStream stream(resource, std::ios_base::binary);
    return nlohmann::json::parse(stream, nullptr, false);
  }

  /** Replaces "$ref"s by what they point to, so that one operation can be read on its own. */
  nlohmann::json ResolveReferences(const nlohmann::json& value, const nlohmann::json& document, int depth = 0)
  {
    if (depth > 10)
      return value;

    if (value.is_object())
    {
      if (value.contains("$ref") && value["$ref"].is_string())
      {
        const auto reference = value["$ref"].get<std::string>();

        if (0 == reference.compare(0, 2, "#/"))
        {
          try
          {
            return ResolveReferences(document.at(nlohmann::json::json_pointer(reference.substr(1))), document, depth + 1);
          }
          catch (const nlohmann::json::exception&)
          {
            return value;
          }
        }
      }

      auto resolved = nlohmann::json::object();

      for (const auto& [key, item] : value.items())
        resolved[key] = ResolveReferences(item, document, depth + 1);

      return resolved;
    }

    if (value.is_array())
    {
      auto resolved = nlohmann::json::array();

      for (const auto& item : value)
        resolved.push_back(ResolveReferences(item, document, depth + 1));

      return resolved;
    }

    return value;
  }
}

RestServer::RestServer()
  : m_Bridge(std::make_unique<DataStorageBridge>())
  , m_RenderWindowBridge(std::make_unique<RenderWindowBridge>())
  , m_Router(std::make_unique<RestRouter>())
  , m_BuiltInRouter(std::make_unique<RestRouter>())
  , m_StopGate(std::make_shared<RestStopGate>())
{
  m_Bridge->SetStopGate(m_StopGate);
  m_RenderWindowBridge->SetStopGate(m_StopGate);

  RestRoute endpoints;
  endpoints.Method = "GET";
  endpoints.Path = ApiV1 + "/endpoints";
  endpoints.Summary = "List the endpoints of the API";
  endpoints.Description =
    "Query parameters: path (optional) lists only endpoints whose path starts with it, like "
    "/api/v1/manual; details=true adds what each endpoint takes and returns. "
    "Returns {data: [{method, path, summary, description?, experimental?}]}. Path parameters are written {name}. "
    "Experimental endpoints may still change; the others are those of the REST API specification.";
  endpoints.Handler = [this](const RestRequest& request) { return this->HandleGET_endpoints(request); };
  m_Router->Add(std::move(endpoints));

  RestRoute jobs;
  jobs.Method = "GET";
  jobs.Path = ApiV1 + "/jobs";
  jobs.Summary = "List the jobs that run and the latest ended ones";
  jobs.Description =
    "Jobs are work that endpoints started and that takes longer than a request should. "
    "Returns {data: [job]}, newest first; see GET /api/v1/jobs/{id} for a job.";
  jobs.Handler = [this](const RestRequest& request) { return this->HandleGET_jobs(request); };
  m_Router->Add(std::move(jobs));

  RestRoute job;
  job.Method = "GET";
  job.Path = ApiV1 + "/jobs/:id";
  job.Summary = "Get a job, optionally waiting for it to end";
  job.Description =
    "Query parameter: wait (optional), seconds to wait for the job to end, at most 120. At most 4 requests wait "
    "at the same time; a further one answers at once, with 202 while the job runs. Returns {data: job}, "
    "a job being {id, description, state: running|succeeded|failed|canceled, seconds, and cancelable while it "
    "runs, result once it succeeded, error once it failed}.";
  job.Handler = [this](const RestRequest& request) { return this->HandleGET_job(request); };
  m_Router->Add(std::move(job));

  RestRoute cancelJob;
  cancelJob.Method = "DELETE";
  cancelJob.Path = ApiV1 + "/jobs/:id";
  cancelJob.Summary = "Cancel a job";
  cancelJob.Description = "Returns {data: job}, or 409 if the job cannot be canceled or has ended.";
  cancelJob.Handler = [this](const RestRequest& request) { return this->HandleDELETE_job(request); };
  m_Router->Add(std::move(cancelJob));
}

RestServer::~RestServer()
{
  this->Stop();
}

bool RestServer::Start()
{
  return this->Start(this->GetPendingConfig(), std::string());
}

bool RestServer::Start(const RestServerConfig& config, const std::string& startedBy)
{
  std::unique_lock<std::mutex> lock(m_Mutex);

  if (m_Running)
  {
    return true;  // Already running
  }

  // Join any stale server thread from a previous failed start.
  // When listen() fails asynchronously the thread exits but m_ServerThread is never
  // joined. Destroying a joinable std::thread is undefined behaviour (terminate or
  // resource_deadlock_would_occur on MSVC), so we must join it before proceeding.
  if (m_ServerThread)
  {
    std::unique_ptr<std::thread> staleThread = std::move(m_ServerThread);
    lock.unlock();
    staleThread->join();
    lock.lock();
  }

  m_RequestLog.clear();
  ++m_RequestLogVersion;
  m_ClientIPs.clear();
  m_RateLimitMap.clear();
  m_RateLimitCheckCount = 0;

  if (!config.enabled)
  {
    m_LastError = "Server is disabled in configuration";
    MITK_ERROR << *m_LastError;
    return false;
  }

  // Validate HTTPS configuration if enabled
  if (config.httpsEnabled)
  {
    if (config.sslCertPath.empty())
    {
      m_LastError = "HTTPS enabled but no certificate path configured";
      MITK_ERROR << *m_LastError;
      return false;
    }
    if (config.sslKeyPath.empty())
    {
      m_LastError = "HTTPS enabled but no private key path configured";
      MITK_ERROR << *m_LastError;
      return false;
    }
    if (!fs::exists(config.sslCertPath))
    {
      m_LastError = "SSL certificate file not found: " + SanitizeForLog(config.sslCertPath);
      MITK_ERROR << *m_LastError;
      return false;
    }
    if (!fs::exists(config.sslKeyPath))
    {
      m_LastError = "SSL private key file not found: " + SanitizeForLog(config.sslKeyPath);
      MITK_ERROR << *m_LastError;
      return false;
    }
  }

  try
  {
    // Create appropriate server (HTTPS or HTTP)
    if (config.httpsEnabled)
    {
      m_Server = std::make_unique<httplib::SSLServer>(
        config.sslCertPath.c_str(),
        config.sslKeyPath.c_str());
    }
    else
    {
      m_Server = std::make_unique<httplib::Server>();
    }

    m_Server->set_read_timeout(config.readTimeoutSeconds);
    m_Server->set_write_timeout(config.writeTimeoutSeconds);

    const auto threadPoolSize = static_cast<size_t>(std::max(1, config.threadPoolSize));
    m_Server->new_task_queue = [threadPoolSize]() { return new httplib::ThreadPool(threadPoolSize); };

    // Apply payload size limit
    m_Server->set_payload_max_length(
      static_cast<size_t>(config.maxPayloadSizeMB) * 1024 * 1024);

    // Setup temp directory for data serialization. Done before bind_to_port() so a
    // failure here returns without a bound socket (see the bind_to_port() comment below).
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
    m_RenderingController = std::make_unique<RenderingController>(*m_Bridge);
    m_RenderingController->SetRenderWindowBridge(m_RenderWindowBridge.get());
    m_RenderingController->SetStopGate(m_StopGate);
    this->SyncDispatcherToController();

    // Pass file access config to controllers
    m_DataStorageController->SetFileAccessConfig(
      config.fileAccessMode, config.allowedFileDirectories, m_TempDirectory);
    m_DataStorageController->SetMaxActiveTempDirsPerIp(config.maxActiveTempDirsPerIp);
    m_DataStorageController->SetFileAccessCheck(
      [this](const httplib::Request& req) { return this->HasFileAccess(req); });
    m_HealthController->SetFileAccessConfig(
      config.fileAccessMode, config.allowedFileDirectories);
    m_HealthController->SetMaxActiveTempDirsPerIp(config.maxActiveTempDirsPerIp);

    // Connect uptime callback to HealthController
    m_HealthController->SetUptimeCallback([this]() -> std::optional<int64_t> {
      return this->GetUptimeSeconds();
    });

    // Register routes
    this->RegisterRoutes(config);

    // The Qt workbench plugin (org.mitk.gui.qt.restapi) configures the bridge
    // callbacks during its activation. If we reach Start() without those
    // callbacks in place, every /rendering/editors/* request will fail with
    // 503 RENDER_WINDOW_NOT_AVAILABLE for an opaque reason. Surface this at
    // startup so the misconfiguration is visible in the log before the first
    // request lands.
    if (m_RenderWindowBridge != nullptr && !m_RenderWindowBridge->HasEditorListProvider())
    {
      MITK_WARN << "REST server starting without a configured RenderWindowBridge: "
                   "all /rendering/editors/* endpoints will return 503 "
                   "RENDER_WINDOW_NOT_AVAILABLE until the Qt workbench plugin "
                   "'org.mitk.gui.qt.restapi' activates and wires the bridge "
                   "callbacks. The plugin activates lazily; open the REST API "
                   "view once to activate it in this application.";
    }

    // Bind to the port synchronously so failures (port already in use, permission
    // denied) are detected and reported here, before the server thread is launched;
    // listen_after_bind() then runs on the thread in ServerThreadFunc.
    //
    // Bind is intentionally the last fallible step. httplib offers no way to close a
    // socket that was bound but never entered its listen loop (~Server is defaulted
    // and stop() is a no-op until is_running_), so any failure between bind and the
    // running listen loop leaks the bound socket: the catch block's m_Server.reset()
    // cannot close it either. Doing all other setup first shrinks this window to two
    // residual throws -- copying the running config (std::bad_alloc) and launching the
    // server thread (std::system_error) -- both extremely unlikely but not impossible.
    // Port 0 lets the system choose a free port.
    int port = config.port;
    const bool bound = port == 0
      ? (port = m_Server->bind_to_any_port(config.host)) > 0
      : m_Server->bind_to_port(config.host, port);

    if (!bound)
    {
      m_LastError = "Port " + std::to_string(config.port) +
                    " on " + SanitizeForLog(config.host) +
                    " is already in use or cannot be bound";
      MITK_ERROR << *m_LastError;
      // The temp directory was created above; drop it so a failed bind does not orphan it.
      this->CleanupTempDirectory(m_TempDirectory);
      m_TempDirectory.clear();
      return false;
    }

    // Record start time for uptime tracking
    m_StartTime = std::chrono::steady_clock::now();

    // Copy the config to the running config before starting
    m_RunningConfig = config;
    m_RunningConfig->port = port;
    m_StartedBy = startedBy;

    // Start server in separate thread
    m_Running = true;
    m_ServerThread = std::make_unique<std::thread>(&RestServer::ServerThreadFunc, this);

    // Wait for httplib to actually enter its listening loop before returning.
    // Without this, a caller could immediately invoke Stop(), which calls
    // httplib::Server::stop(). If the server thread has not yet entered
    // listen_after_bind(), stop() is a no-op (httplib only acts when its
    // internal is_running_ flag is true), and the thread will then block in
    // accept() forever, causing Stop()'s join() to deadlock.
    //
    // wait_until_ready() blocks until httplib sets either is_running_ (success)
    // or is_decommissioned (listen failure). Since bind_to_port() already
    // succeeded, the only delay is OS thread scheduling, which is bounded.
    lock.unlock();
    m_Server->wait_until_ready();
    lock.lock();

    // wait_until_ready() can also return because listen failed (is_decommissioned).
    // Check is_running() to distinguish success from failure.
    if (!m_Server->is_running())
    {
      m_Running = false;
      m_RunningConfig = std::nullopt;
      m_StartedBy.clear();
      m_StartTime = std::nullopt;

      // The server thread is exiting -- join it before returning.
      if (m_ServerThread && m_ServerThread->joinable())
      {
        auto failedThread = std::move(m_ServerThread);
        lock.unlock();
        failedThread->join();
        lock.lock();
      }

      m_LastError = "Server listen failed after successful port bind";
      MITK_ERROR << *m_LastError;
      this->CleanupTempDirectory(m_TempDirectory);
      m_TempDirectory.clear();
      // Unlike the catch path, m_Server is intentionally not reset here: resetting it
      // would not close the bound socket either (see the bind_to_port() note above),
      // and m_Server is unconditionally reassigned at the top of the next Start().
      return false;
    }

    const std::string protocol = config.httpsEnabled ? "HTTPS" : "HTTP";
    MITK_INFO << "REST API server started (" << protocol << ") on "
              << SanitizeForLog(config.host) << ":" << port
              << (startedBy.empty() ? std::string() : " by " + startedBy);

    return true;
  }
  catch (const std::exception& e)
  {
    m_LastError = std::string("Failed to start server: ") + e.what();
    m_Running = false;
    m_RunningConfig = std::nullopt;
    m_StartTime = std::nullopt;
    m_Server.reset();
    this->CleanupTempDirectory(m_TempDirectory);
    m_TempDirectory.clear();
    MITK_ERROR << *m_LastError;
    return false;
  }
}

void RestServer::Stop()
{
  std::unique_ptr<std::thread> threadToJoin;
  bool wasRunning = false;
  std::string tempDirToCleanup;

  // The join below waits for every request in flight. Those that wait
  // themselves, for this thread or for a job, give up now.
  this->SetStopping(true);

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

  this->SetStopping(false);

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Their handlers use the controllers reset below.
    for (const auto id : m_BuiltInRoutes)
      m_BuiltInRouter->Remove(id);

    m_BuiltInRoutes.clear();

    // Clean up server resources
    m_Running = false;
    m_RunningConfig = std::nullopt;
    m_StartedBy.clear();
    m_StartTime = std::nullopt;
    m_Server.reset();
    m_HealthController.reset();
    m_DataStorageController.reset();
    m_SwaggerController.reset();
    m_RenderingController.reset();

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
  m_Dispatcher = dispatcher;
  m_Bridge->SetDispatcher(dispatcher);
  m_RenderWindowBridge->SetDispatcher(dispatcher);
  this->SyncDispatcherToController();
}

void RestServer::SyncDispatcherToController()
{
  if (m_RenderingController)
    m_RenderingController->SetDispatcher(m_Dispatcher.Lock());
}

RenderWindowBridge* RestServer::GetRenderWindowBridge()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_RenderWindowBridge.get();
}

DataStorage::Pointer RestServer::GetDataStorage() const
{
  // Not under m_Mutex, which the GUI thread takes to read the server's
  // state: the bridge is the same for the server's life.
  return m_Bridge->GetDataStorage();
}

RestRouteId RestServer::AddRoute(RestRoute route)
{
  if (route.Path.empty() || route.Path.front() != '/')
    mitkThrow() << "The path of a route must start with a slash: \"" << route.Path << "\"";

  route.Path = ApiV1 + route.Path;
  return m_Router->Add(std::move(route));
}

void RestServer::RemoveRoute(RestRouteId id)
{
  m_Router->Remove(id);
}

RestResponse RestServer::HandleRequest(RestRequest request)
{
  if (!m_Running)
  {
    return RestResponse::FromError(ErrorResponse::Create("SERVER_NOT_RUNNING", "Service Unavailable",
      "The REST API server is not running.", 503, request.Path));
  }

  if (request.ClientAddress.empty())
    request.ClientAddress = "127.0.0.1";

  RestResponse response;
  const auto token = request.GetBearerToken();
  const bool hasCreatedToken = token.has_value() && this->GetTokenRights(*token).has_value();

  // The handlers of the built-in routes log their requests themselves.
  bool logged = false;

  const auto config = this->GetRunningConfig();

  if (!hasCreatedToken && config.has_value() && config->requireAuth && !IsAuthExemptPath(request.Path) &&
      !(token.has_value() && ConstantTimeCompare(*token, config->apiToken)))
  {
    response = RestResponse::FromError(ErrorResponse::Unauthorized("Invalid or missing API token", request.Path));
  }
  else
  {
    logged = m_BuiltInRouter->Find(request.Method, request.Path).has_value();
    response = this->Route(request);
  }

  if (!logged)
    this->RecordRequest(request.Path, request.Method, response.Status, request.ClientAddress);

  return response;
}

std::shared_ptr<RestJob> RestServer::CreateJob(const std::string& description)
{
  // Ended jobs beyond this many are forgotten, oldest first.
  constexpr std::size_t MaxEndedJobs = 100;

  std::lock_guard<std::mutex> lock(m_JobMutex);

  auto job = std::make_shared<RestJob>("job_" + std::to_string(m_NextJobNumber++), description);
  m_Jobs.push_back(job);

  job->SetWaitsInterrupted(m_StopGate->IsStopping());

  const auto endedCount = static_cast<std::size_t>(std::count_if(m_Jobs.begin(), m_Jobs.end(),
    [](const std::shared_ptr<RestJob>& item) { return item->HasEnded(); }));

  for (auto excess = endedCount > MaxEndedJobs ? endedCount - MaxEndedJobs : 0; excess > 0; --excess)
  {
    m_Jobs.erase(std::find_if(m_Jobs.begin(), m_Jobs.end(),
      [](const std::shared_ptr<RestJob>& item) { return item->HasEnded(); }));
  }

  return job;
}

DataNodeHandle RestServer::FindNode(const std::string& uid) const
{
  return this->HoldNode(m_Bridge->FindNode(uid));
}

DataNodeHandle RestServer::HoldNode(DataNode* node) const
{
  if (node == nullptr)
    return nullptr;

  StorageThreadDispatcherBase::Pointer dispatcher;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    dispatcher = m_Dispatcher.Lock();
  }

  return DataNodeHandle(node, [held = DataNode::Pointer(node), dispatcher = std::move(dispatcher)](DataNode*) mutable {
    if (dispatcher.IsNull() || dispatcher->IsDispatchThread())
    {
      held = nullptr;
      return;
    }

    dispatcher->Post([held = std::move(held)]() {});
  });
}

std::string RestServer::GetNodeUid(DataNode* node) const
{
  return m_Bridge->GetNodeUid(node);
}

void RestServer::Dispatch(const std::function<void()>& task) const
{
  StorageThreadDispatcherBase::Pointer dispatcher;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    dispatcher = m_Dispatcher.Lock();
  }

  RestStopGate::Run(m_StopGate, dispatcher, task);
}

void RestServer::SetStopping(bool stopping)
{
  m_StopGate->SetStopping(stopping);

  std::lock_guard<std::mutex> lock(m_JobMutex);

  for (const auto& job : m_Jobs)
    job->SetWaitsInterrupted(stopping);
}

void RestServer::Post(std::function<void()> task) const
{
  StorageThreadDispatcherBase::Pointer dispatcher;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    dispatcher = m_Dispatcher.Lock();
  }

  if (dispatcher.IsNotNull())
  {
    dispatcher->Post([task = std::move(task), origin = ActionOriginScope::GetCurrent()]() {
      const ActionOriginScope scope(origin);
      task();
    });
  }
  else
  {
    task();
  }
}

std::shared_ptr<RestJob> RestServer::FindJob(const RestRequest& request) const
{
  const auto& id = request.PathParams.at("id");

  std::lock_guard<std::mutex> lock(m_JobMutex);

  for (const auto& job : m_Jobs)
  {
    if (job->GetId() == id)
      return job;
  }

  throw RestException(ErrorResponse::Create("JOB_NOT_FOUND", "Job Not Found",
    "There is no job " + id + ". Ended jobs are forgotten after a while.", 404, request.Path));
}

RestResponse RestServer::HandleGET_jobs(const RestRequest&) const
{
  auto jobs = nlohmann::json::array();

  std::lock_guard<std::mutex> lock(m_JobMutex);

  for (auto job = m_Jobs.rbegin(); job != m_Jobs.rend(); ++job)
    jobs.push_back((*job)->ToJson());

  return RestResponse::FromData(jobs);
}

RestResponse RestServer::HandleGET_job(const RestRequest& request) const
{
  auto response = RestJob::Respond(request, this->FindJob(request));

  // The job resource itself exists either way.
  response.Status = 200;
  response.Headers.erase("Location");

  return response;
}

RestResponse RestServer::HandleDELETE_job(const RestRequest& request)
{
  const auto job = this->FindJob(request);

  if (!job->Cancel())
  {
    return RestResponse::FromError(ErrorResponse::Create("JOB_NOT_CANCELABLE", "Conflict",
      job->HasEnded() ? "The job has ended already." : "The job cannot be canceled.", 409, request.Path));
  }

  return RestResponse::FromData(job->ToJson());
}

std::string RestServer::GetStartedBy() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_StartedBy;
}

std::vector<RestTokenInfo> RestServer::GetTokens() const
{
  std::lock_guard<std::mutex> lock(m_TokenMutex);

  std::vector<RestTokenInfo> tokens;

  for (const auto& [token, info] : m_Tokens)
    tokens.push_back(info);

  return tokens;
}

std::string RestServer::CreateToken(const std::string& client, const RestTokenRights& rights)
{
  std::array<unsigned char, 32> bytes;

  if (1 != RAND_bytes(bytes.data(), static_cast<int>(bytes.size())))
    mitkThrow() << "Cannot generate a random token";

  static constexpr char Hex[] = "0123456789abcdef";
  std::string token;

  for (const auto byte : bytes)
  {
    token += Hex[byte >> 4];
    token += Hex[byte & 15];
  }

  std::lock_guard<std::mutex> lock(m_TokenMutex);
  m_Tokens[token] = RestTokenInfo{ client, rights };

  return token;
}

void RestServer::RemoveToken(const std::string& token)
{
  std::lock_guard<std::mutex> lock(m_TokenMutex);
  m_Tokens.erase(token);
}

std::optional<RestTokenInfo> RestServer::FindToken(const std::string& token) const
{
  std::lock_guard<std::mutex> lock(m_TokenMutex);

  for (const auto& [knownToken, info] : m_Tokens)
  {
    if (ConstantTimeCompare(token, knownToken))
      return info;
  }

  return std::nullopt;
}

std::optional<RestTokenRights> RestServer::GetTokenRights(const std::string& token) const
{
  if (const auto info = this->FindToken(token); info.has_value())
    return info->Rights;

  return std::nullopt;
}

bool RestServer::HasFileAccess(const httplib::Request& req) const
{
  const auto token = GetBearerToken(req);

  if (!token.has_value())
    return true;

  const auto rights = this->GetTokenRights(*token);
  return !rights.has_value() || rights->FileAccess;
}

void RestServer::AddBuiltInRoute(const std::string& method, const std::string& pattern, HttpHandler handler)
{
  if (method == "GET")
    m_Server->Get(pattern, handler);
  else if (method == "POST")
    m_Server->Post(pattern, handler);
  else if (method == "PUT")
    m_Server->Put(pattern, handler);
  else if (method == "PATCH")
    m_Server->Patch(pattern, handler);
  else if (method == "DELETE")
    m_Server->Delete(pattern, handler);
  else
    mitkThrow() << "Unknown HTTP method \"" << method << "\" for the route " << pattern;

  // HandleRequest() reaches the controllers through the same handler, with an
  // httplib request built from the RestRequest.
  RestRoute route;
  route.Method = method;
  route.Path = pattern;
  route.Handler = [handler](const RestRequest& request) {
    httplib::Request req;
    req.method = request.Method;
    req.path = request.Path;
    req.params.insert(request.QueryParams.begin(), request.QueryParams.end());
    req.path_params.insert(request.PathParams.begin(), request.PathParams.end());
    req.body = request.Body;
    req.remote_addr = request.ClientAddress;

    for (const auto& [name, value] : request.Headers)
      req.headers.emplace(name, value);

    httplib::Response res;
    handler(req, res);

    RestResponse response;
    response.Status = res.status == -1 ? 200 : res.status;
    response.Body = std::move(res.body);

    for (const auto& [name, value] : res.headers)
    {
      if (ToLower(name) == "content-type")
        response.ContentType = value;
      else
        response.Headers[name] = value;
    }

    return response;
  };

  m_BuiltInRoutes.push_back(m_BuiltInRouter->Add(std::move(route)));
}

void RestServer::HandleRoutedHttp(const httplib::Request& req, httplib::Response& res)
{
  RestResponse response;

  // Same lock-free read as in CheckClientAccess().
  const auto refusalReason = GetBrowserRefusalReason(req, m_RunningConfig ? m_RunningConfig->host : std::string());

  // Rejecting what a browser could send keeps web pages from driving the
  // Workbench through the user's browser, whatever the CORS headers say.
  if (!refusalReason.empty())
  {
    response = RestResponse::FromError(ErrorResponse::AccessDenied(refusalReason, req.path));
  }
  else
  {
    RestRequest request;
    request.Method = req.method;
    request.Path = req.path;
    request.QueryParams.insert(req.params.begin(), req.params.end());
    request.Body = req.body;
    request.ClientAddress = req.remote_addr;

    for (const auto& [name, value] : req.headers)
      request.Headers[ToLower(name)] = value;

    response = this->Route(request);
  }

  res.status = response.Status;

  for (const auto& [name, value] : response.Headers)
    res.set_header(name, value);

  if (!response.ContentType.empty())
    res.set_content(response.Body, response.ContentType);
  else if (!response.Body.empty())
    res.set_content(response.Body, "application/octet-stream");

  this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
}

RestResponse RestServer::Route(RestRequest& request) const
{
  // What the request changes counts for its client, not for the user: the
  // name of a token of CreateToken(), or a generic name for any other client.
  const auto bearer = request.GetBearerToken();
  const auto tokenInfo = bearer.has_value() ? this->FindToken(*bearer) : std::optional<RestTokenInfo>();
  const ActionOriginScope originScope(
    tokenInfo.has_value() && !tokenInfo->Client.empty() ? tokenInfo->Client : std::string("REST client"));

  // Built-in routes first, as httplib tries them before it hands the rest to
  // the router.
  auto match = m_BuiltInRouter->Find(request.Method, request.Path);

  if (!match.has_value())
    match = m_Router->Find(request.Method, request.Path);

  if (!match.has_value())
  {
    return RestResponse::FromError(ErrorResponse::Create("NOT_FOUND", "Not Found",
      "There is no endpoint " + request.Method + " " + request.Path + ". GET /api/v1/endpoints lists them.",
      404, request.Path));
  }

  request.PathParams = match->PathParams;

  try
  {
    return match->Handler(request);
  }
  catch (const RestException& e)
  {
    return RestResponse::FromError(e.GetError());
  }
  catch (const nlohmann::json::exception& e)
  {
    // Handlers read the fields of a body without checking each type first.
    return RestResponse::FromError(ErrorResponse::InvalidRequest(e.what(), request.Path));
  }
  catch (const itk::ExceptionObject& e)
  {
    // what() starts with the source file of the throw.
    return RestResponse::FromError(ErrorResponse::InternalError(e.GetDescription(), request.Path));
  }
  catch (const std::exception& e)
  {
    return RestResponse::FromError(ErrorResponse::InternalError(e.what(), request.Path));
  }
  catch (...)
  {
    return RestResponse::FromError(ErrorResponse::InternalError("Unknown error", request.Path));
  }
}

RestResponse RestServer::HandleGET_endpoints(const RestRequest& request) const
{
  static const nlohmann::json OpenApi = LoadOpenApi();

  const auto prefix = request.GetQueryParam("path").value_or("");
  const bool details = request.GetQueryParam("details").value_or("") == "true";

  auto endpoints = nlohmann::json::array();

  const auto matchesPrefix = [&prefix](const std::string& path) {
    return 0 == path.compare(0, prefix.size(), prefix);
  };

  // The built-in routes are documented in openapi.json.
  if (OpenApi.is_object() && OpenApi.contains("paths"))
  {
    for (const auto& [path, operations] : OpenApi["paths"].items())
    {
      const auto fullPath = ApiV1 + path;

      if (!matchesPrefix(fullPath))
        continue;

      for (const auto& [method, operation] : operations.items())
      {
        if (!operation.is_object() || !operation.contains("summary"))
          continue;

        std::string upperMethod = method;
        std::transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), [](unsigned char c) {
          return static_cast<char>(std::toupper(c));
        });

        nlohmann::json endpoint = {
          { "method", upperMethod },
          { "path", fullPath },
          { "summary", operation["summary"] }
        };

        if (details)
        {
          auto description = nlohmann::json::object();

          for (const auto* key : { "description", "parameters", "requestBody" })
          {
            if (operation.contains(key))
              description[key] = ResolveReferences(operation[key], OpenApi);
          }

          if (operation.contains("responses"))
          {
            for (const auto& [status, response] : operation["responses"].items())
            {
              if (!status.empty() && status.front() == '2')
                description["responses"][status] = ResolveReferences(response, OpenApi);
            }
          }

          endpoint["description"] = description;
        }

        endpoints.push_back(std::move(endpoint));
      }
    }
  }

  for (const auto& route : m_Router->GetRoutes(ApiV1))
  {
    const auto path = ToOpenApiPath(route.Path);

    if (!matchesPrefix(path))
      continue;

    nlohmann::json endpoint = {
      { "method", route.Method },
      { "path", path },
      { "summary", route.Summary },
      { "experimental", true }
    };

    if (details)
      endpoint["description"] = route.Description;

    endpoints.push_back(std::move(endpoint));
  }

  return RestResponse::FromData(endpoints);
}

std::optional<std::string> RestServer::GetServerUrl() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (!m_Running || !m_RunningConfig)
  {
    return std::nullopt;
  }

  const std::string protocol = m_RunningConfig->httpsEnabled ? "https" : "http";
  const std::string& host = m_RunningConfig->host;

  // Wildcard binds are not directly addressable — report them as "localhost"
  // so the URL is something a client can actually open.
  std::string hostPart;
  if (host == "::" || host == "0.0.0.0")
  {
    hostPart = "localhost";
  }
  else if (host.find(':') != std::string::npos)
  {
    // IPv6 literal — must be wrapped in brackets per RFC 3986.
    hostPart = "[" + host + "]";
  }
  else
  {
    hostPart = host;
  }

  return protocol + "://" + hostPart + ":" + std::to_string(m_RunningConfig->port);
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

    default:
      MITK_WARN << "Unknown clientAccessMode value: " << static_cast<int>(config.clientAccessMode);
      break;
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

  bool rateLimitExceeded = false;
  int retryAfterSeconds = 0;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // NOTE: rate limiting keys on req.remote_addr (TCP source address). Behind a NAT
    // or reverse proxy all clients share one bucket. X-Forwarded-For is intentionally
    // not used as it can be spoofed by the client.
    auto& timestamps = m_RateLimitMap[req.remote_addr];

    // Remove entries older than the window
    while (!timestamps.empty() && timestamps.front() < windowStart)
    {
      timestamps.pop_front();
    }

    if (static_cast<int>(timestamps.size()) >= config.rateLimitPerMinute)
    {
      // Calculate retry-after as seconds until the oldest entry expires
      const auto oldestExpiry = timestamps.front() + windowDuration;
      const auto retryAfter =
        std::chrono::duration_cast<std::chrono::seconds>(oldestExpiry - now).count();
      retryAfterSeconds = std::max(1, static_cast<int>(retryAfter));
      rateLimitExceeded = true;
    }
    else
    {
      // Record this request's timestamp
      timestamps.push_back(now);

      // Periodically clean up stale IP entries. NOTE: cleanup runs only every
      // kRateLimitCleanupInterval checks, so m_RateLimitMap can grow proportionally
      // to the number of unique source IPs seen between sweeps.
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
    }
  } // lock released here; RecordRequest must be called outside the lock
    // to avoid a deadlock (RecordRequest acquires m_Mutex internally).

  if (rateLimitExceeded)
  {
    auto error = ErrorResponse::RateLimitExceeded(retryAfterSeconds, req.path);
    res.status = 429;
    res.set_header("Retry-After", std::to_string(retryAfterSeconds));
    res.set_content(error.dump(), "application/json");

    this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    return false;
  }

  return true;
}

bool RestServer::CheckAuthentication(const httplib::Request& req, httplib::Response& res)
{
  const auto& config = *m_RunningConfig;

  // Tokens of CreateToken() are always accepted; their rights are checked
  // where they matter, see HasFileAccess().
  if (const auto token = GetBearerToken(req); token.has_value() && this->GetTokenRights(*token).has_value())
  {
    return true;
  }

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
  const std::string bearerPrefix = "bearer ";
  if (authHeader.size() <= bearerPrefix.size() ||
      ToLower(authHeader.substr(0, bearerPrefix.size())) != bearerPrefix)
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
    // Log every failed auth attempt for audit purposes.
    // NOTE: auth-specific brute-force throttling is not implemented separately.
    // Enable rate limiting (rateLimitEnabled=true) to bound the overall request
    // volume from any single IP when authentication is required.
    MITK_WARN << "REST API: authentication failed from " << SanitizeForLog(req.remote_addr);

    auto error = ErrorResponse::Unauthorized("Invalid API token", req.path);
    res.status = 401;
    res.set_header("WWW-Authenticate", "Bearer");
    res.set_content(error.dump(), "application/json");

    this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    return false;
  }

  return true;
}

void RestServer::RegisterRoutes(const RestServerConfig& config)
{
  const std::string apiBase = "/api/v1";

  // Left over from a start that failed after registering.
  for (const auto id : m_BuiltInRoutes)
    m_BuiltInRouter->Remove(id);

  m_BuiltInRoutes.clear();

  // Enable CORS for browser-based clients. When authentication is required, restrict
  // the allowed origin to localhost to prevent arbitrary web pages from reading
  // authenticated responses. With auth disabled, wildcard is acceptable for a local
  // development server.
  const std::string corsOrigin = config.requireAuth ? "http://localhost" : "*";
  m_Server->set_default_headers({
    {"Access-Control-Allow-Origin", corsOrigin},
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
  this->AddBuiltInRoute("GET", apiBase + "/health",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_health(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/info",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_info(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // File access config discovery endpoint
  this->AddBuiltInRoute("GET", apiBase + "/config/file-access",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_config_file_access(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // Node endpoints
  this->AddBuiltInRoute("GET", apiBase + "/datastorage/nodes",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("POST", apiBase + "/datastorage/nodes",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePOST_nodes(req, res);
      this->RecordRequest(req.path, "POST", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/datastorage/nodes/:uid",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PATCH", apiBase + "/datastorage/nodes/:uid",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePATCH_nodes_uid(req, res);
      this->RecordRequest(req.path, "PATCH", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("DELETE", apiBase + "/datastorage/nodes/:uid",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleDELETE_nodes_uid(req, res);
      this->RecordRequest(req.path, "DELETE", res.status, req.remote_addr);
    });

  // Children endpoints
  this->AddBuiltInRoute("GET", apiBase + "/datastorage/nodes/:uid/children",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_children(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("POST", apiBase + "/datastorage/nodes/:uid/children",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePOST_nodes_uid_children(req, res);
      this->RecordRequest(req.path, "POST", res.status, req.remote_addr);
    });

  // Data endpoints
  this->AddBuiltInRoute("GET", apiBase + "/datastorage/nodes/:uid/data",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_data(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/datastorage/nodes/:uid/data",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePUT_nodes_uid_data(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  // Property endpoints
  this->AddBuiltInRoute("GET", apiBase + "/datastorage/nodes/:uid/properties",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_properties(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/datastorage/nodes/:uid/properties/:property_key",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleGET_nodes_uid_properties_key(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/datastorage/nodes/:uid/properties/:property_key",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePUT_nodes_uid_properties_key(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("DELETE", apiBase + "/datastorage/nodes/:uid/properties/:property_key",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandleDELETE_nodes_uid_properties_key(req, res);
      this->RecordRequest(req.path, "DELETE", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/datastorage/nodes/:uid/properties",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePUT_nodes_uid_properties(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PATCH", apiBase + "/datastorage/nodes/:uid/properties",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_DataStorageController->HandlePATCH_nodes_uid_properties(req, res);
      this->RecordRequest(req.path, "PATCH", res.status, req.remote_addr);
    });

  // Rendering endpoints
  this->AddBuiltInRoute("POST", apiBase + "/rendering/update",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePOST_update(req, res);
      this->RecordRequest(req.path, "POST", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("POST", apiBase + "/rendering/reinit",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePOST_reinit(req, res);
      this->RecordRequest(req.path, "POST", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/selected-position",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_selectedPosition(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/selected-position",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_selectedPosition(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/selected-time",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_selectedTime(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/selected-time",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_selectedTime(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/screenshot",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_screenshot(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // Register the list path before the parameterised path so the list isn't
  // captured as a window id.
  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_editors(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiInfo(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti/windows",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiWindows(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti/windows/:id",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiWindow(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti/windows/:id/camera",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiCamera(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/editors/stdmulti/windows/:id/camera",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_stdmultiCamera(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti/windows/:id/selected-slice",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiSelectedSlice(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/editors/stdmulti/windows/:id/selected-slice",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_stdmultiSelectedSlice(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti/screenshot",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiScreenshot(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/stdmulti/windows/:id/screenshot",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_stdmultiWindowScreenshot(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // ---- MxN editor ----
  // List endpoints register before their `:name` siblings so the more
  // specific route is not shadowed by the path-parameter route.

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnInfo(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/windows",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnWindows(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/windows/:id",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnWindow(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/layout",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnLayout(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/editors/mxn/layout",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_mxnLayout(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/windows/:id/camera",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnCamera(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/editors/mxn/windows/:id/camera",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_mxnCamera(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/windows/:id/selected-slice",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnSelectedSlice(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/editors/mxn/windows/:id/selected-slice",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_mxnSelectedSlice(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/windows/:id/selected-position",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnSelectedPosition(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("PUT", apiBase + "/rendering/editors/mxn/windows/:id/selected-position",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandlePUT_mxnSelectedPosition(req, res);
      this->RecordRequest(req.path, "PUT", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/screenshot",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnScreenshot(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/rendering/editors/mxn/windows/:id/screenshot",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_RenderingController->HandleGET_mxnWindowScreenshot(req, res);
      this->RecordRequest(req.path, "GET", res.status, req.remote_addr);
    });

  // Documentation endpoints (Swagger UI and OpenAPI spec)
  // Redirect /docs to /docs/ so relative URLs in the HTML resolve correctly
  // regardless of any reverse-proxy prefix (proxy-agnostic relative redirect).
  this->AddBuiltInRoute("GET", apiBase + "/docs",
    [this](const httplib::Request& req, httplib::Response& res) {
      res.status = 302;
      res.set_header("Location", "docs/");
      this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/docs/",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_docs(req, res);
      this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/docs/swagger-ui.css",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_docs_css(req, res);
      this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/docs/swagger-ui-bundle.js",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_docs_js(req, res);
      this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    });

  this->AddBuiltInRoute("GET", apiBase + "/openapi.json",
    [this](const httplib::Request& req, httplib::Response& res) {
      m_SwaggerController->HandleGET_openapi(req, res);
      this->RecordRequest(req.path, req.method, res.status, req.remote_addr);
    });

  // Everything else goes through the router, as other modules and plugins add
  // and remove routes there while the server runs. httplib tries handlers in
  // the order they were registered, so this has to come last.
  const auto routedPaths = apiBase + "/.*";
  const auto handleRouted = [this](const httplib::Request& req, httplib::Response& res) {
    this->HandleRoutedHttp(req, res);
  };

  m_Server->Get(routedPaths, handleRouted);
  m_Server->Post(routedPaths, handleRouted);
  m_Server->Put(routedPaths, handleRouted);
  m_Server->Patch(routedPaths, handleRouted);
  m_Server->Delete(routedPaths, handleRouted);
}

void RestServer::ServerThreadFunc()
{
  // The port is already bound via bind_to_port() in Start(). This call blocks
  // until the server is stopped (via Stop()) or an unexpected error occurs.
  const bool result = m_Server->listen_after_bind();

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check if this was an unexpected failure (not triggered by Stop()).
    // m_Running being true here means Stop() has not been called yet.
    if (!result && m_Running)
    {
      const std::string host = m_RunningConfig ? m_RunningConfig->host : "?";
      const int port = m_RunningConfig ? m_RunningConfig->port : 0;
      m_LastError = "Server stopped unexpectedly on " + SanitizeForLog(host) + ":" + std::to_string(port);
      MITK_ERROR << *m_LastError;
    }

    // Server is no longer running regardless of how listen_after_bind() returned.
    // This ensures IsRunning() returns the correct state.
    m_Running = false;
    m_RunningConfig = std::nullopt;
    m_StartedBy.clear();
  }
}

bool RestServer::SetupTempDirectory()
{
  try
  {
    m_TempDirectory = IOUtil::CreateTemporaryDirectory("mitk-rest-XXXXXX");
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
