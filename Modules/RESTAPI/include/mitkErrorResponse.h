/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkErrorResponse_h
#define mitkErrorResponse_h

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * \brief Helper class for generating RFC 7807 compliant error responses.
   *
   * RFC 7807 defines a standard format for HTTP API error responses.
   * This class provides factory methods for common error types.
   */
  class MITKRESTAPI_EXPORT ErrorResponse
  {
  public:
    using Json = nlohmann::json;

    // Common error codes
    static constexpr const char* CODE_INVALID_REQUEST = "INVALID_REQUEST";
    static constexpr const char* CODE_NODE_NOT_FOUND = "NODE_NOT_FOUND";
    static constexpr const char* CODE_PROPERTY_NOT_FOUND = "PROPERTY_NOT_FOUND";
    static constexpr const char* CODE_DATASTORAGE_NOT_AVAILABLE = "DATASTORAGE_NOT_AVAILABLE";
    static constexpr const char* CODE_INTERNAL_ERROR = "INTERNAL_ERROR";
    static constexpr const char* CODE_PROPERTY_PROTECTED = "PROPERTY_PROTECTED";
    static constexpr const char* CODE_NOT_IMPLEMENTED = "NOT_IMPLEMENTED";
    static constexpr const char* CODE_NODE_HAS_CHILDREN = "NODE_HAS_CHILDREN";
    static constexpr const char* CODE_NO_DATA = "NO_DATA";
    static constexpr const char* CODE_NO_GEOMETRY = "NO_GEOMETRY";
    static constexpr const char* CODE_RENDERING_ERROR = "RENDERING_ERROR";
    static constexpr const char* CODE_CIRCULAR_HIERARCHY_REFERENCE = "CIRCULAR_HIERARCHY_REFERENCE";
    static constexpr const char* CODE_UNSUPPORTED_FORMAT = "UNSUPPORTED_FORMAT";
    static constexpr const char* CODE_SERIALIZATION_ERROR = "SERIALIZATION_ERROR";
    static constexpr const char* CODE_FILE_NOT_FOUND = "FILE_NOT_FOUND";
    static constexpr const char* CODE_FILE_READ_ERROR = "FILE_READ_ERROR";
    static constexpr const char* CODE_TRANSFER_MODE_NOT_AVAILABLE = "TRANSFER_MODE_NOT_AVAILABLE";
    static constexpr const char* CODE_ACCESS_DENIED = "ACCESS_DENIED";
    static constexpr const char* CODE_UNAUTHORIZED = "UNAUTHORIZED";
    static constexpr const char* CODE_RATE_LIMIT_EXCEEDED = "RATE_LIMIT_EXCEEDED";
    static constexpr const char* CODE_FILE_ACCESS_DENIED = "FILE_ACCESS_DENIED";
    static constexpr const char* CODE_RENDER_WINDOW_NOT_AVAILABLE = "RENDER_WINDOW_NOT_AVAILABLE";
    static constexpr const char* CODE_TIME_NAVIGATION_NOT_AVAILABLE = "TIME_NAVIGATION_NOT_AVAILABLE";
    static constexpr const char* CODE_TIME_STEPPER_NOT_AVAILABLE = "TIME_STEPPER_NOT_AVAILABLE";

    /**
     * \brief Create an RFC 7807 error response.
     *
     * \param code Error code (e.g., "NODE_NOT_FOUND")
     * \param title Human-readable title
     * \param detail Detailed error message
     * \param status HTTP status code
     * \param instance Request path that caused the error
     * \return JSON object following RFC 7807 format
     */
    static Json Create(
      const std::string& code,
      const std::string& title,
      const std::string& detail,
      int status,
      const std::string& instance = "");

    /**
     * \brief Create a "Node not found" error response.
     *
     * \param uid The UID that was not found
     * \param instance Request path
     * \return JSON error response with status 404
     */
    static Json NodeNotFound(const std::string& uid, const std::string& instance = "");

    /**
     * \brief Create a "Property not found" error response.
     *
     * \param propertyKey The property key that was not found
     * \param nodeUid The node UID
     * \param instance Request path
     * \return JSON error response with status 404
     */
    static Json PropertyNotFound(
      const std::string& propertyKey,
      const std::string& nodeUid,
      const std::string& instance = "");

    /**
     * \brief Create a "DataStorage not available" error response.
     *
     * \param instance Request path
     * \return JSON error response with status 503
     */
    static Json DataStorageNotAvailable(const std::string& instance = "");

    /**
     * \brief Create an "Invalid request" error response.
     *
     * \param detail Description of what was invalid
     * \param instance Request path
     * \return JSON error response with status 400
     */
    static Json InvalidRequest(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create an "Internal error" response.
     *
     * \param detail Description of the error
     * \param instance Request path
     * \return JSON error response with status 500
     */
    static Json InternalError(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "Property protected" error response.
     *
     * \param propertyKey The protected property key
     * \param instance Request path
     * \return JSON error response with status 400
     */
    static Json PropertyProtected(const std::string& propertyKey, const std::string& instance = "");

    /**
     * \brief Create a "Not implemented" error response.
     *
     * \param feature The feature that is not implemented
     * \param instance Request path
     * \return JSON error response with status 501
     */
    static Json NotImplemented(const std::string& feature, const std::string& instance = "");

    /**
     * \brief Create a "Node has children" error response.
     *
     * \param childrenCount The number of children
     * \param instance Request path
     * \return JSON error response with status 409
     */
    static Json NodeHasChildren(int childrenCount, const std::string& instance = "");

    /**
     * \brief Create a "No data" error response.
     *
     * \param nodeUid The node UID that has no data
     * \param instance Request path
     * \return JSON error response with status 404
     */
    static Json NoData(const std::string& nodeUid, const std::string& instance = "");

    /**
     * \brief Create a "No geometry" error response.
     *
     * \param nodeUid The node UID whose data has no usable time geometry
     * \param instance Request path
     * \return JSON error response with status 422
     */
    static Json NoGeometry(const std::string& nodeUid, const std::string& instance = "");

    /**
     * \brief Create a "Rendering error" error response.
     *
     * \param detail Description of the rendering failure
     * \param instance Request path
     * \return JSON error response with status 422
     */
    static Json RenderingError(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "Circular hierarchy reference" error response.
     *
     * \param instance Request path
     * \return JSON error response with status 409
     */
    static Json CircularHierarchyReference(const std::string& instance = "");

    /**
     * \brief Create an "Unsupported format" error response.
     *
     * \param detail Description of the format issue
     * \param instance Request path
     * \return JSON error response with status 415
     */
    static Json UnsupportedFormat(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "Serialization error" error response.
     *
     * \param detail Description of the serialization failure
     * \param instance Request path
     * \return JSON error response with status 500
     */
    static Json SerializationError(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "File not found" error response.
     *
     * \param filePath The file path that was not found
     * \param instance Request path
     * \return JSON error response with status 422
     */
    static Json FileNotFound(const std::string& filePath, const std::string& instance = "");

    /**
     * \brief Create a "File read error" error response.
     *
     * \param filePath The file path that could not be read
     * \param detail Description of the read error
     * \param instance Request path
     * \return JSON error response with status 422
     */
    static Json FileReadError(const std::string& filePath, const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "Transfer mode not available" error response.
     *
     * \param requestedMode The requested transfer mode that is not supported
     * \param availableModes List of available transfer modes
     * \param instance Request path
     * \return JSON error response with status 406
     */
    static Json TransferModeNotAvailable(
      const std::string& requestedMode,
      const std::vector<std::string>& availableModes,
      const std::string& instance = "");

    /**
     * \brief Create an "Access denied" error response.
     *
     * Used when client IP is not allowed to access the server.
     *
     * \param detail Description of why access was denied
     * \param instance Request path
     * \return JSON error response with status 403
     */
    static Json AccessDenied(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create an "Unauthorized" error response.
     *
     * Used when authentication is required but not provided or invalid.
     *
     * \param detail Description of the authentication failure
     * \param instance Request path
     * \return JSON error response with status 401
     */
    static Json Unauthorized(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "Rate limit exceeded" error response.
     *
     * \param retryAfterSeconds Seconds until the client can retry
     * \param instance Request path
     * \return JSON error response with status 429
     */
    static Json RateLimitExceeded(int retryAfterSeconds, const std::string& instance = "");

    /**
     * \brief Create a "File access denied" error response.
     *
     * Used when a file path is outside the allowed directories.
     *
     * \param detail Description of the restriction
     * \param instance Request path
     * \return JSON error response with status 403
     */
    static Json FileAccessDenied(const std::string& detail, const std::string& instance = "");

    /**
     * \brief Create a "Render window not available" error response.
     *
     * Used when the screenshot provider is not connected (headless mode or
     * Qt plugin not loaded).
     *
     * \param instance Request path
     * \return JSON error response with status 503
     */
    static Json RenderWindowNotAvailable(const std::string& instance = "");

    /**
     * \brief Create a "Time navigation not available" error response.
     *
     * Used when the TimeNavigationController is null (e.g. no rendering manager active).
     *
     * \param instance Request path
     * \return JSON error response with status 503
     */
    static Json TimeNavigationNotAvailable(const std::string& instance = "");

    /**
     * \brief Create a "Time stepper not available" error response.
     *
     * Used when the TimeNavigationController's stepper is null.
     *
     * \param instance Request path
     * \return JSON error response with status 500
     */
    static Json TimeStepperNotAvailable(const std::string& instance = "");
  };
}

#endif
