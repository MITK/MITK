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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>
#include <mitkLine.h>

class mitkLineTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkLineTestSuite);
  MITK_TEST(TestTempMethods);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::Line<double,2> m_Line;

public:

  void setUp()
  {
    m_Line = mitk::Line<double,2>();
    itk::Point<double,2> p;
    p[0] = 1;
    p[1] = 2;
    m_Line.SetPoint(p);

    itk::Vector<double,2> direction;
    direction[0] = 0;
    direction[1] = 1;
    m_Line.SetDirection(direction);
  }

  void TestTempMethods()
  {
    CPPUNIT_ASSERT(m_Line.GetPoint()[0]==1);
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkLine)
