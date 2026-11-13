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
#include <functional>
#include <map>
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
  class RestRouter;
  class RestStopGate;

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
    bool Start(const RestServerConfig& config, const std::string& startedBy) override;
    std::string GetStartedBy() const override;
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

    RestRouteId AddRoute(RestRoute route) override;
    void RemoveRoute(RestRouteId id) override;
    RestResponse HandleRequest(RestRequest request) override;

    std::shared_ptr<RestJob> CreateJob(const std::string& description) override;
    DataNode::Pointer FindNode(const std::string& uid) const override;
    std::string GetNodeUid(DataNode* node) const override;
    void Dispatch(const std::function<void()>& task) const override;
    void Post(std::function<void()> task) const override;

    std::string CreateToken(const std::string& client, const RestTokenRights& rights) override;
    void RemoveToken(const std::string& token) override;
    std::vector<RestTokenInfo> GetTokens() const override;

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
    using HttpHandler = std::function<void(const httplib::Request&, httplib::Response&)>;

    void RecordRequest(const std::string& endpoint, const std::string& method,
                       int responseCode, const std::string& clientIP);
    void RegisterRoutes(const RestServerConfig& config);
    void ServerThreadFunc();

    /**
     * \brief Register a route of the server's own controllers with httplib, and
     *        with the built-in router for HandleRequest(). Removed again by Stop().
     */
    void AddBuiltInRoute(const std::string& method, const std::string& pattern, HttpHandler handler);

    /** \brief Answer a request that came over HTTP and that no built-in route took. */
    void HandleRoutedHttp(const httplib::Request& req, httplib::Response& res);

    /** \brief Find the route of a request, built-in routes first, and run its handler. */
    RestResponse Route(RestRequest& request) const;

    RestResponse HandleGET_endpoints(const RestRequest& request) const;
    RestResponse HandleGET_jobs(const RestRequest& request) const;
    RestResponse HandleGET_job(const RestRequest& request) const;
    RestResponse HandleDELETE_job(const RestRequest& request);

    std::shared_ptr<RestJob> FindJob(const RestRequest& request) const;

    /** \brief A token of CreateToken(), or std::nullopt for any other token. */
    std::optional<RestTokenInfo> FindToken(const std::string& token) const;

    /** \brief The rights of a token of CreateToken(), or std::nullopt for any other token. */
    std::optional<RestTokenRights> GetTokenRights(const std::string& token) const;

    /** \brief Whether a request may name files on the server's file system. */
    bool HasFileAccess(const httplib::Request& req) const;

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

    /** \brief The routes of AddRoute() and the server's own newer endpoints. */
    std::unique_ptr<RestRouter> m_Router;

    /** \brief The routes of AddBuiltInRoute(): those of openapi.json, and the documentation. */
    std::unique_ptr<RestRouter> m_BuiltInRouter;
    std::vector<RestRouteId> m_BuiltInRoutes;

    mutable std::mutex m_TokenMutex;
    std::map<std::string, RestTokenInfo> m_Tokens;

    std::string m_StartedBy;

    /** \brief The jobs that run and the latest ended ones, oldest first. */
    mutable std::mutex m_JobMutex;
    std::deque<std::shared_ptr<RestJob>> m_Jobs;
    uint64_t m_NextJobNumber{1};

    /**
     * \brief Tells the requests that wait, in Dispatch() or for a job, that
     *        the server stops, so that Stop() can wait for them on the very
     *        thread they wait for.
     */
    std::shared_ptr<RestStopGate> m_StopGate;
    void SetStopping(bool stopping);

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
