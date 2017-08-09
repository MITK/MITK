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

#include <mitkComputeContourSetNormalsFilter.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
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
  void setUp() override
  {
    m_ContourNormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize ReduceContourSetFilter", m_ContourNormalsFilter.IsNotNull());
  }

  // Compute the normals for a regular contour
  void TestComputeNormals()
  {
    mitk::Surface::Pointer contour =
      dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/SingleContour.vtk"))[0].GetPointer());
    m_ContourNormalsFilter->SetInput(contour);
    m_ContourNormalsFilter->Update();
    // Get the computed normals (basically lines)
    mitk::Surface::Pointer normals = m_ContourNormalsFilter->GetNormalsAsSurface();
    // Get the actual surface object which has the contours stored as normals internally
    mitk::Surface::Pointer contourWithNormals = m_ContourNormalsFilter->GetOutput();

    mitk::Surface::Pointer referenceContour =
      dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/ContourWithNormals.vtk"))[0].GetPointer());
    mitk::Surface::Pointer referenceNormals =
      dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/ContourNormals.vtk"))[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(contourWithNormals->GetVtkPolyData()), *(referenceContour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(normals->GetVtkPolyData()), *(referenceNormals->GetVtkPolyData()), 0.000001, true));
  }

  // Reduce contours with Douglas Peucker
  void TestComputeNormalsWithHole()
  {
    mitk::Image::Pointer segmentationImage =
      dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/LiverSegmentation.nrrd"))[0].GetPointer());

    mitk::Surface::Pointer contour =
      dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/ComputeNormals/ContourWithHoles.vtk"))[0].GetPointer());
    m_ContourNormalsFilter->SetInput(contour);
    m_ContourNormalsFilter->SetSegmentationBinaryImage(segmentationImage);
    m_ContourNormalsFilter->Update();
    mitk::Surface::Pointer contourWithNormals = m_ContourNormalsFilter->GetOutput();
    mitk::Surface::Pointer normals = m_ContourNormalsFilter->GetNormalsAsSurface();

    mitk::Surface::Pointer contourReference =
      dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/ContourWithHolesWithNormals.vtk"))[0].GetPointer());
    mitk::Surface::Pointer normalsReference =
      dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/NormalsWithHoles.vtk"))[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE(
      "Error computing normals",
      mitk::Equal(*(normals->GetVtkPolyData()), *(normalsReference->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Error computing normals",
                           contourWithNormals->GetVtkPolyData()->GetCellData()->GetNormals()->GetNumberOfTuples() ==
                             contourReference->GetVtkPolyData()->GetNumberOfPoints());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkComputeContourSetNormalsFilter)
