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
#include "mitkTestingMacros.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkItkImageFileReader.h"
#include "mitkException.h"

bool CompareSurfacePointPositions(mitk::Surface::Pointer s1, mitk::Surface::Pointer s2)
{
  vtkPoints* p1 = s1->GetVtkPolyData()->GetPoints();
  vtkPoints* p2 = s2->GetVtkPolyData()->GetPoints();

  if(p1->GetNumberOfPoints() != p2->GetNumberOfPoints())
    return false;

  for(int i = 0; i < p1->GetNumberOfPoints(); ++i)
  {
    if(p1->GetPoint(i)[0] != p2->GetPoint(i)[0] ||
      p1->GetPoint(i)[1] != p2->GetPoint(i)[1] ||
      p1->GetPoint(i)[2] != p2->GetPoint(i)[2] )
    {
      return true;
    }
  }
  return false;
}

int mitkImageToSurfaceFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ImageToSurfaceFilterTest");
  mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
  MITK_TEST_CONDITION_REQUIRED(testObject.IsNotNull(), "Testing instantiation of test object");

  mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
  reader->SetFileName(argv[1]);
  reader->Update();
  mitk::Image::Pointer tImage = reader->GetOutput();

  // testing initialization of member variables!
  MITK_TEST_CONDITION_REQUIRED(testObject->GetThreshold() == 1.0f, "Testing initialization of threshold member variable");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetSmooth() == false, "Testing initialization of smooth member variable");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDecimate() == mitk::ImageToSurfaceFilter::NoDecimation, "Testing initialization of decimate member variable");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetTargetReduction() == 0.95f, "Testing initialization of target reduction member variable");

  // test cases excluded until bug 14530 is fixed, since wrong exception is caught!!
  //MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception)
  //testObject->Update();
  //MITK_TEST_FOR_EXCEPTION_END(mitk::Exception)

  //mitk::Image::Pointer emptyImage = mitk::Image::New();
  //testObject->SetInput(emptyImage);
  //MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception)
  //testObject->Update();
  //MITK_TEST_FOR_EXCEPTION_END(mitk::Exception)

  testObject->SetInput(tImage);
  MITK_TEST_CONDITION_REQUIRED(testObject->GetInput() == tImage, "Testing set / get input!");

  testObject->Update();
  mitk::Surface::Pointer resultSurface = NULL;
  resultSurface = testObject->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(testObject->GetOutput() != NULL, "Testing surface generation!");

  mitk::Surface::Pointer testSurface1 = testObject->GetOutput()->Clone();

  testObject->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);
  testObject->SetTargetReduction(0.5f);
  testObject->Update();
  mitk::Surface::Pointer testSurface2 = testObject->GetOutput()->Clone();

  MITK_TEST_CONDITION_REQUIRED(testSurface1->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() > testSurface2->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() , "Testing DecimatePro mesh decimation!");

  testObject->SetDecimate(mitk::ImageToSurfaceFilter::QuadricDecimation);
  testObject->SetTargetReduction(0.5f);
  testObject->Update();
  mitk::Surface::Pointer testSurface3 = testObject->GetOutput()->Clone();

  MITK_TEST_CONDITION_REQUIRED(testSurface1->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() > testSurface3->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() , "Testing QuadricDecimation mesh decimation!");

  testObject->SetSmooth(true);
  testObject->SetDecimate(mitk::ImageToSurfaceFilter::NoDecimation);
  testObject->Update();
  mitk::Surface::Pointer testSurface4 = testObject->GetOutput()->Clone();
  MITK_TEST_CONDITION_REQUIRED( CompareSurfacePointPositions(testSurface1, testSurface4), "Testing smoothing of surface changes point data!");

  // thats it folks
  MITK_TEST_END();
}
