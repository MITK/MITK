/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

/**
 * An animated surface animates through a fragment shader replacement, which fails only at render
 * time: then nothing is drawn at all. The replacement stays once added, so the surface has to
 * render after it stopped animating as well.
 */
class mitkSurfaceVtkMapper3DAnimationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkMapper3DAnimationTestSuite);
  MITK_TEST(Render3D_Pulse_ShowsTheSurface_Success);
  MITK_TEST(Render3D_Color_ShowsTheSurface_Success);
  CPPUNIT_TEST_SUITE_END();

  void RenderAnimatedAndNoLongerAnimated(const std::string& animationProperty)
  {
    auto node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("ball.stl")));
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

public:
  void Render3D_Pulse_ShowsTheSurface_Success()
  {
    this->RenderAnimatedAndNoLongerAnimated("animated.pulse");
  }

  void Render3D_Color_ShowsTheSurface_Success()
  {
    this->RenderAnimatedAndNoLongerAnimated("animated.color");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper3DAnimation)
