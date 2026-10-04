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
  class AnimationTestingRenderingManager : public mitk::RenderingManager
  {
  public:
    mitkClassMacro(AnimationTestingRenderingManager, mitk::RenderingManager);
    itkFactorylessNewMacro(Self);

    using mitk::RenderingManager::OnRenderWindowRendered;

    bool IsUpdateRequested(vtkRenderWindow* renderWindow) const
    {
      return RENDERING_REQUESTED == m_RenderWindowList.at(renderWindow);
    }

    bool IsAnimationTimerRunning() const
    {
      return m_AnimationTimerRunning;
    }

    std::chrono::milliseconds GetAnimationTimerInterval() const
    {
      return m_AnimationTimerInterval;
    }

  protected:
    void GenerateRenderingRequestEvent() override
    {
    }

    void StartAnimationTimer(std::chrono::milliseconds interval) override
    {
      m_AnimationTimerRunning = true;
      m_AnimationTimerInterval = interval;
    }

    void StopAnimationTimer() override
    {
      m_AnimationTimerRunning = false;
    }

  private:
    bool m_AnimationTimerRunning = false;
    std::chrono::milliseconds m_AnimationTimerInterval{ 0 };
  };
}

class mitkRenderingManagerAnimationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRenderingManagerAnimationTestSuite);
  MITK_TEST(RequestAnimationFrame_UnregisteredWindow_ClockStopped);
  MITK_TEST(RequestAnimationFrame_NextFrame_UpdateRequested);
  MITK_TEST(ExecuteAnimationFrame_RequestNotRenewed_ClockStops);
  MITK_TEST(ExecuteAnimationFrame_RequestRenewedByFrame_NextFrameUpdateRequested);
  MITK_TEST(ExecuteAnimationFrame_RenderedBetweenFrames_FrameSkippedRequestKept);
  MITK_TEST(AddAnimationFrameObserver_NoRequests_ObserverCalledClockRunning);
  MITK_TEST(RemoveAnimationFrameObserver_WhileCalled_ClockStops);
  MITK_TEST(SetAnimationFrameRate_ClockRunning_IntervalChanged);
  MITK_TEST(SetAnimationFrameRate_Zero_Throws);
  CPPUNIT_TEST_SUITE_END();

  AnimationTestingRenderingManager::Pointer m_RenderingManager;
  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

