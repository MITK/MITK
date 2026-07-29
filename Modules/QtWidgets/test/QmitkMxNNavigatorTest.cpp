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
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEvents.h>
#include <mitkImageGenerator.h>
#include <mitkInteractionEvent.h>
#include <mitkPlaneGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QFile>

#include <cmath>
#include <memory>

/**
 * Engine-facing tests for the per-cell navigator's synchronization contract:
 * the navigator drives the slice stepper (Slice dimension) and the world
 * crosshair (Crosshair dimension) by firing the same display-action events an
 * interaction would, so a linked cell follows its group and an unlinked cell
 * moves alone. Presentation and gesture are manual acceptance.
 */
class QmitkMxNNavigatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNNavigatorTestSuite);
  MITK_TEST(CrosshairEvent_PropagatesToGroupOnly);
  MITK_TEST(SliceEvent_PropagatesToGroupOnly);
  MITK_TEST(NavigatorMode_TogglesEditorWide);
  MITK_TEST(NavigatorDepthLabel_OmitsRedundantOrientation);
  MITK_TEST(NavigatorSlice_DrivesStepperAndGroup);
  MITK_TEST(NavigatorInPlaneMove_MapsToLocalAxes);
  MITK_TEST(NavigatorVoxelIndex_SetsCrosshair);
  MITK_TEST(SyncBarcode_ReflectsMembership);
  MITK_TEST(AxisGlyphResources_PresentAndThemeable);
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
    for (std::size_t i = 0; i < 3; ++i)
    {
      SliceStepper(i)->SetPos(4);
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
    CPPUNIT_ASSERT(nullptr != stepper);
    return stepper;
  }

  mitk::Point3D Crosshair(std::size_t index) const
  {
    return m_Editor->GetSelectedPosition(CellId(index));
  }

  QmitkMxNCellOverlay* Overlay(std::size_t index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* overlay = cell->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT(nullptr != overlay);
    return overlay;
  }

  template <typename TDisplayEvent, typename... TArgs>
  void Fire(std::size_t senderIndex, TArgs&&... args)
  {
    auto interactionEvent = mitk::InteractionEvent::New(Renderer(senderIndex));
    m_Editor->GetInteractionEventHandler()->InvokeEvent(
      TDisplayEvent(interactionEvent, std::forward<TArgs>(args)...));
  }

  static double Distance(const mitk::Point3D& a, const mitk::Point3D& b)
  {
    return a.EuclideanDistanceTo(b);
  }

  void CrosshairEvent_PropagatesToGroupOnly()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Crosshair, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Crosshair, "nav");

    const mitk::Point3D before2 = Crosshair(2);

    // A known in-volume world point (16 mm cube, 1 mm spacing), off the
    // current position on every axis.
    mitk::Point3D target;
    target[0] = 6.0;
    target[1] = 9.0;
    target[2] = 3.0;
    Fire<mitk::DisplaySetCrosshairEvent>(0, target);

    // The sender and its Crosshair-group peer land on the same position; the
    // unlinked cell keeps its own.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
      "Crosshair link must move the group peer to the sender's position",
      0.0, Distance(Crosshair(0), Crosshair(1)), 1e-3);
    CPPUNIT_ASSERT_MESSAGE("The sender crosshair actually moved",
      Distance(Crosshair(0), before2) > 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
      "An unlinked cell must not follow a crosshair change",
      0.0, Distance(Crosshair(2), before2), 1e-3);
  }

  void SliceEvent_PropagatesToGroupOnly()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    Fire<mitk::DisplayScrollEvent>(0, 1, false);

    CPPUNIT_ASSERT_EQUAL(5u, SliceStepper(0)->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slice-group peer follows", 5u, SliceStepper(1)->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlinked cell does not follow", 4u, SliceStepper(2)->GetPos());
  }

  void NavigatorMode_TogglesEditorWide()
  {
    m_Editor->SetNavigatorExpanded(true);
    for (std::size_t i = 0; i < 3; ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The mode toggle reaches every cell",
        this->Overlay(i)->IsNavigatorExpanded());
    }
    m_Editor->SetNavigatorExpanded(false);
    for (std::size_t i = 0; i < 3; ++i)
    {
      CPPUNIT_ASSERT(!this->Overlay(i)->IsNavigatorExpanded());
    }
  }

  void NavigatorDepthLabel_OmitsRedundantOrientation()
  {
    // The depth row is just "Slice": the orientation is already shown by the
    // plane label, so it is not repeated here for any view direction.
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Axial);
    CPPUNIT_ASSERT_EQUAL(std::string("Slice"),
      this->Overlay(0)->NavigatorDepthLabel().toStdString());

    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Coronal);
    CPPUNIT_ASSERT_EQUAL(std::string("Slice"),
      this->Overlay(0)->NavigatorDepthLabel().toStdString());
  }

  void NavigatorSlice_DrivesStepperAndGroup()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    // The depth row drives the cell's own slice stepper (through the display
    // broadcast, so the slice group follows and an unlinked cell does not).
    this->Overlay(0)->NavigatorSetSlice(6);

    CPPUNIT_ASSERT_EQUAL(6u, SliceStepper(0)->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slice-group peer follows the navigator", 6u, SliceStepper(1)->GetPos());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlinked cell stays put", 4u, SliceStepper(2)->GetPos());
  }

  void NavigatorInPlaneMove_MapsToLocalAxes()
  {
    const auto* plane = Renderer(0)->GetCurrentWorldPlaneGeometry();
    CPPUNIT_ASSERT(nullptr != plane);
    mitk::Vector3D rightUnit = plane->GetAxisVector(0);
    mitk::Vector3D upUnit = plane->GetAxisVector(1);
    rightUnit.Normalize();
    upUnit.Normalize();

    // A horizontal navigator move maps to the plane's own right axis (oblique-
    // safe), leaving the in-plane vertical component unchanged.
    const mitk::Point3D before = Crosshair(0);
    constexpr double delta = 3.0;
    this->Overlay(0)->NavigatorMoveInPlane(delta, 0.0);
    const mitk::Vector3D moved = Crosshair(0) - before;

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Horizontal move follows the plane right axis",
      delta, moved * rightUnit, 1.0);
    CPPUNIT_ASSERT_MESSAGE("Horizontal move does not drift along the vertical axis",
      std::abs(moved * upUnit) < 1.0);
  }

  void NavigatorVoxelIndex_SetsCrosshair()
  {
    mitk::Point3D index;
    index[0] = 5.0;
    index[1] = 6.0;
    index[2] = 3.0;
    this->Overlay(0)->NavigatorSetVoxelIndex(index);

    mitk::Point3D expected;
    m_Image->GetGeometry()->IndexToWorld(index, expected);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Voxel-index entry lands on the reference geometry voxel",
      0.0, Crosshair(0).EuclideanDistanceTo(expected), 1.0);
  }

  void SyncBarcode_ReflectsMembership()
  {
    // The Synchronize macro links the navigation bundle (pan/zoom/slice/
    // crosshair) editor-wide and fires SyncLinksChanged, which refreshes the
    // per-cell utility-strip barcodes.
    m_Editor->Synchronize(true);

    const auto cell = m_Editor->GetRenderWindowWidget(CellId(0));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* utility = cell->GetUtilityWidget();
    CPPUNIT_ASSERT(nullptr != utility);
    auto* barcode = utility->findChild<QmitkMxNSyncBarcodeWidget*>();
    CPPUNIT_ASSERT(nullptr != barcode);

    const auto axisSlots = barcode->Slots();
    // Seven per-dimension axes plus the selection axis.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1,
                         static_cast<int>(axisSlots.size()));
    for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
    {
      const auto dimension = QmitkMxNAllSyncDimensions[i];
      const bool inNavigationBundle =
        dimension == QmitkMxNSyncDimension::Pan || dimension == QmitkMxNSyncDimension::Zoom
        || dimension == QmitkMxNSyncDimension::Slice || dimension == QmitkMxNSyncDimension::Crosshair;
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "A linked dimension is a filled barcode slot in the group hue; an unsynced one is a gap",
        inNavigationBundle, axisSlots[static_cast<int>(i)].color.isValid());
    }
    // Synchronize links only the navigation bundle, not data selection, so the
    // cell stays in the default selection group and its selection slot is a gap.
    CPPUNIT_ASSERT_MESSAGE(
      "The selection slot is a gap while the cell is in the default selection group",
      !axisSlots.back().color.isValid());
  }

  void AxisGlyphResources_PresentAndThemeable()
  {
    // Every axis glyph the barcode draws must be an embedded resource carrying
    // the recolor placeholder, so it can be tinted to a group hue at load. The
    // actual rasterization is a visual-acceptance concern, not asserted here.
    const QStringList paths = {
      QStringLiteral(":/Qmitk/mxn-axis-pan.svg"),         QStringLiteral(":/Qmitk/mxn-axis-zoom.svg"),
      QStringLiteral(":/Qmitk/mxn-axis-slice.svg"),       QStringLiteral(":/Qmitk/mxn-axis-crosshair.svg"),
      QStringLiteral(":/Qmitk/mxn-axis-orientation.svg"), QStringLiteral(":/Qmitk/mxn-axis-windowing.svg"),
      QStringLiteral(":/Qmitk/mxn-axis-lut.svg"),         QStringLiteral(":/Qmitk/mxn-axis-selection.svg") };
    for (const auto& path : paths)
    {
      QFile file(path);
      CPPUNIT_ASSERT_MESSAGE(("axis glyph resource is registered: " + path).toStdString(),
                             file.open(QIODevice::ReadOnly));
      const QString svg = QString::fromUtf8(file.readAll());
      CPPUNIT_ASSERT_MESSAGE(("axis glyph carries the recolor placeholder: " + path).toStdString(),
                             svg.contains(QStringLiteral("#00ff00"), Qt::CaseInsensitive));
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNNavigator)
