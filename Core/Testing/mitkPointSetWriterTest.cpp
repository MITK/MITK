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
#include "mitkPointSetWriter.h"

#include "mitkTestingMacros.h"

#include <iostream>
#include <time.h>

/**
 *  Test for the class "mitkPointSetFileWriter".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkPointSetWriterTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointSetWriter")

  // let's create an object of our class  
  mitk::PointSetWriter::Pointer myPointSetWriter = mitk::PointSetWriter::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myPointSetWriter.IsNotNull(),"Testing instantiation") 

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!

  // create pointSet
  srand(time(NULL));
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  int numberOfPoints = rand()%100;
  for (int i=0; i<=numberOfPoints+1;i++)
  {
    mitk::Point3D point;
    point[0] = rand()%1000;
    point[1] = rand()%1000;
    point[2] = rand()%1000;
    pointSet->SetPoint(i,point);
  }

  MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(),"PointSet creation")

    try{  
      // test for exception handling
      MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      myPointSetWriter->SetInput(pointSet);
      myPointSetWriter->SetFileName("/usr/bin");
      myPointSetWriter->Update(); 
      MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }
  catch(...) {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown 
    std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  /*
  MITK_TEST_OUTPUT( << "Check if filename can be set correctly: ");
  myPointSetWriter->SetFileName("filename");
  const char * filename = myPointSetWriter->GetFileName();
  MITK_TEST_CONDITION_REQUIRED(std::string("filename") == "filename", "Filename set correctly?");

  MITK_TEST_OUTPUT( << "Check if prefix can be set correctly: ");
  myPointSetWriter->SetFilePrefix("pre");
  const char * prefix = myPointSetWriter->GetFilePrefix();
  MITK_TEST_CONDITION_REQUIRED(std::string("pre") == prefix, "Prefix set correctly?");

  MITK_TEST_OUTPUT( << "Check if pattern can be set correctly: ");
  myPointSetWriter->SetFilePattern("pattern");
  const char * pattern = myPointSetWriter->GetFilePattern();
  MITK_TEST_CONDITION_REQUIRED(std::string("pattern") == prefix, "Pattern set correctly?");
  */
  
  MITK_TEST_OUTPUT( << "Check if input can be set correctly: ");
  myPointSetWriter->SetInput(pointSet);
  mitk::PointSet::Pointer pointSet2 = mitk::PointSet::New();
  pointSet2 = myPointSetWriter->GetInput();

  MITK_TEST_CONDITION_REQUIRED( pointSet->GetSize() == pointSet2->GetSize(), "Pointsets have unequal size" ); 
  
  for(int i=0; i<pointSet->GetSize(); i++)
  {
    mitk::Point3D p1 = pointSet->GetPoint(i);
    mitk::Point3D p2 = pointSet2->GetPoint(i);
    MITK_TEST_CONDITION_REQUIRED( p1[0] == p2[0] && p1[0] == p2[0] && p1[0] == p2[0], "Pointsets aren't equal" );
  }

  std::vector< std::string > extensions = myPointSetWriter->GetPossibleFileExtensions();

  // always end with this!
  MITK_TEST_END()
}

