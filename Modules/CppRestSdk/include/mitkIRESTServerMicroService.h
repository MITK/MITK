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

#ifndef mitkIRESTServerMicroService_h
#define mitkIRESTServerMicroService_h

#include "cpprest/http_listener.h"


#include <MitkCppRestSdkExports.h>
#include <mitkIRESTManager.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>

typedef web::http::experimental::listener::http_listener MitkListener;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT IRESTServerMicroService
  {

  public:
    //**
    // * @brief Creates an server listening to the given URI
    // *
    // * @param uri the URI at which the server is listening for requests
    // */
    IRESTServerMicroService();
    ~IRESTServerMicroService();

    virtual web::uri GetUri() = 0;

    /**
     * @brief Opens the listener and starts the listening process
     */
    virtual void OpenListener() = 0;

    /**
     * @brief Closes the listener and stops the listening process
     */
    virtual void CloseListener() = 0;

  protected:
    /**
     * @brief Handle for incoming GET requests
     *
     * @param MitkRequest incoming request object
     */
    virtual void HandleGet(MitkRequest request) = 0;

    MitkListener m_Listener;
    web::uri m_Uri;

    // public slots:
  };
} // namespace mitk
#endif