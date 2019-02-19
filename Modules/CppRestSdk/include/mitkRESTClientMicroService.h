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

#ifndef mitkRESTClientMicroService_h
#define mitkRESTClientMicroService_h

#include "cpprest/asyncrt_utils.h"
#include "cpprest/containerstream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include "cpprest/uri.h"

#include "MitkCppRestSdkExports.h"
#include <mitkIRESTManager.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>

typedef web::http::client::http_client MitkClient;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::uri_builder MitkUriBuilder;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;

namespace mitk
{
  class RESTClientMicroService
  {
  public:
    RESTClientMicroService();
    ~RESTClientMicroService();

   /**
    * @brief Executes a HTTP GET request with the given uri and returns a task waiting for a json object
    *
    * @param uri the URI resulting the target of the HTTP request
    * @return task with to wait for with resulting json object
    */
   pplx::task<web::json::value> Get(web::uri uri);

   /**
    * @brief Executes a HTTP PUT request with given uri and the content given as json
    *
    * @param uri defines the URI resulting the target of the HTTP request
    * @param content the content as json value which should be the body of the request and thus the content of the
    * created resources
    * @return task to wait for with resulting json object
    */
   pplx::task<web::json::value> PUT(web::uri uri, web::json::value content);

   /**
    * @brief Executes a HTTP POST request with given uri and the content given as json
    *
    * @param uri defines the URI resulting the target of the HTTP request
    * @param content the content as json value which should be the body of the request and thus the content of the created resource
    * @return task to wait for with resulting json object
    */
   pplx::task<web::json::value> POST(web::uri uri, web::json::value content);

  };
} // namespace mitk
#endif // !mitkRESTClientMicroService_h
