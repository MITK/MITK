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

#include <httplib.h>
#include <mitkIOUtil.h>
#include <mitkLog.h>
#include <mitkFileSystem.h>

namespace mitk
{

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
  m_ClientIPs.clear();

  if (!m_PendingConfig.enabled)
  {
    m_LastError = "Server is disabled in configuration";
    MITK_ERROR << *m_LastError;
    return false;
  }

  try
  {
    m_Server = std::make_unique<httplib::Server>();

    m_Server->set_read_timeout(m_PendingConfig.readTimeoutSeconds);
    m_Server->set_write_timeout(m_PendingConfig.writeTimeoutSeconds);

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

    MITK_INFO << "REST API server starting on " << m_PendingConfig.host << ":" << m_PendingConfig.port;

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

    // Clear request tracking and log
    m_ClientIPs.clear();
    m_RequestLog.clear();

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

DataStorage* RestServer::GetDataStorage() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Bridge->GetDataStorage().GetPointer();
}

std::optional<std::string> RestServer::GetServerUrl() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (!m_Running || !m_RunningConfig)
  {
    return std::nullopt;
  }

  return "http://" + m_RunningConfig->host + ":" + std::to_string(m_RunningConfig->port);
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

  if (m_LogLimit.has_value())
  {
    while (m_RequestLog.size() > m_LogLimit.value())
    {
      m_RequestLog.pop_front();
    }
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
  m_ClientIPs.insert(clientIP);

  RequestInfo info{endpoint, method, responseCode, clientIP};

  m_RequestLog.push_back(info);

  // Trim log if limit is set and exceeded
  while (m_LogLimit.has_value() && m_RequestLog.size() > m_LogLimit.value())
  {
    m_RequestLog.pop_front();
  }
}

void RestServer::RegisterRoutes()
{
  const std::string apiBase = "/api/v1";

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
      m_LastError = "Server failed to listen on " + host + ":" + std::to_string(port);
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
