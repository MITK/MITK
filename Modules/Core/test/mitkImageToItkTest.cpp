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
#include "mitkTestFixture.h"
#include "mitkITKImageImport.h"
#include <mitkImageAccessByItk.h>
#include "mitkReferenceCountWatcher.h"
#include "itkDiffusionTensor3D.h"
#include <mitkImageCast.h>
#include <mitkIOUtil.h>
#include <fstream>


static mitk::Image::Pointer GetEmptyTestImageWithGeometry(mitk::PixelType pt)
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

class mitkImageToItkTestSuite :
    public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkImageToItkTestSuite);
  MITK_TEST(ImageCastIntToFloat_EmptyImage);
  MITK_TEST(ImageCastDoubleToFloat_EmptyImage);
  MITK_TEST(ImageCastFloatToFloatTensor_EmptyImage_ThrowsException);
  MITK_TEST(ImageCastFloatTensorToFloatTensor_EmptyImage);
  MITK_TEST(ImageCastDoubleToTensorDouble_EmptyImage_ThrowsException);
  MITK_TEST(ImageCastToItkAndBack_SamePointer_Success);
  MITK_TEST(ImageCastToItk_TestImage_Success);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::Image::Pointer m_TestImage;

public:
  void setUp() override
  {
    // empty on purpose
  }

  void tearDown() override
  {
    m_TestImage = nullptr;
  }

  void ImageCastIntToFloat_EmptyImage()
  {
    mitk::Image::Pointer m_TestImage = GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<int>());
    itk::Image<float,3>::Pointer itkImage;

    mitk::CastToItkImage( m_TestImage, itkImage );
    mitk::Image::Pointer mitkImageAfterCast = mitk::ImportItkImage(itkImage);

    CPPUNIT_ASSERT_MESSAGE( "Cast output is not null" , mitkImageAfterCast.IsNotNull() );
  }

  void ImageCastDoubleToFloat_EmptyImage()
  {
    mitk::Image::Pointer m_TestImage=GetEmptyTestImageWithGeometry( mitk::MakeScalarPixelType<double>() );
    itk::Image<float,3>::Pointer diffImage;

    mitk::CastToItkImage( m_TestImage, diffImage );

    CPPUNIT_ASSERT_MESSAGE("Casting scalar double (MITK) image to scalar float tensor (ITK). Result shouldn't be NULL.", diffImage.IsNotNull() );
  }

  void ImageCastFloatToFloatTensor_EmptyImage_ThrowsException()
  {

    mitk::Image::Pointer m_TestImage=GetEmptyTestImageWithGeometry( mitk::MakeScalarPixelType<float>() );
    itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;


    CPPUNIT_ASSERT_THROW_MESSAGE( "Casting scalar float (MITK) image to scalar float (ITK) throws exception.",
                                  mitk::CastToItkImage( m_TestImage, diffImage ),
                                  mitk::AccessByItkException );
  }


  void ImageCastFloatTensorToFloatTensor_EmptyImage()
  {
    typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > ItkTensorImageType;
    mitk::Image::Pointer m_TestImage=GetEmptyTestImageWithGeometry( mitk::MakePixelType< ItkTensorImageType  >() );
    itk::Image<itk::DiffusionTensor3D<float>,3>::Pointer diffImage;

    mitk::CastToItkImage( m_TestImage, diffImage );
    MITK_TEST_CONDITION_REQUIRED(diffImage.IsNotNull(),"Casting float tensor (MITK) image to float tensor (ITK). Result shouldn't be NULL");

    mitk::Image::Pointer mitkImageAfterCast = mitk::ImportItkImage(diffImage);
    MITK_ASSERT_EQUAL( mitkImageAfterCast, m_TestImage, "Same type, images shoul be equal.");
  }

  void ImageCastDoubleToTensorDouble_EmptyImage_ThrowsException()
  {
    mitk::Image::Pointer m_TestImage=GetEmptyTestImageWithGeometry(mitk::MakeScalarPixelType<double>());
    itk::Image<itk::DiffusionTensor3D<double>,3>::Pointer diffImage;

    CPPUNIT_ASSERT_THROW( mitk::CastToItkImage( m_TestImage, diffImage), mitk::AccessByItkException );
  }

  void ImageCastToItkAndBack_SamePointer_Success()
  {
    typedef itk::Image<short, 3> ItkImageType;
    ItkImageType::Pointer itkImage = ItkImageType::New();

    std::string m_ImagePath = GetTestDataFilePath("Pic3D.nrrd");
    mitk::Image::Pointer testDataImage = mitk::IOUtil::LoadImage( m_ImagePath );

    // modify ITK image
    itk::Matrix<double,3,3> dir = itkImage->GetDirection();
    dir *= 2;
    itkImage->SetDirection(dir);

    CPPUNIT_ASSERT_THROW_MESSAGE("No exception thrown for casting back the same memory",
                                    testDataImage = mitk::GrabItkImageMemory( itkImage, testDataImage ),
                                    itk::ExceptionObject );
    CPPUNIT_ASSERT( testDataImage.IsNotNull() );
  }

  void ImageCastToItk_TestImage_Success()
  {
    itk::Image<short, 3>::Pointer itkImage;
    std::string m_ImagePath = GetTestDataFilePath("Pic3D.nrrd");
    mitk::Image::Pointer testDataImage = mitk::IOUtil::LoadImage( m_ImagePath );

    mitk::CastToItkImage( testDataImage, itkImage );
    mitk::Image::Pointer mitkImageAfterCast = mitk::ImportItkImage(itkImage);

    // dereference itk image
    itkImage = 0;

    MITK_ASSERT_EQUAL( mitkImageAfterCast, testDataImage, "Cast with test data followed by import produces same images");
  }

}; // END TEST SUITE CLASS DECL
MITK_TEST_SUITE_REGISTRATION(mitkImageToItk);

