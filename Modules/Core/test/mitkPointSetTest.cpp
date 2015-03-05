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
#include "mitkTestFixture.h"


#include <mitkPointSet.h>
#include <mitkNumericTypes.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>

#include <fstream>



/**
 * TestSuite for PointSet stuff not only operating on an empty PointSet
 */
class mitkPointSetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPointSetTestSuite);

  MITK_TEST(TestIsNotEmpty);
  MITK_TEST(TestSetSelectInfo);
  MITK_TEST(TestGetNumberOfSelected);
  MITK_TEST(TestSearchSelectedPoint);
  MITK_TEST(TestGetPointIfExists);
  MITK_TEST(TestSwapPointPositionUpwards);
  MITK_TEST(TestSwapPointPositionUpwardsNotPossible);
  MITK_TEST(TestSwapPointPositionDownwards);
  MITK_TEST(TestSwapPointPositionDownwardsNotPossible);
  MITK_TEST(TestCreateHoleInThePointIDs);
  MITK_TEST(TestInsertPointWithPointSpecification);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PointSet::Pointer pointSet;
  static const mitk::PointSet::PointIdentifier selectedPointId = 2;

public:

  void setUp()
  {
    //Create PointSet
    pointSet = mitk::PointSet::New();

    // add some points
    mitk::Point3D point2, point3, point4;
    point2.Fill(3);
    point3.Fill(4);
    point4.Fill(5);
    pointSet->InsertPoint(2,point2);
    pointSet->InsertPoint(3,point3);
    pointSet->InsertPoint(4,point4);

    mitk::Point3D point1;
    mitk::FillVector3D(point1, 1.0, 2.0, 3.0);
    pointSet->InsertPoint(1, point1);

    mitk::Point3D point0;
    point0.Fill(1);
    pointSet->InsertPoint(0, point0);

    // select point with id 2
    pointSet->SetSelectInfo(2, true);
  }


  void tearDown()
  {
    pointSet = NULL;
  }

  void TestIsNotEmpty()
  {
    //PointSet can not be empty!
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "check if the PointSet is not empty ",
        true, !pointSet->IsEmptyTimeStep(0) );

        /*
    std::cout << "check if the PointSet is not empty ";
    if (pointSet->IsEmpty(0))
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
         */
  }

  void TestSetSelectInfo()
  {
    //check SetSelectInfo
    pointSet->SetSelectInfo(4, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SetSelectInfo",
        true, pointSet->GetSelectInfo(4));
    /*
      if (!pointSet->GetSelectInfo(2))
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      delete doOp;
      std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestSearchSelectedPoint()
  {
    // check SearchSelectedPoint
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SearchSelectedPoint ",
        true, pointSet->SearchSelectedPoint() == (int) selectedPointId);

          /*
      if( pointSet->SearchSelectedPoint() != 4)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
           */
  }

  void TestGetNumberOfSelected()
  {

    // check GetNumeberOfSelected
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check GetNumeberOfSelected ",
        true, pointSet->GetNumberOfSelected() == 1);

          /*
      if(pointSet->GetNumberOfSelected() != 1)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
           */
  }

  void TestGetPointIfExists()
  {
    //check GetPointIfExists
    mitk::Point3D point4;
    mitk::Point3D tempPoint;
    point4.Fill(5);
    mitk::PointSet::PointType tmpPoint;

    pointSet->GetPointIfExists(4, &tmpPoint);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check GetPointIfExists: ",
        true, tmpPoint == point4);
    /*
      if (tmpPoint != point5)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestSwapPointPositionUpwards()
  {
    //Check SwapPointPosition upwards
    mitk::Point3D point;
    mitk::Point3D tempPoint;
    point = pointSet->GetPoint(1);
    pointSet->SwapPointPosition(1, true);
    tempPoint = pointSet->GetPoint(0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SwapPointPosition upwards",
        true, point == tempPoint);

    /*
      if(point != tempPoint)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
     */
  }


  void TestSwapPointPositionUpwardsNotPossible()
  {
    //Check SwapPointPosition upwards not possible
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SwapPointPosition upwards not possible",
        false, pointSet->SwapPointPosition(0, true));

    /*
if(pointSet->SwapPointPosition(0, true))
{
std::cout<<"[FAILED]"<<std::endl;
return EXIT_FAILURE;
}
std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestSwapPointPositionDownwards()
  {
    //Check SwapPointPosition downwards
    mitk::Point3D point;
    mitk::Point3D tempPoint;
    point = pointSet->GetPoint(0);
    pointSet->SwapPointPosition(0, false);
    tempPoint = pointSet->GetPoint(1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SwapPointPosition down",
        true, point == tempPoint);

    /*
if(point != tempPoint)
{
std::cout<<"[FAILED]"<<std::endl;
return EXIT_FAILURE;
}
std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestSwapPointPositionDownwardsNotPossible()
  {
    mitk::PointSet::Pointer pointSet2 = mitk::PointSet::New();

    int id = 0;
    mitk::Point3D point;
    point.Fill(1);
    pointSet2->SetPoint(id, point);


    //Check SwapPointPosition downwards not possible
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SwapPointPosition downwards not possible",
        false, pointSet2->SwapPointPosition(id, false));

    /*
if(pointSet->SwapPointPosition(1, false))
{
std::cout<<"[FAILED]"<<std::endl;
return EXIT_FAILURE;
}
std::cout<<"[PASSED]"<<std::endl;
     */
  }


  void TestCreateHoleInThePointIDs()
  {
    // create a hole in the point IDs
    mitk::Point3D point(0.);
    mitk::PointSet::PointType p10, p11, p12;
    p10.Fill(10.0);
    p11.Fill(11.0);
    p12.Fill(12.0);
    pointSet->InsertPoint(10, p10);
    pointSet->InsertPoint(11, p11);
    pointSet->InsertPoint(12, p12);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("add points with id 10, 11, 12: ",
        true, (pointSet->IndexExists(10) == true) || (pointSet->IndexExists(11) == true) || (pointSet->IndexExists(12) == true));

    //check OpREMOVE  ExecuteOperation
    int id = 11;
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);
    pointSet->ExecuteOperation(doOp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "remove point id 11: ",
        false, pointSet->IndexExists(id));

    /*
      if(pointSet->IndexExists(id))
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      delete doOp;
      std::cout<<"[PASSED]"<<std::endl;
     */

    //mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p12, 12);
    //pointSet->ExecuteOperation(doOp);
    delete doOp;

    //check OpMOVEPOINTUP  ExecuteOperation
    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p12, 12);
    pointSet->ExecuteOperation(doOp);
    delete doOp;

    mitk::PointSet::PointType newP10 = pointSet->GetPoint(10);
    mitk::PointSet::PointType newP12 = pointSet->GetPoint(12);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpMOVEPOINTUP for point id 12:",
        true, ((newP10 == p12) && (newP12 == p10)));

    //check OpMOVEPOINTDOWN  ExecuteOperation
    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, p10, 10);
    pointSet->ExecuteOperation(doOp);
    delete doOp;
    newP10 = pointSet->GetPoint(10);
    newP12 = pointSet->GetPoint(12);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpMOVEPOINTDOWN for point id 10: ",
        true, ((newP10 == p10) && (newP12 == p12)));
  }


  void TestInsertPointWithPointSpecification()
  {
    //check InsertPoint with PointSpecification
    mitk::Point3D point5;
    mitk::Point3D tempPoint;
    point5.Fill(7);

    pointSet->SetPoint(5, point5, mitk::PTEDGE );
    tempPoint = pointSet->GetPoint(5);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check InsertPoint with PointSpecification" ,
        true, tempPoint == point5);
    /*
      if (tempPoint != point5)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
     */
  }

};


MITK_TEST_SUITE_REGISTRATION(mitkPointSet)








