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

#include <mitkRESTClient.h>
#include <mitkRESTUtil.h>
#include <mitkExceptionMacro.h>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using http_client = web::http::client::http_client;
using http_request = web::http::http_request;
using http_response = web::http::http_response;
using methods = web::http::methods;
using status_codes = web::http::status_codes;
using file_buffer = concurrency::streams::file_buffer<uint8_t>;
using streambuf = concurrency::streams::streambuf<uint8_t>;

mitk::RESTClient::RESTClient()
{
}

mitk::RESTClient::~RESTClient()
{
}

pplx::task<web::json::value> mitk::RESTClient::Get(const web::uri &uri)
{
  auto client = new http_client(uri);
  http_request request;

  return client->request(request).then([=](pplx::task<web::http::http_response> responseTask) {
    try
    {
      auto response = responseTask.get();
      auto status = response.status_code();

      if (status_codes::OK != status)
        mitkThrow();

      auto requestContentType = response.headers().content_type();

      if (_XPLATSTR("application/json") != requestContentType)
        response.headers().set_content_type(_XPLATSTR("application/json"));

      return response.extract_json().get();
    }
    catch (...)
    {
      mitkThrow() << "Getting response went wrong";
    }
  });
}

pplx::task<web::json::value> mitk::RESTClient::Get(const web::uri &uri, const utility::string_t &filePath)
{
  auto client = new http_client(uri);
  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();
  http_request request;

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
    .then([=](size_t) {
      return fileBuffer->close();
    })
    // Return empty JSON object
    .then([=]() {
       return web::json::value();
    });
}

pplx::task<web::json::value> mitk::RESTClient::Put(const web::uri &uri, const web::json::value *content)
{
  auto client = new http_client(uri);
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

      if (_XPLATSTR("application/json") != requestContentType)
        response.headers().set_content_type(_XPLATSTR("application/json"));

      return response.extract_json().get();
    }
    catch (...)
    {
      mitkThrow() << "Getting response went wrong";
    }
  });
}

pplx::task<web::json::value> mitk::RESTClient::Post(const web::uri &uri, const web::json::value *content)
{
  auto client = new http_client(uri);
  http_request request(methods::POST);

  if (nullptr != content)
    request.set_body(*content);

  return client->request(request).then([=](pplx::task<http_response> responseTask) {
    try
    {
      auto response = responseTask.get();
      auto status = response.status_code();

      if (status_codes::Created != status)
        mitkThrow();

      // Parse content type to application/json if it isn't already. This is
      // important if the content type is e.g. application/dicom+json.
      auto requestContentType = response.headers().content_type();
      if (_XPLATSTR("application/json") != requestContentType)
        response.headers().set_content_type(_XPLATSTR("application/json"));

      return response.extract_json().get();
    }
    catch(...)
    {
      mitkThrow() << "Getting response went wrong";
    }
  });
}