public:
  void setUp() override
  {
    m_RenderingManager = AnimationTestingRenderingManager::New();
    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    // A zero-sized render window skips the actual VTK render but still runs the request bookkeeping.
    m_RenderWindow->SetSize(0, 0);

    m_RenderingManager->AddRenderWindow(m_RenderWindow);
  }

  void tearDown() override
  {
    m_RenderingManager->RemoveRenderWindow(m_RenderWindow);
    m_RenderingManager = nullptr;
    m_RenderWindow = nullptr;
  }

  void RequestAnimationFrame_UnregisteredWindow_ClockStopped()
  {
    auto unregisteredRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_RenderingManager->RequestAnimationFrame(unregisteredRenderWindow);

    CPPUNIT_ASSERT(!m_RenderingManager->IsAnimationTimerRunning());
  }

  void RequestAnimationFrame_NextFrame_UpdateRequested()
  {
    m_RenderingManager->RequestAnimationFrame(m_RenderWindow);

    CPPUNIT_ASSERT(m_RenderingManager->IsAnimationTimerRunning());
    CPPUNIT_ASSERT(!m_RenderingManager->IsUpdateRequested(m_RenderWindow));

    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT(m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void ExecuteAnimationFrame_RequestNotRenewed_ClockStops()
  {
    m_RenderingManager->RequestAnimationFrame(m_RenderWindow);
    m_RenderingManager->ExecuteAnimationFrame();
    m_RenderingManager->ExecutePendingRequests();
    m_RenderingManager->OnRenderWindowRendered(m_RenderWindow);

    CPPUNIT_ASSERT_MESSAGE("The clock must not stop before the frame could renew the request",
      m_RenderingManager->IsAnimationTimerRunning());

    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT(!m_RenderingManager->IsAnimationTimerRunning());
    CPPUNIT_ASSERT(!m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void ExecuteAnimationFrame_RequestRenewedByFrame_NextFrameUpdateRequested()
  {
    m_RenderingManager->RequestAnimationFrame(m_RenderWindow);
    m_RenderingManager->ExecuteAnimationFrame();
    m_RenderingManager->ExecutePendingRequests();

    // A mapper that still animates asks for the next frame while the frame renders.
    m_RenderingManager->RequestAnimationFrame(m_RenderWindow);
    m_RenderingManager->OnRenderWindowRendered(m_RenderWindow);

    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT_MESSAGE("The render of the animation frame itself must not skip the next frame",
      m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void ExecuteAnimationFrame_RenderedBetweenFrames_FrameSkippedRequestKept()
  {
    m_RenderingManager->RequestAnimationFrame(m_RenderWindow);

    // For example, the render of a mouse move.
    m_RenderingManager->OnRenderWindowRendered(m_RenderWindow);

    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT(!m_RenderingManager->IsUpdateRequested(m_RenderWindow));
    CPPUNIT_ASSERT(m_RenderingManager->IsAnimationTimerRunning());

    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT(m_RenderingManager->IsUpdateRequested(m_RenderWindow));
  }

  void AddAnimationFrameObserver_NoRequests_ObserverCalledClockRunning()
  {
    unsigned int numberOfCalls = 0;
    double secondsSincePreviousFrame = -1.0;

    const auto tag = m_RenderingManager->AddAnimationFrameObserver([&](double seconds)
      {
        ++numberOfCalls;
        secondsSincePreviousFrame = seconds;
      });

    CPPUNIT_ASSERT(0 != tag);
    CPPUNIT_ASSERT(m_RenderingManager->IsAnimationTimerRunning());

    m_RenderingManager->ExecuteAnimationFrame();
    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT_EQUAL(2u, numberOfCalls);
    CPPUNIT_ASSERT(0.0 <= secondsSincePreviousFrame);
    CPPUNIT_ASSERT(m_RenderingManager->IsAnimationTimerRunning());

    m_RenderingManager->RemoveAnimationFrameObserver(tag);
  }

  void RemoveAnimationFrameObserver_WhileCalled_ClockStops()
  {
    unsigned int numberOfCalls = 0;
    unsigned long tag = 0;

    tag = m_RenderingManager->AddAnimationFrameObserver([&](double)
      {
        ++numberOfCalls;
        m_RenderingManager->RemoveAnimationFrameObserver(tag);
      });

    m_RenderingManager->ExecuteAnimationFrame();
    m_RenderingManager->ExecuteAnimationFrame();

    CPPUNIT_ASSERT_EQUAL(1u, numberOfCalls);
    CPPUNIT_ASSERT(!m_RenderingManager->IsAnimationTimerRunning());
  }

  void SetAnimationFrameRate_ClockRunning_IntervalChanged()
  {
    m_RenderingManager->SetAnimationFrameRate(30);
    m_RenderingManager->RequestAnimationFrame(m_RenderWindow);

    CPPUNIT_ASSERT_EQUAL(std::chrono::milliseconds(33).count(), m_RenderingManager->GetAnimationTimerInterval().count());

    m_RenderingManager->SetAnimationFrameRate(60);

    CPPUNIT_ASSERT_EQUAL(60u, m_RenderingManager->GetAnimationFrameRate());
    CPPUNIT_ASSERT_EQUAL(std::chrono::milliseconds(17).count(), m_RenderingManager->GetAnimationTimerInterval().count());
  }

  void SetAnimationFrameRate_Zero_Throws()
  {
    CPPUNIT_ASSERT_THROW(m_RenderingManager->SetAnimationFrameRate(0), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRenderingManagerAnimation)
