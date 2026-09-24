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

#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

#include <algorithm>

namespace
{
  /** Renders once more without swapping buffers and reads the back buffer, as vtkTesting does. */
  vtkSmartPointer<vtkImageData> Capture(mitk::RenderingTestHelper& renderingHelper)
  {
    auto* renderWindow = renderingHelper.GetVtkRenderWindow();

    const auto swapBuffers = renderWindow->GetSwapBuffers();
    renderWindow->SwapBuffersOff();
    renderingHelper.Render();

    auto grabber = vtkSmartPointer<vtkWindowToImageFilter>::New();
    grabber->SetInput(renderWindow);
    grabber->ShouldRerenderOff();
    grabber->ReadFrontBufferOff();
    grabber->Update();

    renderWindow->SetSwapBuffers(swapBuffers);

    vtkSmartPointer<vtkImageData> capture = grabber->GetOutput();
    return capture;
  }

  /** Whether any pixel differs from the one in the corner, which only the background covers. */
  bool ShowsAnything(vtkImageData* capture)
  {
    int dimensions[3];
    capture->GetDimensions(dimensions);

    const auto components = capture->GetNumberOfScalarComponents();
    const auto* pixels = static_cast<const unsigned char*>(capture->GetScalarPointer());
    const auto numberOfPixels = static_cast<vtkIdType>(dimensions[0]) * dimensions[1];

    for (vtkIdType i = 1; i < numberOfPixels; ++i)
    {
      if (!std::equal(pixels, pixels + components, pixels + i * components))
        return true;
    }

    return false;
  }
}

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

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    CPPUNIT_ASSERT_MESSAGE("The pulsing surface was not drawn", ShowsAnything(Capture(renderingHelper)));

    node->SetBoolProperty("pulsing", false);

    CPPUNIT_ASSERT_MESSAGE("The surface was not drawn after it stopped pulsing",
                           ShowsAnything(Capture(renderingHelper)));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper3DPulsing)
