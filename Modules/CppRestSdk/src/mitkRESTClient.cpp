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

void mitk::RESTClient::executeGETRequest(utility::string_t filePath, utility::string_t uri)
{
  MITK_INFO << "Calling GET with " << utility::conversions::to_utf8string(uri) << " on client "
            << utility::conversions::to_utf8string(m_Client.base_uri().to_string()) << " save into "
            << utility::conversions::to_utf8string(filePath);

  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();

  concurrency::streams::file_buffer<uint8_t>::open(filePath, std::ios::out).then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<MitkResponse>
  {
    *fileBuffer = outFile;

    return m_Client.request(MitkRESTMethods::GET, uri);
  })
    // Write the response body into the file buffer.
    .then([=](MitkResponse response) -> pplx::task<size_t>
  {
      MITK_INFO << "Status code: " << response.status_code();

    return response.body().read_to_end(*fileBuffer);
  })
    // Close the file buffer.
    .then([=](size_t)
  {
    return fileBuffer->close();
  })
    // Wait for the entire response body to be written into the file.
    .wait();

  return;
}

void mitk::RESTClient::executeWADOGET(utility::string_t filePath, std::string studyUID, std::string seriesUID, std::string instanceUID)
{
  MitkUriBuilder builder(U("wado"));
  builder.append_query(U("requestType"), U("WADO"));
  builder.append_query(U("studyUID"), utility::conversions::to_string_t(studyUID));
  builder.append_query(U("seriesUID"), utility::conversions::to_string_t(seriesUID));
  builder.append_query(U("objectUID"), utility::conversions::to_string_t(instanceUID));
  builder.append_query(U("contentType"), U("application/dicom"));
  executeGETRequest(filePath, builder.to_string());
}

void mitk::RESTClient::executeWADOGET(const utility::string_t folderPath, std::string studyUID, std::string seriesUID)
{
  MitkUriBuilder builder(U("instances"));
  builder.append_query(U("studyUID"), utility::conversions::to_string_t(studyUID));
  builder.append_query(U("seriesUID"), utility::conversions::to_string_t(seriesUID));

  m_Client.request(MitkRESTMethods::GET, builder.to_string()).then([=](MitkResponse response)
  {
    MITK_INFO << "search for instances in series with uid " << seriesUID
              << " status: " << response.status_code();

    auto jsonListResult = response.extract_json().get();
    auto resultArray = jsonListResult.as_array();

    for (unsigned short i = 0; i < resultArray.size(); i++)
    {
      auto firstResult = resultArray[i];
      auto sopInstanceUIDKey = firstResult.at(U("00080018"));
      auto sopInstanceObject = sopInstanceUIDKey.as_object();
      auto valueKey = sopInstanceObject.at(U("Value"));
      auto sopInstanceUID = valueKey.as_string();

      auto filePath = utility::string_t(folderPath)
                        .append(sopInstanceUID)
                        .append(U(".dcm"));
      executeWADOGET(filePath, studyUID, seriesUID, utility::conversions::to_utf8string(sopInstanceUID));
    }
  });

}
