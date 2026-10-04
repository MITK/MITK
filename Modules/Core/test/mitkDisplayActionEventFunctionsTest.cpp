/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandlerSynchronized.h>
#include <mitkDisplayActionEvents.h>
#include <mitkImageGenerator.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindowProperty.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTimeNavigationController.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <cstdlib>

/**
 * Headless behavior tests for the predicate-scoped synchronized display
 * actions and for DisplayActionEventHandlerSynchronized's per-dimension
 * wiring.
 *
 * This suite doubles as the harness proof for driving the display-action
 * pipeline without any GUI or recorded interaction: renderers are plain
 * VtkPropRenderers on offscreen vtkRenderWindows registered with the global
 * RenderingManager, geometry comes from a generated image, and events are
 * constructed directly (action functions are called with a Display*Event;
 * handler wiring is exercised by invoking the event on a
 * DisplayActionEventBroadcast). Later synchronization-dimension work builds
 * its behavior tests on this same setup.
 */
class mitkDisplayActionEventFunctionsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDisplayActionEventFunctionsTestSuite);

  MITK_TEST(Scroll_PredicateScopesTargets);
  MITK_TEST(Scroll_UnadmittedSender_NoOp);
  MITK_TEST(Scroll_GroupedSingleSliceMember_DoesNotMoveTime);
  MITK_TEST(Scroll_DirectGestureOnSingleSliceWindow_MovesTime);
  MITK_TEST(Scroll_TargetWithOppositeInversion_MovesSameDisplayedDirection);
  MITK_TEST(Pan_PredicateScopesTargets);
  MITK_TEST(Zoom_PredicateScopesTargets);
  MITK_TEST(Crosshair_PredicateScopesTargets);
  MITK_TEST(LevelWindow_ForeignSender_NoOp);
  MITK_TEST(LevelWindow_UngroupedSender_WritesNodeGlobal);
  MITK_TEST(LevelWindow_GroupedSender_WritesRendererSpecificOnTargets);
  MITK_TEST(NullPredicate_Throws);
  MITK_TEST(Handler_NullPredicate_WiresSenderOnlyAction);
  MITK_TEST(Handler_DimensionsScopeIndependently);
  MITK_TEST(Handler_HalfLevelWindowPair_Throws);

  CPPUNIT_TEST_SUITE_END();

  struct Window
  {
    vtkRenderWindow* vtkWindow = nullptr;
    mitk::VtkPropRenderer::Pointer renderer;
  };

  static constexpr mitk::ScalarType InitialLevel = 100.0;
  static constexpr mitk::ScalarType InitialWindow = 50.0;
  static constexpr mitk::ScalarType LevelDelta = 10.0;
  static constexpr mitk::ScalarType WindowDelta = 20.0;

  mitk::Image::Pointer m_Image;
  Window m_A0; // "editorA__w0" - sender in most tests
  Window m_A1; // "editorA__w1" - same-editor member
  Window m_B0; // "editorB__w0" - foreign window

  // Level-window tests only (see AttachImageNode).
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node;

