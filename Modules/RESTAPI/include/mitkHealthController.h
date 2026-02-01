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
#include <httplib.h>

#include <MitkRESTAPIExports.h>

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
     * @brief Construct a HealthController.
     *
     * @param bridge Reference to the DataStorageBridge for status checks. It is the instance
     * controlled by the REST server to ensure requests from all controllers are handled thread safe
     * and channeled towards the DataStorage.
     */
    explicit HealthController(DataStorageBridge& bridge);

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

  private:
    DataStorageBridge& m_Bridge;
  };
}

#endif
