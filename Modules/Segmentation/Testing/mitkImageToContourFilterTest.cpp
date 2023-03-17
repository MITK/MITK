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
  MITK_TEST(TestExtractContoursFromDifferentPixelValues);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::ImageToContourFilter::Pointer m_ContourExtractor;

  mitk::Image::Pointer LoadTestImage(const std::string& filename)
  {
    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath(filename));

    CPPUNIT_ASSERT_MESSAGE(
      "Failed to load image for test: [" + filename + "]",
      image.IsNotNull());

    return image;
  }

  void CompareContourToReference(const mitk::Surface* contour, const std::string& referenceFilename)
  {
    CPPUNIT_ASSERT_MESSAGE(
      "No contour has been extracted",
      contour != nullptr);

    auto referenceContour = mitk::IOUtil::Load<mitk::Surface>(this->GetTestDataFilePath(referenceFilename));

    CPPUNIT_ASSERT_MESSAGE(
      "Extracted contour has wrong number of points",
      contour->GetVtkPolyData()->GetNumberOfPoints() == referenceContour->GetVtkPolyData()->GetNumberOfPoints());

    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(contour->GetVtkPolyData()), *(referenceContour->GetVtkPolyData()), 0.000001, true));
  }

public:
  void setUp() override
  {
    m_ContourExtractor = mitk::ImageToContourFilter::New();

    CPPUNIT_ASSERT_MESSAGE(
      "Failed to initialize ImageToContourFilter",
      m_ContourExtractor.IsNotNull());
  }

  // Extract multiple contours from an image containing different pixel values
  void TestExtractContoursFromDifferentPixelValues()
  {
    auto labelsImage = this->LoadTestImage("SurfaceInterpolation/ImageToContour/Labels.nrrd");

    m_ContourExtractor->SetInput(labelsImage);
    m_ContourExtractor->Update();

    mitk::Surface::Pointer contour = m_ContourExtractor->GetOutput();

    this->CompareContourToReference(contour, "SurfaceInterpolation/Reference/Contour_Label1.vtk");

    m_ContourExtractor->SetContourValue(2.0);
    m_ContourExtractor->Update();

    contour = m_ContourExtractor->GetOutput();

    this->CompareContourToReference(contour, "SurfaceInterpolation/Reference/Contour_Label2.vtk");

    m_ContourExtractor->SetContourValue(3.0);
    m_ContourExtractor->Update();

    contour = m_ContourExtractor->GetOutput();

    this->CompareContourToReference(contour, "SurfaceInterpolation/Reference/Contour_Label3.vtk");

    m_ContourExtractor->SetContourValue(4.0);
    m_ContourExtractor->Update();

    contour = m_ContourExtractor->GetOutput();

    CPPUNIT_ASSERT_MESSAGE(
      "Extracted contour is not empty",
      contour->GetVtkPolyData()->GetNumberOfPoints() == 0);

    m_ContourExtractor->SetContourValue(5.0);
    m_ContourExtractor->Update();

    contour = m_ContourExtractor->GetOutput();

    this->CompareContourToReference(contour, "SurfaceInterpolation/Reference/Contour_Label5.vtk");
  }

  // Extract contours from an empty slice
  void TestExtractContoursFromAnEmptySlice()
  {
    auto emptySlice = this->LoadTestImage("SurfaceInterpolation/ImageToContour/EmptySlice.nrrd");

    m_ContourExtractor->SetInput(emptySlice);
    m_ContourExtractor->Update();
    mitk::Surface::Pointer emptyContour = m_ContourExtractor->GetOutput();

    CPPUNIT_ASSERT_MESSAGE(
      "Extracted contour is not empty",
      emptyContour->GetVtkPolyData()->GetNumberOfPoints() == 0);
  }

  // Extract a single contour from a slice
  void TestExtractASingleContourFromASlice()
  {
    auto sliceWithSingleContour = this->LoadTestImage("SurfaceInterpolation/ImageToContour/SliceWithSingleContour.nrrd");

    m_ContourExtractor->SetInput(sliceWithSingleContour);
    m_ContourExtractor->Update();

    CPPUNIT_ASSERT_MESSAGE(
      "ImageToContourFilter has wrong number of outputs",
      m_ContourExtractor->GetNumberOfOutputs() == 1);

    mitk::Surface::Pointer contour = m_ContourExtractor->GetOutput();

    this->CompareContourToReference(contour, "SurfaceInterpolation/Reference/SingleContour.vtk");
  }

  // Extract multiple contours from a single slice
  void TestExtractTwoContoursFromASingleSlice()
  {
    auto sliceWithTwoContours = LoadTestImage("SurfaceInterpolation/ImageToContour/SliceWithTwoContours.nrrd");

    m_ContourExtractor->SetInput(sliceWithTwoContours);
    m_ContourExtractor->Update();

    CPPUNIT_ASSERT_MESSAGE(
      "ImageToContourFilter has wrong number of outputs",
      m_ContourExtractor->GetNumberOfOutputs() == 1);

    mitk::Surface::Pointer contour = m_ContourExtractor->GetOutput(0);

    this->CompareContourToReference(contour, "SurfaceInterpolation/Reference/TwoContours.vtk");

    mitk::Surface::Pointer referenceContour =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/TwoContours.vtk"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageToContourFilter)
