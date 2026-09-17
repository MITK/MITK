/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAddContourTool.h>
#include <mitkBinaryThresholdTool.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkOtsuTool3D.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkToolCanHandleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkToolCanHandleTestSuite);
  MITK_TEST(SegTool2DRequiresImageReference);
  MITK_TEST(SegTool2DRequiresSegmentationWithLabels);
  MITK_TEST(SegWithPreviewToolRequiresLabelsByDefault);
  MITK_TEST(SegWithPreviewToolOptOutRequiresGroupOnly);
  CPPUNIT_TEST_SUITE_END();

  mitk::Image::Pointer m_ReferenceImage;
  mitk::Surface::Pointer m_Surface;
  mitk::MultiLabelSegmentation::Pointer m_NoGroups;
  mitk::MultiLabelSegmentation::Pointer m_EmptyGroup;
  mitk::MultiLabelSegmentation::Pointer m_WithLabel;

public:
  void setUp() override
  {
    m_ReferenceImage = mitk::Image::New();
    unsigned int dimensions[] = { 4, 4, 4 };
    m_ReferenceImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    m_Surface = mitk::Surface::New();

    m_NoGroups = mitk::MultiLabelSegmentation::New();
    m_NoGroups->Initialize(m_ReferenceImage, true, false);

    m_EmptyGroup = mitk::MultiLabelSegmentation::New();
    m_EmptyGroup->Initialize(m_ReferenceImage);

    m_WithLabel = mitk::MultiLabelSegmentation::New();
    m_WithLabel->Initialize(m_ReferenceImage);
    mitk::Color color;
    color.Set(1.0f, 0.0f, 0.0f);
    m_WithLabel->AddLabel("Label", color, 0);
  }

  void tearDown() override
  {
    m_ReferenceImage = nullptr;
    m_Surface = nullptr;
    m_NoGroups = nullptr;
    m_EmptyGroup = nullptr;
    m_WithLabel = nullptr;
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
};

MITK_TEST_SUITE_REGISTRATION(mitkToolCanHandle)
