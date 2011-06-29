/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//mitk includes
#include "mitkTestingMacros.h"
#include "mitkUnstructuredGrid.h"

//vtk includes
#include "vtkUnstructuredGrid.h"  

/**
 *Unit test for the UnstructuredGríd class
 */

int mitkUnstructuredGridTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("UnstructuredGrid")

  // let's create an object of our class  
  mitk::UnstructuredGrid::Pointer testObject = mitk::UnstructuredGrid::New();
  MITK_TEST_CONDITION_REQUIRED(testObject.IsNotNull(),"Testing instantiation")

  mitk::UnstructuredGrid::Pointer cloneObject = testObject->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneObject.IsNotNull(),"Testing instantiation of clone")

  vtkUnstructuredGrid* grid = vtkUnstructuredGrid::New();
  testObject->SetVtkUnstructuredGrid(grid);
  MITK_TEST_CONDITION_REQUIRED(testObject->GetVtkUnstructuredGrid()== grid, "Testing Get/Set-VtkUnstructuredGrid");

  testObject->UpdateOutputInformation();

  mitk::UnstructuredGrid::Pointer copyObject = mitk::UnstructuredGrid::New();
  copyObject->CopyInformation(testObject);
  MITK_TEST_CONDITION_REQUIRED(copyObject->GetLargestPossibleRegion() == testObject->GetLargestPossibleRegion(), "Testing generation of copy object");

  cloneObject = testObject->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneObject->GetRequestedRegion() == testObject->GetRequestedRegion(), "Testing region cloning!");
  
  // always end with this!
  MITK_TEST_END()
}

