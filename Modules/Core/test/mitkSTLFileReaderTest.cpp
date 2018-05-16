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

#include "mitkIOUtil.h"
#include "mitkImage.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSurface.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkSmartPointer.h>

#include <fstream>

class mitkSTLFileReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSTLFileReaderTestSuite);
  MITK_TEST(testReadFile);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  std::string m_SurfacePath;

public:
  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
   * members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override { m_SurfacePath = GetTestDataFilePath("ball.stl"); }
  void tearDown() override {}
  void testReadFile()
  {
    // Read STL-Image from file
    mitk::Surface::Pointer surface = mitk::IOUtil::Load<mitk::Surface>(m_SurfacePath);

    // check some basic stuff
    CPPUNIT_ASSERT_MESSAGE("Reader output not nullptr", surface.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("IsInitialized()", surface->IsInitialized());
    CPPUNIT_ASSERT_MESSAGE("mitk::Surface::SetVtkPolyData()", (surface->GetVtkPolyData() != nullptr));
    CPPUNIT_ASSERT_MESSAGE("Availability of geometry", (surface->GetGeometry() != nullptr));

    // use vtk stl reader for reference
    vtkSmartPointer<vtkSTLReader> myVtkSTLReader = vtkSmartPointer<vtkSTLReader>::New();
    myVtkSTLReader->SetFileName(m_SurfacePath.c_str());
    myVtkSTLReader->Update();
    vtkSmartPointer<vtkPolyData> myVtkPolyData = myVtkSTLReader->GetOutput();
    // vtkPolyData from vtkSTLReader directly
    int n = myVtkPolyData->GetNumberOfPoints();
    // vtkPolyData from mitkSTLFileReader
    int m = surface->GetVtkPolyData()->GetNumberOfPoints();
    CPPUNIT_ASSERT_MESSAGE("Number of Points in VtkPolyData", (n == m));
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSTLFileReader)
