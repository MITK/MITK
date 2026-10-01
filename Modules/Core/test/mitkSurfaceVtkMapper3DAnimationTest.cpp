/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkRenderingTestHelper.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkMatrix4x4.h>
#include <vtkProp3D.h>
#include <vtkTransform.h>

/**
 * Pulse and color animate through a fragment shader replacement, which fails only at render
 * time: then nothing is drawn at all. The replacement stays once added, so the surface has to
 * render after it stopped animating as well. Spin and bounce move the actor, which the checks
 * below test for properties that hold at any time of the animation.
 */
class mitkSurfaceVtkMapper3DAnimationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkMapper3DAnimationTestSuite);
  MITK_TEST(Render3D_Pulse_ShowsTheSurface_Success);
  MITK_TEST(Render3D_Color_ShowsTheSurface_Success);
  MITK_TEST(Render3D_Spin_ShowsTheSurface_Success);
  MITK_TEST(Render3D_Bounce_ShowsTheSurface_Success);
  MITK_TEST(Render3D_SpinAroundX_KeepsTheAxisThroughTheCenter);
  MITK_TEST(Render3D_BounceAlongRotatedGeometry_BouncesAlongItsAxis);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataNode::Pointer CreateNode()
  {
    auto node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("ball.stl")));
    return node;
  }

  void RenderAnimatedAndNoLongerAnimated(const std::string& animationProperty)
  {
    auto node = this->CreateNode();
    node->SetBoolProperty(animationProperty.c_str(), true);

    // Translucent, like the surface of the 3D interpolation: the replacement then has to
    // survive the depth peeling shader as well.
    node->SetOpacity(0.5f);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    CPPUNIT_ASSERT_MESSAGE("The animated surface was not drawn", renderingHelper.RendersAnything());

    node->SetBoolProperty(animationProperty.c_str(), false);

    CPPUNIT_ASSERT_MESSAGE("The surface was not drawn after it stopped animating", renderingHelper.RendersAnything());
  }

  /** The matrix the 3D window draws the surface of the node with, after one render. */
  vtkMatrix4x4* RenderAndGetMatrix(mitk::DataNode* node, mitk::RenderingTestHelper& renderingHelper)
  {
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();
    renderingHelper.Render();

    auto* mapper = dynamic_cast<mitk::SurfaceVtkMapper3D*>(node->GetMapper(mitk::BaseRenderer::Standard3D));
    auto* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
    auto* prop = dynamic_cast<vtkProp3D*>(mapper->GetVtkProp(renderer));

    return prop->GetMatrix();
  }

public:
  void Render3D_Pulse_ShowsTheSurface_Success()
  {
    this->RenderAnimatedAndNoLongerAnimated("animated.pulse");
  }

  void Render3D_Color_ShowsTheSurface_Success()
  {
    this->RenderAnimatedAndNoLongerAnimated("animated.color");
  }

  void Render3D_Spin_ShowsTheSurface_Success()
  {
    this->RenderAnimatedAndNoLongerAnimated("animated.spin");
  }

  void Render3D_Bounce_ShowsTheSurface_Success()
  {
    this->RenderAnimatedAndNoLongerAnimated("animated.bounce");
  }

  void Render3D_SpinAroundX_KeepsTheAxisThroughTheCenter()
  {
    auto node = this->CreateNode();
    node->SetBoolProperty("animated.spin", true);
    node->SetIntProperty("animated.spin.axis", 0);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    auto* matrix = this->RenderAndGetMatrix(node, renderingHelper);

    const auto center = node->GetData()->GetGeometry()->GetCenter();

    for (const double offset : { 0.0, 10.0 })
    {
      const double point[4] = { center[0] + offset, center[1], center[2], 1.0 };
      double spunPoint[4];
      matrix->MultiplyPoint(point, spunPoint);

      for (int i = 0; i < 3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(point[i], spunPoint[i], 1e-6);
    }

    // Unmoved only at the exact start of a turn.
    const double offAxisPoint[4] = { center[0], center[1] + 10.0, center[2], 1.0 };
    double spunOffAxisPoint[4];
    matrix->MultiplyPoint(offAxisPoint, spunOffAxisPoint);

    CPPUNIT_ASSERT_MESSAGE("The surface did not spin",
      offAxisPoint[1] != spunOffAxisPoint[1] || offAxisPoint[2] != spunOffAxisPoint[2]);
  }

  void Render3D_BounceAlongRotatedGeometry_BouncesAlongItsAxis()
  {
    constexpr float relativeHeight = 0.5f;

    auto node = this->CreateNode();
    node->SetBoolProperty("animated.bounce", true);
    node->SetFloatProperty("animated.bounce.height", relativeHeight);

    // Turns the z axis of the geometry, along which the surface bounces by default, to world -y.
    auto* geometry = node->GetData()->GetGeometry();
    auto rotation = vtkSmartPointer<vtkTransform>::New();
    rotation->RotateX(90.0);
    geometry->SetIndexToWorldTransformByVtkMatrix(rotation->GetMatrix());

    const double height = relativeHeight * geometry->GetExtentInMM(2);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    auto* matrix = this->RenderAndGetMatrix(node, renderingHelper);

    // The rotation alone moves nothing, so the translation is the bounce.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, matrix->GetElement(0, 3), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, matrix->GetElement(2, 3), 1e-6);

    // At rest only at the exact start of a bounce.
    CPPUNIT_ASSERT_MESSAGE("The surface did not bounce", matrix->GetElement(1, 3) < 0.0);
    CPPUNIT_ASSERT(-height <= matrix->GetElement(1, 3));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper3DAnimation)
