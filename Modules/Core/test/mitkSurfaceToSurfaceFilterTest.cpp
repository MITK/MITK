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

#include "mitkCommon.h"
#include "mitkNumericTypes.h"
#include "mitkSurface.h"
#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkTestingMacros.h"

#include "vtkPolyData.h"
#include "vtkSphereSource.h"

int mitkSurfaceToSurfaceFilterTest(int /*argc*/, char * /*argv*/ [])
{
  mitk::Surface::Pointer surface;
  surface = mitk::Surface::New();
  vtkSphereSource *sphereSource = vtkSphereSource::New();
  sphereSource->SetCenter(0, 0, 0);
  sphereSource->SetRadius(5.0);
  sphereSource->SetThetaResolution(10);
  sphereSource->SetPhiResolution(10);
  sphereSource->Update();

  vtkPolyData *polys = sphereSource->GetOutput();
  surface->SetVtkPolyData(polys);
  sphereSource->Delete();

  mitk::SurfaceToSurfaceFilter::Pointer filter = mitk::SurfaceToSurfaceFilter::New();
  MITK_TEST_OUTPUT(<< "Testing mitk::SurfaceToSurfaceFilter::SetInput() and ::GetNumberOfInputs() : ");
  filter->SetInput(surface);
  MITK_TEST_CONDITION(filter->GetNumberOfInputs() == 1, "Number of inputs must be 1");

  MITK_TEST_OUTPUT(<< "Testing if GetInput returns the right Input : ");
  mitk::Surface::Pointer input = const_cast<mitk::Surface *>(filter->GetInput());
  MITK_ASSERT_EQUAL(input, surface, "GetInput() should return correct input. ");

  MITK_TEST_CONDITION(filter->GetInput(5) == nullptr, "GetInput(5) should return nullptr. ");

  MITK_TEST_OUTPUT(<< "Testing whether Output is created correctly : ");
  MITK_TEST_CONDITION(filter->GetNumberOfOutputs() == filter->GetNumberOfInputs(),
                      "Test if number of outputs == number of inputs");

  mitk::Surface::Pointer outputSurface = filter->GetOutput();
  MITK_ASSERT_EQUAL(outputSurface, surface, "Test if output == input");

  filter->Update();
  outputSurface = filter->GetOutput();
  MITK_TEST_CONDITION(outputSurface->GetSizeOfPolyDataSeries() == surface->GetSizeOfPolyDataSeries(),
                      "Test if number of PolyDatas in PolyDataSeries of output == number of PolyDatas of input");

  mitk::SurfaceToSurfaceFilter::Pointer s2sFilter = mitk::SurfaceToSurfaceFilter::New();
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, s2sFilter->CreateOutputForInput(500));

  std::vector<vtkPolyData *> polydatas;
  for (unsigned int i = 0; i < 5; ++i)
  {
    sphereSource = vtkSphereSource::New();
    sphereSource->SetCenter(0, i, 0);
    sphereSource->SetRadius(5.0 + i);
    sphereSource->SetThetaResolution(10);
    sphereSource->SetPhiResolution(10);
    sphereSource->Update();

    vtkPolyData *poly = sphereSource->GetOutput();
    mitk::Surface::Pointer s = mitk::Surface::New();
    s->SetVtkPolyData(poly);
    s2sFilter->SetInput(i, s);
    polydatas.push_back(s2sFilter->GetOutput(i)->GetVtkPolyData());
    sphereSource->Delete();
  }

  // Test if the outputs are not recreated each time another input is added
  for (unsigned int i = 0; i < 5; ++i)
  {
    MITK_TEST_CONDITION(s2sFilter->GetOutput(i)->GetVtkPolyData() == polydatas.at(i),
                        "Test if pointers are still equal");
  }

  // Test if the outputs are recreated after calling CreateOutputsForAllInputs()
  s2sFilter->CreateOutputsForAllInputs();
  for (unsigned int i = 0; i < 5; ++i)
  {
    MITK_TEST_CONDITION(s2sFilter->GetOutput(i)->GetVtkPolyData() != polydatas.at(i), "Test if pointers are not equal");
  }

  // std::cout << "Testing RemoveInputs() : " << std::endl;
  // unsigned int numOfInputs = filter->GetNumberOfInputs();
  // filter->RemoveInputs( mitk::Surface::New() );
  // if ( filter->GetNumberOfInputs() != numOfInputs )
  //{
  //  std::cout << "[FAILED] : input was removed that was not set." << std::endl;
  //  return EXIT_FAILURE;
  //}
  // std::cout << "[SUCCESS] : no input was removed that was not set." << std::endl;
  // filter->RemoveInputs( surface );
  // if ( filter->GetNumberOfInputs() != 0 )
  //{
  //  std::cout << "[FAILED] : existing input was not removed correctly." << std::endl;
  //  return EXIT_FAILURE;
  //}
  // std::cout << "[SUCCESS] : existing input was removed correctly." << std::endl;

  return EXIT_SUCCESS;
}
