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

#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkComputeContourSetNormalsFilter.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkExtractSliceFilter.h>
#include <mitkPlaneGeometry.h>

#include <vtkDebugLeaks.h>

class mitkComputeContourSetNormalsFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkComputeContourSetNormalsFilterTestSuite);

  /// \todo Fix leaks of vtkObjects. Bug 18096.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(TestComputeNormals);
  MITK_TEST(TestComputeNormalsWithHole);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ComputeContourSetNormalsFilter::Pointer m_ContourNormalsFilter;

public:

  void setUp()
  {
    m_ContourNormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize ReduceContourSetFilter", m_ContourNormalsFilter.IsNotNull());
  }

  // Compute the normals for a regular contour
  void TestComputeNormals()
  {
    mitk::Surface::Pointer contour = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/SingleContour.vtk"));
    m_ContourNormalsFilter->SetInput(contour);
    m_ContourNormalsFilter->Update();
    // Get the computed normals (basically lines)
    mitk::Surface::Pointer normals = m_ContourNormalsFilter->GetNormalsAsSurface();
    // Get the actual surface object which has the contours stored as normals internally
    mitk::Surface::Pointer contourWithNormals = m_ContourNormalsFilter->GetOutput();

    mitk::Surface::Pointer referenceContour = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/ContourWithNormals.vtk"));
    mitk::Surface::Pointer referenceNormals = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/ContourNormals.vtk"));

    CPPUNIT_ASSERT_MESSAGE("Unequal contours", mitk::Equal(*(contourWithNormals->GetVtkPolyData()), *(referenceContour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Unequal contours", mitk::Equal(*(normals->GetVtkPolyData()), *(referenceNormals->GetVtkPolyData()), 0.000001, true));
  }



  // Reduce contours with Douglas Peucker
  void TestComputeNormalsWithHole()
  {
    mitk::Image::Pointer segmentationImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("SurfaceInterpolation/Reference/LiverSegmentation.nrrd"));

    mitk::Surface::Pointer contour = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/ComputeNormals/ContourWithHoles.vtk"));
    m_ContourNormalsFilter->SetInput(contour);
    m_ContourNormalsFilter->SetSegmentationBinaryImage(segmentationImage);
    m_ContourNormalsFilter->Update();
    mitk::Surface::Pointer contourWithNormals = m_ContourNormalsFilter->GetOutput();
    mitk::Surface::Pointer normals = m_ContourNormalsFilter->GetNormalsAsSurface();

    mitk::Surface::Pointer contourReference = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/ContourWithHolesWithNormals.vtk"));
    mitk::Surface::Pointer normalsReference = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/NormalsWithHoles.vtk"));

    CPPUNIT_ASSERT_MESSAGE("Error computing normals", mitk::Equal(*(normals->GetVtkPolyData()), *(normalsReference->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Error computing normals",
                           contourWithNormals->GetVtkPolyData()->GetCellData()->GetNormals()->GetNumberOfTuples() == contourReference->GetVtkPolyData()->GetNumberOfPoints());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkComputeContourSetNormalsFilter)
