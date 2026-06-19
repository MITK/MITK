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
    /**
     * \brief IP address or hostname to bind to.
     *
     * Defaults to the IPv6 wildcard address \c "::" so the server accepts
     * both IPv6 and IPv4 (4-mapped) connections from a single socket. This
     * avoids the ~2 s IPv6-first / IPv4-fallback latency that Windows
     * clients otherwise pay when resolving \c localhost (which Windows
     * orders as \c [::1, 127.0.0.1]).
     *
     * cpp-httplib is built with \c CPPHTTPLIB_IPV6_V6ONLY=false, so it
     * explicitly clears \c IPV6_V6ONLY on the listening socket; this is
     * required for dual-stack on Windows where the OS default would
     * otherwise be v6-only.
     *
     * Set to \c "127.0.0.1" to pin the server to IPv4 loopback only, to a
     * specific interface address, or to \c "0.0.0.0" for the IPv4 wildcard.
     */
    std::string host = "::";
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

  /**
   * \brief Build the public docs.mitk.org URL of the REST API specification for a
   *        given MITK version.
   *
   * Released builds publish under \c /<major>.<minor>/ with the minor zero-padded
   * to two digits (e.g. \c /2026.06/); development builds publish under
   * \c /nightly/. A patch level of 99 is MITK's development-build sentinel
   * (the top-level build appends the git revision to the version string in that
   * case), so \p patch \c == \c 99 selects the nightly path. Pass the
   * \c MITK_VERSION_MAJOR / \c MINOR / \c PATCH macros from \c mitkVersion.h at the
   * call site; taking them as parameters keeps the mapping rule unit-testable for
   * release versions, which a dev build can never exercise.
   */
  MITKRESTAPI_EXPORT std::string GetRestApiDocumentationUrl(int major, int minor, int patch);
}

#endif
