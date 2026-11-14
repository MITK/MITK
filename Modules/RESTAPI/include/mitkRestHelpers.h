/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestHelpers_h
#define mitkRestHelpers_h

#include <MitkRESTAPIExports.h>

#include <mitkColorProperty.h>
#include <mitkIRestServerService.h>
#include <mitkRestRoute.h>

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Answer the request with an error, as ErrorResponse::Create() describes it.
   *
   * \throw RestException always.
   */
  [[noreturn]] MITKRESTAPI_EXPORT void ThrowRestError(
    const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path);

  /**
   * \brief The node that a UID of the API stands for, see IRestServerService::FindNode().
   *
   * \throw RestException with status 404 if there is no such node.
   */
  MITKRESTAPI_EXPORT DataNodeHandle FindRestNode(const IRestServerService& server, const std::string& uid, const std::string& path);

  /**
   * \brief As above, for a UID in a JSON body.
   *
   * \throw RestException with status 400 if the value is no string.
   */
  MITKRESTAPI_EXPORT DataNodeHandle FindRestNode(const IRestServerService& server, const nlohmann::json& uid, const std::string& path);

  /** \brief A color written as "#rrggbb", digits in either case; no value for any other text. */
  MITKRESTAPI_EXPORT std::optional<Color> ParseRestColor(const std::string& text);

  /** \brief A color as clients read it: "#rrggbb" in lower case. */
  MITKRESTAPI_EXPORT std::string ToRestColor(const Color& color);

  /**
   * \brief Offers routes through the IRestServerService of the application,
   *        and takes them back when it is destroyed.
   *
   * For the activators of plugins. The server loads with the application's
   * modules, before any plugin, so that it is there from the start or not at
   * all.
   */
  class MITKRESTAPI_EXPORT RestRouteRegistration
  {
  public:
    /** \brief Looks the server up; without one, Add() does nothing. */
    RestRouteRegistration();

    /** \brief Removes the routes that Add() added. */
    ~RestRouteRegistration();

    RestRouteRegistration(const RestRouteRegistration&) = delete;
    RestRouteRegistration& operator=(const RestRouteRegistration&) = delete;

    /** \brief The server, or null if the application has none. */
    IRestServerService* GetServer() const;

    /** \brief Add the routes to the server. */
    void Add(const std::vector<RestRoute>& routes);

  private:
    IRestServerService* m_Server = nullptr;
    std::vector<RestRouteId> m_RouteIds;
  };
}

#endif
