/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxN3DCamera.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkTimeNavigationController.h>

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <nlohmann/json.hpp>

#include <cmath>

/**
 * Tests 3D cells of QmitkMxNMultiWidget without rendering (camera state is
 * read from the renderer's vtkCamera):
 *   - Loading a 3D cell switches its mapper and installs the MxN interactor
 *     style; render mode and direction round-trip through the layout.
 *   - A 3D cell looks along minus the normal of the plane a 2D cell with the
 *     same direction and reference geometry shows, with the plane's second
 *     axis as view-up, also for a rotated geometry.
 *   - Switching 2D <-> 3D keeps the direction and the global time step; a
 *     'free' cell turns 2D with the nearest standard plane.
 *   - Camera rotations turn an unlinked cell 'free'; pan and dolly do not.
 *   - A view initialization, which turns 3D renderers anterior, is followed
 *     by the cell's direction again; 'free' cells are left alone.
 *   - Links on axes without 3D meaning are inactive: a 3D member never
 *     anchors or converges a pan, zoom or slice group.
 *   - Name-based window lookups prefer cells of the requested mode.
 */
class QmitkMxN3DCellTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxN3DCellTestSuite);

  MITK_TEST(Load_ThreeDCell_HasMapperAndStyle);
  MITK_TEST(Load_FreeCell_ReportsFree);
  MITK_TEST(RoundTrip_ThreeDAndFree_SerializesBack);
  MITK_TEST(RoundTrip_TwoDOnly_HasNoRenderMode);
  MITK_TEST(ViewFrame_MatchesTwoDCell_AxisAligned);
  MITK_TEST(ViewFrame_MatchesTwoDCell_RotatedGeometry);
  MITK_TEST(SetViewDirection_OriginalAndFreeOn2D_Throw);
  MITK_TEST(SetViewDirection_FreeOnLinkedCell_ThrowsConflict);
  MITK_TEST(OrientationLink_FreeCellAlone_ThrowsConflict);
  MITK_TEST(OrientationLink_FreeCellJoins_SnapsToGroup);
  MITK_TEST(Switch_TwoDThreeDTwoD_KeepsDirection);
  MITK_TEST(Switch_FreeToTwoD_TakesNearestPlane);
  MITK_TEST(Switch_KeepsTimeStep);
  MITK_TEST(Switch_EmitsSyncLinksChanged);
  MITK_TEST(Lifecycle_ReusedIdOfFreeCell_IsCanonical);
  MITK_TEST(Rotation_Azimuth_TurnsCellFree);
  MITK_TEST(Rotation_Roll_TurnsCellFree);
  MITK_TEST(Rotation_Dolly_KeepsDirection);
  MITK_TEST(Rotation_LinkedCell_StaysCanonical);
  MITK_TEST(Reinit_RestoresDirection);
  MITK_TEST(Reinit_LeavesFreeCellAlone);
  MITK_TEST(Reinit_AfterLoad_RestoresDirection);
  MITK_TEST(Reinit_TwoEditors_EachKeepsItsDirection);
  MITK_TEST(Applicability_PerRenderMode);
  MITK_TEST(Gate_ZoomGroup_ThreeDNeverAnchors);
  MITK_TEST(Gate_PanGroup_ThreeDNeverAnchors);
  MITK_TEST(Gate_SliceGroup_ThreeDNeverAnchors);
  MITK_TEST(Gate_AllThreeDZoomGroup_ReconvergeIsNoOp);
  MITK_TEST(Routing_ThreeD_PrefersActiveThenFirst);
  MITK_TEST(Routing_Axial_PrefersTwoD);
  MITK_TEST(Routing_NoThreeDCell_ReturnsActive);
  MITK_TEST(InitializeViews_ThreeDActive_BroadcastsTwoDPosition);

  CPPUNIT_TEST_SUITE_END();

  using RenderMode = QmitkMxNMultiWidget::RenderMode;
  using ViewDirection = QmitkMxNMultiWidget::ViewDirection;

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    // Anisotropic extents, so the three standard planes differ in size.
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 12, 8, 1.0f, 1.0f, 1.0f);

    // Explicit layer for the node-table-model comparator workaround (see
    // QmitkMxNSyncGroupApiTest).
    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_ImageNode);

    m_Editor = this->MakeEditor("mxn");
    m_Editor->SetLayout(1, 3); // cells: mxn__widget0 .. mxn__widget2
    this->InitializeCells(*m_Editor, m_Image->GetTimeGeometry());
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_ImageNode = nullptr;
    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  std::unique_ptr<QmitkMxNMultiWidget> MakeEditor(const QString& name) const
  {
    auto editor = std::make_unique<QmitkMxNMultiWidget>(nullptr, Qt::WindowFlags(), name);
    editor->SetDataStorage(m_DataStorage);
    editor->InitializeMultiWidget();
    return editor;
  }

  static void InitializeCells(const QmitkMxNMultiWidget& editor, const mitk::TimeGeometry* geometry)
  {
    for (const auto& [name, cell] : editor.GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(cell->GetRenderWindow()->GetVtkRenderWindow(), geometry);
    }
  }

  static QString CellId(std::size_t index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  mitk::BaseRenderer* Renderer(const QString& id) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(id);
    CPPUNIT_ASSERT(nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
  }

  mitk::BaseRenderer* Renderer(std::size_t index) const
  {
    return this->Renderer(CellId(index));
  }

  mitk::SliceNavigationController* Snc(std::size_t index) const
  {
    return m_Editor->GetRenderWindowWidget(CellId(index))->GetSliceNavigationController();
  }

  static vtkCamera* Camera(mitk::BaseRenderer* renderer)
  {
    return renderer->GetVtkRenderer()->GetActiveCamera();
  }

  static mitk::Vector3D DirectionOfProjection(mitk::BaseRenderer* renderer)
  {
    mitk::Vector3D result;
    Camera(renderer)->GetDirectionOfProjection(result.GetDataPointer());
    return result;
  }

  static mitk::Vector3D ViewUp(mitk::BaseRenderer* renderer)
  {
    mitk::Vector3D result;
    Camera(renderer)->GetViewUp(result.GetDataPointer());
    return result;
  }

  static bool Near(const mitk::Vector3D& a, const mitk::Vector3D& b)
  {
    return (a - b).GetNorm() < 1e-6;
  }

  static std::string ToString(const mitk::Vector3D& v)
  {
    return "(" + std::to_string(v[0]) + ", " + std::to_string(v[1]) + ", " + std::to_string(v[2]) + ")";
  }

  /** The camera frame a 3D cell must have to look at 'plane' like a 2D cell. */
  static void AssertLooksAt(mitk::BaseRenderer* threeD, const mitk::PlaneGeometry* plane, const std::string& what)
  {
    CPPUNIT_ASSERT_MESSAGE(what + ": the 2D cell has a plane", nullptr != plane);
    auto normal = plane->GetNormal();
    normal.Normalize();
    auto axis1 = plane->GetAxisVector(1);
    axis1.Normalize();
    const auto dop = DirectionOfProjection(threeD);
    const auto up = ViewUp(threeD);
    CPPUNIT_ASSERT_MESSAGE(what + ": direction of projection " + ToString(dop) + " == -normal " + ToString(-normal),
                           Near(dop, -normal));
    CPPUNIT_ASSERT_MESSAGE(what + ": view-up " + ToString(up) + " == axis 1 " + ToString(axis1), Near(up, axis1));
  }

  /** The canonical frame of 'plane' on the editor's reference geometry. */
  mitk::PlaneGeometry::ConstPointer StandardPlane(mitk::AnatomicalPlane plane, const mitk::TimeGeometry* geometry)
  {
    auto* snc = Snc(2);
    snc->SetDefaultViewDirection(plane);
    mitk::RenderingManager::GetInstance()->InitializeView(Renderer(2)->GetRenderWindow(), geometry, true);
    return snc->GetCurrentPlaneGeometry();
  }

  static nlohmann::json ThreeDDoc()
  {
    return nlohmann::json::parse(R"json({
      "version": "3.0",
      "groups": { "main": { "select_all": true } },
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__slice", "view_direction": "sagittal",
            "links": { "selection": "main" } },
          { "type": "window", "id": "mxn__volume", "view_direction": "sagittal", "render_mode": "3d",
            "links": { "selection": "main" } },
          { "type": "window", "id": "mxn__overview", "view_direction": "free", "render_mode": "3d",
            "links": { "selection": "main" } }
        ]
      }
    })json");
  }

  static QmitkMxN3DInteractorStyle* Style(mitk::BaseRenderer* renderer)
  {
    auto* interactor = renderer->GetRenderWindow()->GetInteractor();
    return nullptr != interactor ? QmitkMxN3DInteractorStyle::SafeDownCast(interactor->GetInteractorStyle()) : nullptr;
  }

  /** A gesture as the interactor style reports it, with 'move' as the camera change. */
  template <typename Move>
  static void Gesture(mitk::BaseRenderer* renderer, Move move)
  {
    auto* style = Style(renderer);
    CPPUNIT_ASSERT(nullptr != style);
    style->InvokeEvent(vtkCommand::StartInteractionEvent);
    move(Camera(renderer));
    style->InvokeEvent(vtkCommand::EndInteractionEvent);
  }

  static mitk::Image::Pointer Tilted(const mitk::Image* image)
  {
    auto tilted = image->Clone();
    auto* geometry = tilted->GetGeometry();
    auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->DeepCopy(geometry->GetVtkMatrix());
    const double angle = 20.0 * 3.14159265358979 / 180.0;
    auto rotation = vtkSmartPointer<vtkMatrix4x4>::New();
    rotation->Identity();
    rotation->SetElement(1, 1, std::cos(angle));
    rotation->SetElement(1, 2, -std::sin(angle));
    rotation->SetElement(2, 1, std::sin(angle));
    rotation->SetElement(2, 2, std::cos(angle));
    auto tiltedMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Multiply4x4(rotation, matrix, tiltedMatrix);
    geometry->SetIndexToWorldTransformByVtkMatrix(tiltedMatrix);
    return tilted;
  }

  void Load_ThreeDCell_HasMapperAndStyle()
  {
    m_Editor->ApplyLayout(ThreeDDoc());

    CPPUNIT_ASSERT(RenderMode::ThreeD == m_Editor->GetRenderMode("mxn__volume"));
    CPPUNIT_ASSERT(RenderMode::TwoD == m_Editor->GetRenderMode("mxn__slice"));
    CPPUNIT_ASSERT(mitk::BaseRenderer::Standard3D == Renderer(QStringLiteral("mxn__volume"))->GetMapperID());
    CPPUNIT_ASSERT_MESSAGE("The 3D cell navigates with the MxN interactor style",
                           nullptr != Style(Renderer(QStringLiteral("mxn__volume"))));
    CPPUNIT_ASSERT(ViewDirection::Sagittal == m_Editor->GetViewDirection("mxn__volume"));
  }

  void Load_FreeCell_ReportsFree()
  {
    m_Editor->ApplyLayout(ThreeDDoc());
    CPPUNIT_ASSERT(ViewDirection::Free == m_Editor->GetViewDirection("mxn__overview"));

    const auto descriptors = m_Editor->ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), descriptors.size());
    CPPUNIT_ASSERT_EQUAL(std::string("free"), descriptors[2].viewDirection.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("3d"), descriptors[2].renderMode.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("2d"), descriptors[0].renderMode.toStdString());
  }

  /** The window nodes of a document, without the live splitter sizes. */
  static nlohmann::json Windows(nlohmann::json doc)
  {
    auto children = doc["root"]["children"];
    for (auto& child : children)
    {
      child.erase("size");
    }
    return children;
  }

  void RoundTrip_ThreeDAndFree_SerializesBack()
  {
    const auto doc = ThreeDDoc();
    m_Editor->ApplyLayout(doc);
    const auto saved = m_Editor->SerializeLayout();
    CPPUNIT_ASSERT_EQUAL(Windows(doc).dump(), Windows(saved).dump());

    // And the saved document loads into the same state.
    auto other = this->MakeEditor("mxn");
    other->ApplyLayout(saved);
    CPPUNIT_ASSERT_EQUAL(saved.dump(), other->SerializeLayout().dump());
  }

  void RoundTrip_TwoDOnly_HasNoRenderMode()
  {
    const auto saved = m_Editor->SerializeLayout();
    for (const auto& child : saved["root"]["children"].front()["children"])
    {
      CPPUNIT_ASSERT_MESSAGE("2D windows serialize as before", !child.contains("render_mode"));
    }
  }

  void AssertFramesMatch(const mitk::TimeGeometry* geometry, const std::string& what)
  {
    auto* renderingManager = mitk::RenderingManager::GetInstance();
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    for (const auto plane : { mitk::AnatomicalPlane::Axial, mitk::AnatomicalPlane::Coronal,
                              mitk::AnatomicalPlane::Sagittal })
    {
      Snc(0)->SetDefaultViewDirection(plane);
      renderingManager->InitializeView(Renderer(0)->GetRenderWindow(), geometry, true);
      renderingManager->InitializeView(Renderer(1)->GetRenderWindow(), geometry, true);
      m_Editor->SetViewDirection(CellId(1), plane);

      AssertLooksAt(Renderer(1), Snc(0)->GetCurrentPlaneGeometry(),
                    what + " plane " + std::to_string(static_cast<int>(plane)));
    }
  }

  void ViewFrame_MatchesTwoDCell_AxisAligned()
  {
    this->AssertFramesMatch(m_Image->GetTimeGeometry(), "axis-aligned");
  }

  void ViewFrame_MatchesTwoDCell_RotatedGeometry()
  {
    const auto tilted = Tilted(m_Image);
    this->AssertFramesMatch(tilted->GetTimeGeometry(), "tilted");
  }

  void SetViewDirection_OriginalAndFreeOn2D_Throw()
  {
    CPPUNIT_ASSERT_THROW(m_Editor->SetViewDirection(CellId(0), ViewDirection::Original), mitk::Exception);
    CPPUNIT_ASSERT_THROW(m_Editor->SetViewDirection(CellId(0), ViewDirection::Free), mitk::Exception);
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    CPPUNIT_ASSERT_THROW(m_Editor->SetViewDirection(CellId(0), ViewDirection::Original), mitk::Exception);
    CPPUNIT_ASSERT_THROW(m_Editor->SetViewDirection("mxn__nowhere", ViewDirection::Axial), mitk::Exception);
  }

  void SetViewDirection_FreeOnLinkedCell_ThrowsConflict()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "planes");
    CPPUNIT_ASSERT_THROW(m_Editor->SetViewDirection(CellId(0), ViewDirection::Free),
                         QmitkMxNOrientationConflictException);
    CPPUNIT_ASSERT(ViewDirection::Free != m_Editor->GetViewDirection(CellId(0)));
  }

  void OrientationLink_FreeCellAlone_ThrowsConflict()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(0), ViewDirection::Free);
    CPPUNIT_ASSERT_THROW(m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes"),
                         QmitkMxNOrientationConflictException);
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation).has_value());
    CPPUNIT_ASSERT(ViewDirection::Free == m_Editor->GetViewDirection(CellId(0)));
  }

  void OrientationLink_FreeCellJoins_SnapsToGroup()
  {
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Sagittal);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes");

    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(1), mitk::AnatomicalPlane::Axial);
    Camera(Renderer(1))->Azimuth(30.0);
    m_Editor->SetViewDirection(CellId(1), ViewDirection::Free);

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "planes");

    CPPUNIT_ASSERT(ViewDirection::Sagittal == m_Editor->GetViewDirection(CellId(1)));
    AssertLooksAt(Renderer(1), Snc(0)->GetCurrentPlaneGeometry(), "joined free cell");
  }

  void Switch_TwoDThreeDTwoD_KeepsDirection()
  {
    m_Editor->SetViewDirection(CellId(1), mitk::AnatomicalPlane::Sagittal);
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    CPPUNIT_ASSERT(ViewDirection::Sagittal == m_Editor->GetViewDirection(CellId(1)));
    m_Editor->SetRenderMode(CellId(1), RenderMode::TwoD);

    CPPUNIT_ASSERT(RenderMode::TwoD == m_Editor->GetRenderMode(CellId(1)));
    CPPUNIT_ASSERT(mitk::BaseRenderer::Standard2D == Renderer(1)->GetMapperID());
    CPPUNIT_ASSERT(ViewDirection::Sagittal == m_Editor->GetViewDirection(CellId(1)));
    CPPUNIT_ASSERT(mitk::AnatomicalPlane::Sagittal == Snc(1)->GetViewDirection());
  }

  void Switch_FreeToTwoD_TakesNearestPlane()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(1), mitk::AnatomicalPlane::Coronal);
    Gesture(Renderer(1), [](vtkCamera* camera) { camera->Azimuth(10.0); });
    CPPUNIT_ASSERT(ViewDirection::Free == m_Editor->GetViewDirection(CellId(1)));

    m_Editor->SetRenderMode(CellId(1), RenderMode::TwoD);
    CPPUNIT_ASSERT(ViewDirection::Coronal == m_Editor->GetViewDirection(CellId(1)));
  }

  void Switch_KeepsTimeStep()
  {
    const auto image4D = mitk::ImageGenerator::GenerateRandomImage<short>(16, 12, 8, 4);
    this->InitializeCells(*m_Editor, image4D->GetTimeGeometry());
    auto* stepper = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetStepper();
    stepper->SetPos(2);

    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    CPPUNIT_ASSERT_EQUAL(2u, stepper->GetPos());
    m_Editor->SetRenderMode(CellId(1), RenderMode::TwoD);
    CPPUNIT_ASSERT_EQUAL(2u, stepper->GetPos());
  }

  void Switch_EmitsSyncLinksChanged()
  {
    int notifications = 0;
    QObject::connect(m_Editor.get(), &QmitkMxNMultiWidget::SyncLinksChanged, [&notifications]() { ++notifications; });
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    CPPUNIT_ASSERT_EQUAL(1, notifications);
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No change, no notification", 1, notifications);
  }

  void Lifecycle_ReusedIdOfFreeCell_IsCanonical()
  {
    m_Editor->SetRenderMode(CellId(2), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(2), ViewDirection::Free);
    m_Editor->SetLayout(1, 2);
    m_Editor->SetLayout(1, 3);

    CPPUNIT_ASSERT(nullptr != m_Editor->GetRenderWindowWidget(CellId(2)));
    CPPUNIT_ASSERT(ViewDirection::Free != m_Editor->GetViewDirection(CellId(2)));
    CPPUNIT_ASSERT(RenderMode::TwoD == m_Editor->GetRenderMode(CellId(2)));
  }

  void Rotation_Azimuth_TurnsCellFree()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    Gesture(Renderer(1), [](vtkCamera* camera) { camera->Azimuth(10.0); });
    CPPUNIT_ASSERT(ViewDirection::Free == m_Editor->GetViewDirection(CellId(1)));
  }

  void Rotation_Roll_TurnsCellFree()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    Gesture(Renderer(1), [](vtkCamera* camera) { camera->Roll(10.0); });
    CPPUNIT_ASSERT_MESSAGE("A change of view-up alone is a rotation",
                           ViewDirection::Free == m_Editor->GetViewDirection(CellId(1)));
  }

  void Rotation_Dolly_KeepsDirection()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    const auto before = m_Editor->GetViewDirection(CellId(1));
    Gesture(Renderer(1), [](vtkCamera* camera) { camera->Dolly(1.5); });
    CPPUNIT_ASSERT(before == m_Editor->GetViewDirection(CellId(1)));
  }

  void Rotation_LinkedCell_StaysCanonical()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Orientation, "planes");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Orientation, "planes");
    Gesture(Renderer(1), [](vtkCamera* camera) { camera->Azimuth(10.0); });
    CPPUNIT_ASSERT(ViewDirection::Free != m_Editor->GetViewDirection(CellId(1)));
  }

  void Reinit_RestoresDirection()
  {
    // Sagittal: coronal coincides with the anterior view the reset produces.
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(1), mitk::AnatomicalPlane::Sagittal);
    const auto expected = this->StandardPlane(mitk::AnatomicalPlane::Sagittal, m_Image->GetTimeGeometry());

    mitk::RenderingManager::GetInstance()->InitializeView(
      Renderer(1)->GetRenderWindow(), m_Image->GetTimeGeometry(), true);
    AssertLooksAt(Renderer(1), expected, "after a reset");

    mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(Renderer(1)->GetRenderWindow(), m_DataStorage);
    AssertLooksAt(Renderer(1), expected, "after a reset to the data bounds");
  }

  void Reinit_LeavesFreeCellAlone()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(1), ViewDirection::Free);
    Camera(Renderer(1))->Azimuth(25.0);
    const auto dop = DirectionOfProjection(Renderer(1));

    mitk::RenderingManager::GetInstance()->InitializeView(
      Renderer(1)->GetRenderWindow(), m_Image->GetTimeGeometry(), false);
    CPPUNIT_ASSERT_MESSAGE("A view initialization without reset keeps the free camera",
                           Near(dop, DirectionOfProjection(Renderer(1))));
    CPPUNIT_ASSERT(ViewDirection::Free == m_Editor->GetViewDirection(CellId(1)));
  }

  void Reinit_AfterLoad_RestoresDirection()
  {
    m_Editor->ApplyLayout(ThreeDDoc());
    mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(
      Renderer(QStringLiteral("mxn__slice"))->GetRenderWindow(), m_DataStorage);
    mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(
      Renderer(QStringLiteral("mxn__volume"))->GetRenderWindow(), m_DataStorage);

    const auto* slicePlane =
      m_Editor->GetRenderWindowWidget("mxn__slice")->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    AssertLooksAt(Renderer(QStringLiteral("mxn__volume")), slicePlane, "loaded 3D cell after its first reinit");
  }

  void Reinit_TwoEditors_EachKeepsItsDirection()
  {
    auto other = this->MakeEditor("other");
    other->SetLayout(1, 1);
    const auto otherId = QStringLiteral("other__widget0");
    auto* otherRenderer = mitk::BaseRenderer::GetInstance(
      other->GetRenderWindowWidget(otherId)->GetRenderWindow()->GetVtkRenderWindow());
    mitk::RenderingManager::GetInstance()->InitializeView(otherRenderer->GetRenderWindow(), m_Image->GetTimeGeometry());
    other->SetRenderMode(otherId, RenderMode::ThreeD);
    other->SetViewDirection(otherId, mitk::AnatomicalPlane::Axial);

    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetViewDirection(CellId(1), mitk::AnatomicalPlane::Sagittal);

    const auto axial = this->StandardPlane(mitk::AnatomicalPlane::Axial, m_Image->GetTimeGeometry());
    const auto sagittal = this->StandardPlane(mitk::AnatomicalPlane::Sagittal, m_Image->GetTimeGeometry());
    mitk::RenderingManager::GetInstance()->InitializeViews(m_Image->GetTimeGeometry());

    AssertLooksAt(Renderer(1), sagittal, "first editor's 3D cell");
    AssertLooksAt(otherRenderer, axial, "second editor's 3D cell");
  }

  void Applicability_PerRenderMode()
  {
    for (int slot = 0; slot < QmitkMxNSyncAxisCount; ++slot)
    {
      CPPUNIT_ASSERT(m_Editor->IsAxisApplicable(CellId(0), *QmitkMxNSyncAxisFromSlot(slot)));
    }
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    for (int slot = 0; slot < QmitkMxNSyncAxisCount; ++slot)
    {
      const auto axis = *QmitkMxNSyncAxisFromSlot(slot);
      const bool expected = QmitkMxNSyncAxis::Orientation == axis || QmitkMxNSyncAxis::Selection == axis;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("slot " + std::to_string(slot), expected,
                                   m_Editor->IsAxisApplicable(CellId(0), axis));
    }
    CPPUNIT_ASSERT_THROW(m_Editor->IsAxisApplicable("mxn__nowhere", QmitkMxNSyncAxis::Pan), mitk::Exception);
  }

  void Gate_ZoomGroup_ThreeDNeverAnchors()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    Camera(Renderer(0))->SetParallelScale(3.0);
    Camera(Renderer(1))->SetParallelScale(7.0);

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "nav");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, Camera(Renderer(1))->GetParallelScale(), 1e-9);

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Zoom, "nav");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, Camera(Renderer(1))->GetParallelScale(), 1e-9);
  }

  void Gate_PanGroup_ThreeDNeverAnchors()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    auto* controller = Renderer(1)->GetCameraController();
    mitk::Point2D target;
    target[0] = 3.0;
    target[1] = 2.0;
    controller->MoveCameraToPoint(target);
    const auto before = controller->GetCameraPositionOnPlane();

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, "nav");
    CPPUNIT_ASSERT((before - controller->GetCameraPositionOnPlane()).GetNorm() < 1e-9);

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Pan, "nav");
    CPPUNIT_ASSERT((before - controller->GetCameraPositionOnPlane()).GetNorm() < 1e-9);
  }

  void Gate_SliceGroup_ThreeDNeverAnchors()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    Snc(1)->GetStepper()->SetPos(3);
    Snc(0)->GetStepper()->SetPos(0);

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT_EQUAL(3u, Snc(1)->GetStepper()->GetPos());

    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT_EQUAL(3u, Snc(1)->GetStepper()->GetPos());
  }

  void Gate_AllThreeDZoomGroup_ReconvergeIsNoOp()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "nav");
    m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Zoom, "nav");
    // A plane change re-converges the cell's groups; no member is applicable.
    m_Editor->SetViewDirection(CellId(0), mitk::AnatomicalPlane::Axial);
    CPPUNIT_ASSERT_THROW(m_Editor->ReconvergeSyncGroup(QmitkMxNSyncDimension::Zoom, "unknown"), mitk::Exception);
  }

  QmitkRenderWindow* Window(std::size_t index) const
  {
    return m_Editor->GetRenderWindowWidget(CellId(index))->GetRenderWindow();
  }

  void Activate(std::size_t index)
  {
    m_Editor->SetActiveRenderWindowWidget(m_Editor->GetRenderWindowWidget(CellId(index)));
  }

  void Routing_ThreeD_PrefersActiveThenFirst()
  {
    m_Editor->SetRenderMode(CellId(1), RenderMode::ThreeD);
    m_Editor->SetRenderMode(CellId(2), RenderMode::ThreeD);

    this->Activate(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("first 3D cell in document order", Window(1), m_Editor->GetRenderWindow("3d"));
    this->Activate(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the active 3D cell", Window(2), m_Editor->GetRenderWindow("3d"));
  }

  void Routing_Axial_PrefersTwoD()
  {
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    this->Activate(0);
    CPPUNIT_ASSERT_EQUAL(Window(1), m_Editor->GetRenderWindow("axial"));
    CPPUNIT_ASSERT_EQUAL(Window(1), m_Editor->GetRenderWindow(mitk::AnatomicalPlane::Coronal));
    this->Activate(2);
    CPPUNIT_ASSERT_EQUAL(Window(2), m_Editor->GetRenderWindow("sagittal"));
  }

  void Routing_NoThreeDCell_ReturnsActive()
  {
    this->Activate(1);
    CPPUNIT_ASSERT_EQUAL(Window(1), m_Editor->GetRenderWindow("3d"));
    CPPUNIT_ASSERT_EQUAL(Window(1), m_Editor->GetRenderWindow("axial"));
  }

  void InitializeViews_ThreeDActive_BroadcastsTwoDPosition()
  {
    // Cells 1 and 2 show the same plane, so the broadcast position selects the
    // same slice in both; a position off the volume's center tells the first
    // 2D cell's position from the 3D cell's.
    m_Editor->SetRenderMode(CellId(0), RenderMode::ThreeD);
    mitk::Point3D position;
    position[0] = 2.0;
    position[1] = 2.0;
    position[2] = 2.0;
    m_Editor->SetSelectedPosition(position, CellId(1));
    CPPUNIT_ASSERT_MESSAGE("precondition: the cells start on different slices",
                           Snc(1)->GetStepper()->GetPos() != Snc(2)->GetStepper()->GetPos());

    this->Activate(0);
    m_Editor->InitializeViews(m_Image->GetTimeGeometry(), false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The other 2D cell is moved to the first 2D cell's position",
                                 Snc(1)->GetStepper()->GetPos(), Snc(2)->GetStepper()->GetPos());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxN3DCell)
