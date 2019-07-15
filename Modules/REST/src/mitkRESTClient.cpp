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

#include <mitkExceptionMacro.h>
#include <mitkRESTClient.h>
#include <mitkRESTUtil.h>

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>

using http_client = web::http::client::http_client;
using http_request = web::http::http_request;
using http_response = web::http::http_response;
using methods = web::http::methods;
using status_codes = web::http::status_codes;
using file_buffer = concurrency::streams::file_buffer<uint8_t>;
using streambuf = concurrency::streams::streambuf<uint8_t>;

mitk::RESTClient::RESTClient()
{
  m_ClientConfig.set_validate_certificates(false);
}

mitk::RESTClient::~RESTClient() {}

pplx::task<web::json::value> mitk::RESTClient::Get(const web::uri &uri,
                                                   const std::map<utility::string_t, utility::string_t> headers)
{
  auto client = new http_client(uri, m_ClientConfig);
  http_request request;

  for (auto param : headers)
  {
    request.headers().add(param.first, param.second);
  }

  return client->request(request).then([=](pplx::task<web::http::http_response> responseTask) {
    try
    {
      auto response = responseTask.get();
      auto status = response.status_code();

      if (status_codes::OK != status)
      {
        MITK_INFO << status;
        MITK_INFO << mitk::RESTUtil::convertToUtf8(response.to_string());
        mitkThrow();
	  }
	  
      auto requestContentType = response.headers().content_type();

      if (U("application/json") != requestContentType)
        response.headers().set_content_type(U("application/json"));

      return response.extract_json().get();
    }
    catch (std::exception &e)
    {
      MITK_INFO << e.what();
      mitkThrow() << "Getting response went wrong";
    }
  });
}

pplx::task<web::json::value> mitk::RESTClient::Get(const web::uri &uri,
                                                   const utility::string_t &filePath,
                                                   const std::map<utility::string_t, utility::string_t> headers)
{
  auto client = new http_client(uri, m_ClientConfig);
  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();
  http_request request;

  for (auto param : headers)
  {
    request.headers().add(param.first, param.second);
  }

  // Open file stream for the specified file path
  return file_buffer::open(filePath, std::ios::out)
    .then([=](streambuf outFile) -> pplx::task<http_response> {
      *fileBuffer = outFile;
      return client->request(methods::GET);
    })
    // Write the response body into the file buffer
    .then([=](http_response response) -> pplx::task<size_t> {
      auto status = response.status_code();

      if (status_codes::OK != status)
        mitkThrow() << "GET ended up with response " << RESTUtil::convertToUtf8(response.to_string());

      return response.body().read_to_end(*fileBuffer);
    })
    // Close the file buffer
    .then([=](size_t) { return fileBuffer->close(); })
    // Return empty JSON object
    .then([=]() { return web::json::value(); });
}

pplx::task<web::json::value> mitk::RESTClient::Put(const web::uri &uri, const web::json::value *content)
{
  auto client = new http_client(uri, m_ClientConfig);
  http_request request(methods::PUT);

  if (nullptr != content)
    request.set_body(*content);

  return client->request(request).then([=](pplx::task<http_response> responseTask) {
    try
    {
      auto response = responseTask.get();
      auto status = response.status_code();

      if (status_codes::OK != status)
        mitkThrow();

      // Parse content type to application/json if it isn't already. This is
      // important if the content type is e.g. application/dicom+json.
      auto requestContentType = response.headers().content_type();

      if (U("application/json") != requestContentType)
        response.headers().set_content_type(U("application/json"));

      return response.extract_json().get();
    }
    catch (std::exception &e)
    {
      MITK_INFO << e.what();
      mitkThrow() << "Getting response went wrong";
    }
  });
}

pplx::task<web::json::value> mitk::RESTClient::Post(const web::uri &uri,
                                                    const std::vector<unsigned char> *content,
                                                    const std::map<utility::string_t, utility::string_t> headers)
{
  auto request = InitRequest(headers);
  request.set_method(methods::POST);

  if (nullptr != content)
    request.set_body(*content);

  return ExecutePost(uri, request);
}

pplx::task<web::json::value> mitk::RESTClient::Post(const web::uri &uri,
                                                    const web::json::value *content,
                                                    const std::map<utility::string_t, utility::string_t> headers)
{
  auto request = InitRequest(headers);
  request.set_method(methods::POST);

  if (nullptr != content)
    request.set_body(*content);

  return ExecutePost(uri, request);
}

http_request mitk::RESTClient::InitRequest(const std::map<utility::string_t, utility::string_t> headers)
{
  http_request request;

  for (auto param : headers)
  {
    request.headers().add(param.first, param.second);
  }
  return request;
}

pplx::task<web::json::value> mitk::RESTClient::ExecutePost(const web::uri &uri, http_request request)
{
  auto client = new http_client(uri, m_ClientConfig);
  return client->request(request).then([=](pplx::task<http_response> responseTask) {
    try
    {
      auto response = responseTask.get();
      auto status = response.status_code();
      auto requestContentType = response.headers().content_type();
      MITK_INFO << status;

      if (status_codes::OK != status)
        mitkThrow();

      MITK_INFO << mitk::RESTUtil::convertToUtf8(requestContentType);
      if (U("application/json") != requestContentType)
      {
        auto json = web::json::value::object();
        json[U("test")] = web::json::value(17);
        response.set_body(json); // use random json object in response body to perform return value (Linux Fix)
        response.headers().set_content_type(U("application/json"));
      }

      return response.extract_json().get();
    }
    catch (std::exception &e)
    {
      MITK_INFO << e.what();
      mitkThrow() << "Getting response went wrong";
    }
  });
}
