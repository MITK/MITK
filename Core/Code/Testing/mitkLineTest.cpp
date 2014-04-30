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
  MITK_TEST(Test2DLine);
  MITK_TEST(Test3DLine);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::Line<double,2> m_2DLine;
  mitk::Line<double,3> m_3DLine;

public:

  void setUp()
  {
    //set up simple 2D Line
    m_2DLine = mitk::Line<double,2>();
    itk::Point<double,2> p;
    p[0] = 1;
    p[1] = 2;
    m_2DLine.SetPoint(p);

    itk::Vector<double,2> direction;
    direction[0] = 0;
    direction[1] = 1;
    m_2DLine.SetDirection(direction);

    //set up simple 3D Line
    m_3DLine = mitk::Line<double,3>();
    mitk::Point3D p3D;
    mitk::FillVector3D(p3D,0,1,2);
    m_3DLine.SetPoint(p3D);

    mitk::Vector3D direction3D;
    mitk::FillVector3D(direction3D,4,5,6);
    m_3DLine.SetDirection(direction3D);
  }

  void Test2DLine()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing 2D Line (point[0])",m_2DLine.GetPoint()[0]==1);
    CPPUNIT_ASSERT_MESSAGE("Testing 2D Line (point[1])",m_2DLine.GetPoint()[1]==2);
    CPPUNIT_ASSERT_MESSAGE("Testing 2D Line (direction[0])",m_2DLine.GetDirection()[0]==0);
    CPPUNIT_ASSERT_MESSAGE("Testing 2D Line (direction[1])",m_2DLine.GetDirection()[1]==1);
  }

  void Test3DLine()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing 3D Line (point[0])",m_3DLine.GetPoint()[0]==0);
    CPPUNIT_ASSERT_MESSAGE("Testing 3D Line (point[1])",m_3DLine.GetPoint()[1]==1);
    CPPUNIT_ASSERT_MESSAGE("Testing 3D Line (point[2])",m_3DLine.GetPoint()[2]==2);
    CPPUNIT_ASSERT_MESSAGE("Testing 3D Line (direction[0])",m_3DLine.GetDirection()[0]==4);
    CPPUNIT_ASSERT_MESSAGE("Testing 3D Line (direction[1])",m_3DLine.GetDirection()[1]==5);
    CPPUNIT_ASSERT_MESSAGE("Testing 3D Line (direction[2])",m_3DLine.GetDirection()[2]==6);
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkLine)
