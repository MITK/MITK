/*
 * mitkPoinSetPointOperationsTest.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: wirkert
 */


#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"


#include <mitkPointSet.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>

#include <fstream>


/**
 * TestSuite for all PointSet manipulations done by PointOperations
 */
class mitkPointSetPointOperationsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPointSetPointOperationsTestSuite);

  MITK_TEST(TestCreateOperationAndAddPoint);
  MITK_TEST(TestPointOperationOpMove);
  MITK_TEST(TestPointOperationOpRemove);
  MITK_TEST(TestPointOperationOpSelectPoint);
  MITK_TEST(TestOpDeselectPoint);
  MITK_TEST(TestOpMovePointUp);
  MITK_TEST(TestOpMovePointDown);
  MITK_TEST(TestOpMovePointUpOnFirstPoint);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PointSet::Pointer pointSet;
  mitk::PointOperation* doOp;

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
  }

  void tearDown()
  {
    pointSet = NULL;
    delete doOp;
  }

  void TestCreateOperationAndAddPoint()
  {
    int id = 0;
    mitk::Point3D point;
    point.Fill(1);

    doOp = new mitk::PointOperation(mitk::OpINSERT, point, id);

    pointSet->ExecuteOperation(doOp);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if added points exists",
        true, pointSet->GetSize()==4 && pointSet->IndexExists(id));


    mitk::Point3D tempPoint;
    tempPoint.Fill(0);

    tempPoint = pointSet->GetPoint(id);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if added point contains real value",
        true, point == tempPoint);
  }


  void TestPointOperationOpMove()
  {
    //check opMOVE  ExecuteOperation
    int id=1;
    mitk::Point3D point1;
    mitk::Point3D tempPoint;
    point1.Fill(2);

    doOp = new mitk::PointOperation(mitk::OpMOVE, point1, id);

    pointSet->ExecuteOperation(doOp);
    tempPoint = pointSet->GetPoint(id);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpMove ",
        true, tempPoint == point1);

    /*
    if (tempPoint != point1)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    delete doOp;
    std::cout<<"[PASSED]"<<std::endl;
     */
  }


  void TestPointOperationOpRemove()
  {
    //check OpREMOVE  ExecuteOperation
    int id=0;
    mitk::Point3D point;
    mitk::Point3D tempPoint;

    point = pointSet->GetPoint(id);

    doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);

    pointSet->ExecuteOperation(doOp);
    tempPoint = pointSet->GetPoint(id);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpREMOVE ",
        false, pointSet->IndexExists(id) );

    /*
    if(pointSet->IndexExists(id))
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    delete doOp;
    std::cout<<"[PASSED]"<<std::endl;
     */
  }


  void TestPointOperationOpSelectPoint()
  {
    mitk::Point3D point3(0.);
    //check OpSELECTPOINT  ExecuteOperation

    doOp = new mitk::PointOperation(mitk::OpSELECTPOINT, point3,3);

    pointSet->ExecuteOperation(doOp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpSELECTPOINT ",
        true, pointSet->GetSelectInfo(3));

    /*
    if (!pointSet->GetSelectInfo(4))
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    delete doOp;
    std::cout<<"[PASSED]"<<std::endl;
     */
  }


  void TestOpDeselectPoint()
  {
    //check OpDESELECTPOINT  ExecuteOperation
    mitk::Point3D point4(0.);

    doOp = new mitk::PointOperation(mitk::OpDESELECTPOINT, point4,4);

    pointSet->ExecuteOperation(doOp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpDESELECTPOINT ",
        false, pointSet->GetSelectInfo(4));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check GetNumeberOfSelected ",
        true, pointSet->GetNumberOfSelected() == 0 );

    /*
    if (pointSet->GetSelectInfo(4))
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    delete doOp;
    std::cout<<"[PASSED]"<<std::endl;


    if(pointSet->GetNumberOfSelected() != 0)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestOpMovePointUp()
  {
    //check OpMOVEPOINTUP  ExecuteOperation
    const int id = 4;

    mitk::Point3D point = pointSet->GetPoint(id);

    mitk::Point3D point4(0.);
    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, point4, id);

    pointSet->ExecuteOperation(doOp);
    mitk::Point3D tempPoint = pointSet->GetPoint(id-1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpMOVEPOINTUP ",
        true, tempPoint == point);

    /*
    if (tempPoint != point)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    delete doOp;
    std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestOpMovePointDown()
  {
    //check OpMOVEPOINTDown  ExecuteOperation

    const int id = 2;

    mitk::Point3D point = pointSet->GetPoint(id);
    mitk::Point3D point2(0.);
    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, point2, id);
    pointSet->ExecuteOperation(doOp);
    mitk::Point3D tempPoint = pointSet->GetPoint(id+1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpMOVEPOINTDOWN ",
        true, tempPoint == point);

    /*
    if (tempPoint != point)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestOpMovePointUpOnFirstPoint()
  {
    //check OpMOVEPOINTUP  on first point ExecuteOperation

    mitk::PointSet::PointType p1 = pointSet->GetPoint(1);
    mitk::PointSet::PointType p2 = pointSet->GetPoint(2);

    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p1, 1);

    pointSet->ExecuteOperation(doOp);


    mitk::PointSet::PointType newP1 = pointSet->GetPoint(1);
    mitk::PointSet::PointType newP2 = pointSet->GetPoint(2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check PointOperation OpMOVEPOINTUP for point id 1: ",
        true, ((newP1 == p1) && (newP2 == p2)));

    /*
      if (((newP1 == p1) && (newP2 == p2)) == false)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
     */
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetPointOperations)
