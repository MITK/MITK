/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHealthController_h
#define mitkHealthController_h

#include "mitkDataStorageBridge.h"
#include <mitkRestServerConfig.h>
#include <httplib.h>

#include <MitkRESTAPIExports.h>

#include <functional>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * @brief Handles health and info endpoints.
   *
   * Endpoints:
   * - GET /api/v1/health -> {"status": "ok"}
   * - GET /api/v1/info   -> Server info including datastorage availability
   */
  class MITKRESTAPI_EXPORT HealthController
  {
  public:
    /**
     * @brief Callback type for retrieving server uptime.
     *
     * Returns the server uptime in seconds, or std::nullopt if not available.
     */
    using UptimeCallback = std::function<std::optional<int64_t>()>;

    /**
     * @brief Construct a HealthController.
     *
     * @param bridge Reference to the DataStorageBridge for status checks. It is the instance
     * controlled by the REST server to ensure requests from all controllers are handled thread safe
     * and channeled towards the DataStorage.
     */
    explicit HealthController(DataStorageBridge& bridge);

    /**
     * @brief Set the callback for retrieving server uptime.
     *
     * @param callback Function that returns uptime in seconds, or std::nullopt if unavailable.
     */
    void SetUptimeCallback(UptimeCallback callback);

    /**
     * @brief Handle GET /health request.
     *
     * Returns basic health status.
     *
     * @param req The HTTP request.
     * @param res The HTTP response to populate.
     */
    void HandleGET_health(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle GET /info request.
     *
     * Returns detailed server information.
     *
     * @param req The HTTP request.
     * @param res The HTTP response to populate.
     */
    void HandleGET_info(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle GET /config/file-access request.
     *
     * Returns the current file access configuration, including the mode
     * and, when restrictions are active, the list of allowed directories.
     *
     * @param req The HTTP request.
     * @param res The HTTP response to populate.
     */
    void HandleGET_config_file_access(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Set the file access configuration.
     *
     * @pre \a mode must be a valid FileAccessMode value.
     * @pre When \a mode is AllowedDirectories, \a allowedDirs must not be empty.
     *
     * @param mode The file access mode to apply.
     * @param allowedDirs Directories to allow when mode is AllowedDirectories.
     */
    void SetFileAccessConfig(FileAccessMode mode, const std::vector<std::string>& allowedDirs);

  private:
    DataStorageBridge& m_Bridge;
    mutable std::mutex m_Mutex;
    UptimeCallback m_UptimeCallback;
    FileAccessMode m_FileAccessMode = FileAccessMode::Unrestricted;
    std::vector<std::string> m_AllowedFileDirectories;
  };
}

#endif
