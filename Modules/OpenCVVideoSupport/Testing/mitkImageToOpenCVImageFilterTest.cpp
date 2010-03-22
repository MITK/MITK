/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fr, 12 Mrz 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkImageToOpenCVImageFilter.h"
#include "mitkOpenCVToMitkImageFilter.h"
#include <mitkTestingMacros.h>
#include <mitkITKImageImport.h>
#include <iostream>

// #include <itkImageFileWriter.h>
// #include <highgui.h>

/**Documentation
 *  test for the class "ImageTOOpenCVImageFilter".
 */
int mitkImageTOOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ImageTOOpenCVImageFilter")

  // create itk rgb image
  typedef unsigned char PixelType;
  typedef itk::Image< itk::RGBPixel<PixelType>, 2 > ImageType;
  ImageType::Pointer itkImage = ImageType::New();

  ImageType::IndexType start;
  start[0] = 0; // first index on X
  start[1] = 0; // first index on Y
  ImageType::SizeType size;
  size[0] = 50; // size along X
  size[1] = 40; // size along Y
  ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  itkImage->SetRegions( region );
  itkImage->Allocate();

  typedef itk::ImageRegionIterator<ImageType> IteratorType;
  IteratorType it(itkImage, region);
  float twoThirdsTheWidth = size[0] / 4;
  unsigned int x=0, y=0;
  // create rgb pic
  for ( it.GoToBegin(); !it.IsAtEnd(); ++it ) 
  {
    ImageType::PixelType newPixel;
    newPixel.SetRed(0);
    newPixel.SetGreen(0);
    // create asymmetric pic
    if( x > twoThirdsTheWidth )
      newPixel.SetBlue(0);
    else
      newPixel.SetBlue(255);
    it.Set(newPixel);   

    ++x;
    // next line found
    if( x == size[0] )
      x = 0;
  }

  // debugging
//   itk::ImageFileWriter< ImageType >::Pointer writer = itk::ImageFileWriter< ImageType >::New();
//   writer->SetFileName( "c:\\image.png" );
//   writer->SetInput ( itkImage );
//   writer->Update();

  // import rgb image as MITK image
  mitk::Image::Pointer mitkImage = mitk::ImportItkImage( itkImage );

  mitk::ImageToOpenCVImageFilter::Pointer _ImageToOpenCVImageFilter =
    mitk::ImageToOpenCVImageFilter::New();

  _ImageToOpenCVImageFilter->SetImage( mitkImage.GetPointer() );
  
  IplImage* openCVImage = _ImageToOpenCVImageFilter->GetOpenCVImage();

  // check byte size
  const unsigned int expectedSize = size[0] * size[1] * 3 * sizeof( PixelType );
  const unsigned int realSize = openCVImage->width * openCVImage->height * openCVImage->nChannels * sizeof  ( PixelType );  
  MITK_TEST_CONDITION_REQUIRED( expectedSize == realSize, "Test expectedSize == realSize");

  // check pixel values
  PixelType expectedBlueValue;
  CvScalar s;
  for (y = 0; y < openCVImage->height; ++y)
  {
    for (x = 0; x < openCVImage->width; ++x)
    {
      expectedBlueValue = 255;
      if(x > twoThirdsTheWidth)
        expectedBlueValue = 0;

      s = cvGet2D(openCVImage,y,x);
      if( s.val[0] != expectedBlueValue || s.val[1] != 0 || s.val[2] != 0 )
      {
        std::cout << "Wrong RGB values in created OpenCV image" << std::endl;
        throw mitk::TestFailedException();
      }
    }
  }

//   cvNamedWindow( "test" );
//   cvShowImage( "test" , openCVImage );
//   cvWaitKey();

  // always end with this!
  MITK_TEST_END();

}


