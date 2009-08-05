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

#include "mitkSurfaceVtkWriter.h"

#include "mitkTestingMacros.h"

#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>

#include <iostream>

/**
 *  Simple example for a test for the (non-existent) class "ClassName".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkSurfaceVtkWriterTest(int /*argc*/ , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("SurfaceVtkWriter")

    // let's create an object of our class  
    mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer mySurfaceVtkWriter = mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(mySurfaceVtkWriter.IsNotNull(),"Testing instantiation") 

    // create contour
    vtkPolyDataReader* reader = vtkPolyDataReader::New();
  reader->SetFileName(argv[1]);
  reader->Update();
  if (reader->GetOutput())
  {
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(reader->GetOutput());
    surface->Update();

    MITK_TEST_CONDITION_REQUIRED(surface.IsNotNull(),"Surface creation")

      try{  
        // test for exception handling
        MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
          mySurfaceVtkWriter->SetInput(surface);
        mySurfaceVtkWriter->SetFileName("/usr/bin");
        mySurfaceVtkWriter->Update(); 
        MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
    }
    catch(...) {
      //this means that a wrong exception (i.e. no itk:Exception) has been thrown 
      std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
      return EXIT_FAILURE;
    }

    // write your own tests here and use the macros from mitkTestingMacros.h !!!
    // do not write to std::cout and do not return from this function yourself!

    // always end with this!
  }
  MITK_TEST_END()

}

