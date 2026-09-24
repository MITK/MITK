/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkImageWriteAccessor.h>
#include <mitkIPropertyTransience.h>
#include <mitkLabelSetImage.h>
#include <mitkMultiLabelSegmentationVtkMapper3D.h>
#include <mitkRenderingTestHelper.h>
#include <mitkVectorProperty.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

#include <algorithm>

namespace
{
  /** A 40 x 40 x 40 segmentation with an opaque cube of label 1 in its middle. */
  mitk::MultiLabelSegmentation::Pointer MakeCubeSegmentation()
  {
    auto referenceImage = mitk::Image::New();
    unsigned int dimensions[3] = { 40, 40, 40 };
    referenceImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);

    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(referenceImage);

    auto label = mitk::Label::New(1, "Cube");
    mitk::Color color;
    color.Set(1.0f, 0.5f, 0.0f);
    label->SetColor(color);
    label->SetOpacity(1.0f);
    segmentation->AddLabel(label, 0);

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
 * Without a thread that owns the data storage, as here, the 3D mapper extracts the surfaces
 * while rendering, so the first frame already has to show them.
 */
class mitkMultiLabelSegmentationVtkMapper3DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationVtkMapper3DTestSuite);
  MITK_TEST(Render3D_WithoutAnOwningThread_ShowsTheSurface_Success);
  MITK_TEST(Render3D_LabelHiddenIn3D_IsNotDrawnUntilShownAgain_Success);
  MITK_TEST(HiddenLabelsIn3D_AreTransient_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void Render3D_LabelHiddenIn3D_IsNotDrawnUntilShownAgain_Success()
  {
    const auto* propertyName = mitk::MultiLabelSegmentationVtkMapper3D::PROPERTY_NAME_3D_HIDDEN_LABELS();

    auto hiddenLabels = mitk::IntVectorProperty::New();
    hiddenLabels->SetValue({ 1 });

    auto node = mitk::DataNode::New();
    node->SetData(MakeCubeSegmentation());
    node->SetProperty(propertyName, hiddenLabels);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    CPPUNIT_ASSERT_MESSAGE("A label hidden in 3D was drawn", !ShowsAnything(Capture(renderingHelper)));

    // The 3D interpolation empties the list rather than removing it.
    hiddenLabels->SetValue({});

    CPPUNIT_ASSERT_MESSAGE("A label no longer hidden in 3D was not drawn", ShowsAnything(Capture(renderingHelper)));
  }

  void HiddenLabelsIn3D_AreTransient_Success()
  {
    mitk::CoreServicePointer<mitk::IPropertyTransience> transience(mitk::CoreServices::GetPropertyTransience());

    // Set only while a 3D interpolation is on display; a saved scene must not keep the label hidden.
    CPPUNIT_ASSERT_MESSAGE("Hidden labels in 3D have to be transient for segmentations",
                           transience->IsTransient(mitk::MultiLabelSegmentation::New(),
                                                   mitk::MultiLabelSegmentationVtkMapper3D::PROPERTY_NAME_3D_HIDDEN_LABELS()));
  }

  void Render3D_WithoutAnOwningThread_ShowsTheSurface_Success()
  {
    auto node = mitk::DataNode::New();
    node->SetData(MakeCubeSegmentation());

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    const auto capture = Capture(renderingHelper);

    CPPUNIT_ASSERT_MESSAGE("The surface of the segmentation was not drawn", ShowsAnything(capture));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationVtkMapper3D)
