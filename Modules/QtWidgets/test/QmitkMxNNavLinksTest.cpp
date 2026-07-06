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
#include <mitkCameraController.h>
#include <mitkDisplayActionEvents.h>
#include <mitkImageGenerator.h>
#include <mitkInteractionEvent.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

/**
 * Behavior tests for the MxN per-dimension navigation links: predicate
 * scoping through the editor's group map, converge-on-link with the
 * pre-order seed as reference, the slice / zoom / pan init-offsets,
 * re-converge after drift, and the Synchronize macro semantics.
 *
 * Events are driven synthetically through the editor's display-action
 * broadcast, exactly as in QmitkMxNSynchronizeScopeTest.
 */
class QmitkMxNNavLinksTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNNavLinksTestSuite);

  MITK_TEST(SliceLink_PropagatesToGroupOnly);
  MITK_TEST(Converge_Slice_MovieFrameOffsets);
  MITK_TEST(Converge_Slice_SeedAtZero_SignedClamp);
  MITK_TEST(Converge_Zoom_FactorOffset);
  MITK_TEST(Converge_Pan_WorldOffset);
  MITK_TEST(Reconverge_RestoresOffsetAfterBoundaryClamp);
  MITK_TEST(PanOffset_DriftsUnderZoom_ReconvergeRestores);
  MITK_TEST(Macro_LinksAllCellsWithoutConverge);
  MITK_TEST(Macro_Off_RestoresIndependence);
  MITK_TEST(Macro_NewCellAutoJoins);
  MITK_TEST(SetSyncLink_ContractViolations_Throw);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3); // cells: mxn__widget0 .. mxn__widget2

    for (const auto& [name, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }
    for (std::size_t i = 0; i < 3; ++i)
    {
      SetSlicePos(i, 4); // mid-stack baseline, both scroll directions in range
    }
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  static QString CellId(std::size_t index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  mitk::BaseRenderer* Renderer(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
  }

  mitk::Stepper* SliceStepper(std::size_t index) const
  {
    auto* stepper = Renderer(index)->GetSliceNavigationController()->GetStepper();
    CPPUNIT_ASSERT(nullptr != stepper && stepper->GetSteps() > 4);
    return stepper;
  }

  void SetSlicePos(std::size_t index, unsigned int pos) const
  {
    SliceStepper(index)->SetPos(pos);
  }

  unsigned int SlicePos(std::size_t index) const
  {
    return SliceStepper(index)->GetPos();
  }

  vtkCamera* Camera(std::size_t index) const
  {
    return Renderer(index)->GetVtkRenderer()->GetActiveCamera();
  }

  mitk::Point2D PlanePosition(std::size_t index) const
  {
    return Renderer(index)->GetCameraController()->GetCameraPositionOnPlane();
  }

  template <typename TDisplayEvent, typename... TArgs>
  void Fire(std::size_t senderIndex, TArgs&&... args)
  {
    auto interactionEvent = mitk::InteractionEvent::New(Renderer(senderIndex));
    m_Editor->GetInteractionEventHandler()->InvokeEvent(
      TDisplayEvent(interactionEvent, std::forward<TArgs>(args)...));
  }

  void FireScroll(std::size_t senderIndex, int delta)
  {
    Fire<mitk::DisplayScrollEvent>(senderIndex, delta, false);
  }

  void FireZoom(std::size_t senderIndex, float factor)
  {
    mitk::Point2D startCoordinate;
    startCoordinate.Fill(8.0);
    Fire<mitk::DisplayZoomEvent>(senderIndex, factor, startCoordinate);
  }

  void SliceLink_PropagatesToGroupOnly()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    FireScroll(0, 1);

    CPPUNIT_ASSERT_EQUAL(5u, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL(5u, SlicePos(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlinked cell must not follow", 4u, SlicePos(2));

    // Slice linkage must not bleed into other dimensions: a zoom gesture
    // stays sender-only for cells linked on slice alone.
    const double cell1Scale = Camera(1)->GetParallelScale();
    FireZoom(0, 2.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Zoom must not propagate over a slice-only link",
      cell1Scale, Camera(1)->GetParallelScale(), 1e-6);
  }

  void Converge_Slice_MovieFrameOffsets()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav"); // seed at 4
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", -1);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav", 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Member converges to seed + offset", 3u, SlicePos(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Member converges to seed + offset", 5u, SlicePos(2));

    FireScroll(0, 1);
    CPPUNIT_ASSERT_EQUAL(5u, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset preserved while scrolling in range", 4u, SlicePos(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset preserved while scrolling in range", 6u, SlicePos(2));
  }

  void Converge_Slice_SeedAtZero_SignedClamp()
  {
    SetSlicePos(0, 0);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", -1);

    // Unsigned wrap-around would land on the LAST slice; the signed converge
    // clamps to the first.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Negative offset from a seed at slice 0 must clamp to 0, not wrap to the last slice",
      0u, SlicePos(1));
  }

  void Converge_Zoom_FactorOffset()
  {
    const double seedScale = Camera(0)->GetParallelScale();

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "z");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "z", 2.0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Offset 2.0 keeps the member twice as zoomed",
      seedScale / 2.0, Camera(1)->GetParallelScale(), 1e-6);

    FireZoom(0, 2.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(seedScale / 2.0, Camera(0)->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Identical factors preserve the zoom ratio",
      seedScale / 4.0, Camera(1)->GetParallelScale(), 1e-6);
  }

  void Converge_Pan_WorldOffset()
  {
    mitk::Vector2D offset;
    offset[0] = 10.0;
    offset[1] = 5.0;

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "p");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, "p", offset);

    const auto seedPosition = PlanePosition(0);
    const auto memberPosition = PlanePosition(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(seedPosition[0] + 10.0, memberPosition[0], 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(seedPosition[1] + 5.0, memberPosition[1], 1e-3);

    // The offset is preserved under a shared pan (identical world-mm deltas).
    mitk::Vector2D moveVector;
    moveVector[0] = 7.0;
    moveVector[1] = -2.0;
    Fire<mitk::DisplayMoveEvent>(0, moveVector);

    const auto seedAfter = PlanePosition(0);
    const auto memberAfter = PlanePosition(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(seedAfter[0] + 10.0, memberAfter[0], 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(seedAfter[1] + 5.0, memberAfter[1], 1e-3);
  }

  void Reconverge_RestoresOffsetAfterBoundaryClamp()
  {
    // Park the seed one step below the last slice so the member's +1 offset
    // puts it exactly on the boundary (independent of the cells' default
    // view direction, which determines the step count).
    const unsigned int last = SliceStepper(0)->GetSteps() - 1;
    SetSlicePos(0, last - 1);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", 1);
    CPPUNIT_ASSERT_EQUAL(last, SlicePos(1));

    // Scrolling forward clamps the member at the last slice; scrolling back
    // moves both, so the offset is now lost.
    FireScroll(0, 1);
    FireScroll(0, -1);
    CPPUNIT_ASSERT_EQUAL(last - 1, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Boundary clamping destroys the offset", last - 1, SlicePos(1));

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Re-converge restores seed + offset", last, SlicePos(1));
  }

  void PanOffset_DriftsUnderZoom_ReconvergeRestores()
  {
    mitk::Vector2D offset;
    offset[0] = 10.0;
    offset[1] = 0.0;

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "p");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, "p", offset);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "z");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "z");

    // Zoom re-centers every member on the sender's coordinate, which scales
    // the pan offset between the cells - the documented drift.
    FireZoom(0, 2.0f);
    const auto driftedDelta = PlanePosition(1)[0] - PlanePosition(0)[0];
    CPPUNIT_ASSERT_MESSAGE("Zoom must perturb the pan offset (documented limitation)",
                           std::abs(driftedDelta - 10.0) > 1.0);

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Pan, "p");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Re-converge restores the declared offset",
      10.0, PlanePosition(1)[0] - PlanePosition(0)[0], 1e-3);
  }

  void Macro_LinksAllCellsWithoutConverge()
  {
    SetSlicePos(0, 2);
    SetSlicePos(1, 5);
    SetSlicePos(2, 6);

    m_Editor->Synchronize(true);

    // The macro couples in place - no convergence.
    CPPUNIT_ASSERT_EQUAL(2u, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL(5u, SlicePos(1));
    CPPUNIT_ASSERT_EQUAL(6u, SlicePos(2));

    const auto link = m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT(link.has_value());

    FireScroll(0, 1);
    CPPUNIT_ASSERT_EQUAL(3u, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL(6u, SlicePos(1));
    CPPUNIT_ASSERT_EQUAL(7u, SlicePos(2));
  }

  void Macro_Off_RestoresIndependence()
  {
    m_Editor->Synchronize(true);
    m_Editor->Synchronize(false);

    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice).has_value());

    FireScroll(0, 1);
    CPPUNIT_ASSERT_EQUAL(5u, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlinking restores independence", 4u, SlicePos(1));
  }

  void Macro_NewCellAutoJoins()
  {
    m_Editor->Synchronize(true);
    m_Editor->SetLayout(1, 4); // creates mxn__widget3 while the macro is active

    const auto link = m_Editor->GetSyncLink(CellId(3), QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT_MESSAGE("A cell created under the active macro joins the macro group",
                           link.has_value());
    CPPUNIT_ASSERT_EQUAL(m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice)->group,
                         link->group);
  }

  void SetSyncLink_ContractViolations_Throw()
  {
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetSyncLink("mxn__nosuch", QmitkMxNSyncDimension::Slice, "nav"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "not a group"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav", 2.0), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Crosshair, "nav", 1), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "nav", -1.0), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Crosshair, "nav"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "unknown-group"), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNNavLinks)
