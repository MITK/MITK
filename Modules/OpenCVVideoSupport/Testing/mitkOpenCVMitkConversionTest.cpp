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

// mitk includes
#include "mitkImageToOpenCVImageFilter.h"
#include "mitkOpenCVToMitkImageFilter.h"
#include <mitkTestingMacros.h>
#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>
#include <mitkIOUtil.h>
#include "mitkImageReadAccessor.h"
#include "mitkImageSliceSelector.h"

// itk includes
#include <itkRGBPixel.h>

#include <iostream>
#include <highgui.h>

// define test pixel indexes and intensities and other values
typedef itk::RGBPixel< unsigned char > TestUCRGBPixelType;

cv::Size testImageSize;

cv::Point pos1;
cv::Point pos2;
cv::Point pos3;

cv::Vec3b color1;
cv::Vec3b color2;
cv::Vec3b color3;

uchar greyValue1;
uchar greyValue2;
uchar greyValue3;



/*! Documentation
*   Test for image conversion of OpenCV images and mitk::Images. It tests the classes
*   OpenCVToMitkImageFilter and ImageToOpenCVImageFilter
*/

// Some declarations
template<typename TPixel, unsigned int VImageDimension>
void ComparePixels( itk::Image<itk::RGBPixel<TPixel>,VImageDimension>* image );
void ReadImageDataAndConvertForthAndBack(std::string imageFileName);
void ConvertIplImageForthAndBack(mitk::Image::Pointer inputForCVMat, std::string imageFileName);
void ConvertCVMatForthAndBack(mitk::Image::Pointer inputForCVMat, std::string imageFileName);


// Begin the test for mitkImage to OpenCV image conversion and back.
int mitkOpenCVMitkConversionTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ImageToOpenCVImageFilter")

  // the first part of this test checks the conversion of a cv::Mat style OpenCV image.

  // we build an cv::Mat image
  MITK_INFO << "setting test values";
  testImageSize = cv::Size(11,11);

  pos1 = cv::Point(0,0);
  pos2 = cv::Point(5,5);
  pos3 = cv::Point(10,10);

  color1 = cv::Vec3b(50,0,0);
  color2 = cv::Vec3b(0,128,0);
  color3 = cv::Vec3b(0,0,255);

  greyValue1 = 0;
  greyValue2 = 128;
  greyValue3 = 255;

  MITK_INFO << "generating test OpenCV image (RGB)";
  cv::Mat testRGBImage = cv::Mat::zeros( testImageSize, CV_8UC3 );

  // generate some test intensity values
  testRGBImage.at<cv::Vec3b>(pos1)= color1;
  testRGBImage.at<cv::Vec3b>(pos2)= color2;
  testRGBImage.at<cv::Vec3b>(pos3)= color3;

  //cv::namedWindow("debug", CV_WINDOW_FREERATIO );
  //cv::imshow("debug", testRGBImage.clone());
  //cv::waitKey(0);

  // convert it to a mitk::Image
  MITK_INFO << "converting OpenCV test image to mitk image and comparing scalar rgb values";
  mitk::OpenCVToMitkImageFilter::Pointer openCvToMitkFilter =
    mitk::OpenCVToMitkImageFilter::New();
  openCvToMitkFilter->SetOpenCVMat( testRGBImage );
  openCvToMitkFilter->Update();

  mitk::Image::Pointer mitkImage = openCvToMitkFilter->GetOutput();
  AccessFixedTypeByItk(mitkImage.GetPointer(), ComparePixels,
    (itk::RGBPixel<unsigned char>), // rgb image
    (2) );

  // convert it back to OpenCV image
  MITK_INFO << "converting mitk image to OpenCV image and comparing scalar rgb values";
  mitk::ImageToOpenCVImageFilter::Pointer mitkToOpenCv = mitk::ImageToOpenCVImageFilter::New();
  mitkToOpenCv->SetImage( mitkImage );
  cv::Mat openCvImage = mitkToOpenCv->GetOpenCVMat();

  // and test equality of the sentinel pixel
  cv::Vec3b convertedColor1 = openCvImage.at<cv::Vec3b>(pos1);
  cv::Vec3b convertedColor2 = openCvImage.at<cv::Vec3b>(pos2);
  cv::Vec3b convertedColor3 = openCvImage.at<cv::Vec3b>(pos3);

  MITK_TEST_CONDITION( color1 == convertedColor1, "Testing if initially created color values " << static_cast<int>( color1[0] ) << ", " << static_cast<int>( color1[1] ) << ", " << static_cast<int>( color1[2] ) << " matches the color values " << static_cast<int>( convertedColor1[0] ) << ", " << static_cast<int>( convertedColor1[1] ) << ", " << static_cast<int>( convertedColor1[2] ) << " at the same position " << pos1.x << ", " << pos1.y << " in the back converted OpenCV image" )

  MITK_TEST_CONDITION( color2 == convertedColor2, "Testing if initially created color values " << static_cast<int>( color2[0] ) << ", " << static_cast<int>( color2[1] ) << ", " << static_cast<int>( color2[2] ) << " matches the color values " << static_cast<int>( convertedColor2[0] ) << ", " << static_cast<int>( convertedColor2[1] ) << ", " << static_cast<int>( convertedColor2[2] ) << " at the same position " << pos2.x << ", " << pos2.y << " in the back converted OpenCV image" )

  MITK_TEST_CONDITION( color3 == convertedColor3, "Testing if initially created color values " << static_cast<int>( color3[0] ) << ", " << static_cast<int>( color3[1] ) << ", " << static_cast<int>( color3[2] ) << " matches the color values " << static_cast<int>( convertedColor3[0] ) << ", " << static_cast<int>( convertedColor3[1] ) << ", " << static_cast<int>( convertedColor3[2] ) << " at the same position " << pos3.x << ", " << pos3.y << " in the back converted OpenCV image" )

  // the second part of this test checks the conversion of mitk::Images to Ipl images and cv::Mat and back.
  for(unsigned int i = 1; i < argc; ++i )
  {
    ReadImageDataAndConvertForthAndBack(argv[i]);
  }

  MITK_TEST_END();
}

