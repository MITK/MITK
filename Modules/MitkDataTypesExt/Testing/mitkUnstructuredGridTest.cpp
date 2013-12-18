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
  MITK_TEST_CONDITION_REQUIRED( testObject->VerifyRequestedRegion(), "Requested region verification after initialization!");

  //create a clone object
  mitk::UnstructuredGrid::Pointer cloneObject = testObject->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneObject.IsNotNull(),"Testing instantiation of clone")
  MITK_TEST_CONDITION_REQUIRED( cloneObject->VerifyRequestedRegion(), "Requested region verification after initialization of clone!");

    // set some grid data to the class
  vtkUnstructuredGrid* grid = vtkUnstructuredGrid::New();
  testObject->SetVtkUnstructuredGrid(grid);
  MITK_TEST_CONDITION_REQUIRED(testObject->GetVtkUnstructuredGrid()== grid, "Testing Set/Get vtkUnstructuredGrid");

  // specify a region and set it to the test object
  mitk::UnstructuredGrid::RegionType* ugRegion = new mitk::UnstructuredGrid::RegionType;
  mitk::UnstructuredGrid::RegionType::SizeType size;
  size[0] = 100;
  size[1] = 100;
  size[2] = 100;
  size[3] = 100;
  size[4] = 100;

  mitk::UnstructuredGrid::RegionType::IndexType index;
  index[0] = 0;
  index[1] = 0;
  index[2] = 0;

  ugRegion->SetSize(size);
  ugRegion->SetIndex(index);
  testObject->SetRequestedRegion(ugRegion);
  testObject->UpdateOutputInformation();
  MITK_TEST_CONDITION_REQUIRED(testObject->GetRequestedRegion().GetSize() == ugRegion->GetSize(), "Testing Set/Get of Requested Region by RegionType1!");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetRequestedRegion().GetIndex() == ugRegion->GetIndex(), "Testing Set/Get of Requested Region by RegionType2!");
  // VerifyRequested Region should be false due to the chosen size parameter!
  MITK_TEST_CONDITION_REQUIRED(!testObject->VerifyRequestedRegion(), "Requested Region verification!");

  mitk::UnstructuredGrid::Pointer copyObject = mitk::UnstructuredGrid::New();
  copyObject->CopyInformation(testObject);
  copyObject->SetRequestedRegion(testObject);
  MITK_TEST_CONDITION_REQUIRED(copyObject->GetLargestPossibleRegion() == testObject->GetLargestPossibleRegion(), "Testing generation of copy object");
  MITK_TEST_CONDITION_REQUIRED(copyObject->GetRequestedRegion().GetSize() == ugRegion->GetSize(), "Testing SetRequestedRegion by DataObject1");
  MITK_TEST_CONDITION_REQUIRED(copyObject->GetRequestedRegion().GetIndex() == ugRegion->GetIndex(), "Testing SetRequestedRegion by DataObject2");


  cloneObject = testObject->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneObject->GetRequestedRegion() == testObject->GetRequestedRegion(), "Testing region cloning!");
  MITK_TEST_CONDITION_REQUIRED(cloneObject->GetVtkUnstructuredGrid()== grid, "Testing Get/Set-VtkUnstructuredGrid cloning");
  MITK_TEST_CONDITION_REQUIRED(cloneObject->GetRequestedRegion().GetSize() == ugRegion->GetSize(), "Testing Set/Get of Requested Region by RegionType1 in clone!");
  MITK_TEST_CONDITION_REQUIRED(cloneObject->GetRequestedRegion().GetIndex() == ugRegion->GetIndex(), "Testing Set/Get of Requested Region by RegionType2 in clone!");
  // VerifyRequested Region should be false due to the chosen size parameter!
  MITK_TEST_CONDITION_REQUIRED(!cloneObject->VerifyRequestedRegion(), "Requested Region verification!");

  // test this at the very end otherwise several other test cases need to be adapted!!
  testObject->SetRequestedRegionToLargestPossibleRegion();
  MITK_TEST_CONDITION_REQUIRED(testObject->GetRequestedRegion().GetSize() == testObject->GetLargestPossibleRegion().GetSize(), "Testing Set/Get LargestPossibleRegion!");

  // always end with this!
  MITK_TEST_END()
}

