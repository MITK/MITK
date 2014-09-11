/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkImageCast.h"
#include "mitkImageToItk.h"

class mitkImageCastTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkImageCastTestSuite);
  MITK_TEST(Cast_ToMultipleConstItkImages_Succeeds);
  MITK_TEST(Cast_ToMultipleNonConstItkImages_Fails);
  CPPUNIT_TEST_SUITE_END();

private:

  /**
   * Create a test image with a single pixel value. The image size is determined by the input parameter.
   *
   * @param value the pixel value the created image is filled with
   * @param size the number of voxels in each dimension
   */
  mitk::Image::Pointer CreateTestImageFixedValue(const mitk::PixelType& pixelType, unsigned int size)
  {
    mitk::Image::Pointer mitkImage = mitk::Image::New();
    unsigned int dimensions[] = { size, size, size };
    mitkImage->Initialize(pixelType, 3, dimensions);
    return mitkImage;
  }

public:

  void Cast_ToMultipleConstItkImages_Succeeds()
  {
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<short>();
    mitk::Image::ConstPointer mitkConstImage(CreateTestImageFixedValue(pixelType, 3).GetPointer());
    CPPUNIT_ASSERT(mitkConstImage.IsNotNull());

    typedef itk::Image<short,3> ItkImageType;
    // This locks the MITK image for reading
    ItkImageType::ConstPointer itkImage = mitk::ImageToItkImage<ItkImageType::PixelType, ItkImageType::ImageDimension>(mitkConstImage);

    // Trying to get another const itk::Image should work
    CPPUNIT_ASSERT((mitk::ImageToItkImage<ItkImageType::PixelType, ItkImageType::ImageDimension>(mitkConstImage)));
  }

  void Cast_ToMultipleNonConstItkImages_Fails()
  {
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<short>();
    mitk::Image::Pointer mitkImage = CreateTestImageFixedValue(pixelType, 3);
    mitk::Image::ConstPointer mitkConstImage(mitkImage.GetPointer());
    CPPUNIT_ASSERT(mitkImage.IsNotNull());

    typedef itk::Image<short,3> ItkImageType;
    // This locks the MITK image for reading (we use the const image)
    ItkImageType::ConstPointer itkImage = mitk::ImageToItkImage<ItkImageType::PixelType, ItkImageType::ImageDimension>(mitkConstImage);

    // Using the non-const MITK image to convert to ITK should fail
    CPPUNIT_ASSERT_THROW((mitk::ImageToItkImage<ItkImageType::PixelType, ItkImageType::ImageDimension>(mitkImage)), mitk::Exception);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkImageCast)
