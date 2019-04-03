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

#ifndef mitkRESTClient_h
#define mitkRESTClient_h

#include <MitkRESTExports.h>
#include <cpprest/http_client.h>

namespace mitk
{
  class MITKREST_EXPORT RESTClient
  {
  public:
    RESTClient();
    ~RESTClient();

    /**
     * @brief Executes a HTTP GET request with the given uri and returns a task waiting for a json object
     *
     * @throw mitk::Exception if request went wrong
     * @param uri the URI resulting the target of the HTTP request
     * @return task to wait for with resulting json object
     */
    pplx::task<web::json::value> Get(const web::uri &uri);

    /**
     * @brief Executes a HTTP GET request with the given uri and and stores the byte stream in a file given by the
     * filePath
     *
     * @throw mitk::Exception if request went wrong
     * @param uri the URI resulting the target of the HTTP request
     * @return task to wait for returning an empty json object
     */
    pplx::task<web::json::value> Get(const web::uri &uri, const utility::string_t &filePath);

    /**
     * @brief Executes a HTTP PUT request with given uri and the content given as json
     *
     * @throw mitk::Exception if request went wrong
     * @param uri defines the URI resulting the target of the HTTP request
     * @param content the content as json value which should be the body of the request and thus the content of the
     * created resources
     * @return task to wait for with resulting json object
     */
    pplx::task<web::json::value> Put(const web::uri &uri, const web::json::value *content);

    /**
     * @brief Executes a HTTP POST request with given uri and the content given as json
     *
     * @throw mitk::Exception if request went wrong
     * @param uri defines the URI resulting the target of the HTTP request
     * @param content the content as json value which should be the body of the request and thus the content of the
     * created resource
     * @return task to wait for with resulting json object
     */
    pplx::task<web::json::value> Post(const web::uri &uri, const web::json::value *content);
  };
}

#endif
