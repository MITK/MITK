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

  int position = 0;
  mitk::Point3D point;
  point.Fill(1);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, position);
  pointSet->ExecuteOperation(doOp);

  //now check new condition!
  if ( (pointSet->GetSize()!=1) ||
    (!pointSet->IndexExists(position)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;

  mitk::Point3D tempPoint;
  tempPoint.Fill(0);
  tempPoint = pointSet->GetPoint(position);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  //add a point directly
  std::cout << "add a point directly ";
  mitk::FillVector3D(point, 1.0, 2.0, 3.0);
  ++position;
  pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);

  //now check new condition!
  if ( (pointSet->GetSize()!=2) ||
    (!pointSet->IndexExists(position)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  tempPoint.Fill(0);
  tempPoint = pointSet->GetPoint(position);
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
  position=1;
  mitk::Point3D point1;
  point1.Fill(2);
  doOp = new mitk::PointOperation(mitk::OpMOVE, point1, position);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(position);
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
  position=0;
  point = pointSet->GetPoint(position);
  int size = pointSet->GetSize();
  doOp = new mitk::PointOperation(mitk::OpREMOVE, point, position);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(position);
  int tempSize = pointSet->GetSize();

  if(pointSet->IndexExists(position))
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
  position = 4;
  point = pointSet->GetPoint(position);
  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, point4, position);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(position-1);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  std::cout<<"[PASSED]"<<std::endl;


  //check OpMOVEPOINTUP  ExecuteOperation	
  std::cout << "check PointOperation OpMOVEPOINTDOWN ";
  position = 2;
  point = pointSet->GetPoint(position);
  doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, point2, position);
  pointSet->ExecuteOperation(doOp);
  tempPoint = pointSet->GetPoint(position+1);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
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
  std::cout<<"check GetPointIfExists"<<std::endl;
  mitk::PointSet::PointType tmpPoint;
  pointSet->GetPointIfExists(5,&tmpPoint);
  if (tmpPoint != point5)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;  	

  //well done!!! Passed!
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
