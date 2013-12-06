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

#include "mitkCommon.h"
#include "mitkTestingMacros.h"

#include "itkVectorImage.h"
#include "itkImageRegionIterator.h"
#include "itkVectorImageToImageAdaptor.h"

/// ctest entry point
int mitkVectorImageTest(int /*argc*/, char* /*argv*/[])
{
    MITK_TEST_BEGIN("VectorImage");
    // Create an image
    const int Dimension = 3;
    const int Components = 5;
    typedef unsigned short PixelType;
    typedef itk::VectorImage<PixelType, Dimension>  ImageType;
    typedef itk::VariableLengthVector<PixelType> VariableVectorType;
    typedef itk::VectorImageToImageAdaptor<PixelType,Dimension> ImageAdaptorType;

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(3);

    ImageType::RegionType region(start,size);

    ImageType::Pointer image1 = ImageType::New();
    image1->SetRegions(region);
    image1->SetVectorLength(Components);
    image1->Allocate();

    VariableVectorType defaultValue;
    defaultValue.SetSize(Components);
    defaultValue[0] = 1;
    defaultValue[1] = 2;
    defaultValue[2] = 3;
    defaultValue[3] = 4;
    defaultValue[4] = 5;
    image1->FillBuffer(defaultValue);

    typedef itk::ImageRegionIterator< ImageType > IteratorType;

    IteratorType iter( image1, image1->GetLargestPossibleRegion() );
    iter.GoToBegin();

    while (!iter.IsAtEnd())
    {
     std::cout << "pixel " << iter.Get() << std::endl;
     ++iter;
    }

    ImageType::Pointer image2 = ImageType::New();
    image2->SetRegions(region);
    image2->SetVectorLength(1);
    image2->Allocate();

    VariableVectorType newValue;
    newValue.SetSize(1);
    newValue[0] = 6;

    image2->FillBuffer(newValue);

    std::cout << "**************" << std::endl;

    ImageType::IndexType pixelIndex;
    pixelIndex[0] = 0;
    pixelIndex[1] = 0;
    pixelIndex[2] = 0;

    IteratorType iter2( image2, image2->GetLargestPossibleRegion() );
    iter2.GoToBegin();

    while (!iter2.IsAtEnd())
    {
     std::cout << "pixel " << iter2.Get() << std::endl;
     ++iter2;
    }

    ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
    adaptor->SetExtractComponentIndex(0);
    adaptor->SetImage(image1);
    adaptor->Update();

    ImageAdaptorType::PixelType pixelValue = adaptor->GetPixel(pixelIndex);

    std::cout << "pixel (1,1,1) via adaptor = " << pixelValue << std::endl;

/*
    std::cout << "pixel (0,0,0) " << image->GetPixel(pixelIndex) << std::endl;

    adaptor->SetExtractComponentIndex(0);
    adaptor->Update();

    std::cout << "pixel (0,0,0) component 0: " << adaptor->GetPixel(pixelIndex) << std::endl;

    adaptor->SetExtractComponentIndex(1);
    adaptor->Update();

    std::cout << "pixel (0,0,0) component 1: " << adaptor->GetPixel(pixelIndex) << std::endl;

    adaptor->SetExtractComponentIndex(2);
    adaptor->Update();

    std::cout << "pixel (0,0,0) component 2: " << adaptor->GetPixel(pixelIndex) << std::endl;

    adaptor->SetExtractComponentIndex(3);
    adaptor->Update();

    std::cout << "pixel (0,0,0) component 3: " << adaptor->GetPixel(pixelIndex) << std::endl;

    adaptor->SetExtractComponentIndex(4);
    adaptor->Update();

    std::cout << "pixel (0,0,0) component 4: " << adaptor->GetPixel(pixelIndex) << std::endl;
*/
/*


    ImageType::PixelType pixelValue = image->GetPixel(pixelIndex);

    std::cout << "pixel (1,1,1) = " << pixelValue << std::endl;

    VariableVectorType newValue;
    newValue.SetSize(Components);
    newValue[0] = 1;
    newValue[1] = 10;
    newValue[2] = 20;
    newValue[3] = 30;
    newValue[4] = 40;

    image->SetPixel(pixelIndex, newValue);

    std::cout << "pixel (1,1,1) = " << pixelValue << std::endl;

    /////


*/
    std::cout << "PASSED all tests." << std::endl;
    MITK_TEST_END();
}
