/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
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
  mitk::Label::Pointer MakeLabel(mitk::Label::PixelType value, float red, float green, float blue)
  {
    auto label = mitk::Label::New(value, "Label");
    mitk::Color color;
    color.Set(red, green, blue);
    label->SetColor(color);
    label->SetOpacity(1.0f);
    return label;
  }

  /** A 40 x 40 x 40 segmentation with an opaque cube of label 1 in its middle. */
  mitk::MultiLabelSegmentation::Pointer MakeCubeSegmentation()
  {
    auto referenceImage = mitk::Image::New();
    unsigned int dimensions[3] = { 40, 40, 40 };
    referenceImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);

    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(referenceImage);
    segmentation->AddLabel(MakeLabel(1, 1.0f, 0.5f, 0.0f), 0);

    auto* groupImage = segmentation->GetGroupImage(0);

    {
      mitk::ImageWriteAccessor accessor(groupImage);
      auto* pixels = static_cast<mitk::Label::PixelType*>(accessor.GetData());

      for (unsigned int z = 10; z < 30; ++z)
        for (unsigned int y = 10; y < 30; ++y)
          for (unsigned int x = 10; x < 30; ++x)
            pixels[(z * 40 + y) * 40 + x] = 1;
    }

    groupImage->Modified();

    return segmentation;
  }

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
 * The 2D mapper of multi-label segmentations, rendered the way a test or a command line tool
 * does, without a preferences service.
 */
class mitkLabelSetImageVtkMapper2DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageVtkMapper2DTestSuite);
  MITK_TEST(Render2D_WithoutPreferences_ShowsTheLabel_Success);
  MITK_TEST(Render2D_RemovedLabel_IsNotShownForANewLabelOfTheSameValue_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void Render2D_WithoutPreferences_ShowsTheLabel_Success()
  {
    auto node = mitk::DataNode::New();
    node->SetData(MakeCubeSegmentation());

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetViewDirection(mitk::AnatomicalPlane::Axial);

    CPPUNIT_ASSERT_MESSAGE("The label was not drawn", ShowsAnything(Capture(renderingHelper)));
  }

  void Render2D_RemovedLabel_IsNotShownForANewLabelOfTheSameValue_Success()
  {
    auto segmentation = MakeCubeSegmentation();
    auto node = mitk::DataNode::New();
    node->SetData(segmentation);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetViewDirection(mitk::AnatomicalPlane::Axial);
    Capture(renderingHelper);

    segmentation->RemoveLabel(1);
    Capture(renderingHelper);

    // Removing a label frees its value, and the next label added commonly gets it again.
    segmentation->AddLabel(MakeLabel(1, 0.0f, 1.0f, 0.0f), 0, true, false);

    CPPUNIT_ASSERT_MESSAGE("The content of the removed label is shown for the new one",
                           !ShowsAnything(Capture(renderingHelper)));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageVtkMapper2D)
