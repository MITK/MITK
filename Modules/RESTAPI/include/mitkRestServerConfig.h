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
    Whitelist        ///< Accept connections only from explicitly listed IPs
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

    // Client access control
    ClientAccessMode clientAccessMode = ClientAccessMode::LocalhostOnly;
    std::vector<std::string> allowedClientIPs;

    // Authentication
    bool requireAuth = false;
    std::string apiToken;

    // Payload limits
    int maxPayloadSizeMB = 512;

    // File path restrictions
    FileAccessMode fileAccessMode = FileAccessMode::Unrestricted;
    std::vector<std::string> allowedFileDirectories;

    // Rate limiting
    bool rateLimitEnabled = false;
    int rateLimitPerMinute = 120;

    // Per-IP temp dir quota for file-reference GETs
    size_t maxActiveTempDirsPerIp = 5;

    // HTTPS
    bool httpsEnabled = false;
    std::string sslCertPath;
    std::string sslKeyPath;
  };
}

#endif
