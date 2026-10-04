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
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <QColor>

#include <array>

/**
 * Behavior tests for the MxN per-dimension navigation links: predicate
 * scoping through the editor's group map, converge-on-link and offset
 * changes that move only the written cell (offsets relative to one group
 * reference, whichever member joined first), the slice / zoom / pan
 * init-offsets, re-converge after drift, and the Synchronize macro semantics.
 *
 * Events are driven synthetically through the editor's display-action
 * broadcast, exactly as in QmitkMxNSynchronizeScopeTest.
 */
class QmitkMxNNavLinksTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNNavLinksTestSuite);

  MITK_TEST(SliceLink_PropagatesToGroupOnly);
  MITK_TEST(Scroll_CrossPlaneSliceGroup_KeepsDisplayedRelation);
  MITK_TEST(Converge_Slice_MovieFrameOffsets);
  MITK_TEST(Converge_Slice_SeedAtZero_SignedClamp);
  MITK_TEST(Converge_Zoom_FactorOffset);
  MITK_TEST(Converge_Pan_WorldOffset);
  MITK_TEST(Converge_JoinFromEarlierCell_ConvergesJoiner);
  MITK_TEST(Converge_Slice_OffsetOnSeed_ShiftsSeed);
  MITK_TEST(Converge_Zoom_OffsetOnSeed_ShiftsSeed);
  MITK_TEST(Converge_SeedLeaves_KeepsMemberRelation);
  MITK_TEST(Converge_SaveLoadRoundTrip_KeepsMemberRelations);
  MITK_TEST(Reconverge_RestoresOffsetAfterBoundaryClamp);
  MITK_TEST(PanOffset_DriftsUnderZoom_ReconvergeRestores);
  MITK_TEST(Macro_LinksAllCellsWithoutConverge);
  MITK_TEST(Macro_Off_RestoresIndependence);
  MITK_TEST(Macro_NewCellAutoJoins);
  MITK_TEST(SetSyncLink_ContractViolations_Throw);
  MITK_TEST(Windowing_GroupedGesture_WritesPerRendererToMembers);
  MITK_TEST(Windowing_UnlinkedGesture_KeepsNodeGlobalWrite);
  MITK_TEST(Lut_SetLookupTable_PropagatesToMembersOnly);
  MITK_TEST(Lut_UnlinkReturnsTheCellToNodeGlobal);
  MITK_TEST(Lut_SetLookupTable_Unlinked_WritesNodeGlobal);
  MITK_TEST(SetLevelWindow_Grouped_SetsMembersByValue);
  MITK_TEST(SetLevelWindow_Unlinked_WritesNodeGlobal);
  MITK_TEST(AdjustLevelWindow_Grouped_PreservesMemberDifferences);
  MITK_TEST(LevelWindow_ContractViolations_Throw);
  MITK_TEST(Appearance_RemovedCellLeavesNothingForItsReusedId);
  MITK_TEST(Windowing_UnlinkReturnsTheCellToNodeGlobal);
  MITK_TEST(Windowing_GroupedWriteWithoutLevelWindowCreatesNone);
  MITK_TEST(GroupColor_AssignedByRegistrationOrder);
  MITK_TEST(Maximize_SliceSyncReachesHiddenPeers);
  MITK_TEST(Maximize_ZoomSyncReachesHiddenPeers);
  MITK_TEST(Maximize_DoesNotPerturbSyncedPeers);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    // A real image node: the level-window gesture resolves its target node
    // from the data under the pointer. Explicit layer for the
    // node-table-model comparator workaround (see QmitkMxNSyncGroupApiTest).
    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_ImageNode);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3); // cells: mxn__widget0 .. mxn__widget2

    RealiseViews();
    for (std::size_t i = 0; i < 3; ++i)
    {
      SetSlicePos(i, 4); // mid-stack baseline, both scroll directions in range
    }
  }

  void RealiseViews() const
  {
    for (const auto& [name, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_ImageNode = nullptr;
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

  bool SliceInverted(std::size_t index) const
  {
    auto* renderer = Renderer(index);
    return mitk::SliceNavigationHelper::IsSliceIndexInverted(
      m_Image->GetGeometry(), renderer->GetCurrentWorldGeometry(),
      renderer->GetSliceNavigationController()->GetViewDirection());
  }

  /** The displayed slice index: the one the navigator shows and slice offsets count. */
  unsigned int ShownSlice(std::size_t index) const
  {
    const unsigned int last = SliceStepper(index)->GetSteps() - 1;
    return SliceInverted(index) ? last - SlicePos(index) : SlicePos(index);
  }

  void SetShownSlice(std::size_t index, unsigned int shown) const
  {
    const unsigned int last = SliceStepper(index)->GetSteps() - 1;
    SetSlicePos(index, SliceInverted(index) ? last - shown : shown);
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

  // --- Maximizing one cell must not cost the group anything -------------------

  void Maximize_SliceSyncReachesHiddenPeers()
  {
    // Maximizing hides the peers' widgets. Synchronization runs on the
    // renderers, not on the painted widgets, so a gesture in the maximized cell
    // must still move its group - the hidden peers simply repaint on restore.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    m_Editor->SetMaximizedCell(CellId(0));
    FireScroll(0, 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The maximized cell moves", 5u, SlicePos(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A hidden group member follows while maximized",
                                 5u, SlicePos(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A hidden non-member still must not follow", 4u, SlicePos(2));

    m_Editor->SetMaximizedCell(QString());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Restoring shows the peer at the position it followed to",
                                 5u, SlicePos(1));
  }

  void Maximize_ZoomSyncReachesHiddenPeers()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "zoom");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "zoom");

    const double peerBefore = Camera(1)->GetParallelScale();
    const double outsiderBefore = Camera(2)->GetParallelScale();

    m_Editor->SetMaximizedCell(CellId(0));
    FireZoom(0, 2.0f);
    m_Editor->SetMaximizedCell(QString());

    CPPUNIT_ASSERT_MESSAGE("A hidden group member follows a zoom made while maximized",
                           std::abs(Camera(1)->GetParallelScale() - peerBefore) > 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A cell outside the zoom group is untouched",
                                         outsiderBefore, Camera(2)->GetParallelScale(), 1e-6);
  }

  void Maximize_DoesNotPerturbSyncedPeers()
  {
    // Maximizing is a view op: on its own it must move nothing. The cell's
    // widget changes size, and a size change is exactly what could leak into a
    // zoom-synced group as a spurious propagation.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "zoom");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "zoom");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    const double peerScale = Camera(1)->GetParallelScale();
    const unsigned int peerSlice = SlicePos(1);

    m_Editor->SetMaximizedCell(CellId(0));
    m_Editor->SetMaximizedCell(QString());

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A maximize round trip leaves the group's zoom alone",
                                         peerScale, Camera(1)->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A maximize round trip leaves the group's slice alone",
                                 peerSlice, SlicePos(1));
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

  void Scroll_CrossPlaneSliceGroup_KeepsDisplayedRelation()
  {
    // A slice group across planes whose slice index runs opposite to the
    // stepper in one cell only: joining and scrolling must agree on the
    // displayed slices.
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Axial);
    m_Editor->SetViewDirection(CellId(1), mitk::AnatomicalPlane::Sagittal);
    SetSlicePos(0, 4);
    SetSlicePos(1, 6);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the two cells have opposite inversion",
                           SliceInverted(0) != SliceInverted(1));

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Joining converges the displayed slice", ShownSlice(0), ShownSlice(1));

    FireScroll(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A scroll keeps the displayed slices together", ShownSlice(0), ShownSlice(1));
    FireScroll(1, -1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A scroll from the other cell keeps them together", ShownSlice(0), ShownSlice(1));
  }

  void Converge_Slice_MovieFrameOffsets()
  {
    // Offsets count displayed slices. Axial cells step against the identity
    // image's z index, so stepper arithmetic would show the ramp reversed.
    for (std::size_t i = 0; i < 3; ++i)
    {
      m_Editor->SetViewDirection(CellId(i), mitk::AnatomicalPlane::Axial);
      SetSlicePos(i, 4);
    }
    CPPUNIT_ASSERT_MESSAGE("The fixture exercises an inverted view", SliceInverted(0));

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav"); // seed at 4
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", -1);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav", 1);

    const unsigned int seedShown = ShownSlice(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Member shows seed - 1", seedShown - 1, ShownSlice(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Member shows seed + 1", seedShown + 1, ShownSlice(2));

    FireScroll(0, 1);
    CPPUNIT_ASSERT_EQUAL(5u, SlicePos(0));
    const unsigned int scrolledShown = ShownSlice(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset preserved while scrolling in range", scrolledShown - 1, ShownSlice(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset preserved while scrolling in range", scrolledShown + 1, ShownSlice(2));
  }

  void Converge_Slice_SeedAtZero_SignedClamp()
  {
    SetShownSlice(0, 0);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", -1);

    // Unsigned wrap-around would land on the LAST slice; the signed converge
    // clamps to the first.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Negative offset from a seed at slice 0 must clamp to 0, not wrap to the last slice",
      0u, ShownSlice(1));
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

  void Converge_JoinFromEarlierCell_ConvergesJoiner()
  {
    // The joining cell adapts to the group, wherever it sits in the layout.
    SetShownSlice(2, 6);
    SetShownSlice(0, 2);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The group does not move for a joiner", 6u, ShownSlice(2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A joiner earlier in the layout converges to the group",
                                 6u, ShownSlice(0));
  }

  void Converge_Slice_OffsetOnSeed_ShiftsSeed()
  {
    // The movie frame authored on a group whose first window carries -1:
    // every offset counts from one common reference, the first window's
    // included.
    for (std::size_t i = 0; i < 3; ++i)
    {
      m_Editor->SetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice, "nav");
    }
    const unsigned int reference = ShownSlice(1);

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav", -1);
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", 0);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav", 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The first window shows reference - 1", reference - 1, ShownSlice(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The middle window shows the reference", reference, ShownSlice(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The last window shows reference + 1", reference + 1, ShownSlice(2));

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Re-converge keeps a converged group in place", reference - 1, ShownSlice(0));
    CPPUNIT_ASSERT_EQUAL(reference, ShownSlice(1));
    CPPUNIT_ASSERT_EQUAL(reference + 1, ShownSlice(2));
  }

  void Converge_Zoom_OffsetOnSeed_ShiftsSeed()
  {
    const double scale = Camera(1)->GetParallelScale();
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "z");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "z");

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "z", 2.0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A factor on the first window zooms that window",
      scale / 2.0, Camera(0)->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The written window alone moves",
      scale, Camera(1)->GetParallelScale(), 1e-6);

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Zoom, "z");
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Re-converge keeps the member at the reference",
      scale, Camera(1)->GetParallelScale(), 1e-6);
  }

  void Converge_SeedLeaves_KeepsMemberRelation()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", -1);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav", 1);
    const unsigned int shown1 = ShownSlice(1);
    const unsigned int shown2 = ShownSlice(2);
    CPPUNIT_ASSERT_EQUAL(shown1 + 2, shown2);

    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Slice);
    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "nav");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The new first member stays put", shown1, ShownSlice(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The remaining offsets keep their relation", shown2, ShownSlice(2));
  }

  void Converge_SaveLoadRoundTrip_KeepsMemberRelations()
  {
    // Joined out of layout order: after a save and load the document's first
    // window anchors the group, and every member must land where it was.
    SetShownSlice(2, 6);
    SetShownSlice(0, 2);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav", -1);
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", 1);
    const std::array<unsigned int, 3> before{ ShownSlice(0), ShownSlice(1), ShownSlice(2) };

    m_Editor->ApplyLayout(m_Editor->SerializeLayout());
    RealiseViews();
    SetShownSlice(0, before[0]);
    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "nav");

    for (std::size_t i = 0; i < before.size(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A save/load round trip keeps every member's slice",
                                   before[i], ShownSlice(i));
    }
  }

  void Reconverge_RestoresOffsetAfterBoundaryClamp()
  {
    // Park the seed one step below the last slice so the member's +1 offset
    // puts it exactly on the boundary (independent of the cells' default
    // view direction, which determines the step count).
    const unsigned int last = SliceStepper(0)->GetSteps() - 1;
    SetShownSlice(0, last - 1);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav", 1);
    CPPUNIT_ASSERT_EQUAL(last, ShownSlice(1));

    // Scrolling toward the last shown slice clamps the member there; scrolling
    // back moves both, so the offset is now lost. A scroll counts stepper
    // steps, which can run opposite to the shown index.
    const int forward = SliceInverted(0) ? -1 : 1;
    FireScroll(0, forward);
    FireScroll(0, -forward);
    CPPUNIT_ASSERT_EQUAL(last - 1, ShownSlice(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Boundary clamping destroys the offset", last - 1, ShownSlice(1));

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Re-converge restores seed + offset", last, ShownSlice(1));
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

  /** Fire the level-window gesture from a cell, pointing at the image center. */
  void FireLevelWindowDelta(std::size_t senderIndex, double deltaLevel, double deltaWindow)
  {
    mitk::Point3D imageCenter;
    imageCenter[0] = 8.0;
    imageCenter[1] = 8.0;
    imageCenter[2] = 4.0;
    mitk::Point2D pointerPosition;
    Renderer(senderIndex)->WorldToDisplay(imageCenter, pointerPosition);

    auto positionEvent = mitk::InteractionPositionEvent::New(Renderer(senderIndex), pointerPosition);
    m_Editor->GetInteractionEventHandler()->InvokeEvent(
      mitk::DisplaySetLevelWindowEvent(positionEvent, deltaLevel, deltaWindow));
  }

  /** The renderer-specific 'levelwindow' property of a cell, or null. */
  mitk::LevelWindowProperty* RendererLevelWindow(std::size_t index) const
  {
    return dynamic_cast<mitk::LevelWindowProperty*>(
      m_ImageNode->GetPropertyList(Renderer(index))->GetProperty("levelwindow"));
  }

  void Appearance_RemovedCellLeavesNothingForItsReusedId()
  {
    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(42.0, 84.0)), Renderer(2));
    m_ImageNode->SetProperty("LookupTable",
      mitk::LookupTableProperty::New(mitk::LookupTable::New()), Renderer(2));

    // The shrink removes mxn__widget2; the regrow creates a new cell with the
    // same id, i.e. the same renderer name and property context.
    m_Editor->SetLayout(1, 2);
    m_Editor->SetLayout(1, 3);

    auto* context = m_ImageNode->GetPropertyList(Renderer(2));
    CPPUNIT_ASSERT_MESSAGE("A new cell must not inherit a removed cell's level/window",
                           nullptr == context->GetProperty("levelwindow"));
    CPPUNIT_ASSERT_MESSAGE("A new cell must not inherit a removed cell's lookup table",
                           nullptr == context->GetProperty("LookupTable"));
  }

  void Windowing_UnlinkReturnsTheCellToNodeGlobal()
  {
    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 200.0)));
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, "wl");
    FireLevelWindowDelta(0, 10.0, 20.0);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the grouped gesture writes renderer-specific",
                           nullptr != RendererLevelWindow(0));

    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing);

    CPPUNIT_ASSERT_MESSAGE("An unlinked cell falls back to the node-global level/window",
                           nullptr == RendererLevelWindow(0));
    CPPUNIT_ASSERT_MESSAGE("The remaining group member keeps its value",
                           nullptr != RendererLevelWindow(1));
  }

  void Windowing_GroupedWriteWithoutLevelWindowCreatesNone()
  {
    auto bare = mitk::DataNode::New();
    bare->SetName("no level/window");
    m_DataStorage->Add(bare);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, "wl");

    m_Editor->AdjustLevelWindow(CellId(0), bare, 10.0, 20.0);

    for (std::size_t member : { std::size_t(0), std::size_t(1) })
    {
      CPPUNIT_ASSERT_MESSAGE("A node without level/window gets none made up for it",
                             nullptr == bare->GetPropertyList(Renderer(member))->GetProperty("levelwindow"));
    }
  }

  void Windowing_GroupedGesture_WritesPerRendererToMembers()
  {
    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 200.0)));

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, "wl");

    FireLevelWindowDelta(0, 10.0, 20.0);

    for (std::size_t member : { std::size_t(0), std::size_t(1) })
    {
      auto* property = RendererLevelWindow(member);
      CPPUNIT_ASSERT_MESSAGE("Grouped member must get a renderer-specific levelwindow",
                             nullptr != property);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(110.0, property->GetLevelWindow().GetLevel(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(220.0, property->GetLevelWindow().GetWindow(), 1e-6);
    }
    CPPUNIT_ASSERT_MESSAGE("A non-member must not get a renderer-specific levelwindow",
                           nullptr == RendererLevelWindow(2));

    auto* nodeGlobal = dynamic_cast<mitk::LevelWindowProperty*>(m_ImageNode->GetProperty("levelwindow"));
    CPPUNIT_ASSERT(nullptr != nodeGlobal);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The node-global property stays untouched for grouped cells",
      100.0, nodeGlobal->GetLevelWindow().GetLevel(), 1e-6);
  }

  void Windowing_UnlinkedGesture_KeepsNodeGlobalWrite()
  {
    // A fresh cell links Windowing to "main" by default; clear it so the cell is
    // genuinely unlinked and the classic node-global path is exercised.
    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing);
    m_Editor->ClearSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing);

    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 200.0)));

    FireLevelWindowDelta(0, 10.0, 20.0);

    auto* nodeGlobal = dynamic_cast<mitk::LevelWindowProperty*>(m_ImageNode->GetProperty("levelwindow"));
    CPPUNIT_ASSERT(nullptr != nodeGlobal);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
      "An unlinked cell's gesture keeps the classic node-global write",
      110.0, nodeGlobal->GetLevelWindow().GetLevel(), 1e-6);
    CPPUNIT_ASSERT_MESSAGE("No renderer-specific property appears for unlinked cells",
                           nullptr == RendererLevelWindow(0));
    CPPUNIT_ASSERT_MESSAGE("No renderer-specific property appears for unlinked cells",
                           nullptr == RendererLevelWindow(1));
  }

  void Lut_SetLookupTable_PropagatesToMembersOnly()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Lut, "luts");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Lut, "luts");

    // The image node carries a default node-global LookupTable (mapper
    // default properties); the per-renderer write must leave it alone.
    auto* nodeGlobalBefore = m_ImageNode->GetPropertyList(nullptr)->GetProperty("LookupTable");

    auto lookupTable = mitk::LookupTable::New();
    m_Editor->SetLookupTable(CellId(0), m_ImageNode, lookupTable);

    for (std::size_t member : { std::size_t(0), std::size_t(1) })
    {
      auto* property = dynamic_cast<mitk::LookupTableProperty*>(
        m_ImageNode->GetPropertyList(Renderer(member))->GetProperty("LookupTable"));
      CPPUNIT_ASSERT_MESSAGE("Grouped member must get a renderer-specific LookupTable",
                             nullptr != property);
      CPPUNIT_ASSERT_MESSAGE("Members share the propagated lookup table instance",
                             lookupTable.GetPointer() == property->GetLookupTable());
    }
    CPPUNIT_ASSERT_MESSAGE("A non-member must not get a renderer-specific LookupTable",
      nullptr == m_ImageNode->GetPropertyList(Renderer(2))->GetProperty("LookupTable"));
    CPPUNIT_ASSERT_MESSAGE("The node-global LookupTable stays untouched",
      nodeGlobalBefore == m_ImageNode->GetPropertyList(nullptr)->GetProperty("LookupTable"));
  }

  mitk::LookupTableProperty* RendererLookupTable(std::size_t index) const
  {
    return dynamic_cast<mitk::LookupTableProperty*>(
      m_ImageNode->GetPropertyList(Renderer(index))->GetProperty("LookupTable"));
  }

  void Lut_UnlinkReturnsTheCellToNodeGlobal()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Lut, "luts");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Lut, "luts");
    m_Editor->SetLookupTable(CellId(0), m_ImageNode, mitk::LookupTable::New());
    CPPUNIT_ASSERT_MESSAGE("Fixture: the grouped write is renderer-specific",
                           nullptr != RendererLookupTable(0));

    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Lut);

    CPPUNIT_ASSERT_MESSAGE("An unlinked cell falls back to the node-global lookup table",
                           nullptr == RendererLookupTable(0));
    CPPUNIT_ASSERT_MESSAGE("The remaining group member keeps its lookup table",
                           nullptr != RendererLookupTable(1));
  }

  void Lut_SetLookupTable_Unlinked_WritesNodeGlobal()
  {
    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Lut);

    // Identified by type, not instance: the node already carries a
    // node-global lookup table property, which takes the value as a copy.
    auto lookupTable = mitk::LookupTable::New();
    lookupTable->SetType(mitk::LookupTable::JET);
    m_Editor->SetLookupTable(CellId(0), m_ImageNode, lookupTable);

    auto* nodeGlobal = dynamic_cast<mitk::LookupTableProperty*>(
      m_ImageNode->GetPropertyList(nullptr)->GetProperty("LookupTable"));
    CPPUNIT_ASSERT(nullptr != nodeGlobal);
    CPPUNIT_ASSERT_MESSAGE("An unlinked cell writes the node-global lookup table",
                           mitk::LookupTable::JET == nodeGlobal->GetLookupTable()->GetActiveType());
    CPPUNIT_ASSERT_MESSAGE("No renderer-specific property appears for an unlinked cell",
                           nullptr == RendererLookupTable(0));
  }

  void SetLevelWindow_Grouped_SetsMembersByValue()
  {
    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 200.0)));

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, "wl");

    m_Editor->SetLevelWindow(CellId(0), m_ImageNode, mitk::LevelWindow(42.0, 84.0));

    for (std::size_t member : { std::size_t(0), std::size_t(1) })
    {
      auto* property = RendererLevelWindow(member);
      CPPUNIT_ASSERT_MESSAGE("Grouped member must get a renderer-specific levelwindow",
                             nullptr != property);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(42.0, property->GetLevelWindow().GetLevel(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(84.0, property->GetLevelWindow().GetWindow(), 1e-6);
    }
    CPPUNIT_ASSERT_MESSAGE("A non-member must not get a renderer-specific levelwindow",
                           nullptr == RendererLevelWindow(2));

    auto* nodeGlobal = dynamic_cast<mitk::LevelWindowProperty*>(m_ImageNode->GetProperty("levelwindow"));
    CPPUNIT_ASSERT(nullptr != nodeGlobal);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The node-global property stays untouched for grouped cells",
      100.0, nodeGlobal->GetLevelWindow().GetLevel(), 1e-6);
  }

  void SetLevelWindow_Unlinked_WritesNodeGlobal()
  {
    // A fresh cell links Windowing to "main" by default; clear it so the by-value
    // set falls through to the classic node-global write.
    m_Editor->ClearSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing);

    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 200.0)));

    m_Editor->SetLevelWindow(CellId(0), m_ImageNode, mitk::LevelWindow(42.0, 84.0));

    auto* nodeGlobal = dynamic_cast<mitk::LevelWindowProperty*>(m_ImageNode->GetProperty("levelwindow"));
    CPPUNIT_ASSERT(nullptr != nodeGlobal);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("An unlinked cell's by-value set keeps the node-global write",
      42.0, nodeGlobal->GetLevelWindow().GetLevel(), 1e-6);
    CPPUNIT_ASSERT_MESSAGE("No renderer-specific property appears for unlinked cells",
                           nullptr == RendererLevelWindow(0));
  }

  void AdjustLevelWindow_Grouped_PreservesMemberDifferences()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, "wl");

    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 200.0)), Renderer(0));
    m_ImageNode->SetProperty("levelwindow",
      mitk::LevelWindowProperty::New(mitk::LevelWindow(150.0, 300.0)), Renderer(1));

    m_Editor->AdjustLevelWindow(CellId(0), m_ImageNode, 10.0, 20.0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(110.0, RendererLevelWindow(0)->GetLevelWindow().GetLevel(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(220.0, RendererLevelWindow(0)->GetLevelWindow().GetWindow(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Deltas apply to each member's own value",
      160.0, RendererLevelWindow(1)->GetLevelWindow().GetLevel(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(320.0, RendererLevelWindow(1)->GetLevelWindow().GetWindow(), 1e-6);
  }

  void LevelWindow_ContractViolations_Throw()
  {
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetLevelWindow("mxn__nosuch", m_ImageNode, mitk::LevelWindow(1.0, 2.0)),
      mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->SetLevelWindow(CellId(0), nullptr, mitk::LevelWindow(1.0, 2.0)), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->AdjustLevelWindow("mxn__nosuch", m_ImageNode, 1.0, 2.0), mitk::Exception);
    CPPUNIT_ASSERT_THROW(
      m_Editor->AdjustLevelWindow(CellId(0), nullptr, 1.0, 2.0), mitk::Exception);
    CPPUNIT_ASSERT_THROW(m_Editor->GetSyncGroupColor("never-registered"), mitk::Exception);
  }

  void GroupColor_AssignedByRegistrationOrder()
  {
    // Slot 0 is taken by the default selection group "main" (created by
    // InitializeMultiWidget), so the first linked group lands on slot 1.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");

    CPPUNIT_ASSERT(QColor("#E1707A") == m_Editor->GetSyncGroupColor("main"));
    CPPUNIT_ASSERT(QColor("#6FA8DC") == m_Editor->GetSyncGroupColor("wl"));
    CPPUNIT_ASSERT(QColor("#93C47D") == m_Editor->GetSyncGroupColor("nav"));

    // Re-linking an already known group must not re-order the assignment.
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, "wl");
    CPPUNIT_ASSERT(QColor("#6FA8DC") == m_Editor->GetSyncGroupColor("wl"));
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
