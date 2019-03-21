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

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// MITK includes
#include "mitkRESTServer.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkRESTServerTestSuite : public mitk::TestFixture, mitk::IRESTObserver
{
  CPPUNIT_TEST_SUITE(mitkRESTServerTestSuite);

  MITK_TEST(OpenListener_Succeed);
  MITK_TEST(TwoListenerSameHostSamePort_OnlyOneOpened);
  MITK_TEST(CloseListener_Succeed);
  MITK_TEST(OpenMultipleListenerCloseOne_Succeed);
  MITK_TEST(OpenMultipleListenerCloseAll_Succeed);
  MITK_TEST(OpenListenerGetRequestSamePath_ReturnExpectedJSON);
  MITK_TEST(CloseListener_NoRequestPossible);
  MITK_TEST(OpenListenerGetRequestDifferentPath_ReturnNotFound);
  MITK_TEST(OpenListenerCloseAndReopen_Succeed);
  CPPUNIT_TEST_SUITE_END();

public:
  mitk::IRESTManager *m_Service;
  web::json::value m_Data;

  //TODO: bei Notify uri parameter zuerst
  web::json::value Notify(const web::json::value &data, const web::uri &uri) override
  {
    web::json::value returnData = data;
    returnData[L"userId"] = web::json::value(1);
    returnData[L"id"] = web::json::value(1);
    returnData[L"title"] = web::json::value(U("this is a title"));
    returnData[L"body"] = web::json::value(U("this is a body"));
    return returnData;
  }

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
   * members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override
  {
    m_Data[L"userId"] = web::json::value(1);
    m_Data[L"id"] = web::json::value(1);
    m_Data[L"title"] = web::json::value(U("this is a title"));
    m_Data[L"body"] = web::json::value(U("this is a body"));
    us::ServiceReference<mitk::IRESTManager> serviceRef =
      us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();
    if (serviceRef)
    {
      m_Service = us::GetModuleContext()->GetService(serviceRef);
    }
    //TODO: if(m_Service) überprüfen, exception wenn nicht
  }

  void tearDown() override { m_Service->HandleDeleteObserver(this); }

  void OpenListener_Succeed()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }

  void TwoListenerSameHostSamePort_OnlyOneOpened()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
      m_Service->ReceiveRequest(L"http://localhost:8080/example", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open two listener with a different path,same host, same port, observer map size is two",
                           2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener with a different path,same host, same port, server map size is one",
                           1 == m_Service->GetServerMap().size());
  }

  void CloseListener_Succeed()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());
    if (m_Service)
    {
      m_Service->HandleDeleteObserver(this);
    }
    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());
  }

  void OpenMultipleListenerCloseOne_Succeed()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
      m_Service->ReceiveRequest(L"http://localhost:8090/example", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open two listener, observer map size is two", 2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener, server map size is two", 2 == m_Service->GetServerMap().size());
    if (m_Service)
    {
      m_Service->HandleDeleteObserver(this, L"http://localhost:8080/test");
    }
    CPPUNIT_ASSERT_MESSAGE("Closed one of two listeners, observer map is size is one",
                           1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed one of two listener, server map size is one",
                           1 == m_Service->GetServerMap().size());
  }

  void OpenMultipleListenerCloseAll_Succeed()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
      m_Service->ReceiveRequest(L"http://localhost:8090/example", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open two listener, observer map size is two", 2 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open two listener, server map size is two", 2 == m_Service->GetServerMap().size());
    if (m_Service)
    {
      m_Service->HandleDeleteObserver(this);
    }
    CPPUNIT_ASSERT_MESSAGE("Closed all listeners, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed all listeners, server map is empty", 0 == m_Service->GetServerMap().size());
  }

  void OpenListenerGetRequestSamePath_ReturnExpectedJSON()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    web::json::value *result = new web::json::value();
    web::json::value data;
    data[L"userId"] = web::json::value(1);
    data[L"id"] = web::json::value(1);
    data[L"title"] = web::json::value(U("this is a title"));
    data[L"body"] = web::json::value(U("this is a body"));

    if (m_Service)
    {
      m_Service->SendRequest(L"http://localhost:8080/test")
        .then([=](pplx::task<web::json::value> resultTask) {
          try
          {
            *result = resultTask.get();
          }
          catch (const mitk::Exception &exception)
          {
            MITK_ERROR << exception.what();
            return;
          }
        })
        .wait();
    }
    CPPUNIT_ASSERT_MESSAGE("Opened listener and send request to same uri, returned expected JSON", *result == data);
  }

  void RequestToClosedListener()
  {
    web::json::value *result = new web::json::value();
    if (m_Service)
    {
      m_Service->SendRequest(L"http://localhost:8080/test")
        .then([=](pplx::task<web::json::value> resultTask) { *result = resultTask.get(); })
        .wait();
    }
  }
  void CloseListener_NoRequestPossible()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());
    if (m_Service)
    {
      m_Service->HandleDeleteObserver(this);
    }
    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());
    
    CPPUNIT_ASSERT_THROW(RequestToClosedListener(), mitk::Exception);
  }

  void RequestToDifferentPathNotFound()
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    web::json::value *result = new web::json::value();
    web::json::value data;
    data[L"userId"] = web::json::value(1);
    data[L"id"] = web::json::value(1);
    data[L"title"] = web::json::value(U("this is a title"));
    data[L"body"] = web::json::value(U("this is a body"));

    if (m_Service)
    {
      m_Service->SendRequest(L"http://localhost:8080/example")
        .then([=](pplx::task<web::json::value> resultTask) { *result = resultTask.get(); })
        .wait();
    }
  }
  void OpenListenerGetRequestDifferentPath_ReturnNotFound()
  {
    CPPUNIT_ASSERT_THROW(RequestToDifferentPathNotFound(), mitk::Exception);
  }

  void OpenListenerCloseAndReopen_Succeed() 
  {
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Open one listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Open one listener, server map size is one", 1 == m_Service->GetServerMap().size());
    if (m_Service)
    {
      m_Service->HandleDeleteObserver(this);
    }
    CPPUNIT_ASSERT_MESSAGE("Closed listener, observer map is empty", 0 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Closed listener, server map is empty", 0 == m_Service->GetServerMap().size());
    if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
    CPPUNIT_ASSERT_MESSAGE("Reopened listener, observer map size is one", 1 == m_Service->GetObservers().size());
    CPPUNIT_ASSERT_MESSAGE("Reopened listener, server map size is one", 1 == m_Service->GetServerMap().size());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTServer)