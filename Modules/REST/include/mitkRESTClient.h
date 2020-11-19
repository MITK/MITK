/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRESTClient_h
#define mitkRESTClient_h

#include <MitkRESTExports.h>
#include <cpprest/http_client.h>

namespace mitk
{
  class MITKREST_EXPORT RESTClient
  {
  public:
    using http_request = web::http::http_request;
    RESTClient();
    ~RESTClient();

    /**
     * @brief Executes a HTTP GET request with the given uri and returns a task waiting for a json object
     *
     * @throw mitk::Exception if request went wrong
     * @param uri the URI resulting the target of the HTTP request
     * @param headers the additional headers to be set to the HTTP request
     * @return task to wait for with resulting json object
     */
    pplx::task<web::json::value> Get(const web::uri &uri, const std::map<utility::string_t, utility::string_t> headers);

    /**
     * @brief Executes a HTTP GET request with the given uri and and stores the byte stream in a file given by the
     * filePath
     *
     * @throw mitk::Exception if request went wrong
     * @param uri the URI resulting the target of the HTTP request
     * @param filePath
     * @param headers the additional headers to be set to the HTTP request
     * @return task to wait for returning an empty json object
     */
    pplx::task<web::json::value> Get(const web::uri &uri,
                                     const utility::string_t &filePath,
                                     const std::map<utility::string_t, utility::string_t> headers);

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
     * @param headers the additional headers to be set to the HTTP request
     * @return task to wait for with resulting json object
     */
    pplx::task<web::json::value> Post(const web::uri &uri,
                                      const web::json::value *content,
                                      const std::map<utility::string_t, utility::string_t> headers);

    /**
     * @brief Executes a HTTP POST request with given uri and the content given as json
     *
     * @throw mitk::Exception if request went wrong
     * @param uri defines the URI resulting the target of the HTTP request
     * @param content the content as json value which should be the body of the request and thus the content of the
     * created resource
     * @param headers the additional headers to be set to the HTTP request
     * @return task to wait for with resulting json object
     */
    pplx::task<web::json::value> Post(const web::uri &uri,
                                      const std::vector<unsigned char> *content,
                                      const std::map<utility::string_t, utility::string_t> headers);

  private:
    /**
     * @brief Use this to create and init a new request with the given headers. If needed, set the body on the resulting
     * request object to avoid an automatic change of the content type header when setting the body first.
     */
    http_request InitRequest(const std::map<utility::string_t, utility::string_t> headers);

	void CheckResponseContentType(web::http::http_response &response);

    pplx::task<web::json::value> ExecutePost(const web::uri &uri, http_request request);
    web::http::client::http_client_config m_ClientConfig;
  };
} // namespace mitk

#endif
