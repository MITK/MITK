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
#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTimeNavigationController.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <cstdlib>

/**
 * Tests orientation synchronization and the geometry-authority model:
 *   - A plane change on an orientation-linked cell is relayed to its group
 *     exactly once per member (no re-entrant propagation) and leaves
 *     unlinked cells alone.
 *   - Joining an orientation group aligns the joining cell's plane to the
 *     group's, wherever the joining cell sits in the layout.
 *   - A plane change re-converges the cell's slice / zoom / pan offsets even
 *     without an orientation link, and the changed cell adapts to its group.
 *   - Aligning a cell's geometry keeps its camera and slice position.
 *   - The MxN group reinit re-initializes the connected component of the
 *     slice/orientation link graph (the A-slice-B, B-orientation-C chain
 *     converges as one) to a shared geometry, leaves singletons untouched,
 *     and re-converges the component's slice offsets.
 *   - An application-global reinit still resets every cell (the documented
 *     Data-Manager quirk).
 */
class QmitkMxNGeometryAuthorityTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNGeometryAuthorityTestSuite);

  MITK_TEST(Orientation_PropagatesToGroup_ExactlyOncePerMember);
  MITK_TEST(Orientation_UnlinkedCellDoesNotPropagate);
  MITK_TEST(OrientationLink_AlignsJoiningCellToSeedPlane);
  MITK_TEST(OrientationLink_JoinFromEarlierCell_AdoptsGroupPlane);
  MITK_TEST(OrientationLink_JoinOnSamePlane_KeepsSliceOffset);
  MITK_TEST(OrientationLink_JoinAdoptingPlane_ReconvergesSliceOffset);
  MITK_TEST(PlaneChange_UnlinkedOrientation_ReconvergesSliceOffset);
  MITK_TEST(PlaneChange_UnlinkedOrientationOnSeed_KeepsGroup);
  MITK_TEST(SliceLink_GeometryAlignment_KeepsMemberZoom);
  MITK_TEST(SliceLink_GeometryAlignment_KeepsTimeStep);
  MITK_TEST(SliceLink_GeometryAlignment_TimeOutsideReference);
  MITK_TEST(GroupReinit_ChainConvergesComponentOnly);
  MITK_TEST(GroupReinit_ReconvergesSliceOffsets);
  MITK_TEST(GlobalReinit_StillResetsAllCells);

  CPPUNIT_TEST_SUITE_END();

  class TestableEditor : public QmitkMxNMultiWidget
  {
  public:
    using QmitkMxNMultiWidget::QmitkMxNMultiWidget;
    using QmitkMxNMultiWidget::GetOrientationApplyCount;
  };

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  std::unique_ptr<TestableEditor> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);

    // A real image node so the group reinit has bounding content; explicit
    // layer for the node-table-model comparator workaround (see
    // QmitkMxNSyncGroupApiTest).
    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_ImageNode);

    m_Editor = std::make_unique<TestableEditor>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 4); // cells: mxn__widget0 .. mxn__widget3

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

  mitk::SliceNavigationController* Snc(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    return cell->GetSliceNavigationController();
  }

  mitk::AnatomicalPlane Plane(std::size_t index) const
  {
    return Snc(index)->GetDefaultViewDirection();
  }

  mitk::BaseRenderer* Renderer(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
  }

  /** The displayed slice index: the one the navigator shows and slice offsets count. */
  unsigned int ShownSlice(std::size_t index) const
  {
    auto* renderer = Renderer(index);
    const auto* stepper = Snc(index)->GetStepper();
    const unsigned int last = stepper->GetSteps() - 1;
    const bool inverted = mitk::SliceNavigationHelper::IsSliceIndexInverted(
      Snc(index)->GetInputWorldTimeGeometry()->GetGeometryForTimeStep(0),
      renderer->GetCurrentWorldGeometry(), Snc(index)->GetViewDirection());
    return inverted ? last - stepper->GetPos() : stepper->GetPos();
  }

  /** Re-initialize every cell from the bounding geometry of the data, as a
   *  global reinit does. A plane change re-initializes from the same geometry,
   *  so the cells then share it and are not re-aligned to each other. */
  void InitializeCellsByBoundingObjects() const
  {
    for (const auto& [name, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_DataStorage);
    }
  }

  /** A plane different from every cell's current default, so a change is observable. */
  mitk::AnatomicalPlane OtherPlane() const
  {
    return (mitk::AnatomicalPlane::Axial == Plane(0)) ? mitk::AnatomicalPlane::Coronal
                                                      : mitk::AnatomicalPlane::Axial;
  }

  void Orientation_PropagatesToGroup_ExactlyOncePerMember()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "planes");

    const auto before = Plane(2);
    const auto target = OtherPlane();
    const auto applyCountBefore = m_Editor->GetOrientationApplyCount();

    m_Editor->SetViewDirection(CellId(0), target);

    CPPUNIT_ASSERT_EQUAL(target, Plane(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group member follows the plane change", target, Plane(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlinked cell keeps its plane", before, Plane(2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "One change with one group member applies exactly once (no re-entrant propagation)",
      applyCountBefore + 1, m_Editor->GetOrientationApplyCount());
  }

  void Orientation_UnlinkedCellDoesNotPropagate()
  {
    const auto target = OtherPlane();
    const auto planeBefore = Plane(1);

    m_Editor->SetViewDirection(CellId(0), target);

    CPPUNIT_ASSERT_EQUAL(target, Plane(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Without an orientation link nothing propagates",
                                 planeBefore, Plane(1));
    CPPUNIT_ASSERT_EQUAL(0u, m_Editor->GetOrientationApplyCount());
  }

  void OrientationLink_AlignsJoiningCellToSeedPlane()
  {
    const auto target = OtherPlane();
    m_Editor->SetViewDirection(CellId(0), target); // unlinked: local only
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes"); // seed

    CPPUNIT_ASSERT(target != Plane(1));
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "planes");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Joining an orientation group adopts the seed's plane",
                                 target, Plane(1));
  }

  void OrientationLink_JoinFromEarlierCell_AdoptsGroupPlane()
  {
    const auto target = OtherPlane();
    m_Editor->SetViewDirection(CellId(2), target); // unlinked: local only
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Orientation, "planes");

    CPPUNIT_ASSERT(target != Plane(0));
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A joiner earlier in the layout adopts the group's plane",
                                 target, Plane(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The group keeps its plane", target, Plane(2));
  }

  void OrientationLink_JoinOnSamePlane_KeepsSliceOffset()
  {
    InitializeCellsByBoundingObjects();
    // Off the middle slice, so a reset is told apart.
    Snc(0)->GetStepper()->SetPos(5);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s", 1);
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Orientation, "o");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: the joiner already shows the group's plane", Plane(2), Plane(1));
    const unsigned int anchorPos = Snc(0)->GetStepper()->GetPos();

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "o");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The slice group does not move for a member's orientation join",
                                 anchorPos, Snc(0)->GetStepper()->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The joiner keeps its slice offset",
                                 ShownSlice(0) + 1, ShownSlice(1));
  }

  void OrientationLink_JoinAdoptingPlane_ReconvergesSliceOffset()
  {
    InitializeCellsByBoundingObjects();
    // Off the middle slice, so a reset is told apart.
    Snc(0)->GetStepper()->SetPos(5);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s", 1);
    const auto target = OtherPlane();
    m_Editor->SetViewDirection(CellId(2), target); // unlinked: local only
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Orientation, "o");
    const unsigned int anchorPos = Snc(0)->GetStepper()->GetPos();

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "o");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: the joiner adopted the group's plane", target, Plane(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The slice group does not move for a member's orientation join",
                                 anchorPos, Snc(0)->GetStepper()->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The joiner is re-converged to its offset after adopting the plane",
                                 ShownSlice(0) + 1, ShownSlice(1));
  }

  void PlaneChange_UnlinkedOrientation_ReconvergesSliceOffset()
  {
    // Off the middle slice, so a plane change's reset is told apart.
    Snc(0)->GetStepper()->SetPos(5);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s", 1);
    const unsigned int anchorPos = Snc(0)->GetStepper()->GetPos();

    m_Editor->SetViewDirection(CellId(1), OtherPlane());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The group does not move for a member's plane change",
                                 anchorPos, Snc(0)->GetStepper()->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The member is re-converged to its offset after the plane change",
                                 ShownSlice(0) + 1, ShownSlice(1));
  }

  void PlaneChange_UnlinkedOrientationOnSeed_KeepsGroup()
  {
    // Off the middle slice, so a plane change's reset is told apart.
    Snc(0)->GetStepper()->SetPos(5);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s", 1);
    const unsigned int memberPos = Snc(1)->GetStepper()->GetPos();

    m_Editor->SetViewDirection(CellId(0), OtherPlane());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The group is not pulled to the first window's reset state",
                                 memberPos, Snc(1)->GetStepper()->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The first window adapts to the group after its plane change",
                                 ShownSlice(1) - 1, ShownSlice(0));
  }

  void SliceLink_GeometryAlignment_KeepsMemberZoom()
  {
    // Cells 2 and 3 show a larger image; 3 is coupled to 2 by orientation only.
    // Linking 2 into the slice group of cell 0 aligns the whole component to
    // cell 0's geometry, which must not cost 2 its zoom or 3 its slice.
    const auto larger = mitk::ImageGenerator::GenerateGradientImage<short>(32, 32, 8, 1.0f, 1.0f, 1.0f);
    for (const std::size_t i : { std::size_t(2), std::size_t(3) })
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        m_Editor->GetRenderWindowWidget(CellId(i))->GetRenderWindow()->GetVtkRenderWindow(),
        larger->GetTimeGeometry());
    }
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Orientation, "o");
    m_Editor->SetSyncLink(CellId(3), QmitkMxNSyncDimension::Orientation, "o");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");

    auto* camera = Renderer(2)->GetVtkRenderer()->GetActiveCamera();
    const double zoomedScale = camera->GetParallelScale() / 4.0;
    camera->SetParallelScale(zoomedScale);
    Snc(3)->GetStepper()->SetPos(1);
    const auto* slicePlane = Snc(3)->GetCurrentPlaneGeometry();
    mitk::Point3D slicePoint;
    slicePlane->Project(slicePlane->GetCenter(), slicePoint);

    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "s");

    const auto* reference = Snc(0)->GetInputWorldTimeGeometry();
    CPPUNIT_ASSERT_MESSAGE("Fixture: the linked member is aligned to the group geometry",
                           mitk::Equal(*reference, *Snc(2)->GetInputWorldTimeGeometry(), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Fixture: the orientation-coupled cell is aligned as well",
                           mitk::Equal(*reference, *Snc(3)->GetInputWorldTimeGeometry(), mitk::eps, false));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("A slice-only member keeps its zoom through the alignment",
      zoomedScale, Renderer(2)->GetVtkRenderer()->GetActiveCamera()->GetParallelScale(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("An orientation-only member keeps its slice world position",
      0.0, Snc(3)->GetCurrentPlaneGeometry()->DistanceFromPlane(slicePoint), 1e-3);
  }

  /** Global time step after linking cell 1 (a 3-step image) into the slice
   *  group of cell 0 (showing 'reference'), with time step 2 selected. */
  mitk::TimeStepType TimeStepAfterAlignment(const mitk::Image* reference)
  {
    const auto member = mitk::ImageGenerator::GenerateRandomImage<short>(32, 32, 8, 3);
    mitk::RenderingManager::GetInstance()->InitializeView(
      m_Editor->GetRenderWindowWidget(CellId(0))->GetRenderWindow()->GetVtkRenderWindow(),
      reference->GetTimeGeometry());
    mitk::RenderingManager::GetInstance()->InitializeView(
      m_Editor->GetRenderWindowWidget(CellId(1))->GetRenderWindow()->GetVtkRenderWindow(),
      member->GetTimeGeometry());
    auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
    timeNavigation->GetStepper()->SetPos(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: time step 2 is selected", mitk::TimeStepType(2),
                                 timeNavigation->GetSelectedTimeStep());

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s");
    CPPUNIT_ASSERT_MESSAGE("Fixture: the member was aligned to the reference geometry",
      mitk::Equal(*Snc(0)->GetInputWorldTimeGeometry(), *Snc(1)->GetInputWorldTimeGeometry(), mitk::eps, false));
    return timeNavigation->GetSelectedTimeStep();
  }

  void SliceLink_GeometryAlignment_KeepsTimeStep()
  {
    const auto reference = mitk::ImageGenerator::GenerateRandomImage<short>(16, 16, 8, 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Aligning a member keeps the selected time step",
                                 mitk::TimeStepType(2), this->TimeStepAfterAlignment(reference));
  }

  void SliceLink_GeometryAlignment_TimeOutsideReference()
  {
    // The reference covers no time point beyond its single step, so there is
    // no step to restore; the time stays where the re-initialization left it.
    const auto reference = mitk::ImageGenerator::GenerateRandomImage<short>(16, 16, 8, 1);
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(0), this->TimeStepAfterAlignment(reference));
  }

  void GroupReinit_ChainConvergesComponentOnly()
  {
    // A-slice-B and B-orientation-C force A, B, C onto one geometry
    // (connected component); D stays a singleton.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "o");
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Orientation, "o");

    const auto* singletonGeometryBefore = Snc(3)->GetInputWorldTimeGeometry();

    m_Editor->ReinitSyncGroupGeometry(CellId(0));

    const auto* geometryA = Snc(0)->GetInputWorldTimeGeometry();
    const auto* geometryB = Snc(1)->GetInputWorldTimeGeometry();
    const auto* geometryC = Snc(2)->GetInputWorldTimeGeometry();
    CPPUNIT_ASSERT(nullptr != geometryA && nullptr != geometryB && nullptr != geometryC);
    CPPUNIT_ASSERT_MESSAGE("Chain member B shares the component geometry",
                           mitk::Equal(*geometryA, *geometryB, mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transitively coupled C shares the component geometry",
                           mitk::Equal(*geometryA, *geometryC, mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("The singleton cell is not touched by a component reinit",
                           singletonGeometryBefore == Snc(3)->GetInputWorldTimeGeometry());
  }

  void GroupReinit_ReconvergesSliceOffsets()
  {
    Snc(0)->GetStepper()->SetPos(4);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s", 1);
    // The offset counts displayed slices, so its stepper sign depends on the
    // view's inversion; what must hold is that the reinit keeps it.
    auto stepperDelta = [this]()
    {
      return static_cast<int>(Snc(1)->GetStepper()->GetPos()) - static_cast<int>(Snc(0)->GetStepper()->GetPos());
    };
    const int linkedDelta = stepperDelta();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The member sits one slice from the seed", 1, std::abs(linkedDelta));

    m_Editor->ReinitSyncGroupGeometry(CellId(0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The declared offset survives a component reinit",
      linkedDelta, stepperDelta());
  }

  void GlobalReinit_StillResetsAllCells()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "s");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "s");

    const auto* singletonGeometryBefore = Snc(3)->GetInputWorldTimeGeometry();

    // The Data Manager's global reinit path: initializes ALL registered
    // render windows, collapsing per-component scoping. Documented quirk,
    // remedied by the MxN group reinit / re-converge.
    mitk::RenderingManager::GetInstance()->InitializeViews(m_Image->GetTimeGeometry());

    CPPUNIT_ASSERT_MESSAGE("A global reinit reaches even unlinked singleton cells",
                           singletonGeometryBefore != Snc(3)->GetInputWorldTimeGeometry());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNGeometryAuthority)
