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
  CPPUNIT_TEST_SUITE_END();

public:
  mitk::IRESTManager *m_Service;
  web::json::value m_Data;

  web::json::value Notify(const web::uri &, const web::json::value &) override
  {
    return m_Data;
  }

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
   * members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override
  {
    m_Data = web::json::value();
    m_Data[_XPLATSTR("userId")] = web::json::value(1);
    m_Data[_XPLATSTR("id")] = web::json::value(1);
    m_Data[_XPLATSTR("title")] = web::json::value(U("this is a title"));
    m_Data[_XPLATSTR("body")] = web::json::value(U("this is a body"));

    auto serviceRef = us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();

    if (serviceRef)
      m_Service = us::GetModuleContext()->GetService(serviceRef);

    if (!m_Service)
      CPPUNIT_FAIL("Getting Service in setUp() failed");
  }

  void tearDown() override
  {
    m_Service->HandleDeleteObserver(this);
  }

  void OpenListener_Succeed()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }

  void TwoListenerSameHostSamePort_OnlyOneOpened()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/serverexample"), this);

    CPPUNIT_ASSERT_MESSAGE("Open two listener with a different path,same host, same port, observer map size is two",
                           2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener with a different path,same host, same port, server map size is one",
                           1 == m_Service->GetServerMap().size());
  }

  void CloseListener_Succeed()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());
  }

  void OpenMultipleListenerCloseOne_Succeed()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8090/serverexample"), this);

    CPPUNIT_ASSERT_MESSAGE("Open two listener, observer map size is two", 2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener, server map size is two", 2 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this, _XPLATSTR("http://localhost:8080/servertest"));

    CPPUNIT_ASSERT_MESSAGE("Closed one of two listeners, observer map is size is one",
                           1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed one of two listener, server map size is one",
                           1 == m_Service->GetServerMap().size());
  }

  void OpenMultipleListenerCloseAll_Succeed()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8090/serverexample"), this);

    CPPUNIT_ASSERT_MESSAGE("Open two listener, observer map size is two", 2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener, server map size is two", 2 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed all listeners, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed all listeners, server map is empty", 0 == m_Service->GetServerMap().size());
  }

  void OpenListenerGetRequestSamePath_ReturnExpectedJSON()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    web::json::value result;

    m_Service->SendRequest(_XPLATSTR("http://localhost:8080/servertest"))
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

    m_Service->SendRequest(_XPLATSTR("http://localhost:8080/servertest"))
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }

  void CloseListener_NoRequestPossible()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());

    CPPUNIT_ASSERT_THROW(RequestToClosedListener(), mitk::Exception);
  }

  void RequestToDifferentPathNotFound()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    web::json::value result;

    m_Service->SendRequest(_XPLATSTR("http://localhost:8080/serverexample"))
      .then([&](pplx::task<web::json::value> resultTask) { result = resultTask.get(); })
      .wait();
  }

  void OpenListenerGetRequestDifferentPath_ReturnNotFound()
  {
    CPPUNIT_ASSERT_THROW(RequestToDifferentPathNotFound(), mitk::Exception);
  }

  void OpenListenerCloseAndReopen_Succeed()
  {
    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());

    m_Service->HandleDeleteObserver(this);

    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());

    m_Service->ReceiveRequest(_XPLATSTR("http://localhost:8080/servertest"), this);

    CPPUNIT_ASSERT_MESSAGE("Reopened listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Reopened listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTServer)
