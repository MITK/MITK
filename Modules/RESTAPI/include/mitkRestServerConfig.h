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
#include <vector>

namespace mitk
{
  /** @brief Access mode for client IP filtering. */
  enum class ClientAccessMode
  {
    LocalhostOnly,  ///< Only accept connections from 127.0.0.1 and ::1 (default)
    AllowAll,        ///< Accept connections from any IP (not recommended without auth)
    Whitelist        ///< Accept connections from listed IPs (localhost always allowed)
  };

  /** @brief Access mode for file system paths. */
  enum class FileAccessMode
  {
    Unrestricted,       ///< No file path restrictions (default)
    AllowedDirectories  ///< Only allow paths under configured directories
  };

  /**
   * @brief Configuration for the REST API server.
   *
   * @ingroup MicroServices_Interfaces
   */
  struct MITKRESTAPI_EXPORT RestServerConfig
  {
    // Server settings
    std::string host = "127.0.0.1";
    int port = 8080;
    bool enabled = false;
    int threadPoolSize = 4;
    int readTimeoutSeconds = 30;
    int writeTimeoutSeconds = 30;

    // Client access control (Phase 4.2)
    ClientAccessMode clientAccessMode = ClientAccessMode::LocalhostOnly;
    std::vector<std::string> allowedClientIPs;

    // Authentication (Phase 4.3)
    bool requireAuth = false;
    std::string apiToken;

    // Payload limits (Phase 4.4)
    int maxPayloadSizeMB = 512;

    // File path restrictions (Phase 4.5)
    FileAccessMode fileAccessMode = FileAccessMode::Unrestricted;
    std::vector<std::string> allowedFileDirectories;

    // Rate limiting (Phase 4.6)
    bool rateLimitEnabled = false;
    int rateLimitPerMinute = 120;

    // HTTPS (Phase 4.7)
    bool httpsEnabled = false;
    std::string sslCertPath;
    std::string sslKeyPath;
  };
}

#endif
