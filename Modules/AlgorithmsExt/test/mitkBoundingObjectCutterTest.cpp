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


#include <mitkImage.h>
#include <mitkImageDataItem.h>
#include <mitkImageCast.h>

#include <mitkBoundingObject.h>
#include <mitkCuboid.h>
#include <mitkBoundingObjectCutter.h>

#include <itkImage.h>

#include <fstream>

#include <vtkImageData.h>

#include <mitkTestingMacros.h>


int mitkBoundingObjectCutterTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkBoundingObjectCutterTest);

  ////Create Image out of nowhere
  //mitk::Image::Pointer image;
  //mitk::PixelType pt(mitk::MakeScalarPixelType<int>() );
  //unsigned int dim[]={100,100,20};

  //MITK_TEST_OUTPUT(<< "Creating Image as imput for cutting: ");
  //image=mitk::Image::New();
  //image->Initialize(mitk::MakeScalarPixelType<int>(), 3, dim);
  // mitk::ImageReadAccessor imgAcc(image);
  //int *p = (int*)imgAcc.GetData();
  //unsigned int i;
  //unsigned int size = dim[0]*dim[1]*dim[2];
  //for(i=0; i<size; ++i, ++p)
  //  *p= (signed int)i;
  //std::cout<<"[PASSED]"<<std::endl;

  //MITK_TEST_OUTPUT(<< "Testing mitk::BoundingObject::FitGeometry(image->GetGeometry()) with an mitk::Cuboid (sub-class of mitk::BoundingObject): ");
  //mitk::Cuboid::Pointer cuboid = mitk::Cuboid::New();
  //cuboid->FitGeometry(image->GetGeometry());
  //std::cout<<"[PASSED]"<<std::endl;

  // MITK_TEST_OUTPUT(<< "Testing whether corners of the cuboid are identical to corners of the image: ");
  // int c;
  // for(c=0; c<6; ++c)
  // {
  //   MITK_TEST_OUTPUT(<< " Testing GetCornerPoint(" << c << "): ");
  //   MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetCornerPoint(c),cuboid->GetGeometry()->GetCornerPoint(c)-1), "");
  // }

  //MITK_TEST_OUTPUT(<< "Testing whether diagonal^2 of fitted mitk::Cuboid is identical to diagonal^2 of image: ");
  //MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetDiagonalLength2(),cuboid->GetGeometry()->GetDiagonalLength2()), "");


  //MITK_TEST_OUTPUT(<< "Testing mitk::BoundingObjectCutter: ");
  //mitk::BoundingObjectCutter::Pointer boCutter = mitk::BoundingObjectCutter::New();
  //boCutter->SetInput(image);
  //boCutter->SetBoundingObject(cuboid);
  //MITK_TEST_OUTPUT(<< " Testing mitk::BoundingObjectCutter::UpdateLargestPossibleRegion():: ");
  //boCutter->UpdateLargestPossibleRegion();
  //std::cout<<"[PASSED]"<<std::endl;

  //mitk::Image::Pointer cuttedImage = boCutter->GetOutput();

  //MITK_TEST_OUTPUT(<< " Testing whether origin of cutted image is identical to origin of original image: ");
  //MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetOrigin(),cuttedImage->GetGeometry()->GetOrigin()), "");

  //MITK_TEST_OUTPUT(<< " Testing whether spacing of cutted image is identical to spacing of original image: ");
  //MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetSpacing(),cuttedImage->GetGeometry()->GetSpacing()), "");

  //MITK_TEST_OUTPUT(<< " Testing whether center of cutted image is identical to center of original image: ");
  //MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetCenter(),cuttedImage->GetGeometry()->GetCenter()), "");

  //MITK_TEST_OUTPUT(<< " Testing whether diagonal^2 of cutted image is identical to diagonal^2 of original image: ");
  //MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetDiagonalLength2(),cuttedImage->GetGeometry()->GetDiagonalLength2()), "");

  //MITK_TEST_OUTPUT(<< " Testing whether corners of cutted image are identical to corners of original image: ");
  //for(c=0; c<6; ++c)
  //{
  //  MITK_TEST_OUTPUT(<< "  Testing GetCornerPoint(" << c << "): ");
  //  MITK_TEST_CONDITION_REQUIRED( mitk::Equal(image->GetGeometry()->GetCornerPoint(c),cuttedImage->GetGeometry()->GetCornerPoint(c)), "");
  //}

  //MITK_TEST_OUTPUT(<< " Testing whether pixel data of cutted image are identical to pixel data of original image: ");
  //mitk::ImageReadAccessor imgAcc(image);
  //p = (int*)imgAcc.GetData();
  //mitk::ImageReadAccessor cuttedImage(cuttedImage);
  //int *pCutted = (int*)cuttedImage.GetData();
  //for(i=0; i<size; ++i, ++p, ++pCutted)
  //{
  //  if(*p!=*pCutted)
  //    break;
  //}
  //MITK_TEST_CONDITION_REQUIRED(i==size, "");

  //MITK_TEST_OUTPUT(<< " Testing whether geometry of cutted image has ImageGeometry==true: ");
  //MITK_TEST_CONDITION_REQUIRED(cuttedImage->GetGeometry()->GetImageGeometry(), "");

  MITK_TEST_END();

  return EXIT_SUCCESS;
}
