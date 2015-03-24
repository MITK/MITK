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

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include "mitkIOUtil.h"

#include <mitkImageToEigenTransform.h>
#include <mitkEigentoImageTransform.h>


class mitkImageToEigenTransformTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkImageToEigenTransformTestSuite  );

  MITK_TEST(ImageToEigenToImageTest);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::ImageToEigenTransform image_eigen_transform;
  mitk::EigenToImageTransform eigen_image_transform;
  mitk::Image::Pointer input_image,mask;

  typedef mitk::ImageToEigenTransform::MatrixType MatrixType;
  typedef mitk::ImageToEigenTransform::VectorType VectorType;
public:

  void setUp(void)
  {

    input_image = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Transform/image.nrrd"));
    mask = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Transform/mask.nrrd"));

  }

  void ImageToEigenToImageTest()
  {

    VectorType eigen_matrix_from_image = image_eigen_transform(input_image,mask);
    mitk::Image::Pointer image_from_eigen_matrix = eigen_image_transform(eigen_matrix_from_image,mask);

    MITK_ASSERT_EQUAL(image_from_eigen_matrix,input_image,"Output 0 should be equal to i0");

//    mitk::Image::Pointer o0 = eigen_image_transform->GetOutput(0);
//    mitk::Image::Pointer o1 = eigen_image_transform->GetOutput(1);
//    mitk::Image::Pointer o2 = eigen_image_transform->GetOutput(2);

//
//    MITK_ASSERT_EQUAL(o1,i1,"Output 1 should be equal to i1");
//    MITK_ASSERT_EQUAL(o2,i2,"Output 2 should be equal to i2");

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkImageToEigenTransform)


