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
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>

#include <itkImage.h>
#include <mitkCompositePixelValueToString.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <sstream>

class mitkCompositePixelValueToStringTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCompositePixelValueToStringTestSuite);
  MITK_TEST(TestConvertRGBUnsignedChar2D_CorrectResult);
  MITK_TEST(TestConvertRGBUnsignedChar3D_CorrectResult);
  MITK_TEST(TestConvertRGBAUnsignedChar2D_CorrectResult);
  MITK_TEST(TestConvertRGBAUnsignedChar3D_CorrectResult);
  MITK_TEST(TestConvertRGBLong_Exception);
  MITK_TEST(TestConvertInt_Exception);
  MITK_TEST(TestConvertRGBIndexOutside_NoResult);
  MITK_TEST(TestConvertNullImage_Exception);

  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() override {}
  void TestConvertRGBUnsignedChar2D_CorrectResult()
  {
    typedef itk::RGBPixel<unsigned char> PixelType;
    typedef itk::Image<PixelType, 2> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    PixelType rgbPixel;
    rgbPixel.Set(0, 125, 250);

    image->SetPixel(pixelIndex, rgbPixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    pixelValue = ConvertCompositePixelValueToString(mitkImage, index3D);

    CPPUNIT_ASSERT(pixelValue.compare("0  125  250") == 0);
  }

  void TestConvertRGBUnsignedChar3D_CorrectResult()
  {
    typedef itk::RGBPixel<unsigned char> PixelType;
    typedef itk::Image<PixelType, 3> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    PixelType rgbPixel;
    rgbPixel.Set(0, 125, 250);

    image->SetPixel(pixelIndex, rgbPixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    pixelValue = ConvertCompositePixelValueToString(mitkImage, index3D);

    CPPUNIT_ASSERT(pixelValue.compare("0  125  250") == 0);
  }

  void TestConvertRGBAUnsignedChar2D_CorrectResult()
  {
    typedef itk::RGBAPixel<unsigned char> PixelType;
    typedef itk::Image<PixelType, 2> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    PixelType rgbPixel;
    rgbPixel.Set(0, 125, 250, 42);

    image->SetPixel(pixelIndex, rgbPixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    pixelValue = ConvertCompositePixelValueToString(mitkImage, index3D);

    CPPUNIT_ASSERT(pixelValue.compare("0  125  250  42") == 0);
  }

  void TestConvertRGBAUnsignedChar3D_CorrectResult()
  {
    typedef itk::RGBAPixel<unsigned char> PixelType;
    typedef itk::Image<PixelType, 3> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    PixelType rgbPixel;
    rgbPixel.Set(0, 125, 250, 42);

    image->SetPixel(pixelIndex, rgbPixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    pixelValue = ConvertCompositePixelValueToString(mitkImage, index3D);

    CPPUNIT_ASSERT(pixelValue.compare("0  125  250  42") == 0);
  }

  void TestConvertRGBLong_Exception()
  {
    typedef itk::RGBAPixel<long> PixelType;
    typedef itk::Image<PixelType, 3> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    PixelType rgbPixel;
    rgbPixel.Set(0, 125, 250, 42);

    image->SetPixel(pixelIndex, rgbPixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    CPPUNIT_ASSERT_THROW(ConvertCompositePixelValueToString(mitkImage, index3D), mitk::AccessByItkException);
  }

  void TestConvertInt_Exception()
  {
    typedef itk::Image<int, 3> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    int pixel = 21;

    image->SetPixel(pixelIndex, pixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    CPPUNIT_ASSERT_THROW(ConvertCompositePixelValueToString(mitkImage, index3D), mitk::AccessByItkException);
  }

  void TestConvertRGBIndexOutside_NoResult()
  {
    typedef itk::RGBPixel<unsigned char> PixelType;
    typedef itk::Image<PixelType, 2> ImageType;

    ImageType::RegionType region;
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(1);

    region.SetSize(size);
    region.SetIndex(start);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->Allocate();

    ImageType::IndexType pixelIndex;
    pixelIndex.Fill(0);

    PixelType rgbPixel;
    rgbPixel.Set(0, 125, 250);

    image->SetPixel(pixelIndex, rgbPixel);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(5);

    pixelValue = ConvertCompositePixelValueToString(mitkImage, index3D);

    CPPUNIT_ASSERT(pixelValue.compare("Out of bounds") == 0);
  }

  void TestConvertNullImage_Exception()
  {
    mitk::Image::Pointer mitkImage;

    std::string pixelValue;
    itk::Index<3> index3D;
    index3D.Fill(0);

    CPPUNIT_ASSERT_THROW(ConvertCompositePixelValueToString(mitkImage, index3D), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCompositePixelValueToString)
