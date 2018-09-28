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

mitk::RESTClient::RESTClient(utility::string_t url) : m_Client(url) {
}

mitk::RESTClient::~RESTClient() {}

pplx::task<void> mitk::RESTClient::Get(utility::string_t filePath, utility::string_t uri)
{
  MITK_DEBUG << "Calling GET with " << utility::conversions::to_utf8string(uri) << " on client "
            << utility::conversions::to_utf8string(m_Client.base_uri().to_string()) << " save into "
            << utility::conversions::to_utf8string(filePath);

  auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();

  return concurrency::streams::file_buffer<uint8_t>::open(filePath, std::ios::out).then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<MitkResponse>
  {
    *fileBuffer = outFile;

    return m_Client.request(MitkRESTMethods::GET, uri);
  })
    // Write the response body into the file buffer.
    .then([=](MitkResponse response) -> pplx::task<size_t>
  {
      MITK_DEBUG << "Status code: " << response.status_code();

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
  MITK_INFO << "Calling POST with " << utility::conversions::to_utf8string(uri) << " on client "
            << utility::conversions::to_utf8string(m_Client.base_uri().to_string());

  MitkRequest postRequest(MitkRESTMethods::POST);
  postRequest.set_request_uri(uri);
  postRequest.headers().add(U("Content-Type"), contentType);
  postRequest.set_body(fileStream);

  MITK_INFO << fileStream.is_open();
  MITK_INFO << fileStream.is_valid();
  MITK_INFO << fileStream.streambuf().size();
  MITK_INFO << fileStream.streambuf().can_read();

  MITK_INFO << "Request: " << utility::conversions::to_utf8string(postRequest.to_string());
  
  return m_Client.request(postRequest).then([fileStream](MitkResponse response)
  {
    fileStream.close();
    MITK_INFO << "Response: " << utility::conversions::to_utf8string(response.to_string());
  });
}

pplx::task<void> mitk::RESTClient::WadoRS(utility::string_t filePath, std::string studyUID, std::string seriesUID, std::string instanceUID)
{
  MitkUriBuilder builder(U("wado"));
  builder.append_query(U("requestType"), U("WADO"));
  builder.append_query(U("studyUID"), utility::conversions::to_string_t(studyUID));
  builder.append_query(U("seriesUID"), utility::conversions::to_string_t(seriesUID));
  builder.append_query(U("objectUID"), utility::conversions::to_string_t(instanceUID));
  builder.append_query(U("contentType"), U("application/dicom"));
  return Get(filePath, builder.to_string());
}

pplx::task<std::string> mitk::RESTClient::WadoRS(const utility::string_t folderPath, std::string studyUID, std::string seriesUID)
{
  // this is actually a quido-rs request, should be packed into a seperate method.. at some time.. 
   //but there are many possible requests to support: http://dicom.nema.org/medical/dicom/current/output/chtml/part18/sect_6.7.html
  MitkUriBuilder builder(U("rs/instances"));
  builder.append_query(U("StudyInstanceUID"), utility::conversions::to_string_t(studyUID));
  builder.append_query(U("SeriesInstanceUID"), utility::conversions::to_string_t(seriesUID));

  MITK_INFO << utility::conversions::to_utf8string(builder.to_string());
  MitkRequest getSeries(MitkRESTMethods::GET);
  getSeries.set_request_uri(builder.to_string());
  getSeries.headers().add(U("Accept"), U("application/json"));

  return m_Client.request(getSeries).then([=](MitkResponse response) -> pplx::task<std::string>
  {
    MITK_INFO << "search for instances in series with uid " << seriesUID
      << " status: " << response.status_code();

    auto jsonListResult = response.extract_json().get();
    auto resultArray = jsonListResult.as_array();

    auto firstFileName = std::string();

    std::vector<pplx::task<void>> tasks;

    for (unsigned short i = 0; i < resultArray.size(); i++)
    {
      try
      {
        auto firstResult = resultArray[i];
        auto sopInstanceUIDKey = firstResult.at(U("00080018"));
        auto sopInstanceObject = sopInstanceUIDKey.as_object();
        auto valueKey = sopInstanceObject.at(U("Value"));
        auto valueArray = valueKey.as_array();
        auto sopInstanceUID = valueArray[0].as_string();

        auto fileName = utility::string_t(sopInstanceUID).append(U(".dcm"));

        // save first file name as result to load series
        if (i == 0)
        {
          firstFileName = utility::conversions::to_utf8string(fileName);
        }

        auto filePath = utility::string_t(folderPath).append(fileName);
        auto task = WadoRS(filePath, studyUID, seriesUID, utility::conversions::to_utf8string(sopInstanceUID));
        tasks.push_back(task);
      }
      catch (const web::json::json_exception& e)
      {
        MITK_ERROR << e.what();
      }
    }

    auto joinTask = pplx::when_all(begin(tasks), end(tasks));
    return joinTask.then([=](void)
    {
      return utility::conversions::to_utf8string(folderPath).append(firstFileName);
    });
  });
}

pplx::task<void> mitk::RESTClient::StowRS(utility::string_t filePath, std::string studyUID)
{
  // TODO: add data
  MitkUriBuilder builder(U("rs/studies"));
  builder.append_path(utility::conversions::to_string_t(studyUID));

  return concurrency::streams::file_stream<unsigned char>::open_istream(filePath).then([=](concurrency::streams::basic_istream<unsigned char> fileStream) {
    return Post(builder.to_string(), U("multipart/related; type='application/dicom';"), fileStream);
  });
}