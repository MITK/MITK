/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
  std::cout << "try to get the itkPointSet from a newly created PointSet";
  mitk::PointSet::DataType::Pointer itkdata = NULL;
  itkdata = pointSet->GetPointSet();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //fresh PointSet has to be empty!
  std::cout << "check if the PointSet is empty";
  if (pointSet->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //create an operation and add a point.
  std::cout << "create an operation and add a point";

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
  std::cout << "add a point directly";
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

  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
