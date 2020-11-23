/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkIRESTManager.h>
#include <mitkIRESTObserver.h>
#include <mitkRESTClient.h>
#include <mitkRESTUtil.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include <vtkDebugLeaks.h>

#include <atomic>

class mitkRESTClientTestSuite : public mitk::TestFixture, mitk::IRESTObserver
{
  CPPUNIT_TEST_SUITE(mitkRESTClientTestSuite);
  // MITK_TEST(GetRequestValidURI_ReturnsExpectedJSON); GET requests do not support content yet?
  MITK_TEST(MultipleGetRequestValidURI_AllTasksFinish);
  // MITK_TEST(PutRequestValidURI_ReturnsExpectedJSON); Does not work reliably on dart clients
  // MITK_TEST(PostRequestValidURI_ReturnsExpectedJSON);              -- " --
  MITK_TEST(GetRequestInvalidURI_ThrowsException);
  MITK_TEST(PutRequestInvalidURI_ThrowsException);
  MITK_TEST(PostRequestInvalidURI_ThrowsException);
  CPPUNIT_TEST_SUITE_END();

public:
  mitk::IRESTManager *m_Service;
  web::json::value m_Data;

  web::http::http_response Notify(const web::uri &,
                                  const web::json::value &,
                                  const web::http::method &,
                                  const mitk::RESTUtil::ParamMap &) override
  {
    auto response = web::http::http_response();
    response.set_body(m_Data);
    response.set_status_code(web::http::status_codes::OK);

    return response;
  }

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
   * members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override
  {
    m_Data = web::json::value();
    m_Data[U("userId")] = web::json::value(1);
    m_Data[U("id")] = web::json::value(1);
    m_Data[U("title")] = web::json::value(U("this is a title"));
    m_Data[U("body")] = web::json::value(U("this is a body"));

    us::ServiceReference<mitk::IRESTManager> serviceRef =
      us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();
    if (serviceRef)
    {
      m_Service = us::GetModuleContext()->GetService(serviceRef);
    }

    if (!m_Service)
    {
      CPPUNIT_FAIL("Getting Service in setUp() failed");
    }

    m_Service->ReceiveRequest(U("http://localhost:8080/clienttest"), this);
  }

  void tearDown() override { m_Service->HandleDeleteObserver(this); }

  void GetRequestValidURI_ReturnsExpectedJSON()
  {
    web::json::value result;

    m_Service->SendRequest(U("http://localhost:8080/clienttest"))
      .then([&](pplx::task<web::json::value> resultTask) {
        try
        {
          result = resultTask.get();
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        }
      })
      .wait();

    CPPUNIT_ASSERT_MESSAGE("Result is the expected JSON value", result == m_Data);
  }

  void MultipleGetRequestValidURI_AllTasksFinish()
  {
    std::atomic<int> count {0};

    // Create multiple tasks e.g. as shown below
    std::vector<pplx::task<void>> tasks;
    for (int i = 0; i < 20; ++i)
    {
      pplx::task<void> singleTask = m_Service->SendRequest(U("http://localhost:8080/clienttest"))
                                      .then([&](pplx::task<web::json::value> resultTask) {
                                        // Do something when a single task is done
                                        try
                                        {
                                          resultTask.get();
                                          count += 1;
                                        }
                                        catch (const mitk::Exception &exception)
                                        {
                                          MITK_ERROR << exception.what();
                                          return;
                                        }
                                      });
      tasks.emplace_back(singleTask);
    }
    // Create a joinTask which includes all tasks you've created
    auto joinTask = pplx::when_all(begin(tasks), end(tasks));
    // Run asynchonously
    joinTask
      .then([&](pplx::task<void> resultTask) {
        // Do something when all tasks are finished
        try
        {
          resultTask.get();
          count += 1;
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        }
      })
      .wait();

    CPPUNIT_ASSERT_MESSAGE("Multiple Requests", 21 == count);
  }

  void PutRequestValidURI_ReturnsExpectedJSON()
  {
    // optional: link might get invalid or content is changed
    web::json::value result;

    m_Service
      ->SendJSONRequest(
        U("https://jsonplaceholder.typicode.com/posts/1"), mitk::IRESTManager::RequestType::Put)
      .then([&](pplx::task<web::json::value> resultTask) {
        try
        {
          result = resultTask.get();
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        }
      })
      .wait();

    CPPUNIT_ASSERT_MESSAGE(
      "Result is the expected JSON value, check if the link is still valid since this is an optional test",
      result == m_Data);
  }

  void PostRequestValidURI_ReturnsExpectedJSON()
  {
    // optional: link might get invalid or content is changed
    web::json::value result;
    web::json::value data;

    data[U("userId")] = m_Data[U("userId")];
    data[U("title")] = m_Data[U("title")];
    data[U("body")] = m_Data[U("body")];

    m_Service
      ->SendJSONRequest(U("https://jsonplaceholder.typicode.com/posts"), mitk::IRESTManager::RequestType::Post, &data)
      .then([&](pplx::task<web::json::value> resultTask) {
        try
        {
          result = resultTask.get();
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        }
      })
      .wait();

    data[U("id")] = web::json::value(101);
    CPPUNIT_ASSERT_MESSAGE(
      "Result is the expected JSON value, check if the link is still valid since this is an optional test",
      result == data);
  }

  void PostRequestHeaders_Success()
  {
    mitk::RESTUtil::ParamMap headers;
    headers.insert(mitk::RESTUtil::ParamMap::value_type(
      U("Content-Type"), U("multipart/related; type=\"application/dicom\"; boundary=boundary")));

    m_Service->SendRequest(U("http://localhost:8080/clienttest")).then([&](pplx::task<web::json::value> resultTask) {
      // Do something when a single task is done
      try
      {
        resultTask.get();
      }
      catch (const mitk::Exception &exception)
      {
        MITK_ERROR << exception.what();
        return;
      }
    });
  }

  void GetException()
  {
    // Method which makes a get request to an invalid uri
    web::json::value result;

    m_Service->SendRequest(U("http://localhost:1234/invalid"))
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }
  void GetRequestInvalidURI_ThrowsException() { CPPUNIT_ASSERT_THROW(GetException(), mitk::Exception); }

  void PutException()
  {
    // Method which makes a put request to an invalid uri
    web::json::value result;

    m_Service->SendJSONRequest(U("http://localhost:1234/invalid"), mitk::IRESTManager::RequestType::Put, &m_Data)
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }
  void PutRequestInvalidURI_ThrowsException() { CPPUNIT_ASSERT_THROW(PutException(), mitk::Exception); }

  void PostException()
  {
    // Method which makes a post request to an invalid uri
    web::json::value result;

    m_Service->SendJSONRequest(U("http://localhost:1234/invalid"), mitk::IRESTManager::RequestType::Post, &m_Data)
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }
  void PostRequestInvalidURI_ThrowsException() { CPPUNIT_ASSERT_THROW(PostException(), mitk::Exception); }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTClient)
