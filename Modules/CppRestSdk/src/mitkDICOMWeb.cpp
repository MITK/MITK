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

#include "mitkDICOMWeb.h"
#include "mitkRESTUtil.h"
#include <mitkCommon.h>

mitk::DICOMWeb::DICOMWeb(utility::string_t url) : mitk::RESTClient(url)
{
}

mitk::DICOMWeb::~DICOMWeb() 
{
}

pplx::task<web::json::value> mitk::DICOMWeb::QuidoRSInstances(std::map<std::string, std::string> params)
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
    auto status = response.status_code();
    MITK_INFO << " status: " << status;

    if (status != web::http::status_codes::OK) {
      mitkThrow() << "QUIDO-RS ended up with response " << mitk::RESTUtil::convertToUtf8(response.to_string());
    }

    return response.extract_json().get();
  });
}

pplx::task<void> mitk::DICOMWeb::WadoRS(utility::string_t filePath, std::string studyUID, std::string seriesUID, std::string instanceUID)
{
  MitkUriBuilder builder(U("wado"));
  builder.append_query(U("requestType"), U("WADO"));
  builder.append_query(U("studyUID"), mitk::RESTUtil::convertToTString(studyUID));
  builder.append_query(U("seriesUID"), mitk::RESTUtil::convertToTString(seriesUID));
  builder.append_query(U("objectUID"), mitk::RESTUtil::convertToTString(instanceUID));
  builder.append_query(U("contentType"), U("application/dicom"));
  return Get(filePath, builder.to_string());
}

pplx::task<std::string> mitk::DICOMWeb::WadoRS(const utility::string_t folderPath, std::string studyUID, std::string seriesUID)
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

    auto returnTask = joinTask.then([=](void) -> std::string
    {
      auto folderPathUtf8 = utility::conversions::to_utf8string(folderPath);
      auto result = folderPathUtf8 + firstFileName;

      return result;
    });

    return returnTask;
  });
}

pplx::task<void> mitk::DICOMWeb::StowRS(utility::string_t filePath, std::string studyUID)
{
  // TODO: add data
  MitkUriBuilder builder(U("rs/studies"));
  builder.append_path(mitk::RESTUtil::convertToTString(studyUID));

  return Post(builder.to_string(), U("multipart/related; type='application/dicom'; boundary='boundary'"), filePath);
}