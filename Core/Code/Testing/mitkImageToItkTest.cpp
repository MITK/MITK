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
#include <mitkIOUtil.h>
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

//#############################################################################
//##################### test methods ##########################################
//#############################################################################

void TestCastingMITKIntITKFloat_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(int) to ITK(float) image and back ...");
  mitk::Image::Pointer imgMem = GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<int>());
  itk::Image<float,3>::Pointer itkImage;
  mitk::CastToItkImage( imgMem, itkImage );
  mitk::Image::Pointer mitkImageAfterCast = mitk::ImportItkImage(itkImage);
  MITK_TEST_CONDITION_REQUIRED(mitkImageAfterCast.IsNotNull(),"Checking if result is not NULL.");
}

void TestCastingMITKDoubleITKFloat_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(double) to ITK(float) image and back ...");
  mitk::Image::Pointer imgMem=GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<double>());
  itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;
  mitk::CastToItkImage( imgMem, diffImage );
  MITK_TEST_CONDITION_REQUIRED(diffImage.IsNotNull(),"Checking if result is not NULL.");
}

void TestCastingMITKFloatITKFloat_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(float) to ITK(float) image and back ...");
  mitk::Image::Pointer imgMem=GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<float>());
  //itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;
  itk::Image<float,3>::Pointer diffImage;
  mitk::CastToItkImage( imgMem, diffImage );
  MITK_TEST_CONDITION_REQUIRED(diffImage.IsNotNull(),"Checking if result is not NULL.");
}


void TestCastingMITKFloatTensorITKFloatTensor_EmptyImage()
{
  MITK_TEST_OUTPUT(<<"Testing cast of empty MITK(Tensor<float>) to ITK(Tensor<float>) image and back ...");
  typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > ItkTensorImageType;
  mitk::Image::Pointer imgMem=GetEmptyTestImageWithGeometry( mitk::MakePixelType< ItkTensorImageType  >() );
  itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;
  //itk::Image<float,3>::Pointer diffImage;
  mitk::CastToItkImage( imgMem, diffImage );
  MITK_TEST_CONDITION_REQUIRED(diffImage.IsNotNull(),"Checking if result is not NULL.");
}

void TestCastingMITKDoubleITKTensorDouble_EmptyImage_ThrowsException()
{
  MITK_TEST_OUTPUT(<<"Testing whether cast of empty MITK(double) to ITK(Tensor<double, 3>) image throws an exception...");
  mitk::Image::Pointer imgMem=GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<double>());
  itk::Image<itk::DiffusionTensor3D<double>,3>::Pointer diffImage;
  MITK_TEST_FOR_EXCEPTION_BEGIN(std::exception)
  mitk::CastToItkImage( imgMem, diffImage );
  MITK_TEST_FOR_EXCEPTION_END(std::exception)
}

void TestCastingMITKtoITK_TestImage(mitk::Image::Pointer testImage)
{
itk::Image<short, 3>::Pointer itkImage;

try
{
mitk::CastToItkImage( testImage, itkImage );
}
catch( const std::exception &e)
{
  MITK_TEST_FAILED_MSG(<< e.what() )
}

MITK_TEST_CONDITION_REQUIRED(itkImage.IsNotNull(),"Casting test image to ITK.");

mitk::Image::Pointer mitkImageAfterCast = mitk::ImportItkImage(itkImage);
MITK_TEST_CONDITION_REQUIRED(mitkImageAfterCast.IsNotNull(),"Casting ITK image back.");

try
{
  MITK_ASSERT_EQUAL(testImage, mitkImageAfterCast, "Testing if both images are equal.");
}
catch( const itk::ExceptionObject &e)
{
  MITK_TEST_FAILED_MSG(<< e.what() )
}

}

int mitkImageToItkTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkImageToItkTest");

  MITK_TEST_OUTPUT(<<"Some tests with empty images, no errors should occur: ");
  TestCastingMITKIntITKFloat_EmptyImage();
  TestCastingMITKFloatITKFloat_EmptyImage();
  TestCastingMITKFloatTensorITKFloatTensor_EmptyImage();
  TestCastingMITKDoubleITKTensorDouble_EmptyImage_ThrowsException();

  MITK_TEST_OUTPUT(<<"Test casting with real image data: ");
  mitk::Image::Pointer Pic3DImage;
  if(argc==2) {Pic3DImage = mitk::IOUtil::LoadImage(argv[1]);}
  else {MITK_TEST_OUTPUT(<<"ERROR: test data could not be loaded: "<<argc<<"/"<<argv[1]);}

  TestCastingMITKtoITK_TestImage(Pic3DImage);

  MITK_TEST_END();
}
