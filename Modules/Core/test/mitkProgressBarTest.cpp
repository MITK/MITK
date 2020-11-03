/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkProgressBar.h>
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>
#include <mitkTestingMacros.h>

class mitkProgressBarTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkProgressBarTestSuite);
  MITK_TEST(TestInstantiation);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestInstantiation()
  {
    mitk::ProgressBar::Pointer pb = mitk::ProgressBar::GetInstance();
    CPPUNIT_ASSERT_MESSAGE("Single instance can be created on demand", pb.IsNotNull());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkProgressBar)
