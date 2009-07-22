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

#include "mitkUnstructuredGridVtkWriter.h"

#include "mitkTestingMacros.h"
#include "mitkUnstructuredGrid.h"

#include <mitkVtkUnstructuredGridReader.h>
#include <vtkUnstructuredGridWriter.h>

#include <iostream>

/**
 *  Simple example for a test for the (non-existent) class "ClassName".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkUnstructuredGridVtkWriterTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("UnstructuredGridVtkWriter")

    //let's create an object of our class  
    mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::Pointer myUnstructuredGridVtkWriter = mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myUnstructuredGridVtkWriter.IsNotNull(),"Testing instantiation") 

  if (argc<1)
  {
    LOG_INFO<<"Command line argument missing";
    return;
  }
  // create contour by reading the file given in argv[1]
  mitk::UnstructuredGrid::Pointer unstructuredGrid;
  mitk::VtkUnstructuredGridReader::Pointer reader = mitk::VtkUnstructuredGridReader::New();
  
  try {
    reader->SetFileName(argv[1]);
    reader->Update();
    unstructuredGrid = reader->GetOutput();
  }
  catch (itk::ExceptionObject e)
  {
    LOG_INFO<<e.GetDescription();
    return;
  }

    try{  
      // test if exception is thrown when unstructured grid is tried to be saved in an non-existing/non-writable folder/file
      MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
        myUnstructuredGridVtkWriter->SetInput(unstructuredGrid);
      // set non-existing (windows), non-writable (linux) file name
      myUnstructuredGridVtkWriter->SetFileName("/usr/bin");
      myUnstructuredGridVtkWriter->Update(); 
      MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }
  catch(...) {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown 
    LOG_INFO << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  //write your own tests here and use the macros from mitkTestingMacros.h !!!
  //do not write to std::cout and do not return from this function yourself!

  //always end with this!
  MITK_TEST_END()
}

