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

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <mitkInteractionConst.h>
#include <mitkNumericTypes.h>
#include <mitkPointOperation.h>
#include <mitkPointSet.h>

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
  MITK_TEST(TestRemovePointInterface);
  MITK_TEST(TestMaxIdAccess);
  MITK_TEST(TestInsertPointAtEnd);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::PointSet::Pointer pointSet;
  static const mitk::PointSet::PointIdentifier selectedPointId = 2;

public:
  void setUp() override
  {
    // Create PointSet
    pointSet = mitk::PointSet::New();

    // add some points
    mitk::Point3D point2, point3, point4;
    point2.Fill(3);
    point3.Fill(4);
    point4.Fill(5);
    pointSet->InsertPoint(2, point2);
    pointSet->InsertPoint(3, point3);
    pointSet->InsertPoint(4, point4);

    mitk::Point3D point1;
    mitk::FillVector3D(point1, 1.0, 2.0, 3.0);
    pointSet->InsertPoint(1, point1);

    mitk::Point3D point0;
    point0.Fill(1);
    pointSet->InsertPoint(0, point0);

    // select point with id 2
    pointSet->SetSelectInfo(2, true);
  }

  void tearDown() override { pointSet = nullptr; }
  void TestIsNotEmpty()
  {
    // PointSet can not be empty!
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if the PointSet is not empty ", true, !pointSet->IsEmptyTimeStep(0));

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
    // check SetSelectInfo
    pointSet->SetSelectInfo(4, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SetSelectInfo", true, pointSet->GetSelectInfo(4));
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "check SearchSelectedPoint ", true, pointSet->SearchSelectedPoint() == (int)selectedPointId);

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check GetNumeberOfSelected ", true, pointSet->GetNumberOfSelected() == 1);

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
    // check GetPointIfExists
    mitk::Point3D point4;
    mitk::Point3D tempPoint;
    point4.Fill(5);
    mitk::PointSet::PointType tmpPoint;

    pointSet->GetPointIfExists(4, &tmpPoint);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check GetPointIfExists: ", true, tmpPoint == point4);
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
    // Check SwapPointPosition upwards
    mitk::Point3D point;
    mitk::Point3D tempPoint;
    point = pointSet->GetPoint(1);
    pointSet->SwapPointPosition(1, true);
    tempPoint = pointSet->GetPoint(0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SwapPointPosition upwards", true, point == tempPoint);

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
    // Check SwapPointPosition upwards not possible
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "check SwapPointPosition upwards not possible", false, pointSet->SwapPointPosition(0, true));

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
    // Check SwapPointPosition downwards
    mitk::Point3D point;
    mitk::Point3D tempPoint;
    point = pointSet->GetPoint(0);
    pointSet->SwapPointPosition(0, false);
    tempPoint = pointSet->GetPoint(1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check SwapPointPosition down", true, point == tempPoint);

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

    // Check SwapPointPosition downwards not possible
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "check SwapPointPosition downwards not possible", false, pointSet2->SwapPointPosition(id, false));

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
                                 true,
                                 (pointSet->IndexExists(10) == true) || (pointSet->IndexExists(11) == true) ||
                                   (pointSet->IndexExists(12) == true));

    // check OpREMOVE  ExecuteOperation
    int id = 11;
    auto doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);
    pointSet->ExecuteOperation(doOp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("remove point id 11: ", false, pointSet->IndexExists(id));

    /*
      if(pointSet->IndexExists(id))
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      delete doOp;
      std::cout<<"[PASSED]"<<std::endl;
     */

    // mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p12, 12);
    // pointSet->ExecuteOperation(doOp);
    delete doOp;

    // check OpMOVEPOINTUP  ExecuteOperation
    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p12, 12);
    pointSet->ExecuteOperation(doOp);
    delete doOp;

    mitk::PointSet::PointType newP10 = pointSet->GetPoint(10);
    mitk::PointSet::PointType newP12 = pointSet->GetPoint(12);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "check PointOperation OpMOVEPOINTUP for point id 12:", true, ((newP10 == p12) && (newP12 == p10)));

    // check OpMOVEPOINTDOWN  ExecuteOperation
    doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, p10, 10);
    pointSet->ExecuteOperation(doOp);
    delete doOp;
    newP10 = pointSet->GetPoint(10);
    newP12 = pointSet->GetPoint(12);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "check PointOperation OpMOVEPOINTDOWN for point id 10: ", true, ((newP10 == p10) && (newP12 == p12)));
  }

  void TestInsertPointWithPointSpecification()
  {
    // check InsertPoint with PointSpecification
    mitk::Point3D point5;
    mitk::Point3D tempPoint;
    point5.Fill(7);

    pointSet->SetPoint(5, point5, mitk::PTEDGE);
    tempPoint = pointSet->GetPoint(5);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check InsertPoint with PointSpecification", true, tempPoint == point5);
    /*
      if (tempPoint != point5)
      {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
      }
      std::cout<<"[PASSED]"<<std::endl;
     */
  }

  void TestRemovePointInterface()
  {
    mitk::PointSet::Pointer psClone = pointSet->Clone();
    mitk::PointSet::Pointer refPsLastRemoved = mitk::PointSet::New();
    mitk::Point3D point0, point1, point2, point3, point4;
    point0.Fill(1);
    refPsLastRemoved->InsertPoint(0, point0);
    mitk::FillVector3D(point1, 1.0, 2.0, 3.0);
    refPsLastRemoved->InsertPoint(1, point1);
    point2.Fill(3);
    point3.Fill(4);
    refPsLastRemoved->InsertPoint(2, point2);
    refPsLastRemoved->InsertPoint(3, point3);

    mitk::PointSet::Pointer refPsMiddleRemoved = mitk::PointSet::New();
    refPsMiddleRemoved->InsertPoint(0, point0);
    refPsMiddleRemoved->InsertPoint(1, point1);
    refPsMiddleRemoved->InsertPoint(3, point3);

    // remove non-existent point
    bool removed = pointSet->RemovePointIfExists(5, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove non-existent point", false, removed);
    MITK_ASSERT_EQUAL(pointSet, psClone, "No changes made");

    // remove point from non-existent time-step
    removed = pointSet->RemovePointIfExists(1, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove non-existent point", false, removed);
    MITK_ASSERT_EQUAL(pointSet, psClone, "No changes made");

    // remove max id from non-existent time-step
    mitk::PointSet::PointsIterator maxIt = pointSet->RemovePointAtEnd(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove max id point from non-existent time step", true, maxIt == pointSet->End(2));
    MITK_ASSERT_EQUAL(pointSet, psClone, "No changes made");

    // remove max id from empty point set
    mitk::PointSet::Pointer emptyPS = mitk::PointSet::New();
    maxIt = emptyPS->RemovePointAtEnd(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove max id point from non-existent time step", true, maxIt == emptyPS->End(0));
    int size = emptyPS->GetSize(0);
    unsigned int pointSetSeriesSize = emptyPS->GetPointSetSeriesSize();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Nothing added", true, size == 0 && pointSetSeriesSize == 1);

    // remove max id point
    maxIt = pointSet->RemovePointAtEnd(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Point id 4 removed", false, pointSet->IndexExists(4));
    MITK_ASSERT_EQUAL(pointSet, refPsLastRemoved, "No changes made");

    mitk::PointSet::PointIdentifier id = maxIt.Index();
    mitk::PointSet::PointType refPt;
    refPt[0] = 4.0;
    refPt[1] = 4.0;
    refPt[2] = 4.0;
    mitk::PointSet::PointType pt = maxIt.Value();
    bool equal = mitk::Equal(refPt, pt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Returned iterator pointing at max id", true, id == 3 && equal);

    // remove middle point
    removed = pointSet->RemovePointIfExists(2, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove point id 2", true, removed);
    MITK_ASSERT_EQUAL(pointSet, refPsMiddleRemoved, "Point removed");
  }

  void TestMaxIdAccess()
  {
    typedef mitk::PointSet::PointIdentifier IdType;
    typedef mitk::PointSet::PointsIterator PointsIteratorType;
    PointsIteratorType empty;

    mitk::Point3D new1, new2, new3, new4, refMaxPt;
    new1.Fill(4);
    new2.Fill(5);
    new3.Fill(6);
    new4.Fill(7);
    refMaxPt.Fill(5);

    pointSet->SetPoint(0, new1, 2);
    pointSet->InsertPoint(1, new2, 2);
    pointSet->InsertPoint(3, new3, 2);
    pointSet->InsertPoint(6, new4, 2);

    PointsIteratorType maxIt = pointSet->GetMaxId(1);
    empty = pointSet->End(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check empty time step max id.", true, maxIt == empty);

    maxIt = pointSet->GetMaxId(3);
    empty = pointSet->End(3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check non-existent time step max id.", true, maxIt == empty);

    maxIt = pointSet->GetMaxId(0);
    empty = pointSet->End(0);
    IdType maxId = maxIt.Index();
    mitk::Point3D maxPt = maxIt.Value();
    bool equal = mitk::Equal(maxPt, refMaxPt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check time step 0 max id iterator.", false, maxIt == empty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check time step 0 max id.", true, maxId == 4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check time step 0 max id point.", true, equal);

    maxIt = pointSet->GetMaxId(2);
    empty = pointSet->End(2);
    maxId = maxIt.Index();
    maxPt = maxIt.Value();
    equal = mitk::Equal(maxPt, new4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check time step 2 max id iterator.", false, maxIt == empty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check time step 2 max id.", true, maxId == 6);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check time step 2 max id point.", true, equal);
  }

  void TestInsertPointAtEnd()
  {
    typedef mitk::PointSet::PointType PointType;

    PointType new1, new2, new3, new4, refMaxPt;
    new1.Fill(4);
    new2.Fill(5);
    new3.Fill(6);
    new4.Fill(7);

    pointSet->SetPoint(1, new1, 2);
    pointSet->InsertPoint(3, new2, 2);
    pointSet->InsertPoint(4, new3, 2);
    pointSet->InsertPoint(6, new4, 2);

    PointType in1, in2, in3, in4;
    in1.Fill(8);
    in2.Fill(9);
    in3.Fill(10);
    in4.Fill(11);

    mitk::PointSet::Pointer refPs1 = pointSet->Clone();
    refPs1->SetPoint(5, in1, 0);
    mitk::PointSet::Pointer refPs2 = pointSet->Clone();
    refPs2->SetPoint(5, in1, 0);
    refPs2->SetPoint(0, in2, 1);
    mitk::PointSet::Pointer refPs3 = pointSet->Clone();
    refPs3->SetPoint(5, in1, 0);
    refPs3->SetPoint(0, in2, 1);
    refPs3->SetPoint(7, in3, 2);
    mitk::PointSet::Pointer refPs4 = pointSet->Clone();
    refPs4->SetPoint(5, in1, 0);
    refPs4->SetPoint(0, in2, 1);
    refPs4->SetPoint(7, in3, 2);
    refPs4->SetPoint(0, in4, 7);

    pointSet->InsertPoint(in1, 0);
    MITK_ASSERT_EQUAL(pointSet, refPs1, "Check point insertion for time step 0.");

    pointSet->InsertPoint(in2, 1);
    MITK_ASSERT_EQUAL(pointSet, refPs2, "Check point insertion for time step 1.");

    pointSet->InsertPoint(in3, 2);
    MITK_ASSERT_EQUAL(pointSet, refPs3, "Check point insertion for time step 2.");

    pointSet->InsertPoint(in4, 7);
    MITK_ASSERT_EQUAL(pointSet, refPs4, "Check point insertion for time step 7.");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPointSet)
