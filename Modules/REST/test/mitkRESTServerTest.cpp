/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifdef _WIN32
#include <Windows.h>
#endif

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkIRESTManager.h>
#include <mitkIRESTObserver.h>
#include <mitkRESTServer.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include <vtkDebugLeaks.h>

class mitkRESTServerTestSuite : public mitk::TestFixture, mitk::IRESTObserver
{
  CPPUNIT_TEST_SUITE(mitkRESTServerTestSuite);
  MITK_TEST(OpenListener_Succeed);
  MITK_TEST(TwoListenerSameHostSamePort_OnlyOneOpened);
  MITK_TEST(CloseListener_Succeed);
  MITK_TEST(OpenMultipleListenerCloseOne_Succeed);
  MITK_TEST(OpenMultipleListenerCloseAll_Succeed);
  // MITK_TEST(OpenListenerGetRequestSamePath_ReturnExpectedJSON); GET requests do not support content yet?
  MITK_TEST(CloseListener_NoRequestPossible);
  MITK_TEST(OpenListenerGetRequestDifferentPath_ReturnNotFound);
  MITK_TEST(OpenListenerCloseAndReopen_Succeed);
  MITK_TEST(HandleHeader_Succeed);
  CPPUNIT_TEST_SUITE_END();

public:
  mitk::IRESTManager *m_Service;
  web::json::value m_Data;

  web::http::http_response Notify(const web::uri &,
                                  const web::json::value &,
                                  const web::http::method &,
                                  const mitk::RESTUtil::ParamMap &headers) override
  {
    auto response = web::http::http_response();
    response.set_body(m_Data);
    mitk::RESTUtil::ParamMap::const_iterator contentTypePos = headers.find(U("Content-Type"));
    if (contentTypePos != headers.end() && contentTypePos->second == U("awesome/type"))
    {
      m_Data[U("result")] = web::json::value(U("awesome/type"));
    }

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

    auto serviceRef = us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();

    if (serviceRef)
      m_Service = us::GetModuleContext()->GetService(serviceRef);

    if (!m_Service)
      CPPUNIT_FAIL("Getting Service in setUp() failed");
  }

  void tearDown() override { m_Service->HandleDeleteObserver(this); }

  void OpenListener_Succeed()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }

  void TwoListenerSameHostSamePort_OnlyOneOpened()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);
    m_Service->ReceiveRequest(U("http://localhost:8080/serverexample"), this);

    CPPUNIT_ASSERT_MESSAGE("Open two listener with a different path,same host, same port, observer map size is two",
                           2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener with a different path,same host, same port, server map size is one",
                           1 == m_Service->GetServerMap().size());
  }

  void CloseListener_Succeed()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());
  }

  void OpenMultipleListenerCloseOne_Succeed()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);
    m_Service->ReceiveRequest(U("http://localhost:8090/serverexample"), this);

    CPPUNIT_ASSERT_MESSAGE("Open two listener, observer map size is two", 2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener, server map size is two", 2 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this, U("http://localhost:8080/servertest"));

    CPPUNIT_ASSERT_MESSAGE("Closed one of two listeners, observer map is size is one",
                           1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed one of two listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }

  void OpenMultipleListenerCloseAll_Succeed()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);
    m_Service->ReceiveRequest(U("http://localhost:8090/serverexample"), this);

    CPPUNIT_ASSERT_MESSAGE("Open two listener, observer map size is two", 2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener, server map size is two", 2 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed all listeners, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed all listeners, server map is empty", 0 == m_Service->GetServerMap().size());
  }

  void OpenListenerGetRequestSamePath_ReturnExpectedJSON()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    web::json::value result;
    auto body = web::json::value();
    m_Service->SendRequest(U("http://localhost:8080/servertest"))
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

    CPPUNIT_ASSERT_MESSAGE("Opened listener and send request to same uri, returned expected JSON", result == m_Data);
  }

  void RequestToClosedListener()
  {
    web::json::value result;

    m_Service->SendRequest(U("http://localhost:8080/servertest"))
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }

  void CloseListener_NoRequestPossible()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());

    CPPUNIT_ASSERT_THROW(RequestToClosedListener(), mitk::Exception);
  }

  void RequestToDifferentPathNotFound()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    web::json::value result;

    m_Service->SendRequest(U("http://localhost:8080/serverexample"))
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }

  void OpenListenerGetRequestDifferentPath_ReturnNotFound()
  {
    CPPUNIT_ASSERT_THROW(RequestToDifferentPathNotFound(), mitk::Exception);
  }

  void OpenListenerCloseAndReopen_Succeed()
  {
    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());

    m_Service->ReceiveRequest(U("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Reopened listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Reopened listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }

  void HandleHeader_Succeed()
  {
    mitk::RESTUtil::ParamMap headers;
    headers.insert(mitk::RESTUtil::ParamMap::value_type(U("Content-Type"), U("awesome/type")));

    m_Service->SendRequest(U("http://localhost:8080/clienttest")).then([&](pplx::task<web::json::value> resultTask) {
      // Do something when a single task is done
      try
      {
        auto result = resultTask.get();
        CPPUNIT_ASSERT_MESSAGE("Sent Header is not successfull transfered to server",
                               result[U("result")].as_string() == U("awesome/type"));
      }
      catch (const mitk::Exception &exception)
      {
        MITK_ERROR << exception.what();
        return;
      }
    });
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTServer)
