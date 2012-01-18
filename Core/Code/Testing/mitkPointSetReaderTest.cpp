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

/**
 *  Test for the class "mitkPointSetReader".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test data set for the tests (see CMakeLists.txt).
 */
int mitkPointSetReaderTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointSetReader")

  // let's create an object of our class  
  mitk::PointSetReader::Pointer myPointSetReader = mitk::PointSetReader::New();
  MITK_TEST_CONDITION_REQUIRED(myPointSetReader.IsNotNull(),"Testing instantiation") 

  // testing set / get name with invalid data
  std::string testName = "test1";
  myPointSetReader->SetFileName( testName );
  MITK_TEST_CONDITION_REQUIRED( myPointSetReader->GetFileName()== testName, "Testing set / get file name methods!");
  
  // testing file reading with invalid data
  MITK_TEST_CONDITION_REQUIRED( !myPointSetReader->CanReadFile(testName,"",""), "Testing CanReadFile() method with invalid input file name!");
  myPointSetReader->Update();
  MITK_TEST_CONDITION_REQUIRED( !myPointSetReader->GetSuccess(), "Testing GetSuccess() with invalid input file name!");

  // testing file reading with invalid data
  myPointSetReader->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( myPointSetReader->CanReadFile(argv[1], "", ""), "Testing CanReadFile() method with valid input file name!");
  myPointSetReader->Modified();
  myPointSetReader->Update();
  MITK_TEST_CONDITION_REQUIRED( myPointSetReader->GetSuccess(), "Testing GetSuccess() with valid input file name!");
  
  // evaluate if the read point set is correct
  mitk::PointSet::Pointer resultPS = myPointSetReader->GetOutput();
  MITK_TEST_CONDITION_REQUIRED( resultPS.IsNotNull(), "Testing output generation!");
  MITK_TEST_CONDITION_REQUIRED( resultPS->GetTimeSteps() == 14, "Testing output time step generation!"); // CAVE: Only valid with the specified test data!
  MITK_TEST_CONDITION_REQUIRED( resultPS->GetPointSet(resultPS->GetTimeSteps()-1)->GetNumberOfPoints() == 0, "Testing output time step generation with empty time step!"); // CAVE: Only valid with the specified test data!

  // always end with this!
  MITK_TEST_END()
}

