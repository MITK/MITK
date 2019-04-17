/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkIRESTManager_h
#define mitkIRESTManager_h

#include <mitkServiceInterface.h>

#include <MitkRESTExports.h>

#include <cpprest/json.h>
#include <cpprest/uri.h>

namespace mitk
{
  class IRESTObserver;
  class RESTServer;

  /**
   * @class IRESTManager
   * @brief This is a microservice interface for managing REST requests.
   */
  class MITKREST_EXPORT IRESTManager
  {
  public:
    virtual ~IRESTManager();

    /**
     * @brief request type for client requests by calling SendRequest
     */
    enum class RequestType
    {
      Get,
      Post,
      Put
    };

    /**
     * @brief Executes a HTTP request in the mitkRESTClient class
     *
     * @param uri defines the URI the request is send to
     * @param type the RequestType of the HTTP request (optional)
     * @param body the body for the request (optional)
     * @return task to wait for
     */
    virtual pplx::task<web::json::value> SendRequest(
      const web::uri &uri,
      const RequestType &type = RequestType::Get,
      const web::json::value *body = nullptr,
      const utility::string_t &filePath = {}
    ) = 0;

    /**
     * @brief starts listening for requests if there isn't another observer listening and the port is free
     *
     * @param uri defines the URI for which incoming requests should be send to the observer
     * @param observer the observer which handles the incoming requests
     */
    virtual void ReceiveRequest(const web::uri &uri, IRESTObserver *observer) = 0;

    /**
     * @brief Handles incoming requests by notifying the observer which should receive it
     *
     * @param uri defines the URI of the request
     * @param body the body of the request
     * @return the data which is modified by the notified observer
     */
    virtual web::json::value Handle(const web::uri &uri, const web::json::value &body) = 0;

    /**
     * @brief Handles the deletion of an observer for all or a specific uri
     *
     * @param observer the observer which shouldn't receive requests anymore
     * @param uri the uri for which the observer doesn't handle requests anymore (optional)
     */
    virtual void HandleDeleteObserver(IRESTObserver *observer, const web::uri &uri = {}) = 0;

    virtual const std::map<int, RESTServer *>& GetServerMap() = 0;
    virtual const std::map<std::pair<int, utility::string_t>, IRESTObserver *>& GetObservers() = 0;

  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTManager, "org.mitk.IRESTManager")

#endif
