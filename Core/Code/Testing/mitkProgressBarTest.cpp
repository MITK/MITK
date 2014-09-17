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

#include <mitkTestingMacros.h>
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>
#include <mitkProgressBar.h>

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
