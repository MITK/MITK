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

#include <mitkClosedSpline.h>

#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>
#include <fstream>

int mitkClosedSplineTest(int /*argc*/, char* /*argv*/[])
{  //Create ClosedSpline
  mitk::ClosedSpline::Pointer cs = mitk::ClosedSpline::New();
  //mitk::ClosedSpline::Pointer cs = mitk::ClosedSpline::New();

  //try to get the itkClosedSpline
  std::cout << "Create a RingSpline and try to get the itkClosedSpline";
  mitk::ClosedSpline::DataType::Pointer itkdata = NULL;
  //mitk::ClosedSpline::DataType::Pointer itkdata = NULL;
  itkdata = cs->GetPointSet();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //fresh ClosedSpline has to be empty!
  std::cout << "Is the ClosedSpline empty?";
  if (cs->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //create an operation and add a point.
  int position;
  mitk::Point3D point;
  for (int i = 1; i <6; ++i)
  {
    position = i-1;
    point.Fill( (i%2)?i:i*5 );
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, position);
    cs->ExecuteOperation(doOp);
    
    //now check new condition!
    if ( (cs->GetSize()!= i) )
    {
      std::cout<<"[FAILED] while: "<<cs->GetSize()<<std::endl;
      return EXIT_FAILURE;
    }
  }

  if (!cs->SplineExist())
  {
    std::cout<<"Spline no exist: [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }


  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