template<typename TPixel, unsigned int VImageDimension>
void ComparePixels( itk::Image<itk::RGBPixel<TPixel>,VImageDimension>* image )
{

  typedef itk::RGBPixel<TPixel> PixelType;
  typedef itk::Image<PixelType, VImageDimension> ImageType;

  typename ImageType::IndexType pixelIndex;
  pixelIndex[0] = pos1.x;
  pixelIndex[1] = pos1.y;
  PixelType onePixel = image->GetPixel( pixelIndex );

  MITK_TEST_CONDITION( color1[0] == onePixel.GetBlue(), "Testing if blue value (= " << static_cast<int>(color1[0]) << ") at postion "
    << pos1.x << ", " << pos1.y << " in OpenCV image is "
    << "equals the blue value (= " <<  static_cast<int>(onePixel.GetBlue()) << ")"
    << " in the generated mitk image");


  pixelIndex[0] = pos2.x;
  pixelIndex[1] = pos2.y;
  onePixel = image->GetPixel( pixelIndex );

  MITK_TEST_CONDITION( color2[1] == onePixel.GetGreen(), "Testing if green value (= " << static_cast<int>(color2[1]) << ") at postion "
    << pos2.x << ", " << pos2.y << " in OpenCV image is "
    << "equals the green value (= " <<  static_cast<int>(onePixel.GetGreen()) << ")"
    << " in the generated mitk image");


  pixelIndex[0] = pos3.x;
  pixelIndex[1] = pos3.y;
  onePixel = image->GetPixel( pixelIndex );

  MITK_TEST_CONDITION( color3[2] == onePixel.GetRed(), "Testing if red value (= " << static_cast<int>(color3[2]) << ") at postion "
    << pos3.x << ", " << pos3.y << " in OpenCV image is "
    << "equals the red value (= " <<  static_cast<int>(onePixel.GetRed()) << ")"
    << " in the generated mitk image");

}

void ReadImageDataAndConvertForthAndBack(std::string imageFileName)
{
  // first we load an mitk::Image from the data repository
  mitk::Image::Pointer mitkTestImage = mitk::IOUtil::LoadImage(imageFileName);

  // some format checking
  mitk::Image::Pointer resultImg = NULL;
  if( mitkTestImage->GetDimension() <= 3 )
  {
    if( mitkTestImage->GetDimension() > 2 && mitkTestImage->GetDimension(2) == 1 )
    {
      mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
      sliceSelector->SetInput(mitkTestImage);
      sliceSelector->SetSliceNr(0);
      sliceSelector->Update();
      resultImg = sliceSelector->GetOutput()->Clone();
    }
    else if(mitkTestImage->GetDimension() < 3)
    {
      resultImg = mitkTestImage;
    }
    else
    {
      return; // 3D images are not supported, except with just one slice.
    }
  }
  else
  {
    return;   // 4D images are not supported!
  }

  ConvertIplImageForthAndBack(resultImg, imageFileName);
  ConvertCVMatForthAndBack(resultImg, imageFileName);
}

