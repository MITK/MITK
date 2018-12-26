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

#ifndef MITKRESTCLIENT_H
#define MITKRESTCLIENT_H

#include "cpprest/asyncrt_utils.h"
#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include "cpprest/uri.h"

#include "MitkCppRestSdkExports.h"

typedef web::http::client::http_client MitkClient;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::uri_builder MitkUriBuilder;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTClient
  {
  public:
    ;

    /**
	 * @brief Creates a new client object containing the given base url which is extended by the uri of the member methods
	 * 
	 * @param url represents the base URL which is extended by the member methods and used as target for HTTP requests
	 */
    RESTClient(utility::string_t url);
    virtual ~RESTClient();

    /**
     * @brief Executes a HTTP POST request with given uri and the file given by the filePath
     *
     * @param uri defines the URI which extends the base URL of this object resulting the target of the HTTP request
     * @param filePath the path to the file which shall be included in the message body as multipart
     * @return task to wait for
     */
    pplx::task<void> Post(utility::string_t uri, utility::string_t filePath);

    /**
     * @brief Executes a HTTP GET request with the given uri and stores the byte stream in a file given by the filePath
     *
     * @param filePath the file path at which the received byte stream is stored
     * @param uri the URI which extends the base URL of this object resulting the target of the HTTP request
     * @return task to wait for
     */
    pplx::task<void> Get(const utility::string_t filePath, utility::string_t uri);

    /**
     * @brief Executes a HTTP GET request with the given uri and returns a json object
     *
     * @param uri the URI which extends the base URL of this object resulting the target of the HTTP request
     * @return task with to wait for with resulting json object
     */
    pplx::task<web::json::value> Get(utility::string_t uri);

    /**
     * @brief Executes a HTTP PUT request to the given uri containing the given content json object
	 * 
	 * @param uri the URI which extends the base URL of this object resulting the target of the HTTP request
	 * @param content the json body for the PUT request
     */
    pplx::task<web::json::value> PUT(utility::string_t uri, web::json::value content);

  protected:
    MitkClient *m_Client;
  };
};

#endif // MITKRESTCLIENT_H
