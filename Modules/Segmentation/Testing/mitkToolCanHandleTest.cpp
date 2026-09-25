/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAddContourTool.h>
#include <mitkBinaryThresholdTool.h>
#include <mitkGrowCutTool.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkOtsuTool3D.h>
#include <mitkPickingTool.h>
#include <mitkRegionGrowingTool.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkRGBPixel.h>
#include <itkVectorImage.h>

class mitkToolCanHandleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkToolCanHandleTestSuite);
  MITK_TEST(SegTool2DRequiresImageReference);
  MITK_TEST(SegTool2DRequiresSegmentationWithLabels);
  MITK_TEST(SegTool2DAcceptsAnyImageByDefault);
  MITK_TEST(SegTool2DOptInRequiresScalarReferenceSlice);
  MITK_TEST(SegWithPreviewToolRequiresLabelsByDefault);
  MITK_TEST(SegWithPreviewToolOptOutRequiresGroupOnly);
  MITK_TEST(SegWithPreviewToolRequiresScalarReferenceByDefault);
  MITK_TEST(SegWithPreviewToolOptOutAcceptsAnyPixelType);
  MITK_TEST(SegWithPreviewToolAcceptsAnyDimensionByDefault);
  MITK_TEST(SegWithPreviewToolOptInRequiresVolumetricReference);
  CPPUNIT_TEST_SUITE_END();

  mitk::Image::Pointer m_ReferenceImage;
  mitk::Image::Pointer m_RGBImage;
  mitk::Image::Pointer m_VectorImage;
  mitk::Image::Pointer m_Image2D;
  mitk::Image::Pointer m_Image2DPlusT;
  mitk::Surface::Pointer m_Surface;
  mitk::MultiLabelSegmentation::Pointer m_NoGroups;
  mitk::MultiLabelSegmentation::Pointer m_EmptyGroup;
  mitk::MultiLabelSegmentation::Pointer m_WithLabel;
  mitk::MultiLabelSegmentation::Pointer m_WithLabel2D;
  mitk::MultiLabelSegmentation::Pointer m_WithLabel2DPlusT;

  static mitk::MultiLabelSegmentation::Pointer CreateSegmentationWithLabel(const mitk::Image* templateImage)
  {
    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(templateImage);
    mitk::Color color;
    color.Set(1.0f, 0.0f, 0.0f);
    segmentation->AddLabel("Label", color, 0);
    return segmentation;
  }

