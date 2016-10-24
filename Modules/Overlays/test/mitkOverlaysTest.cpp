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
#include <mitkCommon.h>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkIOUtil.h>

#include "mitkAnnotationPlacer.h"
#include "mitkOverlayLayouter2D.h"


class mitkOverlaysTestSuite : public mitk::TestFixture
{
   CPPUNIT_TEST_SUITE(mitkOverlaysTestSuite);
   MITK_TEST(AnnotationServiceTest);
   CPPUNIT_TEST_SUITE_END();

private:

public:

  void setUp() override
  {
  }

  void AnnotationServiceTest()
  {
    mitk::AnnotationPlacer* ap1_test1 = mitk::AnnotationPlacer::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing availability of AnnotationPlacer service", ap1_test1);
    mitk::AnnotationPlacer* ap2_test1 = mitk::AnnotationPlacer::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing if AnnotationPlacer of same kind stays avaliable", ap1_test1 == ap2_test1);
    mitk::AnnotationPlacer* ap1_test2 = mitk::AnnotationPlacer::GetAnnotationRenderer("test2");
    CPPUNIT_ASSERT_MESSAGE("Testing if new instance can be created by using different ID", ap1_test2 != ap1_test1);

    mitk::OverlayLayouter2D* ol1_test1 = mitk::OverlayLayouter2D::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing availability of OverlayLayouter2D service", ol1_test1);
    mitk::OverlayLayouter2D* ol2_test1 = mitk::OverlayLayouter2D::GetAnnotationRenderer("test1");
    CPPUNIT_ASSERT_MESSAGE("Testing if OverlayLayouter2D of same kind stays avaliable", ol2_test1 == ol1_test1);
    mitk::OverlayLayouter2D* ol1_test2 = mitk::OverlayLayouter2D::GetAnnotationRenderer("test2");
    CPPUNIT_ASSERT_MESSAGE("Testing if new instance can be created by using different ID", ol1_test2 != ol1_test1);

    CPPUNIT_ASSERT_MESSAGE("Testing if OverlayLayouter2D and AnnotationPlacer services are different",
                           (mitk::AbstractAnnotationRenderer*)ol1_test1 != (mitk::AbstractAnnotationRenderer*)ap1_test1);

  }

  void OverlayTest()
  {

  }
};
MITK_TEST_SUITE_REGISTRATION(mitkOverlays)
