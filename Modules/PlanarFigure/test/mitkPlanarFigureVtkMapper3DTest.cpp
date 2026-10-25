/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlaneGeometry.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkVtkMapper.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkTriangle.h>

#include <numbers>

class mitkPlanarFigureVtkMapper3DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureVtkMapper3DTestSuite);
  MITK_TEST(Figure_IsHiddenByDefault);
  MITK_TEST(Stamp_IsHidden);
  MITK_TEST(Fill_LeavesHoleOpen);
  MITK_TEST(Pulse_ScalesFigureAroundItsCenter);
  CPPUNIT_TEST_SUITE_END();

  mitk::RenderingTestHelper m_RenderingTestHelper;
  mitk::DataNode::Pointer m_Node;

  static mitk::Point2D P(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  vtkActor *GetRenderedActor()
  {
    m_RenderingTestHelper.Render();

    auto *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());
    auto *mapper = dynamic_cast<mitk::VtkMapper *>(m_Node->GetMapper(mitk::BaseRenderer::Standard3D));
    CPPUNIT_ASSERT(nullptr != mapper);

    auto *actor = dynamic_cast<vtkActor *>(mapper->GetVtkProp(renderer));
    CPPUNIT_ASSERT(nullptr != actor);

    return actor;
  }

  vtkPolyData *GetRenderedPolyData()
  {
    return vtkPolyData::SafeDownCast(this->GetRenderedActor()->GetMapper()->GetInput());
  }

public:
  mitkPlanarFigureVtkMapper3DTestSuite() : m_RenderingTestHelper(300, 300) {}

  void setUp() override
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(300, 300);
    m_RenderingTestHelper.SetMapperIDToRender3D();

    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(100.0, 100.0);

    // A ring of 20 by 10 mm, 4 mm thick.
    auto ring = mitk::PlanarDoubleEllipse::New();
    ring->SetPlaneGeometry(plane);
    ring->SetFromSpan(P(50, 50), P(70, 50));
    ring->MoveHandle(2, P(50, 60));
    ring->SetThickness(4.0);

    m_Node = mitk::DataNode::New();
    m_Node->SetData(ring);
    m_RenderingTestHelper.AddNodeToStorage(m_Node);
  }

  void tearDown() override
  {
    m_Node = nullptr;
  }

  void Figure_IsHiddenByDefault()
  {
    CPPUNIT_ASSERT_MESSAGE("Hidden by default", !m_RenderingTestHelper.RendersAnything());

    m_Node->SetBoolProperty("planarfigure.3drendering", true);
    CPPUNIT_ASSERT_MESSAGE("Shown in 3D", m_RenderingTestHelper.RendersAnything());
  }

  void Stamp_IsHidden()
  {
    m_Node->SetBoolProperty("planarfigure.3drendering", true);

    auto *figure = dynamic_cast<mitk::PlanarFigure *>(m_Node->GetData());
    figure->BeginPlacement();
    figure->SetShapeLocked(true);
    CPPUNIT_ASSERT_MESSAGE("A preset that follows the mouse is hidden", !m_RenderingTestHelper.RendersAnything());

    figure->EndPlacement();
    CPPUNIT_ASSERT_MESSAGE("A placed preset is shown", m_RenderingTestHelper.RendersAnything());
  }

  void Fill_LeavesHoleOpen()
  {
    m_Node->SetBoolProperty("planarfigure.3drendering", true);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), this->GetRenderedPolyData()->GetNumberOfPolys());

    m_Node->SetBoolProperty("planarfigure.3drendering.fill", true);
    auto *polyData = this->GetRenderedPolyData();

    double area = 0.0;
    auto *points = polyData->GetPoints();

    for (vtkIdType i = 0; i < polyData->GetNumberOfPolys(); ++i)
    {
      const vtkIdType *ids = nullptr;
      vtkIdType numberOfIds = 0;
      polyData->GetPolys()->GetCellAtId(i, numberOfIds, ids);
      CPPUNIT_ASSERT_EQUAL(vtkIdType(3), numberOfIds);

      double p0[3], p1[3], p2[3];
      points->GetPoint(ids[0], p0);
      points->GetPoint(ids[1], p1);
      points->GetPoint(ids[2], p2);
      area += vtkTriangle::TriangleArea(p0, p1, p2);
    }

    // The sampled ellipses are polygons inscribed in the exact ones.
    const auto ringArea = std::numbers::pi * (20.0 * 10.0 - 16.0 * 6.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(ringArea, area, 0.01 * ringArea);
  }

  // The animation time follows a steady clock, so the scale is anywhere within the pulse.
  void Pulse_ScalesFigureAroundItsCenter()
  {
    m_Node->SetBoolProperty("planarfigure.3drendering", true);
    m_Node->SetBoolProperty("animated.pulse", true);

    auto *transform = this->GetRenderedActor()->GetUserTransform();
    CPPUNIT_ASSERT_MESSAGE("The figure pulses", nullptr != transform);

    const auto scale = transform->GetMatrix()->GetElement(0, 0);
    CPPUNIT_ASSERT_MESSAGE("Scale of the pulse", scale >= 1.0 && scale <= 1.08);

    mitk::Point3D center;
    static_cast<mitk::PlanarFigure *>(m_Node->GetData())->GetPlaneGeometry()->Map(P(50, 50), center);

    double transformedCenter[3];
    transform->TransformPoint(center.GetDataPointer(), transformedCenter);

    for (int i = 0; i < 3; ++i)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(center[i], transformedCenter[i], 1e-9);

    m_Node->SetBoolProperty("animated.pulse", false);
    CPPUNIT_ASSERT_MESSAGE("The figure rests", nullptr == this->GetRenderedActor()->GetUserTransform());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureVtkMapper3D)
