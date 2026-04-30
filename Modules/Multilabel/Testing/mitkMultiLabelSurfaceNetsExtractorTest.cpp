/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSurfaceNetsExtractor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPolyData.h>

class mitkMultiLabelSurfaceNetsExtractorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSurfaceNetsExtractorTestSuite);
  MITK_TEST(Extract_NullInput_ReturnsEmpty);
  MITK_TEST(Extract_EmptyLabels_ReturnsEmpty);
  MITK_TEST(Extract_SingleLabel_ProducesBoundaryCells);
  MITK_TEST(Extract_TwoLabels_BoundaryLabelsCarryForegroundValue);
  MITK_TEST(ExtractPerLabel_ReturnsOnePolyDataPerPresentLabel);
  MITK_TEST(ExtractPerLabel_OmitsLabelsWithNoBoundary);
  MITK_TEST(SmoothingToggle_ChangesPointPositions);
  CPPUNIT_TEST_SUITE_END();

private:
  using LabelValueType = mitk::MultiLabelSegmentation::LabelValueType;

  /** Build a 16x16x16 uint16 image with one or two cube-shaped labels. */
  static vtkSmartPointer<vtkImageData> MakeLabeledImage(bool addSecondLabel)
  {
    auto image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(16, 16, 16);
    image->SetSpacing(1.0, 1.0, 1.0);
    image->SetOrigin(0.0, 0.0, 0.0);
    image->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

    auto* scalars = static_cast<unsigned short*>(image->GetScalarPointer());
    const vtkIdType totalVoxels = 16 * 16 * 16;
    std::fill(scalars, scalars + totalVoxels, static_cast<unsigned short>(0));

    auto setVoxel = [&](int x, int y, int z, unsigned short v)
    {
      scalars[z * 16 * 16 + y * 16 + x] = v;
    };

    // Label 1: 4x4x4 cube at [4..7].
    for (int z = 4; z < 8; ++z)
      for (int y = 4; y < 8; ++y)
        for (int x = 4; x < 8; ++x)
          setVoxel(x, y, z, 1);

    if (addSecondLabel)
    {
      // Label 2: 2x2x2 cube at [10..11], well separated from label 1.
      for (int z = 10; z < 12; ++z)
        for (int y = 10; y < 12; ++y)
          for (int x = 10; x < 12; ++x)
            setVoxel(x, y, z, 2);
    }

    return image;
  }

public:
  void Extract_NullInput_ReturnsEmpty()
  {
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(nullptr, {1});
    CPPUNIT_ASSERT(result != nullptr);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), result->GetNumberOfCells());
  }

  void Extract_EmptyLabels_ReturnsEmpty()
  {
    auto image = MakeLabeledImage(false);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(image, {});
    CPPUNIT_ASSERT(result != nullptr);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), result->GetNumberOfCells());
  }

  void Extract_SingleLabel_ProducesBoundaryCells()
  {
    auto image = MakeLabeledImage(false);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(image, {1});

    CPPUNIT_ASSERT(result != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Single-label cube should yield boundary cells", result->GetNumberOfCells() > 0);

    auto* boundaryLabels = result->GetCellData()->GetArray("BoundaryLabels");
    CPPUNIT_ASSERT_MESSAGE("Output must carry the BoundaryLabels cell array", boundaryLabels != nullptr);
    CPPUNIT_ASSERT_EQUAL(2, boundaryLabels->GetNumberOfComponents());
  }

  void Extract_TwoLabels_BoundaryLabelsCarryForegroundValue()
  {
    auto image = MakeLabeledImage(true);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(image, {1, 2});

    auto* boundaryLabels = result->GetCellData()->GetArray("BoundaryLabels");
    CPPUNIT_ASSERT(boundaryLabels != nullptr);

    bool sawLabel1 = false;
    bool sawLabel2 = false;
    for (vtkIdType i = 0; i < boundaryLabels->GetNumberOfTuples(); ++i)
    {
      const auto fg = boundaryLabels->GetComponent(i, 0);
      if (fg == 1.0) sawLabel1 = true;
      else if (fg == 2.0) sawLabel2 = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Component 0 should hold each label's foreground value", sawLabel1 && sawLabel2);
  }

  void ExtractPerLabel_ReturnsOnePolyDataPerPresentLabel()
  {
    auto image = MakeLabeledImage(true);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto results = extractor.ExtractPerLabel(image, {1, 2});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), results.size());
    CPPUNIT_ASSERT(results.find(1) != results.end());
    CPPUNIT_ASSERT(results.find(2) != results.end());
    CPPUNIT_ASSERT(results[1]->GetNumberOfCells() > 0);
    CPPUNIT_ASSERT(results[2]->GetNumberOfCells() > 0);
  }

  void ExtractPerLabel_OmitsLabelsWithNoBoundary()
  {
    auto image = MakeLabeledImage(false); // only label 1
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto results = extractor.ExtractPerLabel(image, {1, 99});

    CPPUNIT_ASSERT(results.find(1) != results.end());
    CPPUNIT_ASSERT_MESSAGE("Labels with no boundary must be absent from the result map",
                           results.find(99) == results.end());
  }

  void SmoothingToggle_ChangesPointPositions()
  {
    auto image = MakeLabeledImage(false);

    mitk::MultiLabelSurfaceNetsExtractor smoothExtractor;
    smoothExtractor.SetSmoothing(true);
    auto smoothPoly = smoothExtractor.Extract(image, {1});

    mitk::MultiLabelSurfaceNetsExtractor exactExtractor;
    exactExtractor.SetSmoothing(false);
    auto exactPoly = exactExtractor.Extract(image, {1});

    CPPUNIT_ASSERT(smoothPoly->GetNumberOfPoints() > 0);
    CPPUNIT_ASSERT(exactPoly->GetNumberOfPoints() > 0);

    // Smoothing relaxes points off the voxel grid; exact output should align with it.
    // Sample a few points from the exact output and confirm at least one smoothed point
    // disagrees on a sub-voxel scale.
    bool foundDifference = false;
    const vtkIdType n = std::min(smoothPoly->GetNumberOfPoints(), exactPoly->GetNumberOfPoints());
    for (vtkIdType i = 0; i < n && !foundDifference; ++i)
    {
      double s[3], e[3];
      smoothPoly->GetPoint(i, s);
      exactPoly->GetPoint(i, e);
      const double dx = s[0] - e[0];
      const double dy = s[1] - e[1];
      const double dz = s[2] - e[2];
      if (dx * dx + dy * dy + dz * dz > 1e-6)
        foundDifference = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Smoothed output should not be point-identical to exact output",
                           foundDifference);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSurfaceNetsExtractor)
