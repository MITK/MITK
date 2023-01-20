/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <httplib.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <nlohmann/json.hpp>

class mitkRESTClientHttpLibTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRESTClientHttpLibTestSuite);
  // MITK_TEST(GetRequestValidURI_ReturnsExpectedJSON);
  MITK_TEST(PostMONAI);
  CPPUNIT_TEST_SUITE_END();

public:
  void GetRequestValidURI_ReturnsExpectedJSON()
  {
    httplib::Client cli("http://eu.httpbin.org");
    auto response = cli.Get("/get");
    if (response->status == 200)
    {
      auto js = nlohmann::json::parse(response->body);
      std::string userAgentWithVersion = js["headers"]["User-Agent"];
      std::string userAgent = userAgentWithVersion.substr(0, userAgentWithVersion.find("/"));
      CPPUNIT_ASSERT_MESSAGE("Result is the expected JSON value", userAgent == "cpp-httplib");
    }
  }

  void PostMONAI()
  {
    std::string filePath = "C://data//dataset//Task02_Heart//imagesTr//la_016.nii.gz";
    std::ifstream input(filePath, std::ios::binary);
    if (!input)
    {
      MITK_WARN << "could not read file to POST";
    }
    std::stringstream buffer_lf_img;
    buffer_lf_img << input.rdbuf();
    input.close();

    httplib::Client cli("localhost", 8000);

    httplib::MultipartFormDataItems items = {
      {"file", buffer_lf_img.str(), "spleen_58.nii.gz", "application/octet-stream"}};

    // httplib::MultipartFormDataMap itemMap = {{"file", {"file", buffer_lf_img.str(), "spleen_58.nii.gz",
    // "application/octet-stream"}}};

    auto response = cli.Post("/infer/deepedit_seg", items);
    if (response->status == 200)
    {
      MITK_INFO << response->body;
      auto h = response->headers;
      std::string contentType = h.find("content-type")->second;
      std::string delimiter = "boundary=";
      std::string boundaryString =
        contentType.substr(contentType.find(delimiter) + delimiter.length(), std::string::npos);
      boundaryString.insert(0, "--");
      MITK_INFO << "boundary hash: " << boundaryString;

      std::string resBody = response->body;
      std::vector<std::string> multiPartResponse = getStringInBoundary(resBody, boundaryString);

      std::string metaData = multiPartResponse[0];
      std::string contentTypeJson = "Content-Type: application/json";
      size_t ctPos = metaData.find(contentTypeJson) + contentTypeJson.length();
      std::string metaDataPart = metaData.substr(ctPos);
      MITK_INFO << metaDataPart;
      auto jsonObj = nlohmann::json::parse(metaDataPart);
      if (jsonObj.is_discarded() || !jsonObj.is_object())
      {
        MITK_ERROR << "Could not parse \"" << /* jsonPath.toStdString() << */ "\" as JSON object!";
        return;
      }

      std::string imagePart = multiPartResponse[1];
      std::string contentTypeStream = "Content-Type: application/octet-stream";
      ctPos = imagePart.find(contentTypeStream) + contentTypeStream.length();
      std::string imageDataPart = imagePart.substr(ctPos);
      MITK_INFO << imageDataPart;
      

      int size1 = imageDataPart.size();
      std::ofstream output("C://Users//a178n//Desktop//output.nii.gz", std::ios::out | std::ios::binary);
      output.write(reinterpret_cast<char *>(&size1), sizeof(int));
      output.write(imageDataPart.c_str(), size1);
      output.flush();
      output.close();
      
    }
  }

  std::vector<std::string> getStringInBoundary(const std::string &body, std::string boundary)
  {
    std::vector<std::string> retVal;
    std::string master = body;
    size_t found = master.find(boundary);
    size_t beginPos = 0;
    while (found != std::string::npos)
    {
      std::string part = master.substr(beginPos, found);
      if (!part.empty())
      {
        retVal.push_back(part);
      }
      master.erase(beginPos, found + boundary.length());
      found = master.find(boundary);
    }
    return retVal;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTClientHttpLib)
