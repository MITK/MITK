/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestServerConfig_h
#define mitkRestServerConfig_h

#include <MitkRESTAPIExports.h>
#include <string>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * @brief Configuration for the REST API server.
   *
   * @ingroup MicroServices_Interfaces
   */
  struct MITKRESTAPI_EXPORT RestServerConfig
  {
    std::string host = "127.0.0.1";
    int port = 8080;
    bool enabled = false;
    int threadPoolSize = 4;
    int readTimeoutSeconds = 30;
    int writeTimeoutSeconds = 30;

    // Security (Phase 2+)
    bool requireAuth = false;
    std::string apiToken;
  };
}

#endif
