/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIRestServerService_h
#define mitkIRestServerService_h

#include <mitkServiceInterface.h>
#include <mitkRestJob.h>
#include <mitkRestRoute.h>
#include <mitkRestServerConfig.h>
#include <mitkDataStorage.h>

namespace mitk { class RenderWindowBridge; }

#include <string>
#include <optional>
#include <vector>
#include <cstdint>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * \brief Information about a REST API request.
   */
  struct MITKRESTAPI_EXPORT RequestInfo
  {
    std::string endpoint;   ///< The endpoint path (e.g., "/nodes", "/nodes/123")
    std::string method;     ///< HTTP method (GET, POST, PUT, DELETE)
    int responseCode;       ///< HTTP response code (200, 404, 500, etc.)
    std::string clientIP;   ///< IP address of the client
  };

  /**
   * \brief Service interface for the REST API server.
   *
   * Allows other modules and plugins to:
   * - Start/stop the server
   * - Configure server settings
   * - Connect a DataStorage instance
   * - Query server status
   *
   * ## Configuration Semantics
   *
   * The server maintains two configuration states:
   * - **Pending configuration**: Set via SetConfig(), takes effect on next Start()
   * - **Running configuration**: The configuration the server is actually running with
   *
   * GetPendingConfig() returns the configuration that will be used on next Start().
   * GetRunningConfig() returns the configuration the server is currently running with,
   * or std::nullopt if the server is not running.
   *
   * \ingroup MicroServices_Interfaces
   */
  struct MITKRESTAPI_EXPORT IRestServerService
  {
    virtual ~IRestServerService();

    // Lifecycle

    /**
     * \brief Start the REST server with the pending configuration.
     *
     * If the server is already running, this is a no-op and returns true.
     * The pending configuration becomes the running configuration upon successful start.
     *
     * \warning Blocks until the server's listen loop is ready. Do not call it
     * synchronously while a platform loader / plugin-activation lock is held: the
     * blocking wait under that lock deadlocks. Defer the call onto the dispatch thread
     * via StorageThreadDispatcherBase::Post() in that case.
     *
     * \return true if the server started successfully (or was already running), false otherwise.
     */
    virtual bool Start() = 0;

    /**
     * \brief Start the REST server with a configuration of the caller, leaving
     *        the pending configuration as it is.
     *
     * For a component that needs the server for itself, like the Agent view,
     * without changing what the user configured. Port 0 lets the system choose
     * a free port; GetRunningConfig() tells which.
     *
     * \param startedBy Who started the server, like "Agent view", for the user to see.
     * \return true if the server started or was running already, with the
     *         configuration it was started with then.
     *
     * \warning See Start().
     */
    virtual bool Start(const RestServerConfig& config, const std::string& startedBy) = 0;

    /** \brief Who started the running server with Start(config, startedBy); empty otherwise. */
    virtual std::string GetStartedBy() const = 0;

    /**
     * \brief Stop the REST server.
     *
     * Blocks until the server is fully stopped. If the server is not running, this is a no-op.
     */
    virtual void Stop() = 0;

    /**
     * \brief Check if the server is currently running.
     * \return true if the server is running, false otherwise.
     */
    virtual bool IsRunning() const = 0;

    // Configuration

    /**
     * \brief Set the pending server configuration.
     *
     * The configuration will take effect on the next Start() call.
     * If the server is currently running, this only changes the pending config;
     * the running config remains unchanged until Restart().
     *
     * \param config The new configuration to use on next start.
     */
    virtual void SetConfig(const RestServerConfig& config) = 0;

    /**
     * \brief Get the pending server configuration.
     *
     * This is the configuration that will be used on the next Start() call.
     *
     * \return The pending configuration.
     */
    virtual RestServerConfig GetPendingConfig() const = 0;

    /**
     * \brief Get the configuration the server is currently running with.
     *
     * \return The running configuration, or std::nullopt if server is not running.
     */
    virtual std::optional<RestServerConfig> GetRunningConfig() const = 0;

    // Render-window integration

    /**
     * \brief Return the server-owned RenderWindowBridge.
     *
     * The bridge is owned by the server. UI-layer plugins (e.g. the Qt workbench
     * plugin) configure it by setting callbacks for screenshot capture and
     * crosshair position access. Callbacks must be cleared before the objects
     * they reference are destroyed.
     *
     * \return Pointer to the bridge. Never null.
     */
    virtual RenderWindowBridge* GetRenderWindowBridge() = 0;

    // DataStorage connection

    /**
     * \brief Set the DataStorage for REST API operations.
     *
     * The server can operate without a DataStorage (graceful degradation),
     * but data storage operations will return appropriate error responses.
     * Can be called while the server is running.
     *
     * \param dataStorage The DataStorage to use, or nullptr to disconnect.
     */
    virtual void SetDataStorage(DataStorage* dataStorage) = 0;

    /**
     * \brief Get the currently connected DataStorage.
     * \return The connected DataStorage, or nullptr if none.
     */
    virtual DataStorage::Pointer GetDataStorage() const = 0;

    // Endpoints of other modules and plugins

    /**
     * \brief Offer an endpoint below /api/v1.
     *
     * Routes can be added and removed at any time, also while the server runs,
     * and they stay across restarts of the server. Their owner removes them
     * before the objects their handlers use are destroyed. The endpoints of
     * the server's own controllers come first: a route with the method and
     * path of one of them is never reached. Clients see added routes as
     * experimental. Requests that a web page could send are refused: those
     * with an Origin header, with a Sec-Fetch-Site header other than none
     * and same-origin, or with a Host header that names neither localhost,
     * an IP address, the host of the server nor the host name of the machine.
     * This keeps web pages from changing state through them. As old browsers
     * send none of these headers with a GET request, GET routes must not
     * change state.
     *
     * \return An id for RemoveRoute().
     * \throw mitk::Exception if the route is invalid, or one with the same
     *        method and path exists.
     */
    virtual RestRouteId AddRoute(RestRoute route) = 0;

    /**
     * \brief Remove a route of AddRoute().
     *
     * Does not wait for requests: a handler that a request has reached
     * already may still run after the call returns.
     */
    virtual void RemoveRoute(RestRouteId id) = 0;

    /**
     * \brief Answer a request without HTTP, as the server answers it over HTTP
     *        from the local host.
     *
     * Authentication works as over HTTP: the request needs an "Authorization:
     * Bearer" header if the server requires authentication, and a token of
     * CreateToken() limits the request to the token's rights. Browser checks
     * do not apply, as the request does not come from a browser, and neither
     * do the list of client addresses and the rate limit. Unlike over HTTP, a
     * built-in route does not match a path with a trailing slash or with an
     * empty parameter.
     *
     * Meant for clients in the same process that relay requests of their own
     * clients, like an endpoint for AI agents. Call it only from within a
     * route handler: Stop() waits for the handlers of the server's own threads
     * before it destroys what requests use, not for other callers. Never call
     * it on the thread that owns the data storage: requests of the built-in
     * routes hold a lock while they wait for that thread.
     *
     * \param request A request with a method and a target set by
     *        RestRequest::SetTarget(), below /api/v1.
     * \return The response; status 503 if the server is not running.
     */
    virtual RestResponse HandleRequest(RestRequest request) = 0;

    /**
     * \brief Make a job, which clients can follow below /api/v1/jobs.
     *
     * \param description What the job does, like "Run TotalSegmentator".
     */
    virtual std::shared_ptr<RestJob> CreateJob(const std::string& description) = 0;

    /**
     * \brief The node that a UID of the API stands for, or null.
     *
     * Can be called from any thread.
     */
    virtual DataNode::Pointer FindNode(const std::string& uid) const = 0;

    /**
     * \brief The UID of a node, as the API names it.
     *
     * Can be called from any thread. A node without a UID gets one in the
     * thread that owns the data storage, for which the call waits then like
     * Dispatch() does.
     */
    virtual std::string GetNodeUid(DataNode* node) const = 0;

    /**
     * \brief Run a task in the thread that owns the data storage, the GUI
     *        thread in the Workbench, and wait for it.
     *
     * For route handlers, which run on threads of the server. Without a
     * dispatcher, the task runs right away. The task must not throw; see
     * mitk::CallInGuiThread() for a variant that passes results and
     * exceptions on.
     *
     * \throw RestException (503) if the server stops before the task has
     *        started. The task does not run then.
     */
    virtual void Dispatch(const std::function<void()>& task) const = 0;

    /**
     * \brief Run a task in the thread that owns the data storage later,
     *        without waiting for it.
     *
     * For long work that a job reports on, like starting a model. Without a
     * dispatcher, the task runs right away.
     */
    virtual void Post(std::function<void()> task) const = 0;

    // Tokens

    /**
     * \brief Make a token for a client that gets only the given rights.
     *
     * Requests with the token are accepted whether the server requires
     * authentication or not. The token stays valid across restarts of the
     * server until RemoveToken().
     *
     * \param client Who uses the token, like "Agent view", for the user to see.
     */
    virtual std::string CreateToken(const std::string& client, const RestTokenRights& rights) = 0;

    virtual void RemoveToken(const std::string& token) = 0;

    /** \brief The tokens of CreateToken() that are valid, without their secrets. */
    virtual std::vector<RestTokenInfo> GetTokens() const = 0;

    // Status

    /**
     * \brief Get the full URL where the server is listening.
     *
     * \return The server URL (e.g., "http://127.0.0.1:8080"), or std::nullopt if not running.
     */
    virtual std::optional<std::string> GetServerUrl() const = 0;

    /**
     * \brief Get the last error message.
     *
     * \return The last error message, or std::nullopt if no error occurred since last Start().
     */
    virtual std::optional<std::string> GetLastError() const = 0;

    // Request tracking

    /**
     * \brief Get the list of client IP addresses that have sent requests.
     *
     * Returns a list of unique IP addresses from clients that have made
     * requests to the server since it was started.
     *
     * \return Vector of client IP addresses, empty if no requests have been made.
     */
    virtual std::vector<std::string> GetClientIPs() const = 0;

    /**
     * \brief Get information about the last request processed.
     *
     * \return The last request info, or std::nullopt if no requests have been processed.
     */
    virtual std::optional<RequestInfo> GetLastRequestInfo() const = 0;

    /**
     * \brief Set the request log limit.
     *
     * Controls how many request entries are kept in the log.
     * The log is cleared when the server starts.
     *
     * \param limit Maximum number of entries to keep, or std::nullopt for unlimited.
     */
    virtual void SetLogLimit(std::optional<unsigned int> limit) = 0;

    /**
     * \brief Get the current request log limit.
     *
     * \return The limit, or std::nullopt if unlimited.
     */
    virtual std::optional<unsigned int> GetLogLimit() const = 0;

    /**
     * \brief Get all logged requests.
     *
     * Returns the request log in chronological order (oldest first).
     *
     * \return Vector of request info entries.
     */
    virtual std::vector<RequestInfo> GetRequestLog() const = 0;

    /**
     * \brief Clear all logged requests.
     */
    virtual void ClearRequestLog() = 0;

    /**
     * \brief Get the request log version counter.
     *
     * Incremented on every log mutation (new entry, clear, trim).
     * Allows consumers to detect changes without copying the full log.
     *
     * \return Monotonically increasing version number.
     */
    virtual uint64_t GetRequestLogVersion() const = 0;

    /**
     * \brief Get the server uptime in seconds.
     *
     * \return Uptime in seconds, or std::nullopt if server is not running.
     */
    virtual std::optional<int64_t> GetUptimeSeconds() const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRestServerService, "org.mitk.IRestServerService")

#endif
