/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkImage.h"
#include "mitkPicFileReader.h"
#include "mitkImageWriter.h"
#include "mitkImageAccessByItk.h"

#include <itksys/SystemTools.hxx>

unsigned int numberOfTestImages = 3;

// create one test image
mitk::Image::Pointer CreateTestImage(unsigned int which)
{
  mitk::Image::Pointer image = mitk::Image::New();

  switch (which)
  {
    case 0:
    {
      unsigned int dim[]={10,10,20}; // image dimensions

      image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
      int *p = (int*)image->GetData(); // pointer to pixel data
      int size = dim[0]*dim[1]*dim[2];
      for(int i=0; i<size; ++i, ++p) *p=i - size/2; // fill image
    }
    break;

    case 1:
    {
      unsigned int dim[]={10,10,20}; // image dimensions

      image->Initialize(mitk::PixelType(typeid(float)), 3, dim);
      float *p = (float*)image->GetData(); // pointer to pixel data
      int size = dim[0]*dim[1]*dim[2];
      for(int i=0; i<size; ++i, ++p) *p=(float)i - size/2; // fill image
    }
    break;

    case 2:
    {
      unsigned int dim[]={10,10,20}; // image dimensions

      image->Initialize(mitk::PixelType(typeid(double)), 3, dim);
      double *p = (double*)image->GetData(); // pointer to pixel data
      int size = dim[0]*dim[1]*dim[2];
      for(int i=0; i<size; ++i, ++p) *p=(double)i - size/2; // fill image
    }
    break;


    default:
    {
      unsigned int dim[]={10,10,20}; // image dimensions

      image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
      int *p = (int*)image->GetData(); // pointer to pixel data
      int size = dim[0]*dim[1]*dim[2];
      for(int i=0; i<size; ++i, ++p) *p=i - size/2; // fill image
    }
  }

  return image;

}

template < typename TPixel, unsigned int VImageDimension >
void ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Image* mitkImage, bool& identical ) 
{
  typename itk::Image< TPixel, VImageDimension >::Pointer itkImage2;

  mitk::CastToItkImage( mitkImage, itkImage2 );

  if (!itkImage2 || !itkImage2.GetPointer())
  {
    identical = false;
    return;
  }

  itk::ImageRegionConstIterator<itk::Image<TPixel, VImageDimension> > iterItkImage1( itkImage, itkImage->GetLargestPossibleRegion() );
  itk::ImageRegionConstIterator<itk::Image<TPixel, VImageDimension> > iterItkImage2( itkImage, itkImage2->GetLargestPossibleRegion() );

  iterItkImage1.GoToBegin();
  iterItkImage2.GoToBegin();

  while (!iterItkImage1.IsAtEnd())
  {
    if (iterItkImage1.Get() != iterItkImage2.Get())
    {
      std::cout << iterItkImage1.Get() << " != " << iterItkImage2.Get() << std::endl;
      identical = false;
      return;
    }

    ++iterItkImage1;
    ++iterItkImage2;
  }

  // if we reach this point, all pixel are the same
  identical = true;
}
 
int mitkPicFileIOTest(int, char*[])
{ 
  unsigned int numberFailed(0);

  for (unsigned int i = 0; i < numberOfTestImages; ++i)
  {
    mitk::Image::Pointer originalImage = CreateTestImage(i);
    mitk::Image::Pointer secondImage;

    // write
      try
      {
        mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
        imageWriter->SetInput(originalImage);
        
        imageWriter->SetFileName("test_image");
        imageWriter->SetExtension(".pic");
        imageWriter->Write();
      }
      catch ( std::exception& e )
      {
        std::cerr << "Error during attempt to write 'test_image.pic' Exception says:" << std::endl;
        std::cerr << e.what() << std::endl;
        ++numberFailed;
        continue;
      }
 
    // load
      try
      {
        mitk::PicFileReader::Pointer imageReader = mitk::PicFileReader::New();
        
        imageReader->SetFileName("test_image.pic");
        imageReader->Update();

        secondImage = imageReader->GetOutput();
      }
      catch ( std::exception& e )
      {
        std::cerr << "Error during attempt to read 'test_image.pic' Exception says:" << std::endl;
        std::cerr << e.what() << std::endl;
        ++numberFailed;
        continue;
      }

      if (secondImage.IsNull())
      {
        std::cerr << "Error reading 'test_image.pic'. No image created." << std::endl;
        ++numberFailed;
        continue;
      }
    
    std::remove( "test_image.pic" );
  
    // compare
  
    bool identical(false);
    AccessFixedDimensionByItk_2( secondImage.GetPointer(), ItkImageProcessing, 3, originalImage.GetPointer(), identical );

    if (!identical)
    {
      std::cerr << "Images differ for testimage " << i << std::endl;
      ++numberFailed;
      continue;
    }
  }

  // if one fails, whole test fails
  if (numberFailed > 0)
  {
    std::cout << "[FAILED]: " << numberFailed << " of " << numberOfTestImages << " sub-tests failed." << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
    return EXIT_SUCCESS;
  }
}