public:
  void setUp() override
  {
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    m_A0 = MakeWindow("editorA__w0");
    m_A1 = MakeWindow("editorA__w1");
    m_B0 = MakeWindow("editorB__w0");

    // Non-boundary baseline so both scroll directions stay in range.
    SetSlicePos(m_A0, 2);
    SetSlicePos(m_A1, 2);
    SetSlicePos(m_B0, 2);
  }

  void tearDown() override
  {
    DestroyWindow(m_A0);
    DestroyWindow(m_A1);
    DestroyWindow(m_B0);
    m_Node = nullptr;
    m_DataStorage = nullptr;
    m_Image = nullptr;
  }

  Window MakeWindow(const char* name, const mitk::Image* image = nullptr)
  {
    Window window;
    window.vtkWindow = vtkRenderWindow::New();
    window.renderer = mitk::VtkPropRenderer::New(name, window.vtkWindow);
    mitk::BaseRenderer::AddInstance(window.vtkWindow, window.renderer);
    mitk::RenderingManager::GetInstance()->AddRenderWindow(window.vtkWindow);
    mitk::RenderingManager::GetInstance()->InitializeView(
      window.vtkWindow, (nullptr != image ? image : m_Image.GetPointer())->GetTimeGeometry());
    return window;
  }

  static void DestroyWindow(Window& window)
  {
    if (nullptr == window.vtkWindow)
    {
      return;
    }
    mitk::RenderingManager::GetInstance()->RemoveRenderWindow(window.vtkWindow);
    mitk::BaseRenderer::RemoveInstance(window.vtkWindow);
    window.renderer = nullptr;
    window.vtkWindow->Delete();
    window.vtkWindow = nullptr;
  }

  static mitk::DisplayActionEventFunctions::TargetPredicate SameEditorPredicate(const std::string& editorPrefix)
  {
    return [editorPrefix](const mitk::BaseRenderer* sender, const mitk::BaseRenderer* target)
    {
      return 0 == std::string(sender->GetName()).rfind(editorPrefix, 0)
          && 0 == std::string(target->GetName()).rfind(editorPrefix, 0);
    };
  }

  static mitk::Stepper* SliceStepper(const Window& window)
  {
    auto* stepper = window.renderer->GetSliceNavigationController()->GetStepper();
    CPPUNIT_ASSERT_MESSAGE("Renderer must have a slice stepper", nullptr != stepper);
    CPPUNIT_ASSERT_MESSAGE("Slice stepper must cover multiple slices", stepper->GetSteps() > 4);
    return stepper;
  }

  static void SetSlicePos(const Window& window, unsigned int pos)
  {
    SliceStepper(window)->SetPos(pos);
  }

  static unsigned int SlicePos(const Window& window)
  {
    return SliceStepper(window)->GetPos();
  }

  static vtkCamera* Camera(const Window& window)
  {
    return window.renderer->GetVtkRenderer()->GetActiveCamera();
  }

  static mitk::DisplayActionEventFunctions::LevelWindowScopeClassifier ConstantScope(
    mitk::DisplayActionEventFunctions::LevelWindowScope scope)
  {
    return [scope](const mitk::BaseRenderer*) { return scope; };
  }

  /** Put the image into a data storage every window renders, with a
   *  node-global level/window for the gestures to change. */
  void AttachImageNode()
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Node = mitk::DataNode::New();
    m_Node->SetName("image");
    m_Node->SetData(m_Image);
    m_Node->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(InitialLevel, InitialWindow)));
    m_DataStorage->Add(m_Node);

    for (auto* window : { &m_A0, &m_A1, &m_B0 })
    {
      window->renderer->SetDataStorage(m_DataStorage);
    }
  }

  /** Run 'action' on a level-window gesture from 'sender', positioned over the
   *  image so the action resolves the image node under the pointer. */
  void FireLevelWindow(const mitk::StdFunctionCommand::ActionFunction& action, const Window& sender) const
  {
    // Display/world conversion needs a viewport with an extent.
    sender.vtkWindow->SetSize(64, 64);

    mitk::Point2D displayPoint;
    sender.renderer->WorldToDisplay(m_Image->GetGeometry()->GetCenter(), displayPoint);
    mitk::Point3D worldPoint;
    sender.renderer->DisplayToWorld(displayPoint, worldPoint);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the gesture position must lie inside the image",
                           m_Image->GetGeometry()->IsInside(worldPoint));

    auto interactionEvent = mitk::InteractionPositionEvent::New(sender.renderer, displayPoint);
    action(mitk::DisplaySetLevelWindowEvent(interactionEvent, LevelDelta, WindowDelta));
  }

  mitk::LevelWindow NodeGlobalLevelWindow() const
  {
    mitk::LevelWindow levelWindow;
    CPPUNIT_ASSERT_MESSAGE("The node must keep a node-global level/window", m_Node->GetLevelWindow(levelWindow));
    return levelWindow;
  }

  /** The renderer-specific level/window written for 'window', null if none. */
  const mitk::LevelWindowProperty* RendererLevelWindow(const Window& window) const
  {
    return dynamic_cast<const mitk::LevelWindowProperty*>(
      m_Node->GetPropertyList(window.renderer)->GetProperty("levelwindow"));
  }

  void Scroll_PredicateScopesTargets()
  {
    auto action = mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(
      SameEditorPredicate("editorA__"));

    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    action(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(m_A0));
    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(m_A1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreign window must not be scrolled", 2u, SlicePos(m_B0));
  }

  void Scroll_UnadmittedSender_NoOp()
  {
    auto action = mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(
      SameEditorPredicate("editorA__"));

    auto interactionEvent = mitk::InteractionEvent::New(m_B0.renderer);
    action(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unadmitted sender must not scroll anything", 2u, SlicePos(m_A0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unadmitted sender must not scroll anything", 2u, SlicePos(m_A1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unadmitted sender must not scroll anything - not even itself",
                                 2u, SlicePos(m_B0));
  }

  /** RAII wrapper so a mid-test assertion failure cannot leak an extra
   *  registered render window into subsequent tests. */
  struct ScopedWindow
  {
    mitkDisplayActionEventFunctionsTestSuite* suite;
    Window window;

    ScopedWindow(mitkDisplayActionEventFunctionsTestSuite* owner, const char* name, const mitk::Image* image)
      : suite(owner), window(owner->MakeWindow(name, image))
    {
    }
    ~ScopedWindow() { DestroyWindow(window); }
  };

  /** RAII backup of the global time stepper so tests can arm it with a
   *  multi-step range and always restore the previous state. */
  struct ScopedTimeStepper
  {
    mitk::Stepper* stepper;
    unsigned int steps;
    unsigned int pos;

    ScopedTimeStepper()
      : stepper(mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetStepper())
      , steps(stepper->GetSteps())
      , pos(stepper->GetPos())
    {
    }
    ~ScopedTimeStepper()
    {
      stepper->SetSteps(steps);
      stepper->SetPos(pos);
    }
  };

  void Scroll_GroupedSingleSliceMember_DoesNotMoveTime()
  {
    const auto singleSliceImage = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 1, 1.0f, 1.0f, 1.0f);
    const ScopedWindow a2(this, "editorA__w2", singleSliceImage);

    const ScopedTimeStepper timeStepper;
    timeStepper.stepper->SetSteps(5);
    timeStepper.stepper->SetPos(2);

    auto action = mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(
      SameEditorPredicate("editorA__"));

    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    action(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(m_A0));
    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(m_A1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A single-slice group member must not be scrolled at all",
      0u, a2.window.renderer->GetSliceNavigationController()->GetStepper()->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Grouped slice propagation must never leak into application-global time",
      2u, timeStepper.stepper->GetPos());
  }

  void Scroll_DirectGestureOnSingleSliceWindow_MovesTime()
  {
    const auto singleSliceImage = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 1, 1.0f, 1.0f, 1.0f);
    const ScopedWindow a2(this, "editorA__w2", singleSliceImage);

    const ScopedTimeStepper timeStepper;
    timeStepper.stepper->SetSteps(5);
    timeStepper.stepper->SetPos(2);

    auto action = mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(
      SameEditorPredicate("editorA__"));

    auto interactionEvent = mitk::InteractionEvent::New(a2.window.renderer);
    action(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A direct gesture on a single-slice window keeps its wheel-drives-time behavior",
      3u, timeStepper.stepper->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A time step moves no peer's slice", 2u, SlicePos(m_A0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A time step moves no peer's slice", 2u, SlicePos(m_A1));
  }

  static bool SliceInverted(const Window& window, const mitk::Image* image)
  {
    return mitk::SliceNavigationHelper::IsSliceIndexInverted(image->GetGeometry(),
      window.renderer->GetCurrentWorldGeometry(),
      window.renderer->GetSliceNavigationController()->GetViewDirection());
  }

  static int ShownSlice(const Window& window, const mitk::Image* image)
  {
    const auto* stepper = window.renderer->GetSliceNavigationController()->GetStepper();
    const int position = static_cast<int>(stepper->GetPos());
    return SliceInverted(window, image) ? static_cast<int>(stepper->GetSteps()) - 1 - position : position;
  }

  void Scroll_TargetWithOppositeInversion_MovesSameDisplayedDirection()
  {
    // Sagittal steps along the identity image's x index, axial against its z
    // index: one scroll must still move both the same displayed direction.
    m_A1.renderer->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Sagittal);
    mitk::RenderingManager::GetInstance()->InitializeView(m_A1.vtkWindow, m_Image->GetTimeGeometry());
    SetSlicePos(m_A1, 5);
    CPPUNIT_ASSERT_MESSAGE("Fixture: sender and target have opposite inversion",
                           SliceInverted(m_A0, m_Image) != SliceInverted(m_A1, m_Image));

    const int senderBefore = ShownSlice(m_A0, m_Image);
    const int targetBefore = ShownSlice(m_A1, m_Image);

    auto action = mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(
      SameEditorPredicate("editorA__"));
    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    action(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    const int senderDelta = ShownSlice(m_A0, m_Image) - senderBefore;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: the sender moved one slice", 1, std::abs(senderDelta));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The target moves the same displayed direction as the sender",
                                 senderDelta, ShownSlice(m_A1, m_Image) - targetBefore);
  }

  void Pan_PredicateScopesTargets()
  {
    auto action = mitk::DisplayActionEventFunctions::MoveCameraSynchronizedAction(
      SameEditorPredicate("editorA__"));

    double a0Before[3];
    double a1Before[3];
    double b0Before[3];
    Camera(m_A0)->GetPosition(a0Before);
    Camera(m_A1)->GetPosition(a1Before);
    Camera(m_B0)->GetPosition(b0Before);

    mitk::Vector2D moveVector;
    moveVector[0] = 5.0;
    moveVector[1] = 5.0;
    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    action(mitk::DisplayMoveEvent(interactionEvent, moveVector));

    CPPUNIT_ASSERT_MESSAGE("Pan must move the sender's camera", Moved(a0Before, Camera(m_A0)));
    CPPUNIT_ASSERT_MESSAGE("Pan must move the member's camera", Moved(a1Before, Camera(m_A1)));
    CPPUNIT_ASSERT_MESSAGE("Pan must not move the foreign camera", !Moved(b0Before, Camera(m_B0)));
  }

  void Zoom_PredicateScopesTargets()
  {
    auto action = mitk::DisplayActionEventFunctions::ZoomCameraSynchronizedAction(
      SameEditorPredicate("editorA__"));

    const double a0Before = Camera(m_A0)->GetParallelScale();
    const double a1Before = Camera(m_A1)->GetParallelScale();
    const double b0Before = Camera(m_B0)->GetParallelScale();

    mitk::Point2D startCoordinate;
    startCoordinate.Fill(8.0);
    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    action(mitk::DisplayZoomEvent(interactionEvent, 2.0f, startCoordinate));

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must halve the sender's parallel scale",
      a0Before / 2.0, Camera(m_A0)->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must halve the member's parallel scale",
      a1Before / 2.0, Camera(m_A1)->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must not change the foreign parallel scale",
      b0Before, Camera(m_B0)->GetParallelScale(), 1e-6);
  }

  void Crosshair_PredicateScopesTargets()
  {
    auto action = mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction(
      SameEditorPredicate("editorA__"));

    mitk::Point3D position;
    position[0] = 8.0;
    position[1] = 8.0;
    position[2] = 6.0;
    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    action(mitk::DisplaySetCrosshairEvent(interactionEvent, position));

    const auto a0Pos = SlicePos(m_A0);
    const auto a1Pos = SlicePos(m_A1);
    CPPUNIT_ASSERT_MESSAGE("Crosshair must move the sender off the baseline slice", 2u != a0Pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Crosshair must move members to the same slice", a0Pos, a1Pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Crosshair must not move the foreign window's slice",
                                 2u, SlicePos(m_B0));
  }

  void LevelWindow_ForeignSender_NoOp()
  {
    using mitk::DisplayActionEventFunctions::LevelWindowScope;
    this->AttachImageNode();
    auto action = mitk::DisplayActionEventFunctions::SetLevelWindowSynchronizedAction(
      ConstantScope(LevelWindowScope::Foreign), SameEditorPredicate("editorA__"));

    this->FireLevelWindow(action, m_B0);

    const auto levelWindow = this->NodeGlobalLevelWindow();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A foreign sender must not change the node-global level",
      InitialLevel, levelWindow.GetLevel(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A foreign sender must not change the node-global window",
      InitialWindow, levelWindow.GetWindow(), 1e-6);
    for (const auto* window : { &m_A0, &m_A1, &m_B0 })
    {
      CPPUNIT_ASSERT_MESSAGE("A foreign sender must not write a renderer-specific level/window",
                             nullptr == this->RendererLevelWindow(*window));
    }
  }

  void LevelWindow_UngroupedSender_WritesNodeGlobal()
  {
    using mitk::DisplayActionEventFunctions::LevelWindowScope;
    this->AttachImageNode();
    auto action = mitk::DisplayActionEventFunctions::SetLevelWindowSynchronizedAction(
      ConstantScope(LevelWindowScope::Ungrouped), SameEditorPredicate("editorA__"));

    this->FireLevelWindow(action, m_A0);

    const auto levelWindow = this->NodeGlobalLevelWindow();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("An ungrouped sender must shift the node-global level",
      InitialLevel + LevelDelta, levelWindow.GetLevel(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("An ungrouped sender must shift the node-global window",
      InitialWindow + WindowDelta, levelWindow.GetWindow(), 1e-6);
    for (const auto* window : { &m_A0, &m_A1, &m_B0 })
    {
      CPPUNIT_ASSERT_MESSAGE("An ungrouped sender must not write a renderer-specific level/window",
                             nullptr == this->RendererLevelWindow(*window));
    }
  }

  void LevelWindow_GroupedSender_WritesRendererSpecificOnTargets()
  {
    using mitk::DisplayActionEventFunctions::LevelWindowScope;
    this->AttachImageNode();
    auto action = mitk::DisplayActionEventFunctions::SetLevelWindowSynchronizedAction(
      ConstantScope(LevelWindowScope::Grouped), SameEditorPredicate("editorA__"));

    this->FireLevelWindow(action, m_A0);

    for (const auto* window : { &m_A0, &m_A1 })
    {
      const auto* property = this->RendererLevelWindow(*window);
      CPPUNIT_ASSERT_MESSAGE("A grouped sender must write a renderer-specific level/window on every target",
                             nullptr != property);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(InitialLevel + LevelDelta, property->GetLevelWindow().GetLevel(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(InitialWindow + WindowDelta, property->GetLevelWindow().GetWindow(), 1e-6);
    }
    CPPUNIT_ASSERT_MESSAGE("A grouped sender must not write on a window its predicate rejects",
                           nullptr == this->RendererLevelWindow(m_B0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A grouped sender must leave the node-global level alone",
      InitialLevel, this->NodeGlobalLevelWindow().GetLevel(), 1e-6);
  }

  void NullPredicate_Throws()
  {
    using mitk::DisplayActionEventFunctions::LevelWindowScope;
    const mitk::DisplayActionEventFunctions::TargetPredicate nullPredicate;

    CPPUNIT_ASSERT_THROW(
      mitk::DisplayActionEventFunctions::MoveCameraSynchronizedAction(nullPredicate), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      mitk::DisplayActionEventFunctions::ZoomCameraSynchronizedAction(nullPredicate), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(nullPredicate), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction(
        mitk::DisplayActionEventFunctions::TargetPredicate()), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      mitk::DisplayActionEventFunctions::SetLevelWindowSynchronizedAction(
        ConstantScope(LevelWindowScope::Grouped), nullPredicate), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      mitk::DisplayActionEventFunctions::SetLevelWindowSynchronizedAction(
        mitk::DisplayActionEventFunctions::LevelWindowScopeClassifier(), SameEditorPredicate("editorA__")),
      mitk::Exception);
  }

  void Handler_NullPredicate_WiresSenderOnlyAction()
  {
    // Slice deliberately not synchronized (null); the other dimensions are.
    mitk::DisplayActionEventHandlerSynchronized::Predicates predicates;
    predicates.pan = SameEditorPredicate("editorA__");
    predicates.zoom = SameEditorPredicate("editorA__");
    predicates.crosshair = SameEditorPredicate("editorA__");

    auto broadcast = mitk::DisplayActionEventBroadcast::New();
    mitk::DisplayActionEventHandlerSynchronized handler(predicates);
    handler.SetObservableBroadcast(broadcast);
    handler.InitActions("editorA__");

    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    broadcast->InvokeEvent(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sender-only fallback must scroll the sender", 3u, SlicePos(m_A0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unsynchronized dimension must not propagate", 2u, SlicePos(m_A1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unsynchronized dimension must not propagate", 2u, SlicePos(m_B0));
  }

  void Handler_DimensionsScopeIndependently()
  {
    // Slice couples the two editorA windows; zoom couples nothing (null).
    mitk::DisplayActionEventHandlerSynchronized::Predicates predicates;
    predicates.slice = SameEditorPredicate("editorA__");

    auto broadcast = mitk::DisplayActionEventBroadcast::New();
    mitk::DisplayActionEventHandlerSynchronized handler(predicates);
    handler.SetObservableBroadcast(broadcast);
    handler.InitActions("editorA__");

    const double a1ScaleBefore = Camera(m_A1)->GetParallelScale();

    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    broadcast->InvokeEvent(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    mitk::Point2D startCoordinate;
    startCoordinate.Fill(8.0);
    broadcast->InvokeEvent(mitk::DisplayZoomEvent(interactionEvent, 2.0f, startCoordinate));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Synchronized slice dimension must propagate", 3u, SlicePos(m_A1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unsynchronized zoom dimension must not propagate",
      a1ScaleBefore, Camera(m_A1)->GetParallelScale(), 1e-6);
  }

  void Handler_HalfLevelWindowPair_Throws()
  {
    using mitk::DisplayActionEventFunctions::LevelWindowScope;

    mitk::DisplayActionEventHandlerSynchronized::Predicates targetsOnly;
    targetsOnly.levelWindow = SameEditorPredicate("editorA__");
    mitk::DisplayActionEventHandlerSynchronized::Predicates scopeOnly;
    scopeOnly.levelWindowScope = ConstantScope(LevelWindowScope::Grouped);

    CPPUNIT_ASSERT_THROW(mitk::DisplayActionEventHandlerSynchronized{ targetsOnly }, mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::DisplayActionEventHandlerSynchronized{ scopeOnly }, mitk::Exception);

    mitk::DisplayActionEventHandlerSynchronized handler(mitk::DisplayActionEventHandlerSynchronized::Predicates{});
    CPPUNIT_ASSERT_THROW(handler.SetPredicates(targetsOnly), mitk::Exception);
    CPPUNIT_ASSERT_THROW(handler.SetPredicates(scopeOnly), mitk::Exception);
  }

private:
  static bool Moved(const double (&before)[3], vtkCamera* camera)
  {
    double after[3];
    camera->GetPosition(after);
    const double dx = after[0] - before[0];
    const double dy = after[1] - before[1];
    const double dz = after[2] - before[2];
    return (dx * dx + dy * dy + dz * dz) > 1e-6;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDisplayActionEventFunctions)
