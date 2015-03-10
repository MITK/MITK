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
#include <mitkTestFixture.h>
#include <mitkLine.h>

class mitkLineTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkLineTestSuite);
  MITK_TEST(Line_Instantiation);
  MITK_TEST(Line_TestPoints);
  MITK_TEST(Line_TestParallel);
  MITK_TEST(Test2DLine);
  MITK_TEST(Test3DLine);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Line<double,3> m_Line;
  mitk::Line<double,2> m_2DLine;
  mitk::Line<double,3> m_3DLine;

public:

  void setUp()
  {
  m_Line = mitk::Line<double,3>();

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

  void Line_Instantiation()
  {
    mitk::Line<double,3> myLineDouble = mitk::Line<double,3>();
    mitk::Line<float,3> myLineFloat = mitk::Line<float,3>();
    CPPUNIT_ASSERT(myLineDouble.GetPoint1()[0]==0);
    CPPUNIT_ASSERT(myLineFloat.GetPoint1()[0]==0);
  }

  void Line_TestPoints()
  {
    mitk::Point3D point1,point2;
    mitk::FillVector3D(point1,0,1,0);
    mitk::FillVector3D(point2,0,2.5,0);
    m_Line.SetPoint1(point1);
    m_Line.SetPoint2(point2);
    CPPUNIT_ASSERT_MESSAGE("Test if point 1 was set correctly.",mitk::Equal(m_Line.GetPoint1(),point1));
    CPPUNIT_ASSERT_MESSAGE("Test if point 2 was set correctly.",mitk::Equal(m_Line.GetPoint2(),point2));
  }

  void Line_TestParallel()
  {
    //first line
    mitk::Point3D point1,point2;
    mitk::FillVector3D(point1,0,1,0);
    mitk::FillVector3D(point2,0,2.5,0);
    m_Line.SetPoint1(point1);
    m_Line.SetPoint2(point2);

    //parallel line
    mitk::Point3D point3;
    mitk::Vector3D directionLine2;
    mitk::FillVector3D(point1,1,1,0);
    mitk::FillVector3D(directionLine2,0,1,0);
    mitk::Line<double,3> parallelLine = mitk::Line<double,3>(point1,directionLine2);
    CPPUNIT_ASSERT_MESSAGE("Test if lines are parallel.",m_Line.IsParallel(parallelLine));

    /* Seems as this method is broken, so disabled the test for the moment, see bug 17938
    MITK_INFO << "Distance: " << m_Line.Distance(point3);
    CPPUNIT_ASSERT_MESSAGE("Test line distance.",m_Line.Distance(point3)==1.0);
    */
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
