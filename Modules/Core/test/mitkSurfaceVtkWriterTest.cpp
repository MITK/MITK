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

#include "mitkTestingMacros.h"
#include "mitkIOUtil.h"
#include "mitkSurface.h"

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

    try
    {
      // test for exception handling
      mitk::IOUtil::Save(surface, "/usr/bin");
      MITK_TEST_FAILED_MSG( << "itk::ExceptionObject expected" )
    }
    catch (const mitk::Exception&) { /* this is expected */ }
    catch(...)
    {
      //this means that a wrong exception (i.e. no itk:Exception) has been thrown
      MITK_TEST_FAILED_MSG( << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" )
    }

    // write your own tests here and use the macros from mitkTestingMacros.h !!!
    // do not write to std::cout and do not return from this function yourself!
  }

  //Delete reader correctly
  reader->Delete();

  // always end with this!
  MITK_TEST_END()
}

