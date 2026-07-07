/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkMultiLabelPredicateHelper.h>

#include <mitkImageGenerator.h>
#include <mitkLabelSetImage.h>
#include <mitkStandaloneDataStorage.h>

class mitkMultiLabelPredicateHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelPredicateHelperTestSuite);

  MITK_TEST(GetGeometryMismatchedSegmentationCount_CountsOnlyMismatchedSegmentations);
  MITK_TEST(GetGeometryMismatchedSegmentationCount_NullArguments_ReturnZero);
  MITK_TEST(GetGeometryMismatchedSegmentationCount_NoMismatch_ReturnsZero);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_ReferenceImage;
  mitk::StandaloneDataStorage::Pointer m_DataStorage;

  static mitk::DataNode::Pointer CreateSegmentationNode(const mitk::Image* geometrySource)
  {
    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(geometrySource);

    auto node = mitk::DataNode::New();
    node->SetData(segmentation);
    return node;
  }

  static mitk::DataNode::Pointer CreateImageNode(mitk::Image* image)
  {
    auto node = mitk::DataNode::New();
    node->SetData(image);
    return node;
  }

public:
  void setUp() override
  {
    // Reference and a fitting image share the same voxel grid; the mismatched
    // image differs in spacing, which fails the sub-geometry check.
    m_ReferenceImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(10, 10, 10, 1, 1.0, 1.0, 1.0);
    auto fittingImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(10, 10, 10, 1, 1.0, 1.0, 1.0);
    auto mismatchedImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(10, 10, 10, 1, 2.0, 2.0, 2.0);

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_DataStorage->Add(CreateImageNode(m_ReferenceImage));          // plain image, not a segmentation
    m_DataStorage->Add(CreateImageNode(mismatchedImage));           // plain image with a differing geometry
    m_DataStorage->Add(CreateSegmentationNode(fittingImage));       // segmentation that fits the reference
    m_DataStorage->Add(CreateSegmentationNode(mismatchedImage));    // segmentation that does not fit
  }

  void tearDown() override
  {
    m_ReferenceImage = nullptr;
    m_DataStorage = nullptr;
  }

  void GetGeometryMismatchedSegmentationCount_CountsOnlyMismatchedSegmentations()
  {
    // Only the segmentation with differing spacing must be counted; the fitting
    // segmentation and both plain images must be ignored.
    CPPUNIT_ASSERT_EQUAL(1u,
      mitk::GetGeometryMismatchedSegmentationCount(m_DataStorage, m_ReferenceImage->GetGeometry()));
  }

  void GetGeometryMismatchedSegmentationCount_NullArguments_ReturnZero()
  {
    CPPUNIT_ASSERT_EQUAL(0u,
      mitk::GetGeometryMismatchedSegmentationCount(m_DataStorage, nullptr));
    CPPUNIT_ASSERT_EQUAL(0u,
      mitk::GetGeometryMismatchedSegmentationCount(nullptr, m_ReferenceImage->GetGeometry()));
  }

  void GetGeometryMismatchedSegmentationCount_NoMismatch_ReturnsZero()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    auto fittingImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(10, 10, 10, 1, 1.0, 1.0, 1.0);
    storage->Add(CreateImageNode(m_ReferenceImage));
    storage->Add(CreateSegmentationNode(fittingImage));

    CPPUNIT_ASSERT_EQUAL(0u,
      mitk::GetGeometryMismatchedSegmentationCount(storage, m_ReferenceImage->GetGeometry()));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelPredicateHelper)
