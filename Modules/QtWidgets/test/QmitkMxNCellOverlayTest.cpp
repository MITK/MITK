/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkAnatomicalPlanes.h>
#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QApplication>

#include <memory>

/**
 * Headless behavior tests for the MxN cell overlay's information
 * architecture: the plane label tracks the cell's view direction (and the
 * VTK cell-id annotation is blanked so the two do not double up), and the
 * group-identity dot follows the mono / complex rule over the cell's
 * per-dimension synchronization. Presentation, animation, and gesture are
 * manual acceptance, not covered here.
 */
class QmitkMxNCellOverlayTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNCellOverlayTestSuite);
  MITK_TEST(CellIdAnnotation_IsBlank);
  MITK_TEST(PlaneLabel_TracksViewDirection);
  MITK_TEST(FrameIdentity_FreshCellIsMonoMain);
  MITK_TEST(FrameIdentity_MonoWhenSingleGroup);
  MITK_TEST(FrameIdentity_ComplexAcrossGroups);
  MITK_TEST(FrameIdentity_SelectionParticipates);
  MITK_TEST(PaintPath_DoesNotCrash);
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

    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_ImageNode);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3); // cells: mxn__widget0 .. mxn__widget2

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

  QmitkMxNCellOverlay* Overlay(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* overlay = cell->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT_MESSAGE("Every cell carries a composite overlay", nullptr != overlay);
    return overlay;
  }

  void CellIdAnnotation_IsBlank()
  {
    for (std::size_t i = 0; i < 3; ++i)
    {
      const auto cell = m_Editor->GetRenderWindowWidget(CellId(i));
      CPPUNIT_ASSERT(nullptr != cell);
      CPPUNIT_ASSERT_MESSAGE("MxN blanks the VTK cell-id corner annotation; the plane "
                             "label lives in the Qt overlay instead",
                             cell->GetCornerAnnotationText().empty());
    }
  }

  void PlaneLabel_TracksViewDirection()
  {
    auto* overlay = this->Overlay(0);

    // The label is resolved live from the cell's view direction, so setting a
    // direction is reflected without a render or a separate signal.
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Axial);
    CPPUNIT_ASSERT_EQUAL(std::string("Axial"), overlay->PlaneLabel().toStdString());

    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Coronal);
    CPPUNIT_ASSERT_EQUAL(std::string("Coronal"), overlay->PlaneLabel().toStdString());

    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Sagittal);
    CPPUNIT_ASSERT_EQUAL(std::string("Sagittal"), overlay->PlaneLabel().toStdString());
  }

  void FrameIdentity_FreshCellIsMonoMain()
  {
    // A fresh cell links Windowing, LUT, and selection to "main", so its frame is
    // a solid "main" hue - there is no group-less "None" resting state.
    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's frame is Mono",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's frame hue is 'main's color",
      identity.hue == m_Editor->GetSyncGroupColor("main"));
  }

  void FrameIdentity_MonoWhenSingleGroup()
  {
    // Wholly one group across every tied axis (the appearance defaults cleared,
    // navigation and selection all on "nav") reads Mono.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      m_Editor->ClearSyncLink(CellId(0), dimension);
    }
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "nav");
    m_Editor->SetCellSelectionGroup(CellId(0), "nav");

    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A cell whose tied axes all name one group is mono",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("A mono cell carries the group hue", identity.hue.isValid());
  }

  void FrameIdentity_ComplexAcrossGroups()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "detail");

    CPPUNIT_ASSERT_MESSAGE("A cell spanning more than one group is complex, not a single hue",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Complex
        == m_Editor->ResolveCellGroupIdentity(CellId(0)).kind);
  }

  void FrameIdentity_SelectionParticipates()
  {
    // Selection is the 8th frame axis. A cell whose only group tie is a deliberate
    // data-selection group (the appearance defaults cleared) reads Mono of that
    // group - selection drives the frame like any other axis.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      m_Editor->ClearSyncLink(CellId(0), dimension);
    }
    m_Editor->SetCellSelectionGroup(CellId(0), "sel");

    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A selection-only cell is Mono of its selection group",
      QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("The frame hue is the selection group's color",
      identity.hue == m_Editor->GetSyncGroupColor("sel"));
  }

  void PaintPath_DoesNotCrash()
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(0));
    CPPUNIT_ASSERT(nullptr != cell);

    // Give the cell real geometry offscreen so paintEvent exercises its
    // arithmetic instead of early-returning on an invalid rect
    // (-platform minimal makes show() work without a display).
    m_Editor->resize(640, 480);
    m_Editor->show();
    QApplication::processEvents();
    CPPUNIT_ASSERT_MESSAGE("The cell must have real geometry for the paint smoke to be meaningful",
      cell->GetRenderWindow()->width() > 0 && cell->GetRenderWindow()->height() > 0);

    // A heterogeneous cell (complex dot bands) with a fully revealed, expanded
    // navigator drives the widest set of paint branches: colorbar tick scale,
    // colormap chip, navigator slider rows, coordinate line, and barcode.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "detail");
    m_Editor->SetNavigatorExpanded(true);

    auto* overlay = this->Overlay(0);
    overlay->SetRevealProgress(1.0);
    overlay->grab();  // forces a synchronous paintEvent
    m_Editor->SetNavigatorExpanded(false);
    overlay->SetRevealProgress(1.0);
    overlay->grab();

    m_Editor->hide();

    // Reaching here means the paint path did not crash across those branches.
    CPPUNIT_ASSERT(true);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNCellOverlay)