void ConvertCVMatForthAndBack(mitk::Image::Pointer inputForCVMat, std::string imageFileName)
{
  // now we convert it to OpenCV IplImage
  mitk::ImageToOpenCVImageFilter::Pointer toOCvConverter = mitk::ImageToOpenCVImageFilter::New();
  toOCvConverter->SetImage(inputForCVMat);
  cv::Mat cvmatTestImage = toOCvConverter->GetOpenCVMat();

  MITK_TEST_CONDITION_REQUIRED( !cvmatTestImage.empty(), "Conversion to cv::Mat successful!");

  mitk::OpenCVToMitkImageFilter::Pointer toMitkConverter = mitk::OpenCVToMitkImageFilter::New();
  toMitkConverter->SetOpenCVMat(cvmatTestImage);
  toMitkConverter->Update();

  // initialize the image with the input image, since we want to test equality and OpenCV does not feature geometries and spacing
  mitk::Image::Pointer result = inputForCVMat->Clone();
  mitk::ImageReadAccessor resultAcc(toMitkConverter->GetOutput(), toMitkConverter->GetOutput()->GetSliceData());
  result->SetImportSlice(const_cast<void*>(resultAcc.GetData()));

  if( result->GetPixelType().GetNumberOfComponents() == 1 )
  {
    MITK_TEST_EQUAL( result, inputForCVMat, "Testing equality of input and output image of cv::Mat conversion for " << imageFileName );
  }
  else if( result->GetPixelType().GetNumberOfComponents() == 3 )
  {
    MITK_TEST_EQUAL( result, inputForCVMat, "Testing equality of input and output image of cv::Mat conversion for " << imageFileName );
  }
  else
  {
    MITK_WARN << "Unhandled number of components used to test equality, please enhance test!";
  }

  // change OpenCV image to test if the filter gets updated
  cv::Mat changedcvmatTestImage = cvmatTestImage.clone();
  std::size_t numBits = result->GetPixelType().GetBitsPerComponent();
  if (result->GetPixelType().GetBitsPerComponent() == sizeof(char)*8)
  {
    changedcvmatTestImage.at<char>(0,0) = cvmatTestImage.at<char>(0,0) != 0 ? 0 : 1;
  }
  else if (result->GetPixelType().GetBitsPerComponent() == sizeof(float)*8)
  {
    changedcvmatTestImage.at<float>(0,0) = cvmatTestImage.at<float>(0,0) != 0 ? 0 : 1;
  }
  /*
  if (result->GetPixelType().GetBitsPerComponent() == 3*sizeof(char))
  {
    changedcvmatTestImage.at<char>(0,0) = cvmatTestImage.at<char>(0,0) != 0 ? 0 : 1;
  }
  */

  toMitkConverter->SetOpenCVMat(changedcvmatTestImage);
  toMitkConverter->Update();

  MITK_TEST_NOT_EQUAL(toMitkConverter->GetOutput(), inputForCVMat, "Converted image must not be the same as before.");
}

void ConvertIplImageForthAndBack(mitk::Image::Pointer inputForIpl, std::string imageFileName)
{
  // now we convert it to OpenCV IplImage
  mitk::ImageToOpenCVImageFilter::Pointer toOCvConverter = mitk::ImageToOpenCVImageFilter::New();
  toOCvConverter->SetImage(inputForIpl);
  IplImage* iplTestImage = toOCvConverter->GetOpenCVImage();

  MITK_TEST_CONDITION_REQUIRED( iplTestImage != NULL, "Conversion to OpenCv IplImage successful!");

  mitk::OpenCVToMitkImageFilter::Pointer toMitkConverter = mitk::OpenCVToMitkImageFilter::New();
  toMitkConverter->SetOpenCVImage(iplTestImage);
  toMitkConverter->Update();

  // initialize the image with the input image, since we want to test equality and OpenCV does not feature geometries and spacing
  mitk::Image::Pointer result = inputForIpl->Clone();
  mitk::ImageReadAccessor resultAcc(toMitkConverter->GetOutput(), toMitkConverter->GetOutput()->GetSliceData());
  result->SetImportSlice(const_cast<void*>(resultAcc.GetData()));

  if( result->GetPixelType().GetNumberOfComponents() == 1 )
  {
    MITK_TEST_EQUAL( result, inputForIpl, "Testing equality of input and output image of IplImage conversion  for " << imageFileName );
  }
  else if( result->GetPixelType().GetNumberOfComponents() == 3 )
  {
    MITK_TEST_EQUAL( result, inputForIpl, "Testing equality of input and output image of cv::Mat conversion for " << imageFileName );
  }
  else
  {
    MITK_WARN << "Unhandled number of components used to test equality, please enhance test!";
  }
}
