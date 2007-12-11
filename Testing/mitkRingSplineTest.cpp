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

#include <mitkRingSpline.h>

#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>
#include <fstream>

int mitkRingSplineTest(int /*argc*/, char* /*argv*/[])
{
  //Create RingSpline
  mitk::RingSpline::Pointer rs = mitk::RingSpline::New();

  //try to get the itkRingSpline
  std::cout << "Create a RingSpline and try to get the itkRingSpline";
  mitk::RingSpline::DataType::Pointer itkdata = NULL;
  itkdata = rs->GetPointSet();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //fresh RingSpline has to be empty!
  std::cout << "Try size of RingSpline is empty:";
  if (rs->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //create an operation and add a point.
  int position;
  mitk::Point3D point;
  for (int i = 1; i <6; ++i)
  {
    //    std::cout<<"Test::: "<<rs->GetSize()<<std::endl;
    position = i-1;
    point[0]= (i%2)?i*2:i*5 ;
    point[1]= (i%3)?i*3:i+2 ;
    point[2]= (i%6)?i*0.5:i-1; 
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, position); rs->ExecuteOperation(doOp);
    //    std::cout<<"Test::: "<<rs->GetSize()<<std::endl;

    //now check new condition! 
    if( (rs->GetSize()!= i))
    {
      std::cout<<"[FAILED] while point set size == "<<rs->GetSize()<<"(expected: "<<i<<")"<<std::endl;
      return EXIT_FAILURE;
    }
  }

  if
    (!rs->SplineExist())
    {
      std::cout<<"Spline no exist: [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }


  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

