/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkCommon.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include "mitkManualPlacementAnnotationRenderer.h"
#include "mitkLayoutAnnotationRenderer.h"

class mitkAnnotationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkAnnotationTestSuite);
  MITK_TEST(AnnotationUtilsTest);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() override {}
  void AnnotationUtilsTest()
  {
    mitk::ManualPlacementAnnotationRenderer *ap1_test1 = mitk::ManualPlacementAnnotationRenderer::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing availability of ManualPlacementAnnotationRenderer service", ap1_test1);
    mitk::ManualPlacementAnnotationRenderer *ap2_test1 = mitk::ManualPlacementAnnotationRenderer::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing if ManualPlacementAnnotationRenderer of same kind stays avaliable", ap1_test1 == ap2_test1);
    mitk::ManualPlacementAnnotationRenderer *ap1_test2 = mitk::ManualPlacementAnnotationRenderer::GetAnnotationRenderer("test2");
    CPPUNIT_ASSERT_MESSAGE("Testing if new instance can be created by using different ID", ap1_test2 != ap1_test1);

    mitk::LayoutAnnotationRenderer *ol1_test1 = mitk::LayoutAnnotationRenderer::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing availability of LayoutAnnotationRenderer service", ol1_test1);
    mitk::LayoutAnnotationRenderer *ol2_test1 = mitk::LayoutAnnotationRenderer::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing if LayoutAnnotationRenderer of same kind stays avaliable", ol2_test1 == ol1_test1);
    mitk::LayoutAnnotationRenderer *ol1_test2 = mitk::LayoutAnnotationRenderer::GetAnnotationRenderer("test2");
    CPPUNIT_ASSERT_MESSAGE("Testing if new instance can be created by using different ID", ol1_test2 != ol1_test1);

    CPPUNIT_ASSERT_MESSAGE(
      "Testing if LayoutAnnotationRenderer and ManualPlacementAnnotationRenderer services are different",
      (mitk::AbstractAnnotationRenderer *)ol1_test1 != (mitk::AbstractAnnotationRenderer *)ap1_test1);
  }

  void AnnotationTest() {}
};
MITK_TEST_SUITE_REGISTRATION(mitkAnnotation)
