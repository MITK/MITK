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

#include "cpprest/uri.h"
#include "cpprest/json.h"
#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>

#include <mitkIRESTObserver.h>
namespace mitk
{
  class IRESTManager
  {
    

  public:
    virtual ~IRESTManager();

    enum RequestType
    {
      get,
      post,
      put
    };

    /**
     * @brief Executes a HTTP request in the mitkRESTClientMicroService class
     *
     * @param uri defines the URI the request is send to
     * @param type the RequestType of the HTTP request (optional)
     * @param body the body for the request (optional)
     * @return task to wait for
     */
    virtual pplx::task<web::json::value> SendRequest(web::uri uri,
                                                     RequestType type = get,
                                                     web::json::value body = NULL,
                                                     utility::string_t filePath = L"") = 0;

    /**
     * @brief starts listening for requests if there isn't another observer listening and the port is free
     *
     * @param uri defines the URI for which incoming requests should be send to the observer
     * @param observer the observer which handles the incoming requests
     */
    virtual void ReceiveRequest(web::uri uri, IRESTObserver *observer) = 0;

    /**
     * @brief Handles incoming requests by notifying the observer which should receive it
     *
     * @param uri defines the URI of the request
     * @param body the body of the request
     * @return the data which is modified by the notified observer
     */
    virtual web::json::value Handle(web::uri uri, web::json::value body) = 0;

    /**
     * @brief Handles the deletion of an observer for all or a specific uri
     *
     * @param observer the observer which shouldn't receive requests anymore
     * @param uri the uri for which the observer doesn't handle requests anymore (optional)
     */
    virtual void HandleDeleteObserver(IRESTObserver *observer, web::uri uri = L"") = 0;
  };
} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTManager, "org.mitk.IRESTManager")

#endif
