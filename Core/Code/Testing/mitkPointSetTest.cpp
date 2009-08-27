/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <mitkPointSet.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>


#include <fstream>
int mitkPointSetTest(int /*argc*/, char* /*argv*/[])
{
  //Create PointSet
  mitk::PointSet::Pointer pointSet;
  pointSet = mitk::PointSet::New();

  //try to get the itkPointSet
  std::cout << "try to get the itkPointSet from a newly created PointSet ";
  mitk::PointSet::DataType::Pointer itkdata = NULL;
  itkdata = pointSet->GetPointSet();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //fresh PointSet has to be empty!
  std::cout << "check if the PointSet size is 0 ";
  if (pointSet->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //fresh PointSet has to be empty!
  std::cout << "check if the PointSet is empty ";
  if (!pointSet->IsEmpty(0))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //create an operation and add a point.
  std::cout << "create an operation and add a point ";

  int id = 0;
  mitk::Point3D point;
  point.Fill(1);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, id);
  pointSet->ExecuteOperation(doOp);

  //now check new condition!
  if ( (pointSet->GetSize()!=1) ||
    (!pointSet->IndexExists(id)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;

  mitk::Point3D tempPoint;
  tempPoint.Fill(0);
  tempPoint = pointSet->GetPoint(id);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  //add a point directly
  std::cout << "add a point directly ";
  mitk::FillVector3D(point, 1.0, 2.0, 3.0);
  ++id;
  pointSet->GetPointSet()->GetPoints()->InsertElement(id, point);

  //now check new condition!
  if ( (pointSet->GetSize()!=2) ||
    (!pointSet->IndexExists(id)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  tempPoint.Fill(0);
  tempPoint = pointSet->GetPoint(id);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //PointSet can not be empty!
  std::cout << "check if the PointSet is not empty ";
  if (pointSet->IsEmpty(0))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //Check SwapPointPosition upwards
  std::cout << "check SwapPointPosition";
  point = pointSet->GetPoint(1);
  pointSet->SwapPointPosition(1, true);
  tempPoint = pointSet->GetPoint(0);
  if(point != tempPoint)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  
  //Check SwapPointPosition upwards not possible
  std::cout << "check SwapPointPosition not possible";
  if(pointSet->SwapPointPosition(0, true))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
    
  //Check SwapPointPosition downwards
  std::cout << "check SwapPointPosition down";
  point = pointSet->GetPoint(0);  
  pointSet->SwapPointPosition(0, false);
  tempPoint = pointSet->GetPoint(1);   
  if(point != tempPoint)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  //Check SwapPointPosition downwards not possible
  std::cout << "check SwapPointPosition downwards not possible";
  if(pointSet->SwapPointPosition(1, false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check opMOVE  ExecuteOperation
  std::cout << "check PointOperation OpMove ";
  id=1;
  mitk::Point3D point1;
  point1.Fill(2);
  doOp = new mitk::PointOperation(mitk::OpMOVE, point1, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id);
  if (tempPoint != point1)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;

  mitk::Point3D point2, point3, point4;
  point2.Fill(3);
  point3.Fill(4);
  point4.Fill(5);
  pointSet->InsertPoint(2,point2);
  pointSet->InsertPoint(3,point3);
  pointSet->InsertPoint(4,point4);


  //check OpREMOVE  ExecuteOperation
  std::cout << "check PointOperation OpREMOVE ";
  id=0;
  point = pointSet->GetPoint(id);
  int size = pointSet->GetSize();
  doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id);
  int tempSize = pointSet->GetSize();

  if(pointSet->IndexExists(id))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;


  //check OpSELECTPOINT  ExecuteOperation
  std::cout << "check PointOperation OpSELECTPOINT ";
  doOp = new mitk::PointOperation(mitk::OpSELECTPOINT, point4,4);
  pointSet->ExecuteOperation(doOp);
  if (!pointSet->GetSelectInfo(4))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;

  // check GetNumeberOfSelected 
  std::cout << "check GetNumeberOfSelected ";
  if(pointSet->GetNumberOfSelected() != 1)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  // check SearchSelectedPoint 
  std::cout << "check SearchSelectedPoint ";
  if( pointSet->SearchSelectedPoint() != 4)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check OpDESELECTPOINT  ExecuteOperation
  std::cout << "check PointOperation OpDESELECTPOINT ";
  doOp = new mitk::PointOperation(mitk::OpDESELECTPOINT, point4,4);
  pointSet->ExecuteOperation(doOp);
  if (pointSet->GetSelectInfo(4))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "check GetNumeberOfSelected ";
  if(pointSet->GetNumberOfSelected() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check OpMOVEPOINTUP  ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTUP ";
  id = 4;
  point = pointSet->GetPoint(id);
  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, point4, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id-1);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;


  //check OpMOVEPOINTUP  ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTDOWN ";
  id = 2;
  point = pointSet->GetPoint(id);
  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, point2, id);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(id+1);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  //check SetSelectInfo
  std::cout << "check SetSelectInfo";
  pointSet->SetSelectInfo(2, true);
  if (!pointSet->GetSelectInfo(2))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;


  //check InsertPoint with PointSpecification
  mitk::Point3D point5;
  point5.Fill(7);
  std::cout << "check InsertPoint with PointSpecification";
  pointSet->SetPoint(5, point5, mitk::PTEDGE );
  tempPoint = pointSet->GetPoint(5);
  if (tempPoint != point5)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;  	
  
  //check GetPointIfExists
  std::cout<<"check GetPointIfExists: ";
  mitk::PointSet::PointType tmpPoint;
  pointSet->GetPointIfExists(5, &tmpPoint);
  if (tmpPoint != point5)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;  	

  // create a hole in the point IDs
  std::cout << "add points with id 10, 11, 12: ";
  mitk::PointSet::PointType p10, p11, p12;
  p10.Fill(10.0);
  p11.Fill(11.0);
  p12.Fill(12.0);
  pointSet->InsertPoint(10, p10);
  pointSet->InsertPoint(11, p11);
  pointSet->InsertPoint(12, p12);
  if ((pointSet->IndexExists(10) == false) || (pointSet->IndexExists(11) == false) || (pointSet->IndexExists(12) == false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //check OpREMOVE  ExecuteOperation
  std::cout << "remove point id 11: ";
  id = 11;
  doOp = new mitk::PointOperation(mitk::OpREMOVE, point, id);
  pointSet->ExecuteOperation(doOp);
  if(pointSet->IndexExists(id))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;

  //check OpMOVEPOINTUP  ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTUP for point id 12: ";

  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p12, 12);
  pointSet->ExecuteOperation(doOp);
  delete doOp;
  mitk::PointSet::PointType newP10 = pointSet->GetPoint(10);
  mitk::PointSet::PointType newP12 = pointSet->GetPoint(12);
  if (((newP10 == p12) && (newP12 == p10)) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check OpMOVEPOINTDOWN  ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTDOWN for point id 10: ";

  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, newP10, 10);
  pointSet->ExecuteOperation(doOp);
  delete doOp;
  newP10 = pointSet->GetPoint(10);
  newP12 = pointSet->GetPoint(12);
  if (((newP10 == p10) && (newP12 == p12)) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check OpMOVEPOINTDOWN  on last point ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTDOWN for point id 12: ";

  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, newP12, 12);
  pointSet->ExecuteOperation(doOp);
  delete doOp;
  newP10 = pointSet->GetPoint(10);
  newP12 = pointSet->GetPoint(12);
  if (((newP10 == p10) && (newP12 == p12)) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check OpMOVEPOINTUP  on first point ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTUP for point id 1: ";
  mitk::PointSet::PointType p1 = pointSet->GetPoint(1);
  mitk::PointSet::PointType p2 = pointSet->GetPoint(2);
  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, p1, 1);
  pointSet->ExecuteOperation(doOp);
  delete doOp;
  mitk::PointSet::PointType newP1 = pointSet->GetPoint(1);
  mitk::PointSet::PointType newP2 = pointSet->GetPoint(2);
  if (((newP1 == p1) && (newP2 == p2)) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //well done!!! Passed!
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
