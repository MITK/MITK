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

#include "mitkPointSet.h"
#include "mitkPointSetReader.h"
#include "mitkTestingMacros.h"

#include <iostream>
#include <time.h>

/**
 *  Test for the class "mitkPointSetFileReader".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkPointSetReaderTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointSetReader")

  // let's create an object of our class  
  mitk::PointSetReader::Pointer myPointSetReader = mitk::PointSetReader::New();
  MITK_TEST_CONDITION_REQUIRED(myPointSetReader.IsNotNull(),"Testing instantiation") 

  std::string testName = "test1";
  myPointSetReader->SetFileName( testName );
  MITK_TEST_CONDITION_REQUIRED( myPointSetReader->GetFileName()== testName, "Testing set / get file name methods!");
  MITK_TEST_CONDITION_REQUIRED( !myPointSetReader->CanReadFile(), "Testing CanReadFile() method with invalid input file name!");
  myPointSetReader->Update();
  MITK_TEST_CONDITION_REQUIRED( !myPointSetReader->GetSuccess(), "Testing GetSuccess() with invalid input file name!");

  myPointSetReader->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( myPointSetReader->CanReadFile(), "Testing CanReadFile() method with valid input file name!");
  myPointSetReader->Modified();
  myPointSetReader->Update();
  MITK_TEST_CONDITION_REQUIRED( myPointSetReader->GetSuccess(), "Testing GetSuccess() with valid input file name!");

  // always end with this!
  MITK_TEST_END()
}

