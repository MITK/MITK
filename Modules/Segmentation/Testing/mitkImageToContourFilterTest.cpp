/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageToContourFilter.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkImageToContourFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageToContourFilterTestSuite);
  MITK_TEST(TestExtractContoursFromAnEmptySlice);
  MITK_TEST(TestExtractASingleContourFromASlice);
  MITK_TEST(TestExtractTwoContoursFromASingleSlice);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_EmptySlice;
  mitk::Image::Pointer m_SliceWithSingleContour;
  mitk::Image::Pointer m_SliceWithTwoContours;
  mitk::ImageToContourFilter::Pointer m_ContourExtractor;

public:
  void setUp() override
  {
    // Load the image
    // TODO Move/create segmentation subfolder
    m_EmptySlice = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("SurfaceInterpolation/ImageToContour/EmptySlice.nrrd"));
    CPPUNIT_ASSERT_MESSAGE("Failed to load image for test: [EmptySlice.nrrd]", m_EmptySlice.IsNotNull());

    m_SliceWithSingleContour =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("SurfaceInterpolation/ImageToContour/SliceWithSingleContour.nrrd"));
    CPPUNIT_ASSERT_MESSAGE("Failed to load image for test: [SliceWithSingleContour.nrrd]",
                           m_SliceWithSingleContour.IsNotNull());

    m_SliceWithTwoContours =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("SurfaceInterpolation/ImageToContour/SliceWithTwoContours.nrrd"));
    CPPUNIT_ASSERT_MESSAGE("Failed to load image for test: [SliceWithTwoContours.nrrd]",
                           m_SliceWithTwoContours.IsNotNull());

    m_ContourExtractor = mitk::ImageToContourFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize ImageToContourFilter", m_ContourExtractor.IsNotNull());
  }

  // Extract contours from an empty slice
  void TestExtractContoursFromAnEmptySlice()
  {
    m_ContourExtractor->SetInput(m_EmptySlice);
    m_ContourExtractor->Update();
    mitk::Surface::Pointer emptyContour = m_ContourExtractor->GetOutput();

    CPPUNIT_ASSERT_MESSAGE("Extracted contour is not empty", emptyContour->GetVtkPolyData()->GetNumberOfPoints() == 0);
  }

  // Extract a single contour from a slice
  void TestExtractASingleContourFromASlice()
  {
    m_ContourExtractor->SetInput(m_SliceWithSingleContour);
    m_ContourExtractor->Update();

    CPPUNIT_ASSERT_MESSAGE("ImageToContourFilter has wrong number of outputs!",
                           m_ContourExtractor->GetNumberOfOutputs() == 1);

    mitk::Surface::Pointer contour = m_ContourExtractor->GetOutput();

    mitk::Surface::Pointer referenceContour =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/SingleContour.vtk"));

    CPPUNIT_ASSERT_MESSAGE(
      "Extracted contour has wrong number of points!",
      contour->GetVtkPolyData()->GetNumberOfPoints() == referenceContour->GetVtkPolyData()->GetNumberOfPoints());

    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(contour->GetVtkPolyData()), *(referenceContour->GetVtkPolyData()), 0.000001, true));
  }

  // Extract multiple contours from a single slice
  void TestExtractTwoContoursFromASingleSlice()
  {
    m_ContourExtractor->SetInput(m_SliceWithTwoContours);
    m_ContourExtractor->Update();

    CPPUNIT_ASSERT_MESSAGE("ImageToContourFilter has wrong number of outputs!",
                           m_ContourExtractor->GetNumberOfOutputs() == 1);

    mitk::Surface::Pointer contour = m_ContourExtractor->GetOutput(0);

    mitk::Surface::Pointer referenceContour =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/TwoContours.vtk"));

    CPPUNIT_ASSERT_MESSAGE(
      "Extracted contour1 has wrong number of points!",
      contour->GetVtkPolyData()->GetNumberOfPoints() == referenceContour->GetVtkPolyData()->GetNumberOfPoints());

    CPPUNIT_ASSERT_MESSAGE(
      "Extracted contour1 has wrong number of points!",
      contour->GetVtkPolyData()->GetNumberOfPolys() == referenceContour->GetVtkPolyData()->GetNumberOfPolys());

    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(contour->GetVtkPolyData()), *(referenceContour->GetVtkPolyData()), 0.000001, true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageToContourFilter)
