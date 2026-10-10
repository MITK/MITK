/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEvents.h>
#include <mitkImageGenerator.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindowProperty.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkVtkPropRenderer.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

/**
 * Tests the scope of QmitkMxNMultiWidget::Synchronize(bool):
 *
 *   - Synchronize(true) couples pan / zoom / slice / crosshair across this
 *     editor's cells (the degenerate one-group-per-dimension case).
 *   - The coupling is editor-scoped: render windows that do not belong to
 *     this editor are neither driven by the editor's gestures nor able to
 *     drive the editor's cells.
 *   - Synchronize(false) restores sender-only behavior.
 *   - Two MxN widgets whose names are prefixes of each other ('mxn' and
 *     'mxn2') stay apart: no gesture in one reaches the other's cells.
 *
 * The display-action pipeline is driven synthetically: the multi widget's
 * DisplayActionEventBroadcast is an itk::Object holding the handler's
 * observers, so invoking a Display*Event on it (with a chosen sender
 * renderer) exercises exactly the code path a real mouse gesture would.
 */
class QmitkMxNSynchronizeScopeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNSynchronizeScopeTestSuite);

  MITK_TEST(Slice_Synchronized_CouplesEditorCells);
  MITK_TEST(Slice_Synchronized_LeavesForeignWindowUntouched);
  MITK_TEST(Slice_ForeignSender_DoesNotDriveEditorCells);
  MITK_TEST(Pan_Synchronized_CouplesEditorCells_NotForeign);
  MITK_TEST(Zoom_Synchronized_CouplesEditorCells_NotForeign);
  MITK_TEST(Crosshair_Synchronized_CouplesEditorCells_NotForeign);
  MITK_TEST(LevelWindow_ForeignSender_DoesNotChangeTheImage);
  MITK_TEST(Desynchronized_ScrollMovesOnlySender);
  MITK_TEST(TwoWidgets_Slice_DoesNotCrossOver);
  MITK_TEST(TwoWidgets_Pan_DoesNotCrossOver);
  MITK_TEST(TwoWidgets_Zoom_DoesNotCrossOver);
  MITK_TEST(TwoWidgets_Crosshair_DoesNotCrossOver);
  MITK_TEST(TwoWidgets_LevelWindow_DoesNotCrossOver);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;

  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

  // A second MxN widget whose name extends the first one's, so a scope check
  // that compared names without the delimiter would mistake one for the other.
  std::unique_ptr<QmitkMxNMultiWidget> m_Second;

  vtkRenderWindow* m_ForeignVtkWindow = nullptr;
  mitk::VtkPropRenderer::Pointer m_ForeignRenderer;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 2); // cells: mxn__widget0, mxn__widget1

    m_Second = std::make_unique<QmitkMxNMultiWidget>(nullptr, Qt::WindowFlags(), QStringLiteral("mxn2"));
    m_Second->SetDataStorage(m_DataStorage);
    m_Second->InitializeMultiWidget();
    m_Second->SetLayout(1, 2); // cells: mxn2__widget0, mxn2__widget1

    // A render window that is registered with the same (global) rendering
    // manager but does not belong to the editor - stand-in for another
    // editor's window in the same application.
    m_ForeignVtkWindow = vtkRenderWindow::New();
    m_ForeignRenderer = mitk::VtkPropRenderer::New("foreign__w0", m_ForeignVtkWindow);
    mitk::BaseRenderer::AddInstance(m_ForeignVtkWindow, m_ForeignRenderer);
    mitk::RenderingManager::GetInstance()->AddRenderWindow(m_ForeignVtkWindow);

    // Give every window a valid slice geometry so steppers and cameras
    // carry real state.
    for (const auto* widget : { m_Editor.get(), m_Second.get() })
    {
      for (const auto& [name, cell] : widget->GetRenderWindowWidgets())
      {
        mitk::RenderingManager::GetInstance()->InitializeView(
          cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
      }
    }
    mitk::RenderingManager::GetInstance()->InitializeView(m_ForeignVtkWindow, m_Image->GetTimeGeometry());

    // Non-boundary baseline so both scroll directions stay in range.
    this->SetSlicePos(this->CellRenderer(0), 2);
    this->SetSlicePos(this->CellRenderer(1), 2);
    this->SetSlicePos(this->SecondRenderer(0), 2);
    this->SetSlicePos(this->SecondRenderer(1), 2);
    this->SetSlicePos(m_ForeignRenderer, 2);
  }

  void tearDown() override
  {
    m_Second.reset();
    m_Editor.reset();

    mitk::RenderingManager::GetInstance()->RemoveRenderWindow(m_ForeignVtkWindow);
    mitk::BaseRenderer::RemoveInstance(m_ForeignVtkWindow);
    m_ForeignRenderer = nullptr;
    m_ForeignVtkWindow->Delete();
    m_ForeignVtkWindow = nullptr;

    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  static mitk::BaseRenderer* RendererOf(const QmitkMxNMultiWidget& widget, std::size_t index)
  {
    const auto name = widget.GetMultiWidgetName() + QStringLiteral("__widget") + QString::number(index);
    const auto cell = widget.GetRenderWindowWidget(name);
    CPPUNIT_ASSERT_MESSAGE("Editor cell must exist: " + name.toStdString(), nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
  }

  mitk::BaseRenderer* CellRenderer(std::size_t index) const
  {
    return this->RendererOf(*m_Editor, index);
  }

  mitk::BaseRenderer* SecondRenderer(std::size_t index) const
  {
    return this->RendererOf(*m_Second, index);
  }

  static mitk::Stepper* SliceStepper(mitk::BaseRenderer* renderer)
  {
    auto* stepper = renderer->GetSliceNavigationController()->GetStepper();
    CPPUNIT_ASSERT_MESSAGE("Renderer must have a slice stepper", nullptr != stepper);
    CPPUNIT_ASSERT_MESSAGE("Slice stepper must cover multiple slices", stepper->GetSteps() > 4);
    return stepper;
  }

  static void SetSlicePos(mitk::BaseRenderer* renderer, unsigned int pos)
  {
    SliceStepper(renderer)->SetPos(pos);
  }

  static unsigned int SlicePos(mitk::BaseRenderer* renderer)
  {
    return SliceStepper(renderer)->GetPos();
  }

  static vtkCamera* Camera(mitk::BaseRenderer* renderer)
  {
    return renderer->GetVtkRenderer()->GetActiveCamera();
  }

  /** Fire a display action event on the editor's broadcast, as if the given
   *  renderer had produced the gesture. The interaction event must outlive
   *  the invocation (the display event stores a raw pointer to it). */
  template <typename TDisplayEvent, typename... TArgs>
  void Fire(mitk::BaseRenderer* sender, TArgs&&... args)
  {
    auto interactionEvent = mitk::InteractionEvent::New(sender);
    m_Editor->GetInteractionEventHandler()->InvokeEvent(
      TDisplayEvent(interactionEvent, std::forward<TArgs>(args)...));
  }

  /** Fire a display action event on both widgets' broadcasts, as the
   *  application does: every broadcast observes every interaction event. */
  template <typename TDisplayEvent, typename... TArgs>
  void FireOnBoth(mitk::BaseRenderer* sender, TArgs&&... args)
  {
    auto interactionEvent = mitk::InteractionEvent::New(sender);
    for (auto* widget : { m_Editor.get(), m_Second.get() })
    {
      widget->GetInteractionEventHandler()->InvokeEvent(TDisplayEvent(interactionEvent, args...));
    }
  }

  void Slice_Synchronized_CouplesEditorCells()
  {
    m_Editor->Synchronize(true);

    this->Fire<mitk::DisplayScrollEvent>(this->CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL(3u, this->SlicePos(this->CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL(3u, this->SlicePos(this->CellRenderer(1)));
  }

  void Slice_Synchronized_LeavesForeignWindowUntouched()
  {
    m_Editor->Synchronize(true);

    this->Fire<mitk::DisplayScrollEvent>(this->CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A synchronized MxN editor must not scroll render windows of other editors",
      2u, this->SlicePos(m_ForeignRenderer));
  }

  void Slice_ForeignSender_DoesNotDriveEditorCells()
  {
    m_Editor->Synchronize(true);

    this->Fire<mitk::DisplayScrollEvent>(m_ForeignRenderer, 1, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A foreign window's gesture must not scroll a synchronized MxN editor's cells",
      2u, this->SlicePos(this->CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A foreign window's gesture must not scroll a synchronized MxN editor's cells",
      2u, this->SlicePos(this->CellRenderer(1)));
  }

  void Pan_Synchronized_CouplesEditorCells_NotForeign()
  {
    m_Editor->Synchronize(true);

    double cell0Before[3];
    double cell1Before[3];
    double foreignBefore[3];
    this->Camera(this->CellRenderer(0))->GetPosition(cell0Before);
    this->Camera(this->CellRenderer(1))->GetPosition(cell1Before);
    this->Camera(m_ForeignRenderer)->GetPosition(foreignBefore);

    mitk::Vector2D moveVector;
    moveVector[0] = 5.0;
    moveVector[1] = 5.0;
    this->Fire<mitk::DisplayMoveEvent>(this->CellRenderer(0), moveVector);

    CPPUNIT_ASSERT_MESSAGE("Pan must move the sending cell's camera",
                           this->Moved(cell0Before, this->Camera(this->CellRenderer(0))));
    CPPUNIT_ASSERT_MESSAGE("Pan must move the synchronized sibling cell's camera",
                           this->Moved(cell1Before, this->Camera(this->CellRenderer(1))));
    CPPUNIT_ASSERT_MESSAGE("Pan must not move a foreign window's camera",
                           !this->Moved(foreignBefore, this->Camera(m_ForeignRenderer)));
  }

  void Zoom_Synchronized_CouplesEditorCells_NotForeign()
  {
    m_Editor->Synchronize(true);

    const double cell0Before = this->Camera(this->CellRenderer(0))->GetParallelScale();
    const double cell1Before = this->Camera(this->CellRenderer(1))->GetParallelScale();
    const double foreignBefore = this->Camera(m_ForeignRenderer)->GetParallelScale();

    mitk::Point2D startCoordinate;
    startCoordinate.Fill(8.0);
    this->Fire<mitk::DisplayZoomEvent>(this->CellRenderer(0), 2.0f, startCoordinate);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must halve the sending cell's parallel scale",
      cell0Before / 2.0, this->Camera(this->CellRenderer(0))->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must halve the synchronized sibling cell's parallel scale",
      cell1Before / 2.0, this->Camera(this->CellRenderer(1))->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must not change a foreign window's parallel scale",
      foreignBefore, this->Camera(m_ForeignRenderer)->GetParallelScale(), 1e-6);
  }

  void Crosshair_Synchronized_CouplesEditorCells_NotForeign()
  {
    m_Editor->Synchronize(true);

    mitk::Point3D position;
    position[0] = 8.0;
    position[1] = 8.0;
    position[2] = 6.0;
    this->Fire<mitk::DisplaySetCrosshairEvent>(this->CellRenderer(0), position);

    const auto cell0Pos = this->SlicePos(this->CellRenderer(0));
    const auto cell1Pos = this->SlicePos(this->CellRenderer(1));
    CPPUNIT_ASSERT_MESSAGE("Crosshair must move the sending cell off the baseline slice",
                           2u != cell0Pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Crosshair must move synchronized cells to the same slice",
                                 cell0Pos, cell1Pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Crosshair must not move a foreign window's slice",
                                 2u, this->SlicePos(m_ForeignRenderer));
  }

  void LevelWindow_ForeignSender_DoesNotChangeTheImage()
  {
    // The image lies under the foreign window's pointer and carries a
    // node-global level/window, so a handler that admitted the foreign
    // gesture would shift it.
    auto node = mitk::DataNode::New();
    node->SetName("image");
    node->SetData(m_Image);
    node->SetIntProperty("layer", 0);
    node->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 50.0)));
    m_DataStorage->Add(node);
    m_ForeignRenderer->SetDataStorage(m_DataStorage);

    // Display/world conversion needs a viewport with an extent.
    m_ForeignVtkWindow->SetSize(64, 64);
    mitk::Point2D displayPoint;
    m_ForeignRenderer->WorldToDisplay(m_Image->GetGeometry()->GetCenter(), displayPoint);
    mitk::Point3D worldPoint;
    m_ForeignRenderer->DisplayToWorld(displayPoint, worldPoint);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the gesture position must lie inside the image",
                           m_Image->GetGeometry()->IsInside(worldPoint));

    auto interactionEvent = mitk::InteractionPositionEvent::New(m_ForeignRenderer, displayPoint);
    m_Editor->GetInteractionEventHandler()->InvokeEvent(
      mitk::DisplaySetLevelWindowEvent(interactionEvent, 10.0, 20.0));

    mitk::LevelWindow levelWindow;
    node->GetLevelWindow(levelWindow);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
      "A foreign window's level/window gesture must not reach an MxN editor's handler",
      100.0, levelWindow.GetLevel(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, levelWindow.GetWindow(), 1e-6);
  }

  void Desynchronized_ScrollMovesOnlySender()
  {
    m_Editor->Synchronize(false);

    this->Fire<mitk::DisplayScrollEvent>(this->CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL(3u, this->SlicePos(this->CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Desynchronized cells must not follow a sibling's scroll",
                                 2u, this->SlicePos(this->CellRenderer(1)));
    CPPUNIT_ASSERT_EQUAL(2u, this->SlicePos(m_ForeignRenderer));
  }

  void TwoWidgets_Slice_DoesNotCrossOver()
  {
    m_Editor->Synchronize(true);
    m_Second->Synchronize(true);

    this->FireOnBoth<mitk::DisplayScrollEvent>(this->CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: the gesture must scroll its own widget's linked cell",
                                 3u, this->SlicePos(this->CellRenderer(1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A gesture in 'mxn' must not scroll the cells of 'mxn2'",
                                 2u, this->SlicePos(this->SecondRenderer(0)));
    CPPUNIT_ASSERT_EQUAL(2u, this->SlicePos(this->SecondRenderer(1)));

    this->FireOnBoth<mitk::DisplayScrollEvent>(this->SecondRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL(3u, this->SlicePos(this->SecondRenderer(1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A gesture in 'mxn2' must not scroll the cells of 'mxn'",
                                 3u, this->SlicePos(this->CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL(3u, this->SlicePos(this->CellRenderer(1)));
  }

  void TwoWidgets_Pan_DoesNotCrossOver()
  {
    m_Editor->Synchronize(true);
    m_Second->Synchronize(true);

    double ownBefore[3];
    double secondBefore[3];
    this->Camera(this->CellRenderer(1))->GetPosition(ownBefore);
    this->Camera(this->SecondRenderer(0))->GetPosition(secondBefore);

    mitk::Vector2D moveVector;
    moveVector[0] = 5.0;
    moveVector[1] = 5.0;
    this->FireOnBoth<mitk::DisplayMoveEvent>(this->CellRenderer(0), moveVector);

    CPPUNIT_ASSERT_MESSAGE("Fixture: the gesture must pan its own widget's linked cell",
                           this->Moved(ownBefore, this->Camera(this->CellRenderer(1))));
    CPPUNIT_ASSERT_MESSAGE("A pan in 'mxn' must not move the cells of 'mxn2'",
                           !this->Moved(secondBefore, this->Camera(this->SecondRenderer(0))));

    double firstBefore[3];
    this->Camera(this->CellRenderer(0))->GetPosition(firstBefore);
    this->FireOnBoth<mitk::DisplayMoveEvent>(this->SecondRenderer(0), moveVector);

    CPPUNIT_ASSERT_MESSAGE("A pan in 'mxn2' must not move the cells of 'mxn'",
                           !this->Moved(firstBefore, this->Camera(this->CellRenderer(0))));
  }

  void TwoWidgets_Zoom_DoesNotCrossOver()
  {
    m_Editor->Synchronize(true);
    m_Second->Synchronize(true);

    const double ownBefore = this->Camera(this->CellRenderer(1))->GetParallelScale();
    const double secondBefore = this->Camera(this->SecondRenderer(0))->GetParallelScale();

    mitk::Point2D startCoordinate;
    startCoordinate.Fill(8.0);
    this->FireOnBoth<mitk::DisplayZoomEvent>(this->CellRenderer(0), 2.0f, startCoordinate);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Fixture: the gesture must zoom its own widget's linked cell",
      ownBefore / 2.0, this->Camera(this->CellRenderer(1))->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A zoom in 'mxn' must not change the cells of 'mxn2'",
      secondBefore, this->Camera(this->SecondRenderer(0))->GetParallelScale(), 1e-6);

    const double firstBefore = this->Camera(this->CellRenderer(0))->GetParallelScale();
    this->FireOnBoth<mitk::DisplayZoomEvent>(this->SecondRenderer(0), 2.0f, startCoordinate);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A zoom in 'mxn2' must not change the cells of 'mxn'",
      firstBefore, this->Camera(this->CellRenderer(0))->GetParallelScale(), 1e-6);
  }

  void TwoWidgets_Crosshair_DoesNotCrossOver()
  {
    m_Editor->Synchronize(true);
    m_Second->Synchronize(true);

    mitk::Point3D position;
    position[0] = 8.0;
    position[1] = 8.0;
    position[2] = 6.0;
    this->FireOnBoth<mitk::DisplaySetCrosshairEvent>(this->CellRenderer(0), position);

    CPPUNIT_ASSERT_MESSAGE("Fixture: the crosshair must move its own widget's linked cell",
                           2u != this->SlicePos(this->CellRenderer(1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A crosshair move in 'mxn' must not move the cells of 'mxn2'",
                                 2u, this->SlicePos(this->SecondRenderer(0)));
    CPPUNIT_ASSERT_EQUAL(2u, this->SlicePos(this->SecondRenderer(1)));

    const auto firstPos = this->SlicePos(this->CellRenderer(0));
    position[2] = 1.0;
    this->FireOnBoth<mitk::DisplaySetCrosshairEvent>(this->SecondRenderer(0), position);

    CPPUNIT_ASSERT_MESSAGE("Fixture: the crosshair must move its own widget's cell",
                           2u != this->SlicePos(this->SecondRenderer(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A crosshair move in 'mxn2' must not move the cells of 'mxn'",
                                 firstPos, this->SlicePos(this->CellRenderer(0)));
  }

  void TwoWidgets_LevelWindow_DoesNotCrossOver()
  {
    auto node = mitk::DataNode::New();
    node->SetName("image");
    node->SetData(m_Image);
    node->SetIntProperty("layer", 0);
    node->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 50.0)));
    m_DataStorage->Add(node);

    // Display/world conversion needs a viewport with an extent, so the gesture
    // position hits the image in the sending cell.
    auto* sender = this->SecondRenderer(0);
    sender->GetRenderWindow()->SetSize(64, 64);
    mitk::Point2D displayPoint;
    sender->WorldToDisplay(m_Image->GetGeometry()->GetCenter(), displayPoint);

    auto interactionEvent = mitk::InteractionPositionEvent::New(sender, displayPoint);
    for (auto* widget : { m_Editor.get(), m_Second.get() })
    {
      widget->GetInteractionEventHandler()->InvokeEvent(
        mitk::DisplaySetLevelWindowEvent(interactionEvent, 10.0, 20.0));
    }

    mitk::LevelWindow senderLevelWindow;
    node->GetLevelWindow(senderLevelWindow, sender);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the gesture must change the level/window its own cell shows",
                           std::abs(senderLevelWindow.GetLevel() - 100.0) > 1e-6);

    for (const std::size_t index : { std::size_t{0}, std::size_t{1} })
    {
      mitk::LevelWindow levelWindow;
      node->GetLevelWindow(levelWindow, this->CellRenderer(index));
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
        "A level/window gesture in 'mxn2' must not change what the cells of 'mxn' show",
        100.0, levelWindow.GetLevel(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, levelWindow.GetWindow(), 1e-6);
    }
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

MITK_TEST_SUITE_REGISTRATION(QmitkMxNSynchronizeScope)
