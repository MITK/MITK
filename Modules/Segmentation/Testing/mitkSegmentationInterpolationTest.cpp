/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

// other
#include <mitkExtractSliceFilter.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkSegmentationInterpolationController.h>
#include <mitkSliceNavigationController.h>
#include <mitkTool.h>
#include <mitkVtkImageOverwrite.h>

#include <algorithm>
#include <vector>

class mitkSegmentationInterpolationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSegmentationInterpolationTestSuite);
  MITK_TEST(Equal_Axial_TestInterpolationAndReferenceInterpolation_ReturnsTrue);
  MITK_TEST(Equal_Coronal_TestInterpolationAndReferenceInterpolation_ReturnsTrue);
  MITK_TEST(Equal_Sagittal_TestInterpolationAndReferenceInterpolation_ReturnsTrue);
  MITK_TEST(InterpolateAll_Axial_WritesInterpolatedSquare);
  MITK_TEST(InterpolateAll_Coronal_WritesInterpolatedSquare);
  MITK_TEST(InterpolateAll_Sagittal_WritesInterpolatedSquare);
  MITK_TEST(Interpolate_SliceSegmentedAfterwards_ReturnsNull);
  MITK_TEST(Interpolate_OtherLabels_AreIgnored);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::PlaneGeometry::ConstPointer GetCenterPlane(mitk::AnatomicalPlane viewDirection) const
  {
    auto navigationController = mitk::SliceNavigationController::New();
    navigationController->SetInputWorldTimeGeometry(m_SegmentationImage->GetTimeGeometry());
    navigationController->Update(viewDirection);
    mitk::Point3D pointMM;
    m_SegmentationImage->GetTimeGeometry()->GetGeometryForTimeStep(0)->IndexToWorld(m_CenterPoint, pointMM);
    navigationController->SelectSliceByPoint(pointMM);
    return navigationController->GetCurrentPlaneGeometry();
  }

  void SetPixels(const std::vector<itk::Index<3>>& indices, mitk::Label::PixelType value)
  {
    mitk::ImagePixelWriteAccessor<mitk::Label::PixelType, 3> writeAccessor(m_SegmentationImage);

    for (const auto& index : indices)
      writeAccessor.SetPixelByIndexSafe(index, value);
  }

  static int SliceDimension(mitk::AnatomicalPlane viewDirection)
  {
    switch (viewDirection)
    {
      case mitk::AnatomicalPlane::Axial:
        return 2;
      case mitk::AnatomicalPlane::Coronal:
        return 1;
      case mitk::AnatomicalPlane::Sagittal:
        return 0;
      default:
        return -1;
    }
  }

  /** Index of the center point moved by i and j along the two axes in the slices perpendicular to dim, and by k along dim. */
  itk::Index<3> CenterOffset(int dim, itk::IndexValueType i, itk::IndexValueType j, itk::IndexValueType k) const
  {
    auto index = m_CenterPoint;
    index[(dim + 1) % 3] += i;
    index[(dim + 2) % 3] += j;
    index[dim] += k;
    return index;
  }

  /**
   * Fills a 3x3 square in the slice below the center slice along dim and a single pixel above one of its corners in
   * the slice above, so that the center slice interpolates to a 2x2 square in that corner.
   */
  void SetEnclosingSlices(int dim, mitk::Label::PixelType value)
  {
    std::vector<itk::Index<3>> indices;

    for (itk::IndexValueType i = -1; i <= 1; ++i)
    {
      for (itk::IndexValueType j = -1; j <= 1; ++j)
        indices.push_back(this->CenterOffset(dim, i, j, -1));
    }

    indices.push_back(this->CenterOffset(dim, 1, 1, 1));
    this->SetPixels(indices, value);
  }

  /** Checks that the 2x2 square interpolated by SetEnclosingSlices() is in the center slice, and nothing around it. */
  void CheckInterpolatedSquare(int dim) const
  {
    mitk::ImagePixelReadAccessor<mitk::Label::PixelType, 3> readAccess(m_SegmentationImage);

    for (itk::IndexValueType i = -1; i <= 2; ++i)
    {
      for (itk::IndexValueType j = -1; j <= 2; ++j)
      {
        const auto index = this->CenterOffset(dim, i, j, 0);

        if (i == -1 || i == 2 || j == -1 || j == 2)
        {
          CPPUNIT_ASSERT_MESSAGE("Have false positive segmentation.", readAccess.GetPixelByIndexSafe(index) == 0);
        }
        else
        {
          CPPUNIT_ASSERT_MESSAGE("Have false negative segmentation.", readAccess.GetPixelByIndexSafe(index) == 1);
        }
      }
    }
  }

  static std::size_t CountPixels(const mitk::Image* slice, mitk::Label::PixelType value)
  {
    mitk::ImagePixelReadAccessor<mitk::Label::PixelType, 2> readAccessor(slice);
    const auto* pixels = readAccessor.GetData();
    const std::size_t numberOfPixels = std::size_t{slice->GetDimension(0)} * slice->GetDimension(1);
    return static_cast<std::size_t>(std::count(pixels, pixels + numberOfPixels, value));
  }

  // The tests all do the same, only in different directions
  void testRoutine(mitk::AnatomicalPlane viewDirection)
  {
    const auto dim = SliceDimension(viewDirection);
    this->SetEnclosingSlices(dim, 1);

    m_InterpolationController->SetSegmentationVolume(m_SegmentationImage, 1);

    const auto plane = this->GetCenterPlane(viewDirection);
    mitk::Image::Pointer interpolationResult =
      m_InterpolationController->Interpolate(dim, m_CenterPoint[dim], plane, 0);

    //        mitk::IOUtil::Save(interpolationResult, "SOME PATH");

    // Write result into segmentation image
    vtkSmartPointer<mitkVtkImageOverwrite> reslicer = vtkSmartPointer<mitkVtkImageOverwrite>::New();
    reslicer->SetInputSlice(
      interpolationResult->GetSliceData()->GetVtkImageAccessor(interpolationResult)->GetVtkImageData());
    reslicer->SetOverwriteMode(true);
    reslicer->Modified();
    mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslicer);
    extractor->SetInput(m_SegmentationImage);
    extractor->SetTimeStep(0);
    extractor->SetWorldGeometry(plane);
    extractor->SetVtkOutputRequest(true);
    extractor->SetResliceTransformByGeometry(m_SegmentationImage->GetTimeGeometry()->GetGeometryForTimeStep(0));
    extractor->Modified();
    extractor->Update();

    this->CheckInterpolatedSquare(dim);
  }

  /** Accepts all interpolations along the view direction by writing each result into the segmentation. */
  void interpolateAllRoutine(mitk::AnatomicalPlane viewDirection)
  {
    const auto dim = SliceDimension(viewDirection);
    this->SetEnclosingSlices(dim, 1);

    m_InterpolationController->SetSegmentationVolume(m_SegmentationImage, 1);

    const mitk::ConstLabelVector labels = {mitk::Label::New(1, "Label").GetPointer()};
    unsigned int numberOfResults = 0;

    m_InterpolationController->InterpolateAll(dim, this->GetCenterPlane(viewDirection), 0,
      [&](unsigned int sliceIndex, const mitk::Image* interpolation)
      {
        ++numberOfResults;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Interpolated another slice.", static_cast<unsigned int>(m_CenterPoint[dim]), sliceIndex);

        mitk::TransferSliceContentAtTimeStep(interpolation, m_SegmentationImage, labels, 0, 1, 1,
          mitk::Label::UNLABELED_VALUE, false, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
      });

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not exactly the center slice was interpolated.", 1u, numberOfResults);
    this->CheckInterpolatedSquare(dim);
  }

  mitk::Image::Pointer m_ReferenceImage;
  mitk::Image::Pointer m_SegmentationImage;
  itk::Index<3> m_CenterPoint;
  mitk::SegmentationInterpolationController::Pointer m_InterpolationController;

public:
  void setUp() override
  {
    m_ReferenceImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
    CPPUNIT_ASSERT_MESSAGE("Failed to load image for test: [Pic3D.nrrd]", m_ReferenceImage.IsNotNull());

    m_InterpolationController = mitk::SegmentationInterpolationController::New();

    // Create empty segmentation
    // Surely there must be a better way to get an image with all zeros?
    m_SegmentationImage = mitk::Image::New();
    const mitk::PixelType pixelType(mitk::MakeScalarPixelType<mitk::Tool::DefaultSegmentationDataType>());
    m_SegmentationImage->Initialize(pixelType, m_ReferenceImage->GetDimension(), m_ReferenceImage->GetDimensions());
    m_SegmentationImage->SetClonedTimeGeometry(m_ReferenceImage->GetTimeGeometry());
    unsigned int size = sizeof(mitk::Tool::DefaultSegmentationDataType);
    for (unsigned int dim = 0; dim < m_SegmentationImage->GetDimension(); ++dim)
    {
      size *= m_SegmentationImage->GetDimension(dim);
    }
    mitk::ImageWriteAccessor imageAccessor(m_SegmentationImage);
    memset(imageAccessor.GetData(), 0, size);

    // Work in the center of the image (Pic3D)
    m_CenterPoint = {{127, 127, 25}};
  }

  void tearDown() override
  {
    m_ReferenceImage = nullptr;
    m_SegmentationImage = nullptr;
    m_CenterPoint = {{0, 0, 0}};
  }

  void Equal_Axial_TestInterpolationAndReferenceInterpolation_ReturnsTrue()
  {
    mitk::AnatomicalPlane viewDirection = mitk::AnatomicalPlane::Axial;
    testRoutine(viewDirection);
  }

  void Equal_Coronal_TestInterpolationAndReferenceInterpolation_ReturnsTrue() // Coronal
  {
    mitk::AnatomicalPlane viewDirection = mitk::AnatomicalPlane::Coronal;
    testRoutine(viewDirection);
  }

  void Equal_Sagittal_TestInterpolationAndReferenceInterpolation_ReturnsTrue()
  {
    mitk::AnatomicalPlane viewDirection = mitk::AnatomicalPlane::Sagittal;
    testRoutine(viewDirection);
  }

  void InterpolateAll_Axial_WritesInterpolatedSquare()
  {
    interpolateAllRoutine(mitk::AnatomicalPlane::Axial);
  }

  void InterpolateAll_Coronal_WritesInterpolatedSquare()
  {
    interpolateAllRoutine(mitk::AnatomicalPlane::Coronal);
  }

  void InterpolateAll_Sagittal_WritesInterpolatedSquare()
  {
    interpolateAllRoutine(mitk::AnatomicalPlane::Sagittal);
  }

  void Interpolate_SliceSegmentedAfterwards_ReturnsNull()
  {
    this->SetEnclosingSlices(2, 1);
    m_InterpolationController->SetSegmentationVolume(m_SegmentationImage, 1);

    const auto plane = this->GetCenterPlane(mitk::AnatomicalPlane::Axial);
    CPPUNIT_ASSERT(m_InterpolationController->Interpolate(2, m_CenterPoint[2], plane, 0).IsNotNull());

    this->SetPixels({m_CenterPoint}, 1);
    m_SegmentationImage->Modified();

    CPPUNIT_ASSERT_MESSAGE("Interpolated a slice that contains the label.",
                           m_InterpolationController->Interpolate(2, m_CenterPoint[2], plane, 0).IsNull());
  }

  void Interpolate_OtherLabels_AreIgnored()
  {
    const mitk::Label::PixelType label = 2;
    const mitk::Label::PixelType otherLabel = 3;

    // Above and below each other next to the shapes, so that they would be interpolated if they
    // counted, and in the center slice, which would then not be interpolated at all.
    this->SetEnclosingSlices(2, label);
    this->SetPixels({this->CenterOffset(2, 3, -1, -1), this->CenterOffset(2, 3, -1, 1), this->CenterOffset(2, -5, -5, 0)}, otherLabel);
    m_InterpolationController->SetSegmentationVolume(m_SegmentationImage, label);

    const auto interpolation = m_InterpolationController->Interpolate(
      2, m_CenterPoint[2], this->GetCenterPlane(mitk::AnatomicalPlane::Axial), 0);

    CPPUNIT_ASSERT_MESSAGE("The other label prevented the interpolation.", interpolation.IsNotNull());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The other label was interpolated.", std::size_t{4}, CountPixels(interpolation, 1));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSegmentationInterpolation)
