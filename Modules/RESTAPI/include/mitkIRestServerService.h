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
#include <mitkRestServerConfig.h>
#include <mitkDataStorage.h>

#include <string>
#include <optional>
#include <vector>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * @brief Information about a REST API request.
   */
  struct MITKRESTAPI_EXPORT RequestInfo
  {
    std::string endpoint;   ///< The endpoint path (e.g., "/nodes", "/nodes/123")
    std::string method;     ///< HTTP method (GET, POST, PUT, DELETE)
    int responseCode;       ///< HTTP response code (200, 404, 500, etc.)
    std::string clientIP;   ///< IP address of the client
  };

  /**
   * @brief Service interface for the REST API server.
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
   * @ingroup MicroServices_Interfaces
   */
  struct MITKRESTAPI_EXPORT IRestServerService
  {
    virtual ~IRestServerService();

    // Lifecycle

    /**
     * @brief Start the REST server with the pending configuration.
     *
     * If the server is already running, this is a no-op and returns true.
     * The pending configuration becomes the running configuration upon successful start.
     *
     * @return true if the server started successfully (or was already running), false otherwise.
     */
    virtual bool Start() = 0;

    /**
     * @brief Stop the REST server.
     *
     * Blocks until the server is fully stopped. If the server is not running, this is a no-op.
     */
    virtual void Stop() = 0;

    /**
     * @brief Check if the server is currently running.
     * @return true if the server is running, false otherwise.
     */
    virtual bool IsRunning() const = 0;

    // Configuration

    /**
     * @brief Set the pending server configuration.
     *
     * The configuration will take effect on the next Start() call.
     * If the server is currently running, this only changes the pending config;
     * the running config remains unchanged until Restart().
     *
     * @param config The new configuration to use on next start.
     */
    virtual void SetConfig(const RestServerConfig& config) = 0;

    /**
     * @brief Get the pending server configuration.
     *
     * This is the configuration that will be used on the next Start() call.
     *
     * @return The pending configuration.
     */
    virtual RestServerConfig GetPendingConfig() const = 0;

    /**
     * @brief Get the configuration the server is currently running with.
     *
     * @return The running configuration, or std::nullopt if server is not running.
     */
    virtual std::optional<RestServerConfig> GetRunningConfig() const = 0;

    // DataStorage connection

    /**
     * @brief Set the DataStorage for REST API operations.
     *
     * The server can operate without a DataStorage (graceful degradation),
     * but data storage operations will return appropriate error responses.
     * Can be called while the server is running.
     *
     * @param dataStorage The DataStorage to use, or nullptr to disconnect.
     */
    virtual void SetDataStorage(DataStorage* dataStorage) = 0;

    /**
     * @brief Get the currently connected DataStorage.
     * @return The connected DataStorage, or nullptr if none.
     */
    virtual DataStorage* GetDataStorage() const = 0;

    // Status

    /**
     * @brief Get the full URL where the server is listening.
     *
     * @return The server URL (e.g., "http://127.0.0.1:8080"), or std::nullopt if not running.
     */
    virtual std::optional<std::string> GetServerUrl() const = 0;

    /**
     * @brief Get the last error message.
     *
     * @return The last error message, or std::nullopt if no error occurred since last Start().
     */
    virtual std::optional<std::string> GetLastError() const = 0;

    // Request tracking

    /**
     * @brief Get the list of client IP addresses that have sent requests.
     *
     * Returns a list of unique IP addresses from clients that have made
     * requests to the server since it was started.
     *
     * @return Vector of client IP addresses, empty if no requests have been made.
     */
    virtual std::vector<std::string> GetClientIPs() const = 0;

    /**
     * @brief Get information about the last request processed.
     *
     * @return The last request info, or std::nullopt if no requests have been processed.
     */
    virtual std::optional<RequestInfo> GetLastRequestInfo() const = 0;

    /**
     * @brief Set the request log limit.
     *
     * Controls how many request entries are kept in the log.
     * The log is cleared when the server starts.
     *
     * @param limit Maximum number of entries to keep, or std::nullopt for unlimited.
     */
    virtual void SetLogLimit(std::optional<unsigned int> limit) = 0;

    /**
     * @brief Get the current request log limit.
     *
     * @return The limit, or std::nullopt if unlimited.
     */
    virtual std::optional<unsigned int> GetLogLimit() const = 0;

    /**
     * @brief Get all logged requests.
     *
     * Returns the request log in chronological order (oldest first).
     *
     * @return Vector of request info entries.
     */
    virtual std::vector<RequestInfo> GetRequestLog() const = 0;

    /**
     * @brief Clear all logged requests.
     */
    virtual void ClearRequestLog() = 0;

    /**
     * @brief Get the server uptime in seconds.
     *
     * @return Uptime in seconds, or std::nullopt if server is not running.
     */
    virtual std::optional<int64_t> GetUptimeSeconds() const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRestServerService, "org.mitk.IRestServerService")

#endif
