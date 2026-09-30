/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGeometry3D.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkSliceNavigationController.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

class mitkBaseRendererTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBaseRendererTestSuite);
  MITK_TEST(SelectSliceByPoint_CurrentSlice_PlaneNotUpdated);
  MITK_TEST(SelectSliceByPoint_OtherSlice_PlaneUpdated);
  MITK_TEST(ReorientSlices_PlaneUpdated);
  MITK_TEST(SetTimeStep_EqualPlanes_PlaneUpdated);
  CPPUNIT_TEST_SUITE_END();

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  mitk::VtkPropRenderer::Pointer m_Renderer;

public:
  void setUp() override
  {
    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_Renderer = mitk::VtkPropRenderer::New("mitkBaseRendererTest", m_RenderWindow);

    const double bounds[6] = { 0.0, 20.0, 0.0, 20.0, 0.0, 20.0 };
    auto geometry = mitk::Geometry3D::New();
    geometry->SetFloatBounds(bounds);

    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry, 2);

    auto* sliceNavigationController = m_Renderer->GetSliceNavigationController();
    sliceNavigationController->SetInputWorldTimeGeometry(timeGeometry);
    sliceNavigationController->SetViewDirection(mitk::AnatomicalPlane::Axial);
    sliceNavigationController->Update();
  }

  void tearDown() override
  {
    m_Renderer = nullptr;
    m_RenderWindow = nullptr;
  }

  void SelectSliceByPoint_CurrentSlice_PlaneNotUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    const auto origin = m_Renderer->GetCurrentWorldPlaneGeometry()->GetOrigin();
    m_Renderer->GetSliceNavigationController()->SelectSliceByPoint(origin);

    CPPUNIT_ASSERT_EQUAL(updateTime, m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
  }

  void SelectSliceByPoint_OtherSlice_PlaneUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    const auto* slicedGeometry = dynamic_cast<const mitk::SlicedGeometry3D*>(m_Renderer->GetCurrentWorldGeometry());
    CPPUNIT_ASSERT(nullptr != slicedGeometry);

    const auto otherSliceOrigin = slicedGeometry->GetPlaneGeometry(5)->GetOrigin();
    m_Renderer->GetSliceNavigationController()->SelectSliceByPoint(otherSliceOrigin);

    CPPUNIT_ASSERT(updateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
    CPPUNIT_ASSERT(mitk::Equal(otherSliceOrigin, m_Renderer->GetCurrentWorldPlaneGeometry()->GetOrigin()));
  }

  void ReorientSlices_PlaneUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();
    const auto normal = m_Renderer->GetCurrentWorldPlaneGeometry()->GetNormal();

    mitk::Vector3D tiltedNormal;
    mitk::FillVector3D(tiltedNormal, 0.0, 1.0, 1.0);

    const auto center = m_Renderer->GetCurrentWorldGeometry()->GetCenter();
    m_Renderer->GetSliceNavigationController()->ReorientSlices(center, tiltedNormal);

    CPPUNIT_ASSERT(updateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
    CPPUNIT_ASSERT(!mitk::Equal(normal, m_Renderer->GetCurrentWorldPlaneGeometry()->GetNormal()));
  }

  void SetTimeStep_EqualPlanes_PlaneUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    m_Renderer->SetTimeStep(1);

    CPPUNIT_ASSERT(updateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBaseRenderer)
