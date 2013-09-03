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

#include "mitkPointSet.h"

#include "mitkTestingMacros.h"
#include "mitkFileWriterRegistry.h"

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

  mitk::FileWriterRegistry writerRegistry;

  // Get PointSet writer(s)
  std::vector<mitk::IFileWriter*> writers = writerRegistry.GetWriters(mitk::PointSet::GetStaticNameOfClass());
  MITK_TEST_CONDITION_REQUIRED(!writers.empty(), "Testing for registered writers")

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

  for (std::vector<mitk::IFileWriter*>::const_iterator iter = writers.begin(),
       end = writers.end(); iter != end; ++iter)
  {
    // test for exception handling
    try
    {
      (*iter)->Write(pointSet, "/usr/bin");
      MITK_TEST_FAILED_MSG( << "itk::ExceptionObject expected" )
    }
    catch (const itk::ExceptionObject&)
    { /* this is expected */ }
    catch(...)
    {
      //this means that a wrong exception (i.e. no itk:Exception) has been thrown
      MITK_TEST_FAILED_MSG( << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]")
    }
  }

  // always end with this!
  MITK_TEST_END()
}

