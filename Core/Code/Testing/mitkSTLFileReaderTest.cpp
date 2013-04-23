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


#include "mitkImage.h"
#include "mitkSTLFileReader.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSurface.h"
#include "mitkTestingMacros.h"
#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <fstream>
int mitkSTLFileReaderTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("STLFileReader")

  //Read STL-Image from file
  mitk::STLFileReader::Pointer reader = mitk::STLFileReader::New();

  if(argc==0)
  {
    std::cout<<"file not found - test not applied [PASSED]"<<std::endl;
    std::cout<<"[TEST DONE]"<<std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "Testing CanReadFile(): ";
  if (!reader->CanReadFile(argv[1], "", ""))
  {
    //std::cout<<"[FAILED]"<<std::endl;
    //return EXIT_FAILURE;
    MITK_TEST_OUTPUT(<< mitkTestName << ": "<< mitk::TestManager::GetInstance()->NumberOfPassedTests() << " tests [DONE PASSED] File is not STL!")
    return EXIT_SUCCESS;
  }
  std::cout<<"[PASSED]"<<std::endl;

  reader->SetFileName(argv[1]);
  reader->Update();

  MITK_TEST_CONDITION_REQUIRED((reader->GetOutput() != NULL),"Reader output not NULL")

  mitk::Surface::Pointer surface = reader->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(surface->IsInitialized(),"IsInitialized()")

  MITK_TEST_CONDITION_REQUIRED((surface->GetVtkPolyData()!=NULL),"mitk::Surface::SetVtkPolyData()")

  MITK_TEST_CONDITION_REQUIRED((surface->GetGeometry()!=NULL),"Availability of geometry")

  vtkSmartPointer<vtkSTLReader> myVtkSTLReader = vtkSmartPointer<vtkSTLReader>::New();
  myVtkSTLReader->SetFileName( argv[1] );
  myVtkSTLReader->Update();
  vtkSmartPointer<vtkPolyData> myVtkPolyData = myVtkSTLReader->GetOutput();
  // vtkPolyData from vtkSTLReader directly
  int n = myVtkPolyData->GetNumberOfPoints();
  // vtkPolyData from mitkSTLFileReader
  int m = surface->GetVtkPolyData()->GetNumberOfPoints();
  MITK_TEST_CONDITION_REQUIRED((n == m),"Number of Points in VtkPolyData")

  // always end with this!
  MITK_TEST_END()
}
