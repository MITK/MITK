/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkImageVtkReadView.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkImageData.h>

#include <numeric>

class mitkImageVtkReadViewTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageVtkReadViewTestSuite);
  MITK_TEST(View_WrapsTheVolumeLikeTheSharedRepresentation);
  MITK_TEST(View_IsNotTheSharedRepresentation);
  MITK_TEST(ViewOfATimeStep_WrapsTheVolumeOfThatTimeStep);
  MITK_TEST(InvalidInput_Throws);
  CPPUNIT_TEST_SUITE_END();

public:
  /** Creates a 4 x 3 x 2 image per time step whose pixels count up from zero across all time steps. */
  static mitk::Image::Pointer CreateImage(unsigned int timeSteps)
  {
    auto image = mitk::Image::New();
    unsigned int dimensions[4] = { 4, 3, 2, timeSteps };
    image->Initialize(mitk::MakeScalarPixelType<short>(), 1 < timeSteps ? 4 : 3, dimensions);

    mitk::Vector3D spacing;
    mitk::FillVector3D(spacing, 0.5, 2.0, 3.0);
    image->SetSpacing(spacing);

    {
      mitk::ImageWriteAccessor accessor(image);
      auto* pixels = static_cast<short*>(accessor.GetData());
      std::iota(pixels, pixels + 24 * timeSteps, static_cast<short>(0));
    }

    return image;
  }

  static void CreateView(const mitk::Image* image, mitk::TimeStepType timeStep)
  {
    const mitk::ImageVtkReadView view(image, timeStep);
  }

  void View_WrapsTheVolumeLikeTheSharedRepresentation()
  {
    auto image = CreateImage(1);
    const mitk::ImageVtkReadView view(image, 0);

    auto* viewImage = view.GetVtkImageData();
    auto* sharedImage = image->GetVtkImageData(0);

    int viewDimensions[3];
    int sharedDimensions[3];
    viewImage->GetDimensions(viewDimensions);
    sharedImage->GetDimensions(sharedDimensions);

    for (int i = 0; i < 3; ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong dimensions", sharedDimensions[i], viewDimensions[i]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong spacing", sharedImage->GetSpacing()[i], viewImage->GetSpacing()[i]);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong origin", sharedImage->GetOrigin()[i], viewImage->GetOrigin()[i]);
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The view copied the pixels instead of wrapping them",
      sharedImage->GetScalarPointer(), viewImage->GetScalarPointer());

    // Pixel (1, 1, 1) is at 1 + 1 * 4 + 1 * 4 * 3.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pixel value", 17.0, viewImage->GetScalarComponentAsDouble(1, 1, 1, 0));
  }

  void View_IsNotTheSharedRepresentation()
  {
    auto image = CreateImage(1);
    const mitk::ImageVtkReadView view(image, 0);

    CPPUNIT_ASSERT_MESSAGE("The view handed out the representation the mappers share",
      view.GetVtkImageData() != image->GetVtkImageData(0));
  }

  void ViewOfATimeStep_WrapsTheVolumeOfThatTimeStep()
  {
    auto image = CreateImage(2);
    const mitk::ImageVtkReadView view(image, 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The view does not show the volume of the time step",
      24.0 + 17.0, view.GetVtkImageData()->GetScalarComponentAsDouble(1, 1, 1, 0));
  }

  void InvalidInput_Throws()
  {
    CPPUNIT_ASSERT_THROW(CreateView(nullptr, 0), mitk::Exception);

    auto image = CreateImage(1);
    CPPUNIT_ASSERT_THROW(CreateView(image, 1), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageVtkReadView)
