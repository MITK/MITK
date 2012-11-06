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
#include "mitkImage.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkImageGenerator.h"

int mitkImageGeneratorTest(int /*argc*/, char* /*argv*/[])
{
    MITK_TEST_BEGIN("ImageGeneratorTest");

    //create some images with arbitrary parameters (corner cases)
    mitk::Image::Pointer image2Da = mitk::ImageGenerator::GenerateRandomImage<float>(120, 205, 0, 0, 0.1, 0.2, 0.3, 577, 23);
    mitk::Image::Pointer image2Db = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(1, 1, 0, 0);
    mitk::Image::Pointer image3Da = mitk::ImageGenerator::GenerateRandomImage<int>(512, 205, 1, 0);
    mitk::Image::Pointer image3Db = mitk::ImageGenerator::GenerateRandomImage<double>(512, 532, 112, 0);
    mitk::Image::Pointer image4Da = mitk::ImageGenerator::GenerateRandomImage<float>(120, 205, 78, 1);
    mitk::Image::Pointer image4Db = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(550, 33, 78, 150);

    mitk::Image::Pointer image3Dc = mitk::ImageGenerator::GenerateGradientImage<unsigned int>(1, 2, 3, 4, 5, 6);


    MITK_TEST_CONDITION_REQUIRED(fabs(image2Da->GetGeometry()->GetSpacing()[0]-0.1)<0.0001, "Testing if spacing x is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(fabs(image2Da->GetGeometry()->GetSpacing()[1]-0.2)<0.0001, "Testing if spacing y is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(fabs(image2Da->GetGeometry()->GetSpacing()[2]-0.3)<0.0001, "Testing if spacing z is set correctly.");

    MITK_TEST_CONDITION_REQUIRED(fabs(image2Db->GetGeometry()->GetSpacing()[0]-1.0)<0.0001, "Testing if default spacing x is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(fabs(image2Db->GetGeometry()->GetSpacing()[1]-1.0)<0.0001, "Testing if default spacing y is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(fabs(image2Db->GetGeometry()->GetSpacing()[2]-1.0)<0.0001, "Testing if default spacing z is set correctly.");

    MITK_TEST_CONDITION_REQUIRED(fabs(image3Dc->GetGeometry()->GetSpacing()[0]-4)<0.0001, "Testing if spacing x is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(fabs(image3Dc->GetGeometry()->GetSpacing()[1]-5)<0.0001, "Testing if spacing y is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(fabs(image3Dc->GetGeometry()->GetSpacing()[2]-6)<0.0001, "Testing if spacing z is set correctly.");

    MITK_TEST_CONDITION_REQUIRED(image2Da->GetDimension() == 2, "Testing if the dimension is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image2Db->GetDimension() == 2, "Testing if the dimension is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetDimension() == 2, "Testing if the dimension is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Db->GetDimension() == 3, "Testing if the dimension is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Da->GetDimension() == 3, "Testing if the dimension is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Db->GetDimension() == 4, "Testing if the dimension is set correctly.");

    MITK_TEST_CONDITION_REQUIRED(image2Da->GetDimension(0) == 120, "Testing if the dimensions are set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image2Db->GetDimension(1) == 1, "Testing if the dimensions are set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetDimension(2) == 1, "Testing if the dimensions are set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Db->GetDimension(2) == 112, "Testing if the dimensions are set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Da->GetDimension(3) == 1, "Testing if the dimensions are set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Db->GetDimension(3) == 150, "Testing if the dimensions are set correctly.");

    MITK_TEST_CONDITION_REQUIRED(image3Dc->GetDimension(0) == 1, "Testing if image3Dc dimension x is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Dc->GetDimension(1) == 2, "Testing if image3Dc dimension y is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Dc->GetDimension(2) == 3, "Testing if image3Dc dimension z is set correctly.");

    itk::ImageIOBase::IOPixelType scalarType = itk::ImageIOBase::SCALAR;

    MITK_TEST_CONDITION_REQUIRED(image2Da->GetPixelType().GetTypeId() == typeid(float), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image2Da->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the pixel type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image2Db->GetPixelType().GetTypeId() == typeid(unsigned char), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image2Db->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetPixelType().GetTypeId() == typeid(int), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the pixel type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Db->GetPixelType().GetTypeId() == typeid(double), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Db->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the pixel type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Da->GetPixelType().GetTypeId() == typeid(float), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Da->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the pixel type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Db->GetPixelType().GetTypeId() == typeid(unsigned char), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Db->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the pixel type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Dc->GetPixelType().GetTypeId() == typeid(unsigned int), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Dc->GetPixelType().GetPixelTypeId() == scalarType, "Testing if the pixel type is set correctly.");

    MITK_TEST_CONDITION_REQUIRED(image2Da->GetStatistics()->GetScalarValueMax() <= 577, "Testing if max value holds");
    MITK_TEST_CONDITION_REQUIRED(image2Da->GetStatistics()->GetScalarValueMin() >= 23, "Testing if min value holds");

    MITK_TEST_CONDITION_REQUIRED(image3Da->GetStatistics()->GetScalarValueMax() <= 1000, "Testing if max value holds");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetStatistics()->GetScalarValueMin() >= 0, "Testing if min value holds");


    unsigned int* image3DcBuffer = (unsigned int*)image3Dc->GetData();
    for(unsigned int i = 0; i < 2*3; i++)
    {
        MITK_TEST_CONDITION_REQUIRED(image3DcBuffer[i]==i, "Testing if gradient image values are set correctly");
    }

    MITK_TEST_END();
}
