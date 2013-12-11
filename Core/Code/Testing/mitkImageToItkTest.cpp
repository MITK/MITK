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


#include "mitkImage.h"
#include "mitkTestingMacros.h"
#include "mitkITKImageImport.h"
#include "mitkReferenceCountWatcher.h"
#include "itkDiffusionTensor3D.h"
#include <mitkImageCast.h>
#include <fstream>

//#############################################################################
//##################### some internal help methods ############################
//#############################################################################

mitk::Image::Pointer GetEmptyTestImageWithGeometry(mitk::PixelType pt)
{
  //create empty image
  mitk::Image::Pointer imgMem;
  imgMem=mitk::Image::New();

  //create geometry information for image
  mitk::Point3D origin;
  mitk::Vector3D right, bottom;
  mitk::Vector3D spacing;
  mitk::FillVector3D(origin, 17.0, 19.92, 7.83);
  mitk::FillVector3D(right, 1.0, 2.0, 3.0);
  mitk::FillVector3D(bottom, 0.0, -3.0, 2.0);
  mitk::FillVector3D(spacing, 0.78, 0.91, 2.23);
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(100, 100, right, bottom, &spacing);
  planegeometry->SetOrigin(origin);

  //initialize image
  imgMem->Initialize(pt, 40, *planegeometry);

  return imgMem;
}

void TestCastingMITKIntITKFloat_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(int) to ITK(float) image and back ... no errors should occur");
  mitk::Image::Pointer imgMem = GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<int>());
  itk::Image<float,3>::Pointer itkImage;
  mitk::CastToItkImage( imgMem, itkImage );
  mitk::Image::Pointer mitkImageAfterCast = mitk::ImportItkImage(itkImage);
  MITK_TEST_CONDITION_REQUIRED(mitkImageAfterCast.IsNotNull(),"Checking if result is not NULL.");
}

void TestCastingMITKDoubleITKFloat_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(double) to ITK(float) image and back ... no errors should occur");
  mitk::Image::Pointer imgMem=GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<double>());
  itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;
  mitk::CastToItkImage( imgMem, diffImage );
  MITK_TEST_CONDITION_REQUIRED(diffImage.IsNotNull(),"Checking if result is not NULL.");
}

void TestCastingMITKDoubleITKDouble_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(double) to ITK(float) image and back ... no errors should occur");
  mitk::Image::Pointer imgMem=GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<double>());
  itk::Image<itk::DiffusionTensor3D<double>,3>::Pointer diffImage;
  mitk::CastToItkImage( imgMem, diffImage );
  MITK_TEST_CONDITION_REQUIRED(diffImage.IsNotNull(),"Checking if result is not NULL.");
}

int mitkImageToItkTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkImageToItkTest");

  TestCastingMITKIntITKFloat_EmptyImage();
  TestCastingMITKDoubleITKFloat_EmptyImage();
  TestCastingMITKDoubleITKDouble_EmptyImage();




  //typedef itk::Image<float,3> ImageType;
  //ImageType::Pointer itkImage;
  //mitk::CastToItkImage( imgMem, itkImage );

  //
  //

  //MITK_TEST_CONDITION_REQUIRED(testBackCasting<ImageType>(imgMem, itkImage, true),"Testing back casting (mitk int to itk float) with disconnect");



  //imgMem->Initialize(pt_double, 40, *planegeometry, false, 1, 6);
  //MITK_TEST_CONDITION_REQUIRED(testImageToItkAndBack<4>(imgMem),"Testing Initialize(const mitk::PixelType& type, int sDim, const mitk::PlaneGeometry& geometry) and GetData()");


  //std::cout << "Testing mitk::CastToItkImage again (mitk float to itk float): " << std::flush;
  //imgMem->Initialize(pt_float, 40, *planegeometry);
  //mitk::CastToItkImage( imgMem, itkImage );
  //std::cout<<"[PASSED]"<<std::endl;

  //mitk::ImageDataItem::Pointer imageDataItem = imgMem->GetChannelData().GetPointer();
  //std::cout << "Testing destruction of original mitk::Image: " << std::flush;
  //imgMem = NULL;
  //std::cout<<"[PASSED]"<<std::endl;

  ///* Fails at the moment
  //std::cout << "Testing reference count mitk::ImageDataItem, which is responsible for the memory still used within the itk::Image: " << std::flush;
  //if(imageDataItem->GetReferenceCount()-1 != 1) // 1 count by imageDataItem itself
  //{
  //  std::cout<< imageDataItem->GetReferenceCount()-1 << " != 1. [FAILED]" << std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;
  //*/

  //std::cout << "Testing destruction of itk::Image: " << std::flush;
  //itkImage = NULL;
  //std::cout<<"[PASSED]"<<std::endl;

  ///* Crashes at the moment
  //std::cout << "Testing reference count mitk::ImageDataItem, which should now have been freed by itk::Image: " << std::flush;
  //if(imageDataItem->GetReferenceCount()-1 != 0) // 1 count by imageDataItem itself
  //{
  //  std::cout<< imageDataItem->GetReferenceCount()-1 << " != 0. [FAILED]" << std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;
  //*/


  //imgMem=mitk::Image::New();
  //itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;
  //imgMem->Initialize(pt_double, 40, *planegeometry);
  //mitk::CastToItkImage( imgMem, diffImage );


  //imgMem->InitializeByItk(diffImage.GetPointer());
  //std::cout<<"[PASSED]"<<std::endl;

  //itk::Image<itk::DiffusionTensor3D<double>,3>::Pointer diffImage2;
  //imgMem->Initialize(pt_double, 40, *planegeometry);
  //mitk::CastToItkImage( imgMem, diffImage2 );
  //imgMem->InitializeByItk(diffImage2.GetPointer());
  //std::cout<<"[PASSED]"<<std::endl;

  ///* seems as ConfidenceDiffusionTensor3D is no longer part of ITK, remove test code?
  //itk::Image<itk::ConfidenceDiffusionTensor3D<float>,3>::Pointer confDiffImage;
  //imgMem->Initialize(mitk::PixelType(typeid(itk::ConfidenceDiffusionTensor3D<float>)), 40, *planegeometry);
  //mitk::CastToItkImage( imgMem, confDiffImage );
  //imgMem->InitializeByItk(confDiffImage.GetPointer());
  //std::cout<<"[PASSED]"<<std::endl;

  //itk::Image<itk::ConfidenceDiffusionTensor3D<double>,3>::Pointer confDiffImage2;
  //imgMem->Initialize(mitk::PixelType(typeid(itk::ConfidenceDiffusionTensor3D<double>)), 40, *planegeometry);
  //mitk::CastToItkImage( imgMem, confDiffImage2 );
  //imgMem->InitializeByItk(confDiffImage2.GetPointer());
  //std::cout<<"[PASSED]"<<std::endl;
  //*/

  //std::cout<<"[TEST DONE]"<<std::endl;
  MITK_TEST_END();
}
