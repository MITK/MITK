/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRenderingManager.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

namespace
{
  class RequestCountingRenderingManager : public mitk::RenderingManager
  {
  public:
    mitkClassMacro(RequestCountingRenderingManager, mitk::RenderingManager);
    itkFactorylessNewMacro(Self);

    bool IsUpdateRequested(vtkRenderWindow* renderWindow) const
    {
      return RENDERING_REQUESTED == m_RenderWindowList.at(renderWindow);
    }

    unsigned int GetNumberOfRequestEvents() const
    {
      return m_NumberOfRequestEvents;
    }

  protected:
    void GenerateRenderingRequestEvent() override
    {
      ++m_NumberOfRequestEvents;
    }

  private:
    unsigned int m_NumberOfRequestEvents = 0;
  };
}

class mitkRenderingManagerSuspendTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRenderingManagerSuspendTestSuite);
  MITK_TEST(RequestUpdate_Suspended_NoRequestEvent);
  MITK_TEST(ExecutePendingRequests_Suspended_RequestKept);
  MITK_TEST(Resume_PendingRequest_RequestExecuted);
  MITK_TEST(Resume_NoPendingRequest_NoRequestEvent);
  MITK_TEST(ForceImmediateUpdate_Suspended_RequestCleared);
  MITK_TEST(RemoveRenderWindow_Suspended_ReaddedNotSuspended);
  CPPUNIT_TEST_SUITE_END();

  RequestCountingRenderingManager::Pointer m_RenderingManager;
  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

public:
  void setUp() override
  {
    m_RenderingManager = RequestCountingRenderingManager::New();
    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_RenderingManager->AddRenderWindow(m_RenderWindow);
  }

  void tearDown() override
  {
    m_RenderingManager->RemoveRenderWindow(m_RenderWindow);
    m_RenderingManager = nullptr;
    m_RenderWindow = nullptr;
  }

  void RequestUpdate_Suspended_NoRequestEvent()
  {
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, true);
    m_RenderingManager->RequestUpdate(m_RenderWindow);

    CPPUNIT_ASSERT_EQUAL(0u, m_RenderingManager->GetNumberOfRequestEvents());
    CPPUNIT_ASSERT(m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void ExecutePendingRequests_Suspended_RequestKept()
  {
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, true);
    m_RenderingManager->RequestUpdate(m_RenderWindow);
    m_RenderingManager->ExecutePendingRequests();

    CPPUNIT_ASSERT(m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void Resume_PendingRequest_RequestExecuted()
  {
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, true);
    m_RenderingManager->RequestUpdate(m_RenderWindow);
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, false);

    CPPUNIT_ASSERT_EQUAL(1u, m_RenderingManager->GetNumberOfRequestEvents());

    m_RenderingManager->ExecutePendingRequests();

    CPPUNIT_ASSERT(!m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void Resume_NoPendingRequest_NoRequestEvent()
  {
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, true);
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, false);

    CPPUNIT_ASSERT_EQUAL(0u, m_RenderingManager->GetNumberOfRequestEvents());
  }

  void ForceImmediateUpdate_Suspended_RequestCleared()
  {
    // A zero-sized render window skips the actual VTK render, which would
    // need a display, but still runs the request bookkeeping.
    m_RenderWindow->SetSize(0, 0);

    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, true);
    m_RenderingManager->RequestUpdate(m_RenderWindow);
    m_RenderingManager->ForceImmediateUpdate(m_RenderWindow);

    CPPUNIT_ASSERT(!m_RenderingManager->IsUpdateRequested(m_RenderWindow));

    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, false);

    CPPUNIT_ASSERT_EQUAL(0u, m_RenderingManager->GetNumberOfRequestEvents());
  }

  void RemoveRenderWindow_Suspended_ReaddedNotSuspended()
  {
    m_RenderingManager->SetRenderingSuspended(m_RenderWindow, true);
    m_RenderingManager->RemoveRenderWindow(m_RenderWindow);
    m_RenderingManager->AddRenderWindow(m_RenderWindow);
    m_RenderingManager->RequestUpdate(m_RenderWindow);

    CPPUNIT_ASSERT_EQUAL(1u, m_RenderingManager->GetNumberOfRequestEvents());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderingManagerSuspend)
