/*
 * mitkPointSetTestOnEmpty.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: wirkert
 */
/**
 * TestSuite for PointSet which tests properties on an empty PointSet
 */

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"


#include <mitkPointSet.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>

#include <fstream>

class mitkPointSetOnEmptyTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPointSetOnEmptyTestSuite);

  MITK_TEST(TestInstantiation);
  MITK_TEST(TestIsEmpty);
  MITK_TEST(TestGetITKPointSet);
  MITK_TEST(TestGetSizeIsZero);
  MITK_TEST(TestAddPointDirectly);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PointSet::Pointer pointSet;

public:

  void setUp()
  {
    //Create PointSet
    pointSet = mitk::PointSet::New();
  }

  void tearDown()
  {
    pointSet = NULL;
  }

  void TestInstantiation()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing instantiation",
        true, pointSet.IsNotNull());
  }

  void TestGetITKPointSet()
  {
    //try to get the itkPointSet
    mitk::PointSet::DataType::Pointer itkdata = NULL;
    itkdata = pointSet->GetPointSet();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("try to get the itkPointSet from a newly created PointSet",
        true,  itkdata.IsNotNull());
  }

  void TestGetSizeIsZero()
  {
    //fresh PointSet has to be empty!
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if the PointSet size is 0 ",
        true, pointSet->GetSize() == 0);
  }

  void TestIsEmpty()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if the PointSet is empty",
        true, pointSet->IsEmptyTimeStep(0));
  }

  void TestAddPointDirectly()
  {
    //add a point directly
    int id=0;
    mitk::Point3D point;
    mitk::FillVector3D(point, 1.0, 2.0, 3.0);
    ++id;
    pointSet->GetPointSet()->GetPoints()->InsertElement(id, point);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if added points exists",
        true, pointSet->GetSize()==1 ||pointSet->IndexExists(id));

    mitk::Point3D tempPoint;

    tempPoint.Fill(0);
    tempPoint = pointSet->GetPoint(id);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if added point contains real value",
        true, point == tempPoint);
  }



};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetOnEmpty)





