/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkInteractionSchemeSwitcher.h>

#include <mitkException.h>
#include <mitkInteractionEventHandler.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkMouseMoveEvent.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseWheelEvent.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  /**
   * The switcher only needs an event handler to load configurations into.
   * A minimal one keeps the test independent of the display interaction
   * state machine and of the micro service registration that comes with
   * mitk::DisplayActionEventBroadcast.
   */
  class TestEventHandler : public mitk::InteractionEventHandler
  {
  public:
    mitkClassMacroItkParent(TestEventHandler, mitk::InteractionEventHandler);
    itkFactorylessNewMacro(Self);
  };

  using MouseButtons = mitk::InteractionEvent::MouseButtons;
  using ModifierKeys = mitk::InteractionEvent::ModifierKeys;

  mitk::Point2D Origin()
  {
    mitk::Point2D position;
    position.Fill(0.0);
    return position;
  }

  std::string MapMousePress(const mitk::EventConfig& config, MouseButtons button, ModifierKeys modifiers)
  {
    // Button states incorporate the event button, as they do in Qt.
    mitk::InteractionEvent::Pointer event =
      mitk::MousePressEvent::New(nullptr, Origin(), button, modifiers, button).GetPointer();

    return config.GetMappedEvent(event);
  }

  std::string MapMouseMove(const mitk::EventConfig& config, MouseButtons buttonStates, ModifierKeys modifiers)
  {
    mitk::InteractionEvent::Pointer event =
      mitk::MouseMoveEvent::New(nullptr, Origin(), buttonStates, modifiers).GetPointer();

    return config.GetMappedEvent(event);
  }

  std::string MapWheel(const mitk::EventConfig& config, int delta)
  {
    mitk::InteractionEvent::Pointer event =
      mitk::MouseWheelEvent::New(nullptr, Origin(), mitk::InteractionEvent::NoButton, mitk::InteractionEvent::NoKey, delta).GetPointer();

    return config.GetMappedEvent(event);
  }

  std::string MapKey(const mitk::EventConfig& config, const std::string& key)
  {
    mitk::InteractionEvent::Pointer event =
      mitk::InteractionKeyEvent::New(nullptr, key, mitk::InteractionEvent::NoKey).GetPointer();

    return config.GetMappedEvent(event);
  }
}

class mitkInteractionSchemeSwitcherTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkInteractionSchemeSwitcherTestSuite);
  MITK_TEST(SetInteractionScheme_NoEventHandler_Throws);
  MITK_TEST(MITKStandard_BindsCrosshairAndNavigation);
  MITK_TEST(MITKRotation_ReplacesCrosshairByRotation);
  MITK_TEST(PACSBase_LeavesLeftMouseButtonUnbound);
  MITK_TEST(PACS_BindsRightMouseButtonInEveryScheme);
  MITK_TEST(PACS_BindsNavigationKeysInEveryScheme);
  MITK_TEST(PACS_BindsLeftMouseButtonPerScheme);
  CPPUNIT_TEST_SUITE_END();

private:

  TestEventHandler::Pointer m_EventHandler;

  mitk::EventConfig Apply(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
  {
    mitk::InteractionSchemeSwitcher::SetInteractionScheme(m_EventHandler, scheme);
    return m_EventHandler->GetEventConfig();
  }

public:

  void setUp() override
  {
    m_EventHandler = TestEventHandler::New();
  }

  void tearDown() override
  {
    m_EventHandler = nullptr;
  }

  void SetInteractionScheme_NoEventHandler_Throws()
  {
    CPPUNIT_ASSERT_THROW(
      mitk::InteractionSchemeSwitcher::SetInteractionScheme(nullptr, mitk::InteractionSchemeSwitcher::MITKStandard),
      mitk::Exception);
  }

  void MITKStandard_BindsCrosshairAndNavigation()
  {
    const auto config = Apply(mitk::InteractionSchemeSwitcher::MITKStandard);

    CPPUNIT_ASSERT_EQUAL(std::string("SetCrosshair"),
      MapMousePress(config, mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::NoKey));
    CPPUNIT_ASSERT_EQUAL(std::string("StartMove"),
      MapMousePress(config, mitk::InteractionEvent::MiddleMouseButton, mitk::InteractionEvent::NoKey));
    CPPUNIT_ASSERT_EQUAL(std::string("StartZoom"),
      MapMousePress(config, mitk::InteractionEvent::RightMouseButton, mitk::InteractionEvent::NoKey));
    CPPUNIT_ASSERT_EQUAL(std::string("PlaneUP"), MapWheel(config, 1));
    CPPUNIT_ASSERT_EQUAL(std::string("PlaneDown"), MapWheel(config, -1));
  }

  void MITKRotation_ReplacesCrosshairByRotation()
  {
    const auto config = Apply(mitk::InteractionSchemeSwitcher::MITKRotationUncoupled);

    CPPUNIT_ASSERT_EQUAL(std::string("StartRotate"),
      MapMousePress(config, mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::NoKey));
  }

  void PACSBase_LeavesLeftMouseButtonUnbound()
  {
    const auto config = Apply(mitk::InteractionSchemeSwitcher::PACSBase);

    CPPUNIT_ASSERT_EQUAL(std::string(),
      MapMousePress(config, mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::NoKey));
    CPPUNIT_ASSERT_EQUAL(std::string(),
      MapMousePress(config, mitk::InteractionEvent::MiddleMouseButton, mitk::InteractionEvent::NoKey));
  }

  void PACS_BindsRightMouseButtonInEveryScheme()
  {
    for (const auto scheme : { mitk::InteractionSchemeSwitcher::PACSBase,
                               mitk::InteractionSchemeSwitcher::PACSStandard,
                               mitk::InteractionSchemeSwitcher::PACSLevelWindow,
                               mitk::InteractionSchemeSwitcher::PACSPan,
                               mitk::InteractionSchemeSwitcher::PACSScroll,
                               mitk::InteractionSchemeSwitcher::PACSZoom })
    {
      const auto config = Apply(scheme);

      CPPUNIT_ASSERT_EQUAL(std::string("StartAdjustLevelWindow"),
        MapMousePress(config, mitk::InteractionEvent::RightMouseButton, mitk::InteractionEvent::NoKey));
      CPPUNIT_ASSERT_EQUAL(std::string("StartZoom"),
        MapMousePress(config, mitk::InteractionEvent::RightMouseButton, mitk::InteractionEvent::ControlKey));
      CPPUNIT_ASSERT_EQUAL(std::string("StartMove"),
        MapMousePress(config, mitk::InteractionEvent::RightMouseButton, mitk::InteractionEvent::ShiftKey));
      CPPUNIT_ASSERT_EQUAL(std::string("PlaneUP"), MapWheel(config, 1));

      // A move without any button resets a state machine left behind by a drag
      // whose modifiers changed halfway through.
      CPPUNIT_ASSERT_EQUAL(std::string("ResetState"),
        MapMouseMove(config, mitk::InteractionEvent::NoButton, mitk::InteractionEvent::NoKey));
    }
  }

  void PACS_BindsNavigationKeysInEveryScheme()
  {
    for (const auto scheme : { mitk::InteractionSchemeSwitcher::MITKStandard,
                               mitk::InteractionSchemeSwitcher::PACSBase,
                               mitk::InteractionSchemeSwitcher::PACSStandard,
                               mitk::InteractionSchemeSwitcher::PACSZoom })
    {
      const auto config = Apply(scheme);

      CPPUNIT_ASSERT_EQUAL(std::string("PlaneUP"), MapKey(config, mitk::InteractionEvent::KeyArrowUp));
      CPPUNIT_ASSERT_EQUAL(std::string("PlaneDown"), MapKey(config, mitk::InteractionEvent::KeyArrowDown));
      CPPUNIT_ASSERT_EQUAL(std::string("IncreaseTimeStep"), MapKey(config, mitk::InteractionEvent::KeyArrowRight));
      CPPUNIT_ASSERT_EQUAL(std::string("DecreaseTimeStep"), MapKey(config, mitk::InteractionEvent::KeyArrowLeft));
    }
  }

  void PACS_BindsLeftMouseButtonPerScheme()
  {
    const std::vector<std::pair<mitk::InteractionSchemeSwitcher::InteractionScheme, std::string>> expected{
      { mitk::InteractionSchemeSwitcher::PACSStandard, "SetCrosshair" },
      { mitk::InteractionSchemeSwitcher::PACSLevelWindow, "StartAdjustLevelWindow" },
      { mitk::InteractionSchemeSwitcher::PACSPan, "StartMove" },
      { mitk::InteractionSchemeSwitcher::PACSScroll, "StartScroll" },
      { mitk::InteractionSchemeSwitcher::PACSZoom, "StartZoom" }
    };

    for (const auto& [scheme, variant] : expected)
    {
      const auto config = Apply(scheme);

      CPPUNIT_ASSERT_EQUAL(variant,
        MapMousePress(config, mitk::InteractionEvent::LeftMouseButton, mitk::InteractionEvent::NoKey));
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkInteractionSchemeSwitcher)
