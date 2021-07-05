/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCompressedImageContainer.h"
#include "mitkCoreObjectFactory.h"
#include "mitkIOUtil.h"
#include "mitkImageDataItem.h"
#include "mitkImageReadAccessor.h"

class mitkCompressedImageContainerTestClass
{
public:
  static void Test(mitk::CompressedImageContainer *container, mitk::Image *image, unsigned int &numberFailed)
  {
    container->CompressImage(image);
    mitk::Image::Pointer uncompressedImage = container->DecompressImage();

    // check dimensions
    if (image->GetDimension() != uncompressedImage->GetDimension())
    {
      ++numberFailed;
      std::cerr << "  (EE) Number of image dimensions wrong after uncompression (was: " << image->GetDimension()
                << ", now: " << uncompressedImage->GetDimension() << ")" << std::endl;
    }

    for (unsigned int dim = 0; dim < image->GetDimension(); ++dim)
    {
      if (image->GetDimension(dim) != uncompressedImage->GetDimension(dim))
      {
        ++numberFailed;
        std::cerr << "  (EE) Image dimension " << dim
                  << " differs after uncompression (was: " << image->GetDimension(dim)
                  << ", now: " << uncompressedImage->GetDimension(dim) << ")" << std::endl;
      }
    }

    // check pixel type
    if (image->GetPixelType() != uncompressedImage->GetPixelType())
    {
      ++numberFailed;
      std::cerr << "  (EE) Pixel type wrong after uncompression:" << std::endl;

      mitk::PixelType m_PixelType = image->GetPixelType();
      std::cout << "Original pixel type:" << std::endl;
      std::cout << " PixelType: " << m_PixelType.GetTypeAsString() << std::endl;
      std::cout << " BitsPerElement: " << m_PixelType.GetBpe() << std::endl;
      std::cout << " NumberOfComponents: " << m_PixelType.GetNumberOfComponents() << std::endl;
      std::cout << " BitsPerComponent: " << m_PixelType.GetBitsPerComponent() << std::endl;

      // m_PixelType = uncompressedImage->GetPixelType();
      std::cout << "Uncompressed pixel type:" << std::endl;
      std::cout << " PixelType: " << uncompressedImage->GetPixelType().GetTypeAsString() << std::endl;
      std::cout << " BitsPerElement: " << uncompressedImage->GetPixelType().GetBpe() << std::endl;
      std::cout << " NumberOfComponents: " << uncompressedImage->GetPixelType().GetNumberOfComponents() << std::endl;
      std::cout << " BitsPerComponent: " << uncompressedImage->GetPixelType().GetBitsPerComponent() << std::endl;
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
      mitk::ImageReadAccessor origImgAcc(image, image->GetVolumeData(timeStep));
      mitk::ImageReadAccessor unCompImgAcc(uncompressedImage, uncompressedImage->GetVolumeData(timeStep));

      auto *originalData((unsigned char *)origImgAcc.GetData());
      auto *uncompressedData((unsigned char *)unCompImgAcc.GetData());

      unsigned long difference(0);
      for (unsigned long byte = 0; byte < oneTimeStepSizeInBytes; ++byte)
      {
        if (originalData[byte] != uncompressedData[byte])
        {
          ++difference;
        }
      }

      if (difference > 0)
      {
        ++numberFailed;
        std::cerr << "  (EE) Pixel data in timestep " << timeStep << " not identical after uncompression. "
                  << difference << " pixels different." << std::endl;
        break; // break "for timeStep"
      }
    }
  }
};

/// ctest entry point
int mitkCompressedImageContainerTest(int argc, char *argv[])
{
  // one big variable to tell if anything went wrong
  unsigned int numberFailed(0);

  // need one parameter (image filename)
  if (argc == 0)
  {
    std::cerr << "No file specified [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // load the image

  mitk::Image::Pointer image = nullptr;
  try
  {
    std::cout << "Testing with parameter '" << argv[1] << "'" << std::endl;
    image = mitk::IOUtil::Load<mitk::Image>(argv[1]);
  }
  catch (const mitk::Exception &)
  {
    std::cout << "File not an image - test will not be applied [PASSED]" << std::endl;
    std::cout << "[TEST DONE]" << std::endl;
    return EXIT_SUCCESS;
  }
  catch (itk::ExceptionObject &ex)
  {
    ++numberFailed;
    std::cerr << "Exception: " << ex << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "  (II) Could load image." << std::endl;

  {
    mitk::CompressedImageContainer container;

    // some real work
    mitkCompressedImageContainerTestClass::Test(&container, image, numberFailed);

    std::cout << "Testing destruction" << std::endl;
  }

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
