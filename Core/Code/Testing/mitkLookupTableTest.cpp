/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Tue, 12 May 2009) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkLookupTable.h>

#include "mitkTestingMacros.h"

#include <iostream>

/**
 *  Simple example for a test for the (non-existent) class "ClassName".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkLookupTableTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("LookupTable")

  // let's create an object of our class  
  mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();

  
  // create a vtkLookupTable and add two values
  vtkLookupTable *lut = vtkLookupTable::New();
  lut->SetTableValue(0, 0.5, 0.5, 0.5, 1.0);
  lut->SetTableValue(0, 0.5, 0.5, 0.5, 0.5);


  myLookupTable->SetVtkLookupTable(lut);

  


  // check if the same lookuptable is returned
  vtkLookupTable *lut2 = myLookupTable->GetVtkLookupTable();

  MITK_TEST_CONDITION_REQUIRED(lut == lut2,"Input and output table are not equal")
  
  myLookupTable->ChangeOpacityForAll(0.5);

  myLookupTable->ChangeOpacityForAll(0.5);
  
  myLookupTable->ChangeOpacity(0, 1.0);  

  

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myLookupTable.IsNotNull(),"Testing instantiation") 

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  



  lut->Delete();
  // always end with this!
  MITK_TEST_END()
}

