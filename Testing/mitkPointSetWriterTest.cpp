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
  
  // always end with this!
  MITK_TEST_END()
}

