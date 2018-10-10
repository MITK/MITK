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

#include "mitkRESTClient.h"
#include "mitkRESTUtil.h"

#include <filesystem>
#include <mitkCommon.h>

mitk::RESTClient::RESTClient(utility::string_t url)
{
  m_Client = new MitkClient(url);
}

mitk::RESTClient::~RESTClient()
{
  delete m_Client;
}

pplx::task<void> mitk::RESTClient::Get(utility::string_t filePath, utility::string_t uri)
{
  MITK_DEBUG << "Calling GET with " << utility::conversions::to_utf8string(uri) << " on client "
    << mitk::RESTUtil::convertToUtf8(m_Client->base_uri().to_string()) << " save into "
    << mitk::RESTUtil::convertToUtf8(filePath);

  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();

  return concurrency::streams::file_buffer<uint8_t>::open(filePath, std::ios::out).then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<MitkResponse>
  {
    *fileBuffer = outFile;

    return m_Client->request(MitkRESTMethods::GET, uri);
  })
    // Write the response body into the file buffer.
    .then([=](MitkResponse response) -> pplx::task<size_t>
  {
    auto status = response.status_code();
    MITK_DEBUG << "Status code: " << status;

    if (status != web::http::status_codes::OK) {
      mitkThrow() << "GET ended up with response " << mitk::RESTUtil::convertToUtf8(response.to_string());
    }

    return response.body().read_to_end(*fileBuffer);
  })
    // Close the file buffer.
    .then([=](size_t)
  {
    return fileBuffer->close();
  });
}

pplx::task<void> mitk::RESTClient::Post(utility::string_t uri,
  utility::string_t contentType,
  concurrency::streams::basic_istream<unsigned char> fileStream)
{
  MITK_INFO << "Calling POST with " << mitk::RESTUtil::convertToUtf8(uri) << " on client "
    << mitk::RESTUtil::convertToUtf8(m_Client->base_uri().to_string());

  // currently not working, but stream approach may be useful for later.. don't use string streams for dcm files...
  concurrency::streams::container_buffer<std::string> inStringBuffer;
  return fileStream.read(inStringBuffer, fileStream.streambuf().size()).then([=](size_t bytesRead) -> pplx::task<void>
  {
    const std::string &text = inStringBuffer.collection();

    std::string body = "";
    body += "\r\n--boundary";
    body += "\r\nContentType: " + mitk::RESTUtil::convertToUtf8(L"application/dicom") + "\r\n\r\n";
    body += text;
    body += "\r\n--boundary--";

    auto utf8String = utility::conversions::to_utf8string(body);

    auto binaryVector = std::vector<unsigned char>(utf8String.begin(), utf8String.end());

    MitkRequest postRequest(MitkRESTMethods::POST);
    postRequest.set_request_uri(uri);
    postRequest.headers().add(U("Content-Type"), contentType);
    postRequest.set_body(binaryVector);

    MITK_INFO << "Request: " << mitk::RESTUtil::convertToUtf8(postRequest.to_string());

    return m_Client->request(postRequest).then([fileStream](MitkResponse response)
    {
      fileStream.close();
      auto status = response.status_code();
      if (status != web::http::status_codes::OK) {
        mitkThrow() << "POST ended up with response " << mitk::RESTUtil::convertToUtf8(response.to_string());
      }
    });
  });
}

pplx::task<void> mitk::RESTClient::Post(utility::string_t uri, utility::string_t contentType, utility::string_t filePath)
{
  // this is the working stow-rs request which supports just one dicom file packed into a multipart message
  std::basic_ifstream<unsigned char> input(filePath, std::ios::binary);

  std::vector<unsigned char> result;
  std::vector<unsigned char> buffer((std::istreambuf_iterator<unsigned char>(input)), (std::istreambuf_iterator<unsigned char>()));

  // reuse 'content-type' variable or struct to be more flexible, in future more than one file should also be supported..
  std::string head = "";
  head += "\r\n--boundary";
  head += "\r\nContent-Type: " + mitk::RESTUtil::convertToUtf8(L"application/dicom") + "\r\n\r\n";

  std::vector<unsigned char> bodyVector(head.begin(), head.end());

  std::string tail = "";
  tail += "\r\n--boundary--";

  result.insert(result.end(), bodyVector.begin(), bodyVector.end());
  result.insert(result.end(), buffer.begin(), buffer.end());
  result.insert(result.end(), tail.begin(), tail.end());

  MitkRequest postRequest(MitkRESTMethods::POST);
  postRequest.set_request_uri(uri);
  postRequest.headers().add(U("Content-Type"), "multipart/related; type=\"application/dicom\"; boundary=boundary");
  postRequest.set_body(result);

  MITK_INFO << "Request: " << mitk::RESTUtil::convertToUtf8(postRequest.to_string());
  return m_Client->request(postRequest).then([](MitkResponse response)
  {
    auto status = response.status_code();
    if (status != web::http::status_codes::OK) {
      mitkThrow() << "POST ended up with response " << mitk::RESTUtil::convertToUtf8(response.to_string());
    }
    MITK_INFO << "Response: " << mitk::RESTUtil::convertToUtf8(response.to_string());
  });

}