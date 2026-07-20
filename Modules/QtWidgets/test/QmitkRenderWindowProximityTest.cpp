/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkRenderWindowProximity.h>

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QElapsedTimer>
#include <QWidget>

#include <memory>
#include <utility>
#include <vector>

/**
 * Tests the idle -> hint -> active state machine of the proximity controller
 * headlessly by feeding synthetic pointer positions through the public
 * HandlePointerMoved / HandlePointerLeft seam (no real mouse events, no
 * rendering):
 *   - upward transitions (reveal) are immediate
 *   - downward transitions (collapse) wait for the collapse delay and are
 *     cancelled when the pointer returns in time
 *   - the hysteresis band prevents state flapping at the activation edge
 *   - regions are evaluated independently
 *   - SetSuppressed forces Idle immediately and unsuppressing re-evaluates
 *   - precondition violations throw
 *
 * Timer-based collapse only fires while the event loop runs, so asserting
 * "still unchanged" right after a move (without processing events) is
 * deterministic; waiting loops process events for comfortably longer than
 * the collapse delay before asserting the collapsed state.
 */
class QmitkRenderWindowProximityTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkRenderWindowProximityTestSuite);
  MITK_TEST(Construct_NullCellThrows);
  MITK_TEST(AddEventSource_NullThrows);
  MITK_TEST(Register_NullCallbackThrows);
  MITK_TEST(Register_StartsIdle);
  MITK_TEST(Query_UnknownIdThrows);
  MITK_TEST(Unregister_UnknownIdThrows);
  MITK_TEST(Move_FarFromRegion_HintImmediately);
  MITK_TEST(Move_NearRegion_ActiveImmediately);
  MITK_TEST(Move_IntoRegionWithButtonHeld_StaysHint);
  MITK_TEST(Button_ReleasedNearRegion_Reveals);
  MITK_TEST(Active_ButtonHeldDoesNotCollapse);
  MITK_TEST(Hysteresis_NoFlappingInBand);
  MITK_TEST(Collapse_DowngradeWaitsForDelay);
  MITK_TEST(Collapse_CancelledWhenPointerReturns);
  MITK_TEST(Leave_CollapsesToIdle);
  MITK_TEST(Suppress_ForcesIdleImmediately);
  MITK_TEST(Unsuppress_ReevaluatesImmediately);
  MITK_TEST(MultiRegion_IndependentStates);
  MITK_TEST(Unregister_StopsEmissions);
  CPPUNIT_TEST_SUITE_END();

  using Proximity = QmitkRenderWindowProximity;
  using State = Proximity::State;

  std::unique_ptr<QWidget> m_Cell;
  std::unique_ptr<Proximity> m_Proximity;

  // Cell is 400x300 with a right-edge region; distances below relate to the
  // controller's ActivationDistance (58) and HysteresisBand (16).
  const QRect m_RightEdgeRegion = QRect(380, 0, 20, 300);
  const QPoint m_FarPoint = QPoint(200, 150);   // 180 px from the region
  const QPoint m_NearPoint = QPoint(340, 150);  // 40 px, inside activation
  const QPoint m_BandPoint = QPoint(315, 150);  // 65 px, inside hysteresis band only

public:

  void setUp() override
  {
    EnsureQApplication();

    m_Cell = std::make_unique<QWidget>();
    m_Cell->resize(400, 300);
    m_Proximity = std::make_unique<Proximity>(m_Cell.get());
  }

  void tearDown() override
  {
    m_Proximity.reset();
    m_Cell.reset();
  }

  /** Records StateChanged emissions for assertions on transition sequences. */
  struct StateRecorder
  {
    std::vector<std::pair<Proximity::RegionId, State>> transitions;
    QMetaObject::Connection conn;

    explicit StateRecorder(Proximity& proximity)
    {
      conn = QObject::connect(&proximity, &Proximity::StateChanged,
                              [this](Proximity::RegionId id, State state)
                              { transitions.emplace_back(id, state); });
    }
    ~StateRecorder() { QObject::disconnect(conn); }
  };

  static void ProcessEventsFor(int milliseconds)
  {
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < milliseconds)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
  }

  static constexpr int WaitPastCollapse = Proximity::CollapseDelayMs + 300;

  Proximity::RegionId RegisterRightEdgeRegion()
  {
    return m_Proximity->RegisterRegion([this]() { return m_RightEdgeRegion; });
  }

  void Construct_NullCellThrows()
  {
    CPPUNIT_ASSERT_THROW(Proximity(nullptr), mitk::Exception);
  }

  void AddEventSource_NullThrows()
  {
    CPPUNIT_ASSERT_THROW(m_Proximity->AddEventSource(nullptr), mitk::Exception);
  }

  void Register_NullCallbackThrows()
  {
    CPPUNIT_ASSERT_THROW(m_Proximity->RegisterRegion(nullptr), mitk::Exception);
  }

  void Register_StartsIdle()
  {
    const auto id = this->RegisterRightEdgeRegion();
    CPPUNIT_ASSERT(State::Idle == m_Proximity->GetRegionState(id));
  }

  void Query_UnknownIdThrows()
  {
    CPPUNIT_ASSERT_THROW(m_Proximity->GetRegionState(4711), mitk::Exception);
  }

  void Unregister_UnknownIdThrows()
  {
    CPPUNIT_ASSERT_THROW(m_Proximity->UnregisterRegion(4711), mitk::Exception);
  }

  void Move_FarFromRegion_HintImmediately()
  {
    const auto id = this->RegisterRightEdgeRegion();
    StateRecorder recorder(*m_Proximity);

    m_Proximity->HandlePointerMoved(m_FarPoint);

    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), recorder.transitions.size());
  }

  void Move_NearRegion_ActiveImmediately()
  {
    const auto id = this->RegisterRightEdgeRegion();

    m_Proximity->HandlePointerMoved(m_NearPoint);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
  }

  void Move_IntoRegionWithButtonHeld_StaysHint()
  {
    const auto id = this->RegisterRightEdgeRegion();

    // A gesture in progress (a mouse button held) must not trigger a reveal,
    // even with the pointer right on top of the region.
    m_Proximity->HandlePointerMoved(m_NearPoint, true);

    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));
  }

  void Button_ReleasedNearRegion_Reveals()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint, true);
    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));

    // Releasing the button turns the same position into a bare hover, which
    // reveals immediately.
    m_Proximity->HandlePointerMoved(m_NearPoint, false);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
  }

  void Active_ButtonHeldDoesNotCollapse()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));

    // A drag begun on already-revealed furniture (button now held, pointer
    // still over the region) must keep it up. Run past the collapse delay: a
    // naive "button forces Hint" rule would arm the timer and tear the frame
    // down mid-drag, so only the delay loop actually proves the property.
    m_Proximity->HandlePointerMoved(m_NearPoint, true);
    m_Proximity->HandlePointerMoved(m_BandPoint, true);
    m_Proximity->HandlePointerMoved(m_NearPoint, true);
    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
  }

  void Hysteresis_NoFlappingInBand()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));

    StateRecorder recorder(*m_Proximity);

    // Inside the band the computed state stays Active, so no collapse timer
    // ever starts; waiting past the delay must produce no transition.
    m_Proximity->HandlePointerMoved(m_BandPoint);
    m_Proximity->HandlePointerMoved(m_NearPoint);
    m_Proximity->HandlePointerMoved(m_BandPoint);
    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
    CPPUNIT_ASSERT(recorder.transitions.empty());
  }

  void Collapse_DowngradeWaitsForDelay()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);

    StateRecorder recorder(*m_Proximity);
    m_Proximity->HandlePointerMoved(m_FarPoint);

    // Without processing events the collapse timer cannot fire.
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
    CPPUNIT_ASSERT(recorder.transitions.empty());

    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), recorder.transitions.size());
    CPPUNIT_ASSERT(State::Hint == recorder.transitions.front().second);
  }

  void Collapse_CancelledWhenPointerReturns()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);

    StateRecorder recorder(*m_Proximity);

    // Away and immediately back, without running the event loop in between:
    // the armed collapse timer is cancelled before it can fire.
    m_Proximity->HandlePointerMoved(m_FarPoint);
    m_Proximity->HandlePointerMoved(m_NearPoint);
    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
    CPPUNIT_ASSERT(recorder.transitions.empty());
  }

  void Leave_CollapsesToIdle()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);

    m_Proximity->HandlePointerLeft();

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));

    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT(State::Idle == m_Proximity->GetRegionState(id));
  }

  void Suppress_ForcesIdleImmediately()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);

    m_Proximity->SetSuppressed(true);

    CPPUNIT_ASSERT(m_Proximity->IsSuppressed());
    CPPUNIT_ASSERT(State::Idle == m_Proximity->GetRegionState(id));

    m_Proximity->HandlePointerMoved(m_NearPoint);

    CPPUNIT_ASSERT(State::Idle == m_Proximity->GetRegionState(id));
  }

  void Unsuppress_ReevaluatesImmediately()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->SetSuppressed(true);
    m_Proximity->HandlePointerMoved(m_NearPoint);
    CPPUNIT_ASSERT(State::Idle == m_Proximity->GetRegionState(id));

    m_Proximity->SetSuppressed(false);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
  }

  void MultiRegion_IndependentStates()
  {
    const auto rightEdge = this->RegisterRightEdgeRegion();
    const auto bottomLeft = m_Proximity->RegisterRegion([]() { return QRect(0, 280, 120, 20); });

    m_Proximity->HandlePointerMoved(m_NearPoint);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(rightEdge));
    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(bottomLeft));
  }

  void Unregister_StopsEmissions()
  {
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_FarPoint);

    m_Proximity->UnregisterRegion(id);

    StateRecorder recorder(*m_Proximity);
    m_Proximity->HandlePointerMoved(m_NearPoint);
    ProcessEventsFor(50);

    CPPUNIT_ASSERT(recorder.transitions.empty());
    CPPUNIT_ASSERT_THROW(m_Proximity->GetRegionState(id), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkRenderWindowProximity)
