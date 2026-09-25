/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDisplayActionEventBroadcast.h>

#include <mitkMouseMoveEvent.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseReleaseEvent.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <utility>

namespace
{
  /**
   * Counts the events that make it past Notify() into the state machine.
   * FilterEvents() is the first thing HandleEvent() calls, so no state
   * machine, configuration, or renderer is needed.
   */
  class CountingBroadcast : public mitk::DisplayActionEventBroadcast
  {
  public:
    mitkClassMacro(CountingBroadcast, mitk::DisplayActionEventBroadcast);
    itkFactorylessNewMacro(Self);

    int NumberOfHandledEvents = 0;

  protected:
    bool FilterEvents(mitk::InteractionEvent*, mitk::DataNode*) override
    {
      ++NumberOfHandledEvents;
      return false;
    }
  };

  using MouseButtons = mitk::InteractionEvent::MouseButtons;
  using ModifierKeys = mitk::InteractionEvent::ModifierKeys;

  mitk::Point2D Origin()
  {
    mitk::Point2D position;
    position.Fill(0.0);
    return position;
  }

  mitk::InteractionEvent::Pointer Press(MouseButtons button, ModifierKeys modifiers = mitk::InteractionEvent::NoKey)
  {
    return mitk::MousePressEvent::New(nullptr, Origin(), button, modifiers, button).GetPointer();
  }

  mitk::InteractionEvent::Pointer Drag(MouseButtons buttonStates)
  {
    return mitk::MouseMoveEvent::New(nullptr, Origin(), buttonStates, mitk::InteractionEvent::NoKey).GetPointer();
  }

  mitk::InteractionEvent::Pointer Release(MouseButtons button)
  {
    return mitk::MouseReleaseEvent::New(nullptr, Origin(), mitk::InteractionEvent::NoButton, mitk::InteractionEvent::NoKey, button).GetPointer();
  }
}

class mitkDisplayActionEventBroadcastTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDisplayActionEventBroadcastTestSuite);
  MITK_TEST(Notify_WithoutBlock_PassesLeftButtonPress);
  MITK_TEST(Notify_HandledEvent_IsSkipped);
  MITK_TEST(BlockLeftButton_SuppressesLeftButtonPressOnly);
  MITK_TEST(BlockLeftButton_SuppressesModifiedLeftButtonPress);
  MITK_TEST(BlockLeftButton_AppliesToBroadcastsCreatedLater);
  MITK_TEST(BlockLeftButton_SurvivesEventConfigChange);
  MITK_TEST(Blocks_AreCounted_InAnyReleaseOrder);
  MITK_TEST(Block_ReleasedOnDestruction);
  MITK_TEST(Block_DefaultConstructed_IsInactive);
  MITK_TEST(Block_Move_TransfersBlock);
  MITK_TEST(Block_MoveAssignment_ReleasesPreviousBlock);
  CPPUNIT_TEST_SUITE_END();

private:

  CountingBroadcast::Pointer m_Broadcast;

  int CountPassed(const mitk::InteractionEvent::Pointer& event)
  {
    const auto before = m_Broadcast->NumberOfHandledEvents;
    m_Broadcast->Notify(event, false);
    return m_Broadcast->NumberOfHandledEvents - before;
  }

  bool IsLeftButtonPressBlocked()
  {
    return 0 == this->CountPassed(Press(mitk::InteractionEvent::LeftMouseButton));
  }

public:

  void setUp() override
  {
    m_Broadcast = CountingBroadcast::New();
  }

  void tearDown() override
  {
    m_Broadcast = nullptr;
  }

  void Notify_WithoutBlock_PassesLeftButtonPress()
  {
    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());
  }

  void Notify_HandledEvent_IsSkipped()
  {
    m_Broadcast->Notify(Press(mitk::InteractionEvent::MiddleMouseButton), true);

    CPPUNIT_ASSERT_EQUAL(0, m_Broadcast->NumberOfHandledEvents);
  }

  void BlockLeftButton_SuppressesLeftButtonPressOnly()
  {
    const auto block = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());
    CPPUNIT_ASSERT_EQUAL(1, this->CountPassed(Press(mitk::InteractionEvent::MiddleMouseButton)));
    CPPUNIT_ASSERT_EQUAL(1, this->CountPassed(Press(mitk::InteractionEvent::RightMouseButton)));
    CPPUNIT_ASSERT_EQUAL(1, this->CountPassed(Drag(mitk::InteractionEvent::LeftMouseButton)));
    CPPUNIT_ASSERT_EQUAL(1, this->CountPassed(Release(mitk::InteractionEvent::LeftMouseButton)));
  }

  void BlockLeftButton_SuppressesModifiedLeftButtonPress()
  {
    const auto block = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    CPPUNIT_ASSERT_EQUAL(0, this->CountPassed(Press(mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::ShiftKey)));
    CPPUNIT_ASSERT_EQUAL(0, this->CountPassed(Press(mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::ControlKey)));
  }

  void BlockLeftButton_AppliesToBroadcastsCreatedLater()
  {
    const auto block = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    m_Broadcast = CountingBroadcast::New();

    CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());
  }

  void BlockLeftButton_SurvivesEventConfigChange()
  {
    const auto block = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    CPPUNIT_ASSERT(m_Broadcast->SetEventConfig("DisplayConfigMITKBase.xml"));
    CPPUNIT_ASSERT(m_Broadcast->AddEventConfig("DisplayConfigCrosshair.xml"));

    CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());
  }

  void Blocks_AreCounted_InAnyReleaseOrder()
  {
    auto first = mitk::DisplayActionEventBroadcast::BlockLeftButton();
    auto second = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    first.Reset();
    CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());

    second.Reset();
    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());

    first = mitk::DisplayActionEventBroadcast::BlockLeftButton();
    second = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    second.Reset();
    CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());

    first.Reset();
    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());
  }

  void Block_ReleasedOnDestruction()
  {
    {
      const auto block = mitk::DisplayActionEventBroadcast::BlockLeftButton();
      CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());
    }

    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());
  }

  void Block_DefaultConstructed_IsInactive()
  {
    mitk::DisplayActionEventBroadcast::LeftButtonBlock block;
    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());

    block.Reset();
    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());
  }

  void Block_Move_TransfersBlock()
  {
    auto source = mitk::DisplayActionEventBroadcast::BlockLeftButton();
    auto target = std::move(source);

    source.Reset();
    CPPUNIT_ASSERT(this->IsLeftButtonPressBlocked());

    target.Reset();
    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());
  }

  void Block_MoveAssignment_ReleasesPreviousBlock()
  {
    auto block = mitk::DisplayActionEventBroadcast::BlockLeftButton();

    // Re-blocking an active block must not leak the previous one.
    block = mitk::DisplayActionEventBroadcast::BlockLeftButton();
    block.Reset();

    CPPUNIT_ASSERT(!this->IsLeftButtonPressBlocked());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDisplayActionEventBroadcast)
