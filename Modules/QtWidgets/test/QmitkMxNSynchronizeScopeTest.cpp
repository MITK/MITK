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
  MITK_TEST(Desynchronized_ScrollMovesOnlySender);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;

  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

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

    // A render window that is registered with the same (global) rendering
    // manager but does not belong to the editor - stand-in for another
    // editor's window in the same application.
    m_ForeignVtkWindow = vtkRenderWindow::New();
    m_ForeignRenderer = mitk::VtkPropRenderer::New("foreign__w0", m_ForeignVtkWindow);
    mitk::BaseRenderer::AddInstance(m_ForeignVtkWindow, m_ForeignRenderer);
    mitk::RenderingManager::GetInstance()->AddRenderWindow(m_ForeignVtkWindow);

    // Give every window a valid slice geometry so steppers and cameras
    // carry real state.
    for (const auto& [name, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }
    mitk::RenderingManager::GetInstance()->InitializeView(m_ForeignVtkWindow, m_Image->GetTimeGeometry());

    // Non-boundary baseline so both scroll directions stay in range.
    SetSlicePos(CellRenderer(0), 2);
    SetSlicePos(CellRenderer(1), 2);
    SetSlicePos(m_ForeignRenderer, 2);
  }

  void tearDown() override
  {
    m_Editor.reset();

    mitk::RenderingManager::GetInstance()->RemoveRenderWindow(m_ForeignVtkWindow);
    mitk::BaseRenderer::RemoveInstance(m_ForeignVtkWindow);
    m_ForeignRenderer = nullptr;
    m_ForeignVtkWindow->Delete();
    m_ForeignVtkWindow = nullptr;

    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  mitk::BaseRenderer* CellRenderer(std::size_t index) const
  {
    const auto name = QStringLiteral("mxn__widget") + QString::number(index);
    const auto cell = m_Editor->GetRenderWindowWidget(name);
    CPPUNIT_ASSERT_MESSAGE("Editor cell must exist: " + name.toStdString(), nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
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

  void Slice_Synchronized_CouplesEditorCells()
  {
    m_Editor->Synchronize(true);

    Fire<mitk::DisplayScrollEvent>(CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(CellRenderer(1)));
  }

  void Slice_Synchronized_LeavesForeignWindowUntouched()
  {
    m_Editor->Synchronize(true);

    Fire<mitk::DisplayScrollEvent>(CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A synchronized MxN editor must not scroll render windows of other editors",
      2u, SlicePos(m_ForeignRenderer));
  }

  void Slice_ForeignSender_DoesNotDriveEditorCells()
  {
    m_Editor->Synchronize(true);

    Fire<mitk::DisplayScrollEvent>(m_ForeignRenderer, 1, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A foreign window's gesture must not scroll a synchronized MxN editor's cells",
      2u, SlicePos(CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A foreign window's gesture must not scroll a synchronized MxN editor's cells",
      2u, SlicePos(CellRenderer(1)));
  }

  void Pan_Synchronized_CouplesEditorCells_NotForeign()
  {
    m_Editor->Synchronize(true);

    double cell0Before[3];
    double cell1Before[3];
    double foreignBefore[3];
    Camera(CellRenderer(0))->GetPosition(cell0Before);
    Camera(CellRenderer(1))->GetPosition(cell1Before);
    Camera(m_ForeignRenderer)->GetPosition(foreignBefore);

    mitk::Vector2D moveVector;
    moveVector[0] = 5.0;
    moveVector[1] = 5.0;
    Fire<mitk::DisplayMoveEvent>(CellRenderer(0), moveVector);

    CPPUNIT_ASSERT_MESSAGE("Pan must move the sending cell's camera",
                           Moved(cell0Before, Camera(CellRenderer(0))));
    CPPUNIT_ASSERT_MESSAGE("Pan must move the synchronized sibling cell's camera",
                           Moved(cell1Before, Camera(CellRenderer(1))));
    CPPUNIT_ASSERT_MESSAGE("Pan must not move a foreign window's camera",
                           !Moved(foreignBefore, Camera(m_ForeignRenderer)));
  }

  void Zoom_Synchronized_CouplesEditorCells_NotForeign()
  {
    m_Editor->Synchronize(true);

    const double cell0Before = Camera(CellRenderer(0))->GetParallelScale();
    const double cell1Before = Camera(CellRenderer(1))->GetParallelScale();
    const double foreignBefore = Camera(m_ForeignRenderer)->GetParallelScale();

    mitk::Point2D startCoordinate;
    startCoordinate.Fill(8.0);
    Fire<mitk::DisplayZoomEvent>(CellRenderer(0), 2.0f, startCoordinate);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must halve the sending cell's parallel scale",
      cell0Before / 2.0, Camera(CellRenderer(0))->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must halve the synchronized sibling cell's parallel scale",
      cell1Before / 2.0, Camera(CellRenderer(1))->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must not change a foreign window's parallel scale",
      foreignBefore, Camera(m_ForeignRenderer)->GetParallelScale(), 1e-6);
  }

  void Crosshair_Synchronized_CouplesEditorCells_NotForeign()
  {
    m_Editor->Synchronize(true);

    mitk::Point3D position;
    position[0] = 8.0;
    position[1] = 8.0;
    position[2] = 6.0;
    Fire<mitk::DisplaySetCrosshairEvent>(CellRenderer(0), position);

    const auto cell0Pos = SlicePos(CellRenderer(0));
    const auto cell1Pos = SlicePos(CellRenderer(1));
    CPPUNIT_ASSERT_MESSAGE("Crosshair must move the sending cell off the baseline slice",
                           2u != cell0Pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Crosshair must move synchronized cells to the same slice",
                                 cell0Pos, cell1Pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Crosshair must not move a foreign window's slice",
                                 2u, SlicePos(m_ForeignRenderer));
  }

  void Desynchronized_ScrollMovesOnlySender()
  {
    m_Editor->Synchronize(false);

    Fire<mitk::DisplayScrollEvent>(CellRenderer(0), 1, false);

    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(CellRenderer(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Desynchronized cells must not follow a sibling's scroll",
                                 2u, SlicePos(CellRenderer(1)));
    CPPUNIT_ASSERT_EQUAL(2u, SlicePos(m_ForeignRenderer));
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
