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
  /**
   * \brief Access mode for client IP filtering.
   *
   * \sa RestServerConfig
   */
  enum class ClientAccessMode
  {
    LocalhostOnly,  ///< \brief Only accept connections from 127.0.0.1 and ::1 (default).
    AllowAll,        ///< \brief Accept connections from any IP (not recommended without authentication).
    Whitelist        ///< \brief Accept connections only from explicitly listed IPs.
  };

  /**
   * \brief Access mode for file system path restrictions.
   *
   * \sa RestServerConfig
   */
  enum class FileAccessMode
  {
    Unrestricted,       ///< \brief No file path restrictions (default).
    AllowedDirectories  ///< \brief Only allow paths under configured directories.
  };

  /**
   * \brief Configuration parameters for the REST API server.
   *
   * This struct holds all configurable settings for the MITK REST API server,
   * including network binding, security, rate limiting, and file access controls.
   * A RestServerConfig instance represents the "pending" configuration that takes
   * effect on the next server start.
   *
   * \sa IRestServerService, RestServer
   */
  struct MITKRESTAPI_EXPORT RestServerConfig
  {
    std::string host = "127.0.0.1";       ///< \brief IP address or hostname to bind to.
    int port = 8080;                       ///< \brief TCP port to listen on.
    bool enabled = false;                  ///< \brief Whether the server should be started automatically.
    int threadPoolSize = 4;                ///< \brief Number of worker threads for handling requests.
    int readTimeoutSeconds = 30;           ///< \brief Socket read timeout in seconds.
    int writeTimeoutSeconds = 30;          ///< \brief Socket write timeout in seconds.

    ClientAccessMode clientAccessMode = ClientAccessMode::LocalhostOnly;  ///< \brief Client IP access control mode.
    std::vector<std::string> allowedClientIPs;  ///< \brief List of allowed client IPs (used with Whitelist mode).

    bool requireAuth = false;              ///< \brief If \c true, require Bearer token authentication.
    std::string apiToken;                  ///< \brief The expected API token for Bearer authentication.

    int maxPayloadSizeMB = 512;            ///< \brief Maximum request payload size in megabytes.

    FileAccessMode fileAccessMode = FileAccessMode::Unrestricted;  ///< \brief File system access restriction mode.
    std::vector<std::string> allowedFileDirectories;  ///< \brief Allowed directories (used with AllowedDirectories mode).

    bool rateLimitEnabled = false;         ///< \brief If \c true, enable per-IP rate limiting.
    int rateLimitPerMinute = 120;          ///< \brief Maximum number of requests per IP per minute.

    size_t maxActiveTempDirsPerIp = 5;     ///< \brief Maximum number of active temporary directories per client IP.

    bool httpsEnabled = false;             ///< \brief If \c true, enable HTTPS with TLS.
    std::string sslCertPath;               ///< \brief Path to the SSL/TLS certificate file.
    std::string sslKeyPath;                ///< \brief Path to the SSL/TLS private key file.
  };
}

#endif
