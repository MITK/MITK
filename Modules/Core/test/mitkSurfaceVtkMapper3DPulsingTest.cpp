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
 * A pulsing surface pulses through a fragment shader replacement, which fails only at render
 * time: then nothing is drawn at all. The replacement stays once added, so the surface has to
 * render after it stopped pulsing as well.
 */
class mitkSurfaceVtkMapper3DPulsingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkMapper3DPulsingTestSuite);
  MITK_TEST(Render3D_PulsingAndNoLongerPulsing_ShowsTheSurface_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void Render3D_PulsingAndNoLongerPulsing_ShowsTheSurface_Success()
  {
    auto node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("ball.stl")));
    node->SetBoolProperty("pulsing", true);

    // Translucent, like the surface of the 3D interpolation: the replacement then has to
    // survive the depth peeling shader as well.
    node->SetOpacity(0.5f);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    CPPUNIT_ASSERT_MESSAGE("The pulsing surface was not drawn", renderingHelper.RendersAnything());

    node->SetBoolProperty("pulsing", false);

    CPPUNIT_ASSERT_MESSAGE("The surface was not drawn after it stopped pulsing", renderingHelper.RendersAnything());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper3DPulsing)
