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
#include <future>
#include <thread>
#include <map>


class mitkRESTServerHttpLibTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRESTServerHttpLibTestSuite);
  MITK_TEST(OpenListener_Succeed);
  MITK_TEST(OpenListenerGetRequestSamePath_ReturnExpectedJSON);
  CPPUNIT_TEST_SUITE_END();

public:
  void OpenListener_Succeed()
  {
    httplib::Server svr;
    auto serverlambda = [&]() { svr.listen("localhost", 8080); };
    svr.Get("/get",
            [](const httplib::Request &, httplib::Response &res)
            { res.set_content("Hello World from MITK!", "text/plain"); });
    std::future<void> ft = std::async(std::launch::async, serverlambda);
    while (!svr.is_running())
      ;
    CPPUNIT_ASSERT_MESSAGE("Server is running", svr.is_running());
    svr.stop();
  }

  void OpenListenerGetRequestSamePath_ReturnExpectedJSON()
  {
    httplib::Server svr;
    auto serverlambda = [&]() { svr.listen("localhost", 8080); };
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
    std::future<void> ft = std::async(std::launch::async, serverlambda);
    while (!svr.is_running())
      ;
    
    httplib::Client cli("http://localhost:8080");
    auto response = cli.Get("/msg/1");
    try
    {
      if (NULL == response)
      {
        CPPUNIT_ASSERT_MESSAGE("A connection duty cannot be established", false);
      }
      if (response->status == 200)
      {
        auto js = nlohmann::json::parse(response->body);
        std::string msg = js["msg"];
        CPPUNIT_ASSERT_MESSAGE("Result is the expected JSON value", msg == msgdb[0]);
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
