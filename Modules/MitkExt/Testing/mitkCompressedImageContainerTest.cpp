/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 3056 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkCompressedImageContainer.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkImageDataItem.h"
#include "itkSmartPointerForwardReference.txx"

class mitkCompressedImageContainerTestClass
{
  public:
  
static void Test( mitk::CompressedImageContainer* container, mitk::Image* image, unsigned int& numberFailed )
{
  container->SetImage( image ); // compress
  mitk::Image::Pointer uncompressedImage = container->GetImage(); // uncompress

  // check dimensions
  if (image->GetDimension() != uncompressedImage->GetDimension())
  {
    ++numberFailed;
    std::cerr << "  (EE) Number of image dimensions wrong after uncompression (was: " << image->GetDimension() << ", now: " << uncompressedImage->GetDimension() << ")" << std::endl;
  }

  for (unsigned int dim = 0; dim < image->GetDimension(); ++dim)
  {
    if (image->GetDimension(dim) != uncompressedImage->GetDimension(dim))
    {
      ++numberFailed;
      std::cerr << "  (EE) Image dimension " << dim << " differs after uncompression (was: " << image->GetDimension(dim) << ", now: " << uncompressedImage->GetDimension(dim) << ")" << std::endl;
    }
  }

   // check pixel type
  if (image->GetPixelType() != uncompressedImage->GetPixelType())
  {
    ++numberFailed;
    std::cerr << "  (EE) Pixel type wrong after uncompression:" << std::endl;
    
    mitk::PixelType m_PixelType = image->GetPixelType();
    std::cout << "Original pixel type:" << std::endl;
    std::cout << " PixelType: " << m_PixelType.GetTypeId()->name() << std::endl;
    std::cout << " BitsPerElement: " << m_PixelType.GetBpe() << std::endl;
    std::cout << " NumberOfComponents: " << m_PixelType.GetNumberOfComponents() << std::endl;
    std::cout << " BitsPerComponent: " << m_PixelType.GetBitsPerComponent() << std::endl;

    m_PixelType = uncompressedImage->GetPixelType();
    std::cout << "Uncompressed pixel type:" << std::endl;
    std::cout << " PixelType: " << m_PixelType.GetTypeId()->name() << std::endl;
    std::cout << " BitsPerElement: " << m_PixelType.GetBpe() << std::endl;
    std::cout << " NumberOfComponents: " << m_PixelType.GetNumberOfComponents() << std::endl;
    std::cout << " BitsPerComponent: " << m_PixelType.GetBitsPerComponent() << std::endl;
  }

  // check data
  mitk::PixelType m_PixelType = image->GetPixelType();
  unsigned long oneTimeStepSizeInBytes = m_PixelType.GetBpe() >> 3; // bits per element divided by 8
  for (unsigned int dim = 0; dim < image->GetDimension(); ++dim)
  {
    if (dim < 3)
    {
      oneTimeStepSizeInBytes *= image->GetDimension(dim);
    }
  }

  unsigned int numberOfTimeSteps(1);
  if (image->GetDimension() > 3) 
  {
    numberOfTimeSteps = image->GetDimension(3);
  }

  for (unsigned int timeStep = 0; timeStep < numberOfTimeSteps; ++timeStep)
  {
    unsigned char* originalData( static_cast<unsigned char*>(image->GetVolumeData(timeStep)->GetData()) );
    unsigned char* uncompressedData( static_cast<unsigned char*>(uncompressedImage->GetVolumeData(timeStep)->GetData()) );

    unsigned long difference(0);
    for (unsigned long byte = 0; byte < oneTimeStepSizeInBytes; ++byte)
    {
      if ( originalData[byte] != uncompressedData[byte] )
      {
        ++difference;
      }
    }

    if ( difference > 0 )
    {
      ++numberFailed;
      std::cerr << "  (EE) Pixel data in timestep " << timeStep << " not identical after uncompression. " << difference << " pixels different." << std::endl;
      break; // break "for timeStep"
    }
  }
 
}

};

/// ctest entry point
int mitkCompressedImageContainerTest(int argc, char* argv[])
{
  // one big variable to tell if anything went wrong
    unsigned int numberFailed(0);
 
  // need one parameter (image filename)
    if(argc==0)
    {
      std::cerr<<"No file specified [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
 
    std::cout << "Creating CoreObjectFactory" << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());

  // load the image

    mitk::Image::Pointer image = NULL;
    mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
    try
    {
      std::cout << "Testing with parameter '" << argv[1] << "'" << std::endl;
      factory->SetFileName( argv[1] );
      factory->Update();

      if(factory->GetNumberOfOutputs()<1)
      {
        std::cerr<<"File could not be loaded [FAILED]"<<std::endl;
        return EXIT_FAILURE;
      }
      mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
      image = dynamic_cast<mitk::Image*>(node->GetData());
      if(image.IsNull())
      {
        std::cout<<"File not an image - test will not be applied [PASSED]"<<std::endl;
        std::cout<<"[TEST DONE]"<<std::endl;
        return EXIT_SUCCESS;
      }
    }
    catch ( itk::ExceptionObject & ex )
    {
      ++numberFailed;
      std::cerr << "Exception: " << ex << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "  (II) Could load image." << std::endl;
    std::cout << "Testing instantiation" << std::endl;

  // instantiation
    mitk::CompressedImageContainer::Pointer container = mitk::CompressedImageContainer::New();
    if (container.IsNotNull())
    {
      std::cout << "  (II) Instantiation works." << std::endl;
    }
    else
    {
      ++numberFailed;
      std::cout << "Test failed, and it's the ugliest one!" << std::endl;
      return EXIT_FAILURE;
    }

  // some real work
    mitkCompressedImageContainerTestClass::Test( container, image, numberFailed );
  
    std::cout << "Testing destruction" << std::endl;

  // freeing
    container = NULL;
      
    std::cout << "  (II) Freeing works." << std::endl;

    if (numberFailed > 0)
    {
      std::cerr << numberFailed << " tests failed." << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      std::cout << "PASSED all tests." << std::endl;
      return EXIT_SUCCESS;
    }
}


