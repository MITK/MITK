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
#include <mitkItkImageFileReader.h>

// #include <itkImageFileWriter.h>
#include <highgui.h>

mitk::Image::Pointer LoadImage( std::string filename )
{
  mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
  reader->SetFileName ( filename.c_str() );
  reader->Update();
  if ( reader->GetOutput() == NULL )
    itkGenericExceptionMacro("File "<<filename <<" could not be read!");
  mitk::Image::Pointer image = reader->GetOutput();
  return image;
}

static void testGeneratedImage()
{
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

    _ImageToOpenCVImageFilter->SetImage( mitkImage );

    IplImage* openCVImage = _ImageToOpenCVImageFilter->GetOpenCVImage();

    MITK_TEST_CONDITION_REQUIRED( openCVImage != NULL, "Image returned by filter is not null.");

    // check byte size
    const unsigned int expectedSize = size[0] * size[1] * 3 * sizeof( unsigned char);// sizeof( PixelType );
    const unsigned int realSize = openCVImage->width * openCVImage->height * openCVImage->nChannels * sizeof( unsigned char);//* sizeof  ( PixelType );
    MITK_TEST_CONDITION_REQUIRED( expectedSize == realSize, "Test expectedSize == realSize");

    // check pixel values
    PixelType expectedBlueValue;
    CvScalar s;
    for (y = 0; (int)y < openCVImage->height; ++y)
    {
      for (x = 0; (int)x < openCVImage->width; ++x)
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

//     cvNamedWindow( "test" );
//     cvShowImage( "test" , openCVImage );
//     cvWaitKey();

}

static void testLoadedImage(std::string mitkImagePath)
{
      mitk::Image::Pointer testImage = LoadImage(mitkImagePath);
    mitk::ImageToOpenCVImageFilter::Pointer _ImageToOpenCVImageFilter =
      mitk::ImageToOpenCVImageFilter::New();

    _ImageToOpenCVImageFilter->SetImage( testImage );

    IplImage* openCVImage = _ImageToOpenCVImageFilter->GetOpenCVImage();
    IplImage* openCVImage_Ref = cvLoadImage(mitkImagePath.c_str());

    MITK_TEST_CONDITION_REQUIRED( openCVImage != NULL, "Image returned by filter is not null.");

    for(int i = 0 ; i<openCVImage->height ; i++)
    {
        for(int j = 0 ; j<openCVImage->width ; j++)
        {
            CvScalar s;
            s=cvGet2D(openCVImage,i,j); // get the (i,j) pixel value
            CvScalar sRef;
            sRef=cvGet2D(openCVImage_Ref,i,j); // get the (i,j) pixel value
            for(int c = 0 ; c < openCVImage->nChannels ; c++)
            {
            MITK_TEST_CONDITION_REQUIRED( s.val[c] == sRef.val[c] , "All pixel values have to be equal");
            }
        }
    }


//     cvNamedWindow( "test" );
//     cvShowImage( "test" , openCVImage );
//     cvWaitKey();
}

/**Documentation
 *  test for the class "ImageToOpenCVImageFilter".
 */
int mitkImageToOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ImageToOpenCVImageFilter")

  testGeneratedImage();
  testLoadedImage(argv[1]);
  // always end with this!
  MITK_TEST_END();

}


