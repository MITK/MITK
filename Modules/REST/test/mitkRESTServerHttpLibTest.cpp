/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <future>
#include <httplib.h>
#include <map>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <nlohmann/json.hpp>
#include <thread>

class mitkRESTServerHttpLibTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRESTServerHttpLibTestSuite);
  MITK_TEST(OpenListener_Succeed);
  MITK_TEST(OpenListenerGetRequestSamePath_ReturnExpectedJSON);
  CPPUNIT_TEST_SUITE_END();
  std::vector<std::future<void>> m_ft;

public:
  void OpenListener_Succeed()
  {
    httplib::Server svr;
    svr.Get("/get",
            [](const httplib::Request &, httplib::Response &res)
            { res.set_content("Hello World from MITK!", "text/plain"); });
    std::future<void> ft = std::async(std::launch::async, [&]() { svr.listen("localhost", 8080); });
    while (!svr.is_running())
      ;
    CPPUNIT_ASSERT_MESSAGE("Server is running", svr.is_running());
    svr.stop();
  }

  void OpenListenerGetRequestSamePath_ReturnExpectedJSON()
  {
    httplib::Server svr;
    std::map<size_t, std::string> msgdb;
    msgdb[0] = "hello_MITK";

    svr.Get(R"(/msg/(\d+))",
            [&](const httplib::Request &req, httplib::Response &res)
            {
              auto n = req.matches[1];
              nlohmann::json jRes;
              try
              {
                jRes["id"] = std::stoi(n);
                jRes["msg"] = msgdb[std::stoi(n)];

                res.set_content(jRes.dump(), "application/json");
              }
              catch (const std::exception &)
              {
                res.set_content("Cannot find the requested message.", "text/plain");
              }
            });
    std::future<void> ft = std::async(std::launch::async, [&]() { svr.listen("localhost", 8080); });
    while (!svr.is_running())
      ;

    httplib::Client cli("http://localhost:8080");
    try
    {
      if (auto response = cli.Get("/msg/0"))
      {
        if (response->status == 200)
        {
          auto js = nlohmann::json::parse(response->body);
          std::string msg = js["msg"];
          CPPUNIT_ASSERT_MESSAGE("Result is the expected JSON value", msg == msgdb[0]);
        }
        else
        {
          CPPUNIT_ASSERT_MESSAGE("A connection cannot be established", false);
        }
      }
      else
      {
        CPPUNIT_ASSERT_MESSAGE("A connection cannot be established", false);
      }
    }
    catch (const std::exception &e)
    {
      svr.stop();
      CPPUNIT_ASSERT_MESSAGE(e.what(), false);
    }
    svr.stop();
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTServerHttpLib)
