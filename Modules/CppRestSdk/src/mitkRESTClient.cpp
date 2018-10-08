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
#include "mitkRestUtil.h"

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
      MITK_INFO << "Response: " << mitk::RESTUtil::convertToUtf8(response.to_string());
    });
  });
}

pplx::task<void> mitk::RESTClient::Post(utility::string_t uri, utility::string_t contentType, utility::string_t filePath)
{
  // this is the working stow-rs request which supports just one dicom file packed into a multipart message
  std::basic_ifstream<unsigned char> input(filePath, std::ios::binary);

  std::vector<unsigned char> result;
  std::vector<unsigned char> buffer((std::istreambuf_iterator<unsigned char>(input)),(std::istreambuf_iterator<unsigned char>()));

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
    MITK_INFO << "Response: " << mitk::RESTUtil::convertToUtf8(response.to_string());
  });

}

pplx::task<web::json::value> mitk::RESTClient::QuidoRSInstances(std::map<std::string, std::string> params)
{
  MitkUriBuilder queryBuilder(U("rs/instances"));

  for (auto const& element : params)
  {
    queryBuilder.append_query(mitk::RESTUtil::convertToTString(element.first), mitk::RESTUtil::convertToTString(element.second));
  }

  MITK_INFO << utility::conversions::to_utf8string(queryBuilder.to_string());
  MitkRequest instances(MitkRESTMethods::GET);
  instances.set_request_uri(queryBuilder.to_string());
  instances.headers().add(U("Accept"), U("application/json"));

  return m_Client->request(instances).then([=](MitkResponse response)
  {
    MITK_INFO << " status: " << response.status_code();

    return response.extract_json().get();
  });
}

pplx::task<void> mitk::RESTClient::WadoRS(utility::string_t filePath, std::string studyUID, std::string seriesUID, std::string instanceUID)
{
  MitkUriBuilder builder(U("wado"));
  builder.append_query(U("requestType"), U("WADO"));
  builder.append_query(U("studyUID"), mitk::RESTUtil::convertToTString(studyUID));
  builder.append_query(U("seriesUID"), mitk::RESTUtil::convertToTString(seriesUID));
  builder.append_query(U("objectUID"), mitk::RESTUtil::convertToTString(instanceUID));
  builder.append_query(U("contentType"), U("application/dicom"));
  return Get(filePath, builder.to_string());
}

pplx::task<std::string> mitk::RESTClient::WadoRS(const utility::string_t folderPath, std::string studyUID, std::string seriesUID)
{
  typedef std::map<std::string, std::string> ParamMap;
  ParamMap seriesInstancesParams;

  seriesInstancesParams.insert(ParamMap::value_type({"StudyInstanceUID"}, studyUID));
  seriesInstancesParams.insert(ParamMap::value_type({"SeriesInstanceUID"}, seriesUID));

  return QuidoRSInstances(seriesInstancesParams).then([=](web::json::value jsonResult) -> pplx::task<std::string>
  {
    auto jsonListResult = jsonResult;
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
        auto task = WadoRS(filePath, studyUID, seriesUID, mitk::RESTUtil::convertToUtf8(sopInstanceUID));
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
  builder.append_path(mitk::RESTUtil::convertToTString(studyUID));

  return Post(builder.to_string(), U("multipart/related; type='application/dicom'; boundary='boundary'"), filePath);
}