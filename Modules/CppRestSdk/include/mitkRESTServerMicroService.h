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

#ifndef mitkRESTServerMicroService_h
#define mitkRESTServerMicroService_h

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "cpprest/http_listener.h"

#include "MitkCppRestSdkExports.h"
#include <mitkIRESTManager.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>
#include <MitkCppRestSdkExports.h>

typedef web::http::experimental::listener::http_listener MitkListener;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTServerMicroService
  {

  public:
    /**
     * @brief Creates an server listening to the given URI
     *
     * @param uri the URI at which the server is listening for requests
     */
    RESTServerMicroService(web::uri uri);
    ~RESTServerMicroService();

    web::uri GetUri();

     /**
     * @brief Opens the listener and starts the listening process
     */
    void OpenListener();

    /**
     * @brief Closes the listener and stops the listening process
     */
    void CloseListener();
  private:
    /**
     * @brief Handle for incoming GET requests
     *
     * @param MitkRequest incoming request object
     */
    void HandleGet(MitkRequest request); 

  protected:
    MitkListener m_Listener;
    web::uri m_Uri;
  };
} // namespace mitk
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif