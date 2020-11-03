/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabeledImageToSurfaceFilter.h"
#include "mitkReferenceCountWatcher.h"
#include <itksys/SystemTools.hxx>

#include "mitkIOUtil.h"

#include <cmath>

bool equals(const mitk::ScalarType &val1, const mitk::ScalarType &val2, mitk::ScalarType epsilon = mitk::eps)
{
  return (std::fabs(val1 - val2) <= epsilon);
}

int mitkLabeledImageToSurfaceFilterTest(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "no path to testing specified [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::string fileIn = argv[1];
  std::cout << "Eingabe Datei: " << fileIn << std::endl;
  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(fileIn);
  if (image->GetPixelType() != mitk::PixelType(mitk::MakeScalarPixelType<char>()) ||
      image->GetPixelType() != mitk::PixelType(mitk::MakeScalarPixelType<unsigned char>()))
  {
    std::cout << "file not a char or unsigned char image - test will not be applied [PASSED]" << std::endl;
    std::cout << "[TEST DONE]" << std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "Testing instantiation: ";
  mitk::LabeledImageToSurfaceFilter::Pointer filter = mitk::LabeledImageToSurfaceFilter::New();
  if (filter.IsNull())
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Create surface with default settings: ";
  filter->SetInput(image);
  filter->Update();

  if (filter->GetNumberOfOutputs() != 1)
  {
    std::cout << "Wrong number of outputs, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput() == nullptr)
  {
    std::cout << "Output is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput()->GetVtkPolyData() == nullptr)
  {
    std::cout << "PolyData of surface is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing index to label conversion: ";
  if (filter->GetLabelForNthOutput(0) != 257)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for label calculation: ";
  if (!equals(filter->GetVolumeForLabel(257), 14.328))
  {
    std::cout << filter->GetVolumeForLabel(257) << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for index calculation: ";
  if (!equals(filter->GetVolumeForNthOutput(0), 14.328))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Create surface using optimised settings: ";
  filter->GenerateAllLabelsOn();
  filter->SetGaussianStandardDeviation(1.5);
  filter->SetSmooth(true); // smooth wireframe
  filter->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);
  filter->SetTargetReduction(0.8);
  if (filter->GetNumberOfOutputs() != 1)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Create surface for label 257: ";
  filter->GenerateAllLabelsOff();
  filter->SetLabel(257);
  filter->Update();
  if (filter->GetNumberOfOutputs() != 1)
  {
    std::cout << "Wrong number of outputs, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput() == nullptr)
  {
    std::cout << "Output is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput()->GetVtkPolyData() == nullptr)
  {
    std::cout << "PolyData of surface is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for label calculation: ";
  if (!equals(filter->GetVolumeForLabel(257), 14.328))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for index calculation: ";
  if (!equals(filter->GetVolumeForNthOutput(0), 14.328))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Create surface for multiple labels: ";
  filter->GenerateAllLabelsOn();
  filter->SetBackgroundLabel(32000);
  filter->Update();
  if (filter->GetNumberOfOutputs() != 2)
  {
    std::cout << "Wrong number of outputs, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput() == nullptr)
  {
    std::cout << "Output 0 is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput()->GetVtkPolyData() == nullptr)
  {
    std::cout << "PolyData of output 0 is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput(1) == nullptr)
  {
    std::cout << "Output 1 is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput(1)->GetVtkPolyData() == nullptr)
  {
    std::cout << "PolyData of output 1 is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for label calculation: ";
  if (!equals(filter->GetVolumeForLabel(257), 14.328))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (!equals(filter->GetVolumeForLabel(0), 12.672))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for index calculation: ";
  if (!equals(filter->GetVolumeForNthOutput(1), 14.328))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (!equals(filter->GetVolumeForNthOutput(0), 12.672))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  mitk::ReferenceCountWatcher::Pointer outputSurface1Watcher =
    new mitk::ReferenceCountWatcher(filter->GetOutput(1), "outputSurface1");
  mitk::ReferenceCountWatcher::Pointer filterWatcher = new mitk::ReferenceCountWatcher(filter, "filter");

  std::cout << "Create surface for background (label 0): " << std::flush;
  filter->GenerateAllLabelsOff();
  filter->SetLabel(0);
  filter->SetBackgroundLabel(257);
  // mitk::Surface::Pointer surface = filter->GetOutput(1);
  // std::cout<< surface->GetReferenceCount() << std::endl;
  filter->Update();
  // surface = nullptr;

  if (filter->GetNumberOfOutputs() != 1)
  {
    std::cout << "Wrong number of outputs, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput() == nullptr)
  {
    std::cout << "Output is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput()->GetVtkPolyData() == nullptr)
  {
    std::cout << "PolyData of surface is nullptr, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }
  std::cout << "Testing reference count correctness of old output 1: " << std::flush;
  if (outputSurface1Watcher->GetReferenceCount() != 0)
  {
    std::cout << "outputSurface1Watcher->GetReferenceCount()==" << outputSurface1Watcher->GetReferenceCount()
              << "!=0, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  std::cout << "Testing reference count correctness of filter: " << std::flush;
  if (filterWatcher->GetReferenceCount() != 2)
  {
    std::cout << "filterWatcher->GetReferenceCount()==" << outputSurface1Watcher->GetReferenceCount() << "!=2, [FAILED]"
              << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing index to label conversion: ";
  if (filter->GetLabelForNthOutput(0) != 0)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for label calculation: ";
  if (!equals(filter->GetVolumeForLabel(filter->GetLabel()), 12.672))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing volume for index calculation: ";
  if (!equals(filter->GetVolumeForNthOutput(0), 12.672))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Create surface for invalid label: ";
  filter->GenerateAllLabelsOff();
  filter->SetLabel(1);
  filter->Update();
  if (filter->GetNumberOfOutputs() != 1)
  {
    std::cout << "Number of outputs != 1, [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else if (filter->GetOutput()->GetVtkPolyData()->GetNumberOfPoints() != 0)
  {
    std::cout << "PolyData is not empty (" << filter->GetOutput()->GetVtkPolyData()->GetNumberOfPoints()
              << "), [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
