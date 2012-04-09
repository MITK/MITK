/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>
#include "mitkImage.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkImageGenerator.h"

int mitkImageGeneratorTest(int /*argc*/, char* /*argv*/[])
{
    MITK_TEST_BEGIN("ToFImageWriter");

    //create some images with arbitrary parameters (corner cases)
    mitk::Image::Pointer image2Da = mitk::ImageGenerator::GenerateRandomImage<float>(120, 205, 0, 0, 577, 23);
    mitk::Image::Pointer image2Db = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(1, 1, 0, 0);
    mitk::Image::Pointer image3Da = mitk::ImageGenerator::GenerateRandomImage<int>(512, 205, 1, 0);
    mitk::Image::Pointer image3Db = mitk::ImageGenerator::GenerateRandomImage<double>(512, 532, 112, 0);
    mitk::Image::Pointer image4Da = mitk::ImageGenerator::GenerateRandomImage<float>(120, 205, 78, 1);
    mitk::Image::Pointer image4Db = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(550, 33, 78, 150);

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

    MITK_TEST_CONDITION_REQUIRED(image2Da->GetPixelType() == typeid(float), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image2Db->GetPixelType() == typeid(unsigned char), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetPixelType() == typeid(int), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image3Db->GetPixelType() == typeid(double), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Da->GetPixelType() == typeid(float), "Testing if the data type is set correctly.");
    MITK_TEST_CONDITION_REQUIRED(image4Db->GetPixelType() == typeid(unsigned char), "Testing if the ddata type is set correctly.");

    MITK_TEST_CONDITION_REQUIRED(image2Da->GetStatistics()->GetScalarValueMax() <= 577, "Testing if max value holds");
    MITK_TEST_CONDITION_REQUIRED(image2Da->GetStatistics()->GetScalarValueMin() >= 23, "Testing if min value holds");

    MITK_TEST_CONDITION_REQUIRED(image3Da->GetStatistics()->GetScalarValueMax() <= 1000, "Testing if max value holds");
    MITK_TEST_CONDITION_REQUIRED(image3Da->GetStatistics()->GetScalarValueMin() >= 0, "Testing if min value holds");

    MITK_TEST_END();
}
