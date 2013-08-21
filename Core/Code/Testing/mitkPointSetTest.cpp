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


#include <mitkPointSet.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>

#include <fstream>

class mitkPointSetTestClass { public:

static void TestGetITKPointSet(mitk::PointSet *pointSet)
{
  //try to get the itkPointSet
  mitk::PointSet::DataType::Pointer itkdata = NULL;
  itkdata = pointSet->GetPointSet();
  MITK_TEST_CONDITION( itkdata.IsNotNull(), "try to get the itkPointSet from a newly created PointSet" )
}

static void TestGetSizeIsZero(mitk::PointSet *pointSet)
{
  //fresh PointSet has to be empty!
  MITK_TEST_CONDITION( pointSet->GetSize() == 0, "check if the PointSet size is 0 " )
}

static void TestIsEmpty(mitk::PointSet *pointSet)
{
  MITK_TEST_CONDITION(pointSet->IsEmptyTimeStep(0), "check if the PointSet is empty" )
}

static void TestCreateOperationAndAddPoint(mitk::PointSet *pointSet)
{
  int id = 0;
  mitk::Point3D point;
  point.Fill(1);

  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, id);
  pointSet->ExecuteOperation(doOp);
  MITK_TEST_CONDITION( pointSet->GetSize()==1 && pointSet->IndexExists(id), "check if added points exists" )

    delete doOp;

  mitk::Point3D tempPoint;
  tempPoint.Fill(0);

  tempPoint = pointSet->GetPoint(id);

  MITK_TEST_CONDITION( point == tempPoint, "check if added point contains real value" )
}


static void TestAddSecondPoint(mitk::PointSet *pointSet)
{
  //add a point directly
  int id=0;
  mitk::Point3D point;
  mitk::FillVector3D(point, 1.0, 2.0, 3.0);
  ++id;
  pointSet->GetPointSet()->GetPoints()->InsertElement(id, point);

  MITK_TEST_CONDITION( pointSet->GetSize()==2 ||pointSet->IndexExists(id), "check if added points exists" )

    mitk::Point3D tempPoint;

  tempPoint.Fill(0);
  tempPoint = pointSet->GetPoint(id);

  MITK_TEST_CONDITION( point == tempPoint, "check if added point contains real value" )
}

