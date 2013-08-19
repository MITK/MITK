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

#include "mitkImageToOpenCVImageFilter.h"
#include "mitkOpenCVToMitkImageFilter.h"
#include <mitkTestingMacros.h>
#include <mitkITKImageImport.h>
#include <iostream>
#include <highgui.h>
#include <itkRGBPixel.h>
#include <mitkImageAccessByItk.h>
#include <mitkIOUtil.h>

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

int mitkOpenCVMitkConversionTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ImageToOpenCVImageFilter")

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

  MITK_INFO << "converting OpenCV test image to mitk image and comparing scalar rgb values";
  mitk::OpenCVToMitkImageFilter::Pointer openCvToMitkFilter =
    mitk::OpenCVToMitkImageFilter::New();
  openCvToMitkFilter->SetOpenCVMat( testRGBImage );
  openCvToMitkFilter->Update();

  mitk::Image::Pointer mitkImage = openCvToMitkFilter->GetOutput();
  AccessFixedTypeByItk(mitkImage.GetPointer(), ComparePixels,
                       (itk::RGBPixel<unsigned char>), // rgb image
                       (2) );


  MITK_INFO << "converting mitk image to OpenCV image and comparing scalar rgb values";
  mitk::ImageToOpenCVImageFilter::Pointer mitkToOpenCv = mitk::ImageToOpenCVImageFilter::New();
  mitkToOpenCv->SetImage( mitkImage );
  cv::Mat openCvImage = mitkToOpenCv->GetOpenCVMat();

  cv::Vec3b convertedColor1 = openCvImage.at<cv::Vec3b>(pos1);
  cv::Vec3b convertedColor2 = openCvImage.at<cv::Vec3b>(pos2);
  cv::Vec3b convertedColor3 = openCvImage.at<cv::Vec3b>(pos3);

  MITK_TEST_CONDITION( color1 == convertedColor1, "Testing if initially created color values " << static_cast<int>( color1[0] ) << ", " << static_cast<int>( color1[1] ) << ", " << static_cast<int>( color1[2] ) << " matches the color values " << static_cast<int>( convertedColor1[0] ) << ", " << static_cast<int>( convertedColor1[1] ) << ", " << static_cast<int>( convertedColor1[2] ) << " at the same position " << pos1.x << ", " << pos1.y << " in the back converted OpenCV image" )

  MITK_TEST_CONDITION( color2 == convertedColor2, "Testing if initially created color values " << static_cast<int>( color2[0] ) << ", " << static_cast<int>( color2[1] ) << ", " << static_cast<int>( color2[2] ) << " matches the color values " << static_cast<int>( convertedColor2[0] ) << ", " << static_cast<int>( convertedColor2[1] ) << ", " << static_cast<int>( convertedColor2[2] ) << " at the same position " << pos2.x << ", " << pos2.y << " in the back converted OpenCV image" )

  MITK_TEST_CONDITION( color3 == convertedColor3, "Testing if initially created color values " << static_cast<int>( color3[0] ) << ", " << static_cast<int>( color3[1] ) << ", " << static_cast<int>( color3[2] ) << " matches the color values " << static_cast<int>( convertedColor3[0] ) << ", " << static_cast<int>( convertedColor3[1] ) << ", " << static_cast<int>( convertedColor3[2] ) << " at the same position " << pos3.x << ", " << pos3.y << " in the back converted OpenCV image" )

  MITK_TEST_END();

}


