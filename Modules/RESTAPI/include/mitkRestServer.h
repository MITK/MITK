/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestServer_h
#define mitkRestServer_h

#include <MitkRESTAPIExports.h>
#include <mitkIRestServerService.h>
#include <mitkStorageThreadDispatcherBase.h>
#include <mitkWeakPointer.h>

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <set>
#include <deque>
#include <chrono>
#include <unordered_map>

// Forward declaration for httplib
namespace httplib {
  class Server;
  class Request;
  class Response;
}

namespace mitk
{
  // Forward declarations
  class DataStorageBridge;
  class HealthController;
  class DataStorageController;
  class SwaggerController;
  class RenderingController;
  class RenderWindowBridge;

  /**
   * \brief HTTP REST server implementation.
   *
   * Manages cpp-httplib server lifecycle and routes requests
   * to appropriate controllers.
   */
  class MITKRESTAPI_EXPORT RestServer : public IRestServerService
  {
  public:
    RestServer();
    ~RestServer() override;

    // Non-copyable, non-movable (due to thread)
    RestServer(const RestServer&) = delete;
    RestServer& operator=(const RestServer&) = delete;
    RestServer(RestServer&&) = delete;
    RestServer& operator=(RestServer&&) = delete;

    bool Start() override;
    void Stop() override;
    bool IsRunning() const override;

    void SetConfig(const RestServerConfig& config) override;
    RestServerConfig GetPendingConfig() const override;
    std::optional<RestServerConfig> GetRunningConfig() const override;

    void SetDataStorage(DataStorage* dataStorage) override;
    DataStorage::Pointer GetDataStorage() const override;

    /**
     * \brief Set the thread dispatcher for DataStorage operations.
     *
     * Forwards to the internal DataStorageBridge. If set, all bridge operations
     * are dispatched to the storage-owning thread.
     *
     * \param dispatcher The dispatcher, or nullptr to clear.
     */
    void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

    RenderWindowBridge* GetRenderWindowBridge() override;

    std::optional<std::string> GetServerUrl() const override;
    std::optional<std::string> GetLastError() const override;

    std::vector<std::string> GetClientIPs() const override;
    std::optional<RequestInfo> GetLastRequestInfo() const override;

    void SetLogLimit(std::optional<unsigned int> limit) override;
    std::optional<unsigned int> GetLogLimit() const override;
    std::vector<RequestInfo> GetRequestLog() const override;
    void ClearRequestLog() override;
    uint64_t GetRequestLogVersion() const override;

    std::optional<int64_t> GetUptimeSeconds() const override;

  private:
    void RecordRequest(const std::string& endpoint, const std::string& method,
                       int responseCode, const std::string& clientIP);
    void RegisterRoutes();
    void ServerThreadFunc();

    /** \brief Propagate the current dispatcher to the rendering controller if it exists.
     *  \pre m_Mutex is held by the caller. */
    void SyncDispatcherToController();

    /**
     * \brief Check if the client IP is allowed to access the server.
     *
     * Implements three modes: LocalhostOnly, AllowAll, Whitelist.
     * Sets 403 response if denied.
     *
     * \pre m_RunningConfig has a value.
     * \return true if allowed, false if denied (response already set).
     */
    bool CheckClientAccess(const httplib::Request& req, httplib::Response& res);

    /**
     * \brief Check if the request exceeds the rate limit.
     *
     * Per-IP sliding window rate limiter.
     * Sets 429 response with Retry-After header if exceeded.
     *
     * \pre m_RunningConfig has a value.
     * \return true if allowed, false if rate limited (response already set).
     */
    bool CheckRateLimit(const httplib::Request& req, httplib::Response& res);

    /**
     * \brief Check if the request has valid authentication.
     *
     * Validates Bearer token from Authorization header.
     * Health and info endpoints are exempt.
     * Sets 401 response if unauthorized.
     *
     * \pre m_RunningConfig has a value.
     * \return true if allowed, false if unauthorized (response already set).
     */
    bool CheckAuthentication(const httplib::Request& req, httplib::Response& res);

    // Server components
    std::unique_ptr<httplib::Server> m_Server;
    std::unique_ptr<std::thread> m_ServerThread;

    // Business logic
    std::unique_ptr<DataStorageBridge> m_Bridge;
    std::unique_ptr<RenderWindowBridge> m_RenderWindowBridge;
    std::unique_ptr<HealthController> m_HealthController;
    std::unique_ptr<DataStorageController> m_DataStorageController;
    std::unique_ptr<SwaggerController> m_SwaggerController;
    std::unique_ptr<RenderingController> m_RenderingController;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;

    // State
    mutable std::mutex m_Mutex;
    RestServerConfig m_PendingConfig;
    std::optional<RestServerConfig> m_RunningConfig;
    std::atomic<bool> m_Running{false};
    std::optional<std::string> m_LastError;

    // Request tracking
    std::set<std::string> m_ClientIPs;

    // Request logging
    std::deque<RequestInfo> m_RequestLog;
    std::optional<unsigned int> m_LogLimit;
    uint64_t m_RequestLogVersion{0};

    // Temporary directory for data serialization
    std::string m_TempDirectory;

    // Server start time for uptime tracking
    std::optional<std::chrono::steady_clock::time_point> m_StartTime;

    // Rate limiting data (protected by m_Mutex)
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> m_RateLimitMap;
    uint64_t m_RateLimitCheckCount{0};

    /**
     * \brief Create and setup the temporary directory for data operations.
     *
     * Called during Start(). Creates a unique temp directory for this server session.
     *
     * \return true if successful.
     */
    bool SetupTempDirectory();

    /**
     * \brief Clean up the temporary directory.
     *
     * Called during Stop(). Removes all files and the directory itself.
     * Takes the path as a parameter to allow thread-safe cleanup after
     * the path has been copied from m_TempDirectory while holding the lock.
     *
     * \param tempDir Path to the temporary directory to clean up.
     */
    static void CleanupTempDirectory(const std::string& tempDir);
  };
}

#endif
