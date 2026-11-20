/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestRoute_h
#define mitkRestRoute_h

#include <MitkRESTAPIExports.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace mitk
{
  class RestWaitInterruption;

  /**
   * \brief A request as route handlers of IRestServerService::AddRoute() see it.
   */
  struct MITKRESTAPI_EXPORT RestRequest
  {
    std::string Method;

    /** \brief The decoded path, without the query. */
    std::string Path;

    /** \brief The values of the route's path parameters, by their names in the route's path. */
    std::map<std::string, std::string> PathParams;

    std::multimap<std::string, std::string> QueryParams;

    /** \brief Header values by lower-case header names. */
    std::map<std::string, std::string> Headers;

    std::string Body;

    /** \brief The address of the client, for logs and per-client limits. */
    std::string ClientAddress;

    /**
     * \brief Lets the sender of an in-process request end its wait for a job
     *        early, see RestJob::Respond(). Null: the request waits as long
     *        as it asks to.
     */
    std::shared_ptr<RestWaitInterruption> WaitInterruption;

    /**
     * \brief Set Path and QueryParams from a request target like
     *        "/api/v1/datastorage/nodes?limit=10", decoding both.
     */
    void SetTarget(const std::string& target);

    /** \brief The first value of a query parameter, if the request has it. */
    std::optional<std::string> GetQueryParam(const std::string& name) const;

    /** \brief The value of a header, if the request has it. The name is not case-sensitive. */
    std::optional<std::string> GetHeader(const std::string& name) const;

    /** \brief The token of an "Authorization: Bearer" header, if the request has one. The scheme is not case-sensitive. */
    std::optional<std::string> GetBearerToken() const;

    /**
     * \brief The body parsed as JSON. An empty body gives an empty object.
     *
     * \throw RestException with status 400 if the body is no valid JSON.
     */
    nlohmann::json GetJsonBody() const;
  };

  /**
   * \brief JSON as the text of an answer.
   *
   * A string that is not valid UTF-8, like a DICOM tag in Latin-1, keeps its
   * characters: wherever its bytes are no UTF-8, they are read as Latin-1.
   */
  MITKRESTAPI_EXPORT std::string DumpRestJson(const nlohmann::json& json);

  struct MITKRESTAPI_EXPORT RestResponse
  {
    int Status = 200;
    std::string ContentType;
    std::string Body;

    /** \brief Headers besides the content type, like Location. */
    std::map<std::string, std::string> Headers;

    /** \brief A JSON body in the envelope of the API: {"data": data}. */
    static RestResponse FromData(const nlohmann::json& data, int status = 200);

    /** \brief An error made by mitk::ErrorResponse. Its status becomes the response's status. */
    static RestResponse FromError(const nlohmann::json& error);

    static RestResponse FromBytes(std::string bytes, const std::string& contentType);

    static RestResponse NoContent();

    bool HasJsonBody() const;
  };

  /**
   * \brief Thrown by route handlers to answer with an error made by mitk::ErrorResponse.
   */
  class MITKRESTAPI_EXPORT RestException : public std::runtime_error
  {
  public:
    explicit RestException(const nlohmann::json& error);

    const nlohmann::json& GetError() const;

  private:
    nlohmann::json m_Error;
  };

  using RestHandler = std::function<RestResponse(const RestRequest&)>;

  /**
   * \brief An endpoint below /api/v1 that a module or plugin offers through
   *        IRestServerService::AddRoute().
   */
  struct MITKRESTAPI_EXPORT RestRoute
  {
    /** \brief GET, POST, PUT, PATCH or DELETE. */
    std::string Method;

    /**
     * \brief The path below /api/v1, starting with a slash.
     *
     * Segments of the form ":name" match any one segment and pass it to the
     * handler as path parameter "name", like in "/manual/sections/:id".
     */
    std::string Path;

    /** \brief One line saying what the endpoint does. */
    std::string Summary;

    /** \brief Parameters, body and response, for clients that discover the API at run time. */
    std::string Description;

    /**
     * \brief Answers the requests.
     *
     * Runs on a thread of the server. Work on the data storage, render windows
     * or widgets has to be handed to the GUI thread. Exceptions become error
     * responses: RestException with its error, exceptions of nlohmann::json
     * (a body of the wrong shape) with status 400, others with status 500.
     */
    RestHandler Handler;
  };

  using RestRouteId = std::uint64_t;

  /**
   * \brief What clients that authenticate with a token of
   *        IRestServerService::CreateToken() may do beyond reading and changing
   *        the data storage and the views.
   *
   * The rights bind only requests that present the token. On a server that
   * does not require authentication, a request without a token is limited by
   * the file access mode of the server alone.
   *
   * Route handlers of plugins cannot see the rights of a request yet, so a
   * route that touches files needs that first.
   */
  struct MITKRESTAPI_EXPORT RestTokenRights
  {
    /**
     * \brief Whether requests may name files on the server's file system, to load data from or to get data as.
     *
     * Also covers uploaded data that refers to other files, like a detached
     * NRRD or MetaImage header. Formats whose reader opens other files in a
     * way the server does not know are not recognized as such.
     */
    bool FileAccess = false;
  };

  /** \brief A token of IRestServerService::CreateToken(), without its secret. */
  struct MITKRESTAPI_EXPORT RestTokenInfo
  {
    /** \brief Who uses the token, like "Agent view", for the user to see. */
    std::string Client;

    RestTokenRights Rights;
  };
}

#endif
