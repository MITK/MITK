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
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStepper.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

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
  MITK_TEST(Pan_PredicateScopesTargets);
  MITK_TEST(Zoom_PredicateScopesTargets);
  MITK_TEST(Crosshair_PredicateScopesTargets);
  MITK_TEST(NullPredicate_Throws);
  MITK_TEST(Handler_NullPredicate_WiresSenderOnlyAction);
  MITK_TEST(Handler_DimensionsScopeIndependently);

  CPPUNIT_TEST_SUITE_END();

  struct Window
  {
    vtkRenderWindow* vtkWindow = nullptr;
    mitk::VtkPropRenderer::Pointer renderer;
  };

  mitk::Image::Pointer m_Image;
  Window m_A0; // "editorA__w0" - sender in most tests
  Window m_A1; // "editorA__w1" - same-editor member
  Window m_B0; // "editorB__w0" - foreign window

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
    m_Image = nullptr;
  }

  Window MakeWindow(const char* name)
  {
    Window window;
    window.vtkWindow = vtkRenderWindow::New();
    window.renderer = mitk::VtkPropRenderer::New(name, window.vtkWindow);
    mitk::BaseRenderer::AddInstance(window.vtkWindow, window.renderer);
    mitk::RenderingManager::GetInstance()->AddRenderWindow(window.vtkWindow);
    mitk::RenderingManager::GetInstance()->InitializeView(window.vtkWindow, m_Image->GetTimeGeometry());
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

  void NullPredicate_Throws()
  {
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
  }

  void Handler_NullPredicate_WiresSenderOnlyAction()
  {
    auto broadcast = mitk::DisplayActionEventBroadcast::New();
    mitk::DisplayActionEventHandlerSynchronized handler;
    handler.SetObservableBroadcast(broadcast);

    // Slice deliberately not synchronized (null); the other dimensions are.
    mitk::DisplayActionEventHandlerSynchronized::Predicates predicates;
    predicates.pan = SameEditorPredicate("editorA__");
    predicates.zoom = SameEditorPredicate("editorA__");
    predicates.crosshair = SameEditorPredicate("editorA__");
    handler.SetPredicates(predicates);
    handler.InitActions("editorA__");

    auto interactionEvent = mitk::InteractionEvent::New(m_A0.renderer);
    broadcast->InvokeEvent(mitk::DisplayScrollEvent(interactionEvent, 1, false));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sender-only fallback must scroll the sender", 3u, SlicePos(m_A0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unsynchronized dimension must not propagate", 2u, SlicePos(m_A1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unsynchronized dimension must not propagate", 2u, SlicePos(m_B0));
  }

  void Handler_DimensionsScopeIndependently()
  {
    auto broadcast = mitk::DisplayActionEventBroadcast::New();
    mitk::DisplayActionEventHandlerSynchronized handler;
    handler.SetObservableBroadcast(broadcast);

    // Slice couples the two editorA windows; zoom couples nothing (null).
    mitk::DisplayActionEventHandlerSynchronized::Predicates predicates;
    predicates.slice = SameEditorPredicate("editorA__");
    handler.SetPredicates(predicates);
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