static void TestIsNotEmpty(mitk::PointSet *pointSet)
{
  //PointSet can not be empty!
  MITK_TEST_CONDITION( !pointSet->IsEmptyTimeStep(0), "check if the PointSet is not empty " )

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


static void TestSwapPointPositionUpwards(mitk::PointSet *pointSet)
{
  //Check SwapPointPosition upwards
  mitk::Point3D point;
  mitk::Point3D tempPoint;
  point = pointSet->GetPoint(1);
  pointSet->SwapPointPosition(1, true);
  tempPoint = pointSet->GetPoint(0);

  MITK_TEST_CONDITION( point == tempPoint, "check SwapPointPosition upwards" )

    /*
    if(point != tempPoint)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestSwapPointPositionUpwardsNotPossible(mitk::PointSet *pointSet)
{
  //Check SwapPointPosition upwards not possible
  MITK_TEST_CONDITION( pointSet->SwapPointPosition(0, true)==false, "check SwapPointPosition upwards not possible" )

    /*
    if(pointSet->SwapPointPosition(0, true))
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestSwapPointPositionDownwards(mitk::PointSet *pointSet)
{
  //Check SwapPointPosition downwards
  mitk::Point3D point;
  mitk::Point3D tempPoint;
  point = pointSet->GetPoint(0);
  pointSet->SwapPointPosition(0, false);
  tempPoint = pointSet->GetPoint(1);

  MITK_TEST_CONDITION( point == tempPoint, "check SwapPointPosition down" )

    /*
    if(point != tempPoint)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestSwapPointPositionDownwardsNotPossible(mitk::PointSet * /*pointSet*/)
{
  mitk::PointSet::Pointer pointSet2 = mitk::PointSet::New();

  int id = 0;
  mitk::Point3D point;
  point.Fill(1);
  pointSet2->SetPoint(id, point);


  //Check SwapPointPosition downwards not possible
  MITK_TEST_CONDITION(!pointSet2->SwapPointPosition(id, false), "check SwapPointPosition downwards not possible" )

    /*
    if(pointSet->SwapPointPosition(1, false))
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestPointOperationOpMove(mitk::PointSet *pointSet)
{
  //check opMOVE  ExecuteOperation
  int id=1;
  mitk::Point3D point1;
  mitk::Point3D tempPoint;
  point1.Fill(2);

  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVE, point1, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id);

  MITK_TEST_CONDITION(tempPoint == point1 , "check PointOperation OpMove " )
    delete doOp;
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

static void TestPointOperationOpRemove(mitk::PointSet *pointSet)
{
  //check OpREMOVE  ExecuteOperation
  int id=0;
  mitk::Point3D point;
  mitk::Point3D tempPoint;

  point = pointSet->GetPoint(id);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id);

  MITK_TEST_CONDITION(!pointSet->IndexExists(id) , "check PointOperation OpREMOVE " )
    delete doOp;
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

static void TestPointOperationOpSelectPoint(mitk::PointSet *pointSet)
{
  mitk::Point3D point4;
  //check OpSELECTPOINT  ExecuteOperation
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpSELECTPOINT, point4,4);
  pointSet->ExecuteOperation(doOp);

  MITK_TEST_CONDITION(pointSet->GetSelectInfo(4) , "check PointOperation OpSELECTPOINT " )
    delete doOp;
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

static void TestGetNumberOfSelected(mitk::PointSet *pointSet)
{
  // check GetNumeberOfSelected
  MITK_TEST_CONDITION(pointSet->GetNumberOfSelected() == 1 , "check GetNumeberOfSelected " )
    /*
    if(pointSet->GetNumberOfSelected() != 1)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestSearchSelectedPoint(mitk::PointSet *pointSet)
{
  // check SearchSelectedPoint
  MITK_TEST_CONDITION(pointSet->SearchSelectedPoint() == 4 , "check SearchSelectedPoint " )

    /*
    if( pointSet->SearchSelectedPoint() != 4)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestOpDeselectPoint(mitk::PointSet *pointSet)
{
  //check OpDESELECTPOINT  ExecuteOperation
  mitk::Point3D point4;
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpDESELECTPOINT, point4,4);
  pointSet->ExecuteOperation(doOp);

  MITK_TEST_CONDITION(!pointSet->GetSelectInfo(4) , "check PointOperation OpDESELECTPOINT " )
    MITK_TEST_CONDITION(pointSet->GetNumberOfSelected() == 0 , "check GetNumeberOfSelected " )
    delete doOp;
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

static void TestOpMovePointUp(mitk::PointSet *pointSet)
{
  //check OpMOVEPOINTUP  ExecuteOperation
  int id = 4;
  mitk::Point3D point4;
  mitk::Point3D point;
  mitk::Point3D tempPoint;

  point = pointSet->GetPoint(id);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, point4, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id-1);

  MITK_TEST_CONDITION(tempPoint == point , "check PointOperation OpMOVEPOINTUP " )
    delete doOp;
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

static void TestOpMovePointDown(mitk::PointSet *pointSet)
{
  //check OpMOVEPOINTDown  ExecuteOperation

  int id = 2;
  mitk::Point3D point;
  mitk::Point3D point2;
  mitk::Point3D tempPoint;

  point = pointSet->GetPoint(id);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, point2, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id+1);

  MITK_TEST_CONDITION(tempPoint == point , "check PointOperation OpMOVEPOINTDOWN " )
    delete doOp;
  /*
  if (tempPoint != point)
  {
  std::cout<<"[FAILED]"<<std::endl;
  return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  */
}

static void TestSetSelectInfo(mitk::PointSet *pointSet)
{
  //check SetSelectInfo
  pointSet->SetSelectInfo(2, true);

  MITK_TEST_CONDITION(pointSet->GetSelectInfo(2) , "check SetSelectInfo" )
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

static void TestInsertPointWithPointSpecification(mitk::PointSet *pointSet)
{
  //check InsertPoint with PointSpecification
  mitk::Point3D point5;
  mitk::Point3D tempPoint;
  point5.Fill(7);

  pointSet->SetPoint(5, point5, mitk::PTEDGE );
  tempPoint = pointSet->GetPoint(5);

  MITK_TEST_CONDITION(tempPoint == point5, "check InsertPoint with PointSpecification" )
    /*
    if (tempPoint != point5)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestGetPointIfExists(mitk::PointSet *pointSet)
{
  //check GetPointIfExists
  mitk::Point3D point5;
  mitk::Point3D tempPoint;
  point5.Fill(7);
  mitk::PointSet::PointType tmpPoint;

  pointSet->GetPointIfExists(5, &tmpPoint);

  MITK_TEST_CONDITION(tmpPoint == point5, "check GetPointIfExists: " )
    /*
    if (tmpPoint != point5)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}

static void TestCreateHoleInThePointIDs(mitk::PointSet *pointSet)
{
  // create a hole in the point IDs
  mitk::Point3D point;
  mitk::PointSet::PointType p10, p11, p12;
  p10.Fill(10.0);
  p11.Fill(11.0);
  p12.Fill(12.0);
  pointSet->InsertPoint(10, p10);
  pointSet->InsertPoint(11, p11);
  pointSet->InsertPoint(12, p12);

  MITK_TEST_CONDITION((pointSet->IndexExists(10) == true) || (pointSet->IndexExists(11) == true) || (pointSet->IndexExists(12) == true), "add points with id 10, 11, 12: " )

  //check OpREMOVE  ExecuteOperation
  int id = 11;
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);
  pointSet->ExecuteOperation(doOp);

  MITK_TEST_CONDITION(!pointSet->IndexExists(id), "remove point id 11: ")

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

  MITK_TEST_CONDITION(((newP10 == p12) && (newP12 == p10)) == true, "check PointOperation OpMOVEPOINTUP for point id 12:" )

  //check OpMOVEPOINTDOWN  ExecuteOperation
  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, p10, 10);
  pointSet->ExecuteOperation(doOp);
  delete doOp;
  newP10 = pointSet->GetPoint(10);
  newP12 = pointSet->GetPoint(12);

  MITK_TEST_CONDITION(((newP10 == p10) && (newP12 == p12)) == true, "check PointOperation OpMOVEPOINTDOWN for point id 10: ")
}


static void TestOpMovePointUpOnFirstPoint(mitk::PointSet *pointSet)
{
  //check OpMOVEPOINTUP  on first point ExecuteOperation

  mitk::PointSet::PointType p1 = pointSet->GetPoint(1);
  mitk::PointSet::PointType p2 = pointSet->GetPoint(2);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p1, 1);
  pointSet->ExecuteOperation(doOp);
  delete doOp;

  mitk::PointSet::PointType newP1 = pointSet->GetPoint(1);
  mitk::PointSet::PointType newP2 = pointSet->GetPoint(2);

  MITK_TEST_CONDITION(((newP1 == p1) && (newP2 == p2)) == true, "check PointOperation OpMOVEPOINTUP for point id 1: ")

    /*
    if (((newP1 == p1) && (newP2 == p2)) == false)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
    */
}
static void TestPointContainerPointDataContainer(mitk::PointSet* ps)
{
  mitk::PointSet::PointsContainer* pc = ps->GetPointSet()->GetPoints();
  mitk::PointSet::PointDataContainer* pd = ps->GetPointSet()->GetPointData();
  MITK_TEST_CONDITION_REQUIRED(pc->Size() == pd->Size(), "PointContainer and PointDataContainer have same size");
  mitk::PointSet::PointsContainer::ConstIterator pIt = pc->Begin();
  mitk::PointSet::PointDataContainer::ConstIterator dIt = pd->Begin();
  bool failed = false;
  for (; pIt != pc->End(); ++pIt, ++dIt)
    if (pIt->Index() != dIt->Index())
    {
      failed = true;
      break;
    }
  MITK_TEST_CONDITION(failed == false, "Indices in PointContainer and PointDataContainer are equal");
}
};




int mitkPointSetTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("PointSet")

    //Create PointSet
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(),"Testing instantiation")

  mitkPointSetTestClass::TestGetITKPointSet(pointSet);
  mitkPointSetTestClass::TestGetSizeIsZero(pointSet);
  mitkPointSetTestClass::TestIsEmpty(pointSet);
  mitkPointSetTestClass::TestCreateOperationAndAddPoint(pointSet);

  mitk::Point3D point2, point3, point4;
  point2.Fill(3);
  point3.Fill(4);
  point4.Fill(5);
  pointSet->InsertPoint(2,point2);
  pointSet->InsertPoint(3,point3);
  pointSet->InsertPoint(4,point4);

  mitkPointSetTestClass::TestAddSecondPoint(pointSet);
  mitkPointSetTestClass::TestIsNotEmpty(pointSet);
  mitkPointSetTestClass::TestSwapPointPositionUpwards(pointSet);
  mitkPointSetTestClass::TestSwapPointPositionUpwardsNotPossible(pointSet);
  mitkPointSetTestClass::TestSwapPointPositionDownwards(pointSet);
  mitkPointSetTestClass::TestSwapPointPositionDownwardsNotPossible(pointSet);
  mitkPointSetTestClass::TestPointOperationOpMove(pointSet);
  mitkPointSetTestClass::TestPointOperationOpRemove(pointSet);
  mitkPointSetTestClass::TestPointOperationOpSelectPoint(pointSet);
  mitkPointSetTestClass::TestGetNumberOfSelected(pointSet);
  mitkPointSetTestClass::TestSearchSelectedPoint(pointSet);
  mitkPointSetTestClass::TestOpDeselectPoint(pointSet);
  mitkPointSetTestClass::TestOpMovePointUp(pointSet);
  mitkPointSetTestClass::TestOpMovePointDown(pointSet);
  mitkPointSetTestClass::TestSetSelectInfo(pointSet);
  mitkPointSetTestClass::TestInsertPointWithPointSpecification(pointSet);
  mitkPointSetTestClass::TestGetPointIfExists(pointSet);
  mitkPointSetTestClass::TestCreateHoleInThePointIDs(pointSet);
  mitkPointSetTestClass::TestOpMovePointUpOnFirstPoint(pointSet);

  MITK_TEST_OUTPUT(<< "Test InsertPoint(), SetPoint() and SwapPointPosition()");
  mitk::PointSet::PointType point;
  mitk::FillVector3D(point, 2.2, 3.3, -4.4);
  /* call everything that might modify PointContainer and PointDataContainer */
  pointSet->InsertPoint(17, point);
  pointSet->SetPoint(4, point);
  pointSet->SetPoint(7, point);
  pointSet->SetPoint(2, point);
  pointSet->SwapPointPosition(7, true);
  pointSet->SwapPointPosition(3, true);
  pointSet->SwapPointPosition(2, false);
  mitkPointSetTestClass::TestPointContainerPointDataContainer(pointSet);

  MITK_TEST_OUTPUT(<< "Test OpREMOVE");
  mitk::PointOperation op1(mitk::OpREMOVE, mitk::Point3D(), 2); // existing index
  pointSet->ExecuteOperation(&op1);
  mitk::PointOperation op1b(mitk::OpREMOVE, mitk::Point3D(), 112); // non existing index
  pointSet->ExecuteOperation(&op1b);
  mitkPointSetTestClass::TestPointContainerPointDataContainer(pointSet);

  MITK_TEST_OUTPUT(<< "Test OpMove");
  mitk::PointOperation op2(mitk::OpMOVE, mitk::Point3D(), 4);  // existing index
  pointSet->ExecuteOperation(&op2);
  mitk::PointOperation op3(mitk::OpMOVE, mitk::Point3D(), 34);  // non existing index
  pointSet->ExecuteOperation(&op3);
  mitkPointSetTestClass::TestPointContainerPointDataContainer(pointSet);

  MITK_TEST_OUTPUT(<< "Test OpINSERT");
  mitk::PointOperation op4(mitk::OpINSERT, mitk::Point3D(), 38);  // non existing index
  pointSet->ExecuteOperation(&op4);
  mitk::PointOperation op5(mitk::OpINSERT, mitk::Point3D(), 17);  // existing index
  pointSet->ExecuteOperation(&op5);
  mitkPointSetTestClass::TestPointContainerPointDataContainer(pointSet);



  pointSet = mitk::PointSet::New();
  pointSet->Expand(3);
  mitk::Point3D new0, new1, new2;
  new0.Fill(0);
  new1.Fill(1);
  new2.Fill(2);
  pointSet->InsertPoint(5,new0, mitk::PTCORNER,0);
  pointSet->InsertPoint(112,new1,0);
  pointSet->InsertPoint(2,new2,0);
  pointSet->InsertPoint(2,new0,1);
  pointSet->InsertPoint(1,new1,1);
  pointSet->InsertPoint(0,new2,1);
  pointSet->InsertPoint(0,new0,2);
  pointSet->InsertPoint(2,new1,2);
  pointSet->InsertPoint(1,new2,2);

  MITK_TEST_OUTPUT( << "... pointset ts: " << pointSet->GetTimeSteps() )
  mitk::PointSet::Pointer clonePS = pointSet->Clone();
  MITK_TEST_OUTPUT( << "... clone pointset ts: " << clonePS->GetTimeSteps() )

  for (unsigned int t=0; t< pointSet->GetTimeSteps(); t++)
  {
    MITK_TEST_OUTPUT( << "testing timestep: " << t )
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSize(t) == clonePS->GetSize(t), "Clone has same size")

    // test for equal point coordinates
    for (mitk::PointSet::PointsConstIterator i = pointSet->Begin(), j = clonePS->Begin();
        i != pointSet->End() && j != clonePS->End(); ++i, ++j)
    {
      MITK_TEST_CONDITION_REQUIRED(i.Index() == j.Index() && mitk::Equal(i.Value(),j.Value()), "Cloned PS and PS have same points")
    }

    // test for equal point data
    mitk::PointSet::PointDataContainer* pointDataCont = pointSet->GetPointSet(t)->GetPointData();
    mitk::PointSet::PointDataContainer* clonePointDataCont = clonePS->GetPointSet(t)->GetPointData();
    MITK_TEST_CONDITION_REQUIRED(pointDataCont && clonePointDataCont, "Valid point data container")
    MITK_TEST_CONDITION_REQUIRED(pointDataCont->Size() == clonePointDataCont->Size(), "Cloned point data container has same size")
    for (mitk::PointSet::PointDataConstIterator i = pointDataCont->Begin(), j = clonePointDataCont->Begin();
        i != pointDataCont->End() && j != clonePointDataCont->End(); ++i, ++j)
    {
      MITK_TEST_CONDITION_REQUIRED(i.Index() == j.Index() && i.Value() == j.Value(), "Cloned PS and PS have same point data")
    }
  }


  mitkPointSetTestClass::TestIsNotEmpty(clonePS);
  MITK_TEST_CONDITION_REQUIRED(clonePS->GetPointSetSeriesSize() == pointSet->GetPointSetSeriesSize(), "Testing cloned point set's size!");
  MITK_TEST_CONDITION_REQUIRED(clonePS.GetPointer() != pointSet.GetPointer(), "Testing that the clone is not the source PS!");
  MITK_TEST_CONDITION_REQUIRED(clonePS->GetGeometry()->GetCenter() == pointSet->GetGeometry()->GetCenter() , "Testing if the geometry is cloned correctly!");
  MITK_TEST_CONDITION_REQUIRED(clonePS->GetPropertyList()->GetMap()->size() == pointSet->GetPropertyList()->GetMap()->size() , "Testing if the property list is cloned correctly!");
  // Also testing, that clone is independent from original
  mitk::Point3D p, p2;
  p.Fill(42);
  p2.Fill(84);
  clonePS->InsertPoint(0,p);
  pointSet->InsertPoint(0,p2);
  p = clonePS->GetPoint(0);
  p2 = pointSet->GetPoint(0);
  MITK_TEST_CONDITION_REQUIRED(p != p2, "Testing that the clone is independent from source!");


  MITK_TEST_END();
}
