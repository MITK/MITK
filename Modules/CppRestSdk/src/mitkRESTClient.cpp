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

#include <mitkCommon.h>

mitk::RESTClient::RESTClient(utility::string_t url) : m_Client(url) {}

mitk::RESTClient::~RESTClient() {}

void mitk::RESTClient::executeGETRequest(utility::string_t uri)
{
  MITK_INFO << "Calling GET with " << utility::conversions::to_utf8string(uri) << " on client " << utility::conversions::to_utf8string(m_Client.base_uri().to_string());

  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();
  const utility::string_t outputFileName = U("downloadWADO.dcm");

  concurrency::streams::file_buffer<uint8_t>::open(outputFileName, std::ios::out).then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<MitkResponse>
  {
    *fileBuffer = outFile;

    return m_Client.request(MitkRESTMethods::GET, uri);
  })
    // Write the response body into the file buffer.
    .then([=](MitkResponse response) -> pplx::task<size_t>
  {
    printf("Response status code %u returned.\n", response.status_code());

    return response.body().read_to_end(*fileBuffer);
  })
    // Close the file buffer.
    .then([=](size_t)
  {
    return fileBuffer->close();
  })
    // Wait for the entire response body to be written into the file.
    .wait();

  //m_Client.request(MitkRESTMethods::GET, uri).then([=](MitkResponse response)
  //{

  //  if (response.status_code() == MitkRestStatusCodes::OK) {
  //    auto fileStream = std::make_shared<concurrency::streams::ostream>();
  //    MITK_INFO << "Received response status code: " << response.status_code();
  //    response.body().read_to_end(fileStream->streambuf());

  //    return;
  //  }

  //});
  return;
}

void mitk::RESTClient::executeWADOGET(std::string studyUID, std::string seriesUID, std::string instanceUID)
{
  MitkUriBuilder builder(U("wado"));
  builder.append_query(U("requestType"), U("WADO"));
  builder.append_query(U("studyUID"), utility::conversions::to_string_t(studyUID));
  builder.append_query(U("seriesUID"), utility::conversions::to_string_t(seriesUID));
  builder.append_query(U("objectUID"), utility::conversions::to_string_t(instanceUID));
  builder.append_query(U("contentType"), U("application/dicom"));
  executeGETRequest(builder.to_string());
}
