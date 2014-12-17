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

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageIOTestSuite);
  MITK_TEST(TestLockLabel);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {

  }

  // Reduce contours with nth point
  void TestLockLabel()
  {

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageIO)
