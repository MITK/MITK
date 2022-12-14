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
  MITK_TEST(GetRequestValidURI_ReturnsExpectedJSON);
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
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTClientHttpLib)
