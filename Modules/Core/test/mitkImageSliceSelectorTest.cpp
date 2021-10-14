/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageSliceSelector.h>

#include <fstream>
int mitkImageSliceSelectorTest(int argc, char *argv[])
{
  int slice_nr = 1;
  std::cout << "Loading file: ";
  if (argc == 0)
  {
    std::cout << "no file specified [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  mitk::Image::Pointer image;
  try
  {
    image = mitk::IOUtil::Load<mitk::Image>(argv[1]);
  }
  catch (const mitk::Exception &)
  {
    std::cout << "file not an image - test will not be applied [PASSED]" << std::endl;
    std::cout << "[TEST DONE]" << std::endl;
    return EXIT_SUCCESS;
  }
  catch ( const itk::ExceptionObject &ex )
  {
    std::cout << "Exception: " << ex.GetDescription() << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  if (image->GetDimension(2) < 2)
    slice_nr = 0;

  // Take a slice
  mitk::ImageSliceSelector::Pointer slice = mitk::ImageSliceSelector::New();
  slice->SetInput(image);
  slice->SetSliceNr(slice_nr);
  slice->Update();

  std::cout << "Testing IsInitialized(): ";
  if (slice->GetOutput()->IsInitialized() == false)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing IsSliceSet(): ";
  if (slice->GetOutput()->IsSliceSet(0) == false)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  try
  {
    slice->UpdateLargestPossibleRegion();
  }
  catch (const itk::ExceptionObject &)
  {
    std::cout << "Part 2 [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Part 2 [PASSED]" << std::endl;

  std::cout << "Testing IsInitialized(): ";
  if (slice->GetOutput()->IsInitialized() == false)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing IsSliceSet(): ";
  if (slice->GetOutput()->IsSliceSet(0) == false)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  if (image->GetDimension(3) > 1)
  {
    int time = image->GetDimension(3) - 1;

    std::cout << "Testing 3D+t: Setting time to " << time << ": ";
    slice->SetTimeNr(time);
    if (slice->GetTimeNr() != time)
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "[PASSED]" << std::endl;

    std::cout << "Testing 3D+t: Updating slice: ";
    slice->Update();
    if (slice->GetOutput()->IsInitialized() == false)
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "[PASSED]" << std::endl;

    std::cout << "Testing 3D+t: IsSliceSet(): ";
    if (slice->GetOutput()->IsSliceSet(0) == false)
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "[PASSED]" << std::endl;

    std::cout << "Testing 3D+t: First slice in reader available: ";
    if (image->IsSliceSet(0, time) == false)
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