public:
  void setUp() override
  {
    unsigned int dimensions[] = { 4, 4, 4 };

    m_ReferenceImage = mitk::Image::New();
    m_ReferenceImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    m_RGBImage = mitk::Image::New();
    m_RGBImage->Initialize(mitk::MakePixelType<itk::RGBPixel<unsigned char>, 3>(3), 3, dimensions);

    m_VectorImage = mitk::Image::New();
    m_VectorImage->Initialize(mitk::MakePixelType<itk::VectorImage<float, 3>>(6), 3, dimensions);

    unsigned int dimensions2D[] = { 4, 4 };
    m_Image2D = mitk::Image::New();
    m_Image2D->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 2, dimensions2D);

    unsigned int dimensions2DPlusT[] = { 4, 4, 1, 3 };
    m_Image2DPlusT = mitk::Image::New();
    m_Image2DPlusT->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 4, dimensions2DPlusT);

    m_Surface = mitk::Surface::New();

    m_NoGroups = mitk::MultiLabelSegmentation::New();
    m_NoGroups->Initialize(m_ReferenceImage, true, false);

    m_EmptyGroup = mitk::MultiLabelSegmentation::New();
    m_EmptyGroup->Initialize(m_ReferenceImage);

    m_WithLabel = CreateSegmentationWithLabel(m_ReferenceImage);
    m_WithLabel2D = CreateSegmentationWithLabel(m_Image2D);
    m_WithLabel2DPlusT = CreateSegmentationWithLabel(m_Image2DPlusT);
  }

  void tearDown() override
  {
    m_ReferenceImage = nullptr;
    m_RGBImage = nullptr;
    m_VectorImage = nullptr;
    m_Image2D = nullptr;
    m_Image2DPlusT = nullptr;
    m_Surface = nullptr;
    m_NoGroups = nullptr;
    m_EmptyGroup = nullptr;
    m_WithLabel = nullptr;
    m_WithLabel2D = nullptr;
    m_WithLabel2DPlusT = nullptr;
  }

  void SegTool2DRequiresImageReference()
  {
    auto tool = mitk::AddContourTool::New();

    CPPUNIT_ASSERT(!tool->CanHandle(nullptr, m_WithLabel));
    CPPUNIT_ASSERT(!tool->CanHandle(m_Surface, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegTool2DRequiresSegmentationWithLabels()
  {
    auto tool = mitk::AddContourTool::New();

    CPPUNIT_ASSERT(!tool->CanHandle(m_ReferenceImage, nullptr));
    CPPUNIT_ASSERT(!tool->CanHandle(m_ReferenceImage, m_ReferenceImage));
    CPPUNIT_ASSERT(!tool->CanHandle(m_ReferenceImage, m_EmptyGroup));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegTool2DAcceptsAnyImageByDefault()
  {
    // Only modifies the segmentation slice, the reference pixel type is irrelevant.
    auto tool = mitk::AddContourTool::New();

    CPPUNIT_ASSERT(tool->CanHandle(m_RGBImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_VectorImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_Image2D, m_WithLabel2D));
    CPPUNIT_ASSERT(tool->CanHandle(m_Image2DPlusT, m_WithLabel2DPlusT));
  }

  void SegTool2DOptInRequiresScalarReferenceSlice()
  {
    // Reads the reference slice. Vector images reduce to the displayed component,
    // composite RGB images do not.
    auto tool = mitk::RegionGrowingTool::New();

    CPPUNIT_ASSERT(!tool->CanHandle(m_RGBImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_VectorImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_Image2D, m_WithLabel2D));
  }

  void SegWithPreviewToolRequiresLabelsByDefault()
  {
    auto tool = mitk::BinaryThresholdTool::New();

    CPPUNIT_ASSERT(!tool->CanHandle(m_ReferenceImage, m_NoGroups));
    CPPUNIT_ASSERT(!tool->CanHandle(m_ReferenceImage, m_EmptyGroup));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegWithPreviewToolOptOutRequiresGroupOnly()
  {
    auto tool = mitk::OtsuTool3D::New();

    CPPUNIT_ASSERT(!tool->CanHandle(m_Surface, m_EmptyGroup));
    CPPUNIT_ASSERT(!tool->CanHandle(m_ReferenceImage, m_NoGroups));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_EmptyGroup));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegWithPreviewToolRequiresScalarReferenceByDefault()
  {
    auto tool = mitk::BinaryThresholdTool::New();

    CPPUNIT_ASSERT(!tool->CanHandle(m_RGBImage, m_WithLabel));
    CPPUNIT_ASSERT(!tool->CanHandle(m_VectorImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegWithPreviewToolOptOutAcceptsAnyPixelType()
  {
    // Picks regions from the segmentation and never reads the reference image.
    auto tool = mitk::PickingTool::New();

    CPPUNIT_ASSERT(tool->CanHandle(m_RGBImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_VectorImage, m_WithLabel));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegWithPreviewToolAcceptsAnyDimensionByDefault()
  {
    auto tool = mitk::BinaryThresholdTool::New();

    CPPUNIT_ASSERT(tool->CanHandle(m_Image2D, m_WithLabel2D));
    CPPUNIT_ASSERT(tool->CanHandle(m_Image2DPlusT, m_WithLabel2DPlusT));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }

  void SegWithPreviewToolOptInRequiresVolumetricReference()
  {
    // The GrowCut filter is fixed to 3D. A 2D+t image qualifies because each
    // of its time steps is a single-slice volume.
    auto tool = mitk::GrowCutTool::New();

    CPPUNIT_ASSERT(!tool->CanHandle(m_Image2D, m_WithLabel2D));
    CPPUNIT_ASSERT(tool->CanHandle(m_Image2DPlusT, m_WithLabel2DPlusT));
    CPPUNIT_ASSERT(tool->CanHandle(m_ReferenceImage, m_WithLabel));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToolCanHandle)
