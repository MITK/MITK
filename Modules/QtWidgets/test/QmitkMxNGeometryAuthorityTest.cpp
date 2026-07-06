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
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

/**
 * Tests orientation synchronization and the geometry-authority model:
 *   - A plane change on an orientation-linked cell is relayed to its group
 *     exactly once per member (no re-entrant propagation) and leaves
 *     unlinked cells alone.
 *   - Joining an orientation group aligns the joining cell's plane to the
 *     group's seed.
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
    CPPUNIT_ASSERT_EQUAL(Snc(0)->GetStepper()->GetPos() + 1, Snc(1)->GetStepper()->GetPos());

    m_Editor->ReinitSyncGroupGeometry(CellId(0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The declared offset survives a component reinit",
      Snc(0)->GetStepper()->GetPos() + 1, Snc(1)->GetStepper()->GetPos());
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
