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
#include "mitkRESTClient.h"

// VTK includes
#include <vtkDebugLeaks.h>

#include "mitkIRESTManager.h"
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>
#include <usServiceReference.h>
#include <usServiceTracker.h>

class mitkRESTClientTestSuite : public mitk::TestFixture, mitk::IRESTObserver
{
  CPPUNIT_TEST_SUITE(mitkRESTClientTestSuite);

  MITK_TEST(GetRequestValidURI_ReturnsExpectedJSON);
  MITK_TEST(MultipleGetRequestValidURI_AllTasksFinish);
  MITK_TEST(PutRequestValidURI_ReturnsExpectedJSON);
  MITK_TEST(PostRequestValidURI_ReturnsExpectedJSON);
  MITK_TEST(GetRequestInvalidURI_ThrowsException);
  MITK_TEST(PutRequestInvalidURI_ThrowsException);
  MITK_TEST(PostRequestInvalidURI_ThrowsException);
  CPPUNIT_TEST_SUITE_END();

public:
  mitk::IRESTManager *m_Service;

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
    us::ServiceReference<mitk::IRESTManager> serviceRef =
      us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();
    if (serviceRef)
    {
      m_Service = us::GetModuleContext()->GetService(serviceRef);
    }
     if (m_Service)
    {
      m_Service->ReceiveRequest(L"http://localhost:8080/test", this);
    }
  }

  void tearDown() override
  {
     if (m_Service)
    {
      m_Service->HandleDeleteObserver(this);
    }
  }

  void GetRequestValidURI_ReturnsExpectedJSON()
  {
    web::json::value *result = new web::json::value();
    web::json::value data;
    //TODO: data als memebrvariable/ in setup/ in methode
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
    CPPUNIT_ASSERT_MESSAGE("Result is the expected JSON value", *result == data);
  }

  void MultipleGetRequestValidURI_AllTasksFinish()
  {
    int *count = new int(0);
    if (m_Service)
    {
      // Create multiple tasks e.g. as shown below
      //TODO: vector konstruktoren anschauen / emplace_back
      std::vector<pplx::task<void>> tasks;
      for (int i = 0; i < 20; ++i)
      {
        pplx::task<void> singleTask =
          m_Service->SendRequest(L"http://localhost:8080/test")
            .then([=](pplx::task<web::json::value> resultTask) {
              // Do something when a single task is done
              try
              {
                resultTask.get();
                *count +=1;
              }
              catch (const mitk::Exception &exception)
              {
                MITK_ERROR << exception.what();
                return;
              }
            });
        tasks.push_back(singleTask);
      }
      // Create a joinTask which includes all tasks you've created
      auto joinTask = pplx::when_all(begin(tasks), end(tasks));
      // Run asynchonously
      joinTask.then([=](pplx::task<void> resultTask) {
        // Do something when all tasks are finished
        try
        {
          resultTask.get();
          *count += 1;
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        }
      }).wait();
     }
    CPPUNIT_ASSERT_MESSAGE("Multiple Requests", 21 == *count);
  }

  void PutRequestValidURI_ReturnsExpectedJSON()
  {
    // optional: link might get invalid or content is changed
    web::json::value *result = new web::json::value();
    web::json::value data;
    if (m_Service)
    {
      data[L"userId"] = web::json::value(1);
      data[L"id"] = web::json::value(1);
      data[L"title"] = web::json::value(U("this is a changed title"));
      data[L"body"] = web::json::value(U("and the body is changed as well"));
      m_Service
        ->SendRequest(L"https://jsonplaceholder.typicode.com/posts/1", mitk::IRESTManager::RequestType::Put, &data)
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
    CPPUNIT_ASSERT_MESSAGE(
      "Result is the expected JSON value, check if the link is still valid since this is an optional test",
      *result == data);
  }

  void PostRequestValidURI_ReturnsExpectedJSON()
  {
    // optional: link might get invalid or content is changed
    web::json::value *result = new web::json::value();
    web::json::value data;
    if (m_Service)
    {
      data[L"userId"] = web::json::value(1);
      data[L"title"] = web::json::value(U("this is a new title"));
      data[L"body"] = web::json::value(U("this is a new body"));
      m_Service->SendRequest(L"https://jsonplaceholder.typicode.com/posts", mitk::IRESTManager::RequestType::Post, &data)
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
    data[L"id"] = web::json::value(101);
    CPPUNIT_ASSERT_MESSAGE(
      "Result is the expected JSON value, check if the link is still valid since this is an optional test",
      *result == data);
  }

  void GetException()
  {
    //Method which makes a get request to an invalid uri
    web::json::value *result = new web::json::value();
    if (m_Service)
    {
      m_Service->SendRequest(L"http://localhost:1234/invalid")
        .then([=](pplx::task<web::json::value> resultTask) { *result = resultTask.get(); })
        .wait();
    }
  }
  void GetRequestInvalidURI_ThrowsException() { CPPUNIT_ASSERT_THROW(GetException(), mitk::Exception); }

  void PutException() 
  {
    //Method which makes a put request to an invalid uri
    web::json::value *result = new web::json::value();
    web::json::value data;
    if (m_Service)
    {
      data[L"userId"] = web::json::value(1);
      data[L"id"] = web::json::value(1);
      data[L"title"] = web::json::value(U("this is a changed title"));
      data[L"body"] = web::json::value(U("and the body is changed as well"));
      m_Service->SendRequest(L"http://localhost:1234/invalid", mitk::IRESTManager::RequestType::Put, &data)
        .then([=](pplx::task<web::json::value> resultTask) {
            *result = resultTask.get();})
        .wait();
        }
  }
  void PutRequestInvalidURI_ThrowsException()
  {
    CPPUNIT_ASSERT_THROW(PutException(), mitk::Exception);
  }

  void PostException()
  {
    //Method which makes a post request to an invalid uri
    web::json::value *result = new web::json::value();
    web::json::value data;
    if (m_Service)
    {
      data[L"userId"] = web::json::value(1);
      data[L"title"] = web::json::value(U("this is a new title"));
      data[L"body"] = web::json::value(U("this is a new body"));
      m_Service->SendRequest(L"http://localhost:1234/invalid", mitk::IRESTManager::RequestType::Post, &data)
        .then([=](pplx::task<web::json::value> resultTask) {
            *result = resultTask.get();
        })
        .wait();
    }
  }
  void PostRequestInvalidURI_ThrowsException()
  {
    CPPUNIT_ASSERT_THROW(PostException(), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRESTClient)