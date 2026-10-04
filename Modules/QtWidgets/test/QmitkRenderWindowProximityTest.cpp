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

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWidget>

#include <memory>
#include <utility>
#include <vector>

/**
 * Tests the idle -> hint -> active state machine of the proximity controller
 * headlessly by feeding synthetic pointer positions through the public
 * HandlePointerMoved / HandlePointerLeft seam, and through the event filter
 * with synthetic Qt events sent to the cell and to a source (no rendering):
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
  MITK_TEST(Register_NullDistanceQueryThrows);
  MITK_TEST(DynamicActivationDistance_QueriedLazily);
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
  MITK_TEST(Pin_HoldsRevealedWhilePointerIsAway);
  MITK_TEST(Pin_LosesToSuppression);
  MITK_TEST(MultiRegion_IndependentStates);
  MITK_TEST(Unregister_StopsEmissions);
  MITK_TEST(EventFilter_ReleaseNearRegionReveals);
  MITK_TEST(EventFilter_SourceMoveMapsToCell);
  MITK_TEST(EventFilter_LeaveOnSourceIsIgnoredLeaveOnCellCollapses);
  MITK_TEST(EventFilter_EnterNearRegionReveals);
  MITK_TEST(EventFilter_ResizeReevaluates);
  MITK_TEST(EventFilter_DragOutOfTheCellLeaves);
  CPPUNIT_TEST_SUITE_END();

  using Proximity = QmitkRenderWindowProximity;
  using State = Proximity::State;

  std::unique_ptr<QWidget> m_Cell;
  std::unique_ptr<Proximity> m_Proximity;

  // Cell is 400x300 with a right-edge region; distances below relate to the
  // controller's ActivationDistance (48) and HysteresisBand (16).
  const QRect m_RightEdgeRegion = QRect(380, 0, 20, 300);
  const QPoint m_FarPoint = QPoint(200, 150);   // 180 px from the region
  const QPoint m_NearPoint = QPoint(340, 150);  // 40 px, inside activation
  const QPoint m_BandPoint = QPoint(324, 150);  // 56 px, inside hysteresis band only

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

  void Register_NullDistanceQueryThrows()
  {
    CPPUNIT_ASSERT_THROW(
      m_Proximity->RegisterRegion([this]() { return m_RightEdgeRegion; }, std::function<int()>()),
      mitk::Exception);
  }

  void DynamicActivationDistance_QueriedLazily()
  {
    int distance = 30;
    const auto id = m_Proximity->RegisterRegion([this]() { return m_RightEdgeRegion; },
                                                std::function<int()>([&distance]() { return distance; }));

    // The near point is 40 px from the region: outside a 30 px activation.
    m_Proximity->HandlePointerMoved(m_NearPoint);
    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));

    // Widening the distance reveals the same position - the query is read fresh
    // on every evaluation, so a size-dependent distance needs no re-registration.
    distance = 80;
    m_Proximity->HandlePointerMoved(m_NearPoint);
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
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

  void Pin_HoldsRevealedWhilePointerIsAway()
  {
    // A popup owned by the furniture takes a pointer grab, so the cell sees a
    // leave; the pin is what keeps the strip the user just clicked on screen.
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);

    m_Proximity->SetPinned(true);
    CPPUNIT_ASSERT(m_Proximity->IsPinned());

    m_Proximity->HandlePointerLeft();
    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT_MESSAGE("A pinned region survives the pointer leaving",
                           State::Active == m_Proximity->GetRegionState(id));

    // Unpinning resolves against where the pointer actually is, which is away.
    m_Proximity->SetPinned(false);
    ProcessEventsFor(WaitPastCollapse);

    CPPUNIT_ASSERT_MESSAGE("Unpinning collapses once the pointer is gone",
                           State::Idle == m_Proximity->GetRegionState(id));
  }

  void Pin_LosesToSuppression()
  {
    // Clean view is absolute: it hides the furniture for a screenshot, and a
    // popup left pinned must not punch a hole in it.
    const auto id = this->RegisterRightEdgeRegion();
    m_Proximity->HandlePointerMoved(m_NearPoint);
    m_Proximity->SetPinned(true);

    m_Proximity->SetSuppressed(true);

    CPPUNIT_ASSERT(State::Idle == m_Proximity->GetRegionState(id));
  }

  void MultiRegion_IndependentStates()
  {
    const auto rightEdge = this->RegisterRightEdgeRegion();
    const auto bottomLeft = m_Proximity->RegisterRegion([]() { return QRect(0, 280, 120, 20); });

    m_Proximity->HandlePointerMoved(m_NearPoint);

    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(rightEdge));
    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(bottomLeft));
  }

  /** Send a mouse event to 'target' at the cell position 'positionInCell'. */
  void SendMouse(QWidget* target, QEvent::Type type, const QPoint& positionInCell, Qt::MouseButton button,
                 Qt::MouseButtons buttons) const
  {
    const QPoint global = m_Cell->mapToGlobal(positionInCell);
    const QPointF local(target->mapFromGlobal(global));
    QMouseEvent event(type, local, local, QPointF(global), button, buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &event);
  }

  void EventFilter_ReleaseNearRegionReveals()
  {
    const auto id = this->RegisterRightEdgeRegion();
    this->SendMouse(m_Cell.get(), QEvent::MouseMove, m_NearPoint, Qt::NoButton, Qt::LeftButton);
    CPPUNIT_ASSERT_MESSAGE("A held button withholds the reveal", State::Hint == m_Proximity->GetRegionState(id));

    this->SendMouse(m_Cell.get(), QEvent::MouseButtonRelease, m_NearPoint, Qt::LeftButton, Qt::NoButton);
    CPPUNIT_ASSERT_MESSAGE("The release ends the withholding where the pointer rests",
                           State::Active == m_Proximity->GetRegionState(id));
  }

  void EventFilter_SourceMoveMapsToCell()
  {
    const auto id = this->RegisterRightEdgeRegion();
    QWidget source(m_Cell.get());
    source.setGeometry(100, 50, 290, 200);
    m_Proximity->AddEventSource(&source);

    this->SendMouse(&source, QEvent::MouseMove, m_FarPoint, Qt::NoButton, Qt::NoButton);
    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));
    this->SendMouse(&source, QEvent::MouseMove, m_NearPoint, Qt::NoButton, Qt::NoButton);
    CPPUNIT_ASSERT_MESSAGE("A source's move is resolved in cell coordinates",
                           State::Active == m_Proximity->GetRegionState(id));
  }

  void EventFilter_LeaveOnSourceIsIgnoredLeaveOnCellCollapses()
  {
    const auto id = this->RegisterRightEdgeRegion();
    QWidget source(m_Cell.get());
    source.setGeometry(100, 50, 290, 200);
    m_Proximity->AddEventSource(&source);
    this->SendMouse(&source, QEvent::MouseMove, m_NearPoint, Qt::NoButton, Qt::NoButton);
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));

    QEvent leave(QEvent::Leave);
    QCoreApplication::sendEvent(&source, &leave);
    ProcessEventsFor(WaitPastCollapse);
    CPPUNIT_ASSERT_MESSAGE("Leaving a source may only mean crossing into a sibling",
                           State::Active == m_Proximity->GetRegionState(id));

    QCoreApplication::sendEvent(m_Cell.get(), &leave);
    ProcessEventsFor(WaitPastCollapse);
    CPPUNIT_ASSERT_MESSAGE("Leaving the cell collapses", State::Idle == m_Proximity->GetRegionState(id));
  }

  void EventFilter_EnterNearRegionReveals()
  {
    const auto id = this->RegisterRightEdgeRegion();
    const QPointF global(m_Cell->mapToGlobal(m_NearPoint));
    QEnterEvent enter(QPointF(m_NearPoint), QPointF(m_NearPoint), global);
    QCoreApplication::sendEvent(m_Cell.get(), &enter);
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));
  }

  void EventFilter_ResizeReevaluates()
  {
    QRect region(0, 0, 10, 10);
    const auto id = m_Proximity->RegisterRegion([&region]() { return region; });
    this->SendMouse(m_Cell.get(), QEvent::MouseMove, m_NearPoint, Qt::NoButton, Qt::NoButton);
    CPPUNIT_ASSERT(State::Hint == m_Proximity->GetRegionState(id));

    // A resize moves the region under the resting pointer.
    region = m_RightEdgeRegion;
    QResizeEvent resize(QSize(400, 300), QSize(380, 280));
    QCoreApplication::sendEvent(m_Cell.get(), &resize);
    CPPUNIT_ASSERT_MESSAGE("A resize re-evaluates against the resting pointer",
                           State::Active == m_Proximity->GetRegionState(id));
    m_Proximity->UnregisterRegion(id);
  }

  void EventFilter_DragOutOfTheCellLeaves()
  {
    const auto id = this->RegisterRightEdgeRegion();
    QWidget source(m_Cell.get());
    source.setGeometry(100, 50, 290, 200);
    m_Proximity->AddEventSource(&source);
    this->SendMouse(&source, QEvent::MouseMove, m_NearPoint, Qt::NoButton, Qt::NoButton);
    CPPUNIT_ASSERT(State::Active == m_Proximity->GetRegionState(id));

    // The implicit grab keeps the source's moves coming outside the cell.
    this->SendMouse(&source, QEvent::MouseMove, QPoint(600, 150), Qt::NoButton, Qt::LeftButton);
    ProcessEventsFor(WaitPastCollapse);
    CPPUNIT_ASSERT_MESSAGE("A drag out of the cell counts as leaving it",
                           State::Idle == m_Proximity->GetRegionState(id));
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
