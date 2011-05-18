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

#include <mitkTestingMacros.h>
#include <mitkVector.h>

int mitkVectorTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkVector");
  vnl_matrix_fixed<mitk::ScalarType,3,3> vnlMatrix3x3_1;
  vnlMatrix3x3_1(0,0) = 1.1;
  vnlMatrix3x3_1(0,1) = 0.4;
  vnlMatrix3x3_1(0,2) = 5.3;
  vnlMatrix3x3_1(1,0) = 2.7;
  vnlMatrix3x3_1(1,1) = 3578.56418;
  vnlMatrix3x3_1(1,2) = 123.56;
  vnlMatrix3x3_1(2,0) = 546.89;
  vnlMatrix3x3_1(2,1) = 0.0001;
  vnlMatrix3x3_1(2,1) = 1.0;
  vnl_matrix_fixed<mitk::ScalarType,3,3> vnlMatrix3x3_2;
  vnlMatrix3x3_1(0,0) = 1.1000009;
  vnlMatrix3x3_1(0,1) = 0.4000009;
  vnlMatrix3x3_1(0,2) = 5.3000009;
  vnlMatrix3x3_1(1,0) = 2.7000009;
  vnlMatrix3x3_1(1,1) = 3578.5641809;
  vnlMatrix3x3_1(1,2) = 123.5600009;
  vnlMatrix3x3_1(2,0) = 546.8900009;
  vnlMatrix3x3_1(2,1) = 0.0001009;
  vnlMatrix3x3_1(2,1) = 1.0000009;

  mitk::ScalarType epsilon = 0.000001;
  MITK_TEST_CONDITION_REQUIRED(mitk::MatrixEqualElementWise(vnlMatrix3x3_1,vnlMatrix3x3_1,0.0),"Test for matrix equality with given epsilon=0.0 and exactly the same matrix elements");
  MITK_TEST_CONDITION_REQUIRED(!mitk::MatrixEqualElementWise(vnlMatrix3x3_1,vnlMatrix3x3_2,epsilon),"Test for matrix equality with given epsilon=0.0 and slightly different matrix elements");
  MITK_TEST_CONDITION_REQUIRED(mitk::MatrixEqualElementWise(vnlMatrix3x3_1,vnlMatrix3x3_2,epsilon),"Test for matrix equality with given epsilon and slightly different matrix elements");
  MITK_TEST_CONDITION_REQUIRED(!mitk::MatrixEqualRMS(vnlMatrix3x3_1,vnlMatrix3x3_2,epsilon),"Test for matrix equality with given epsilon=0.0 and slightly different matrix elements");
  MITK_TEST_CONDITION_REQUIRED(mitk::MatrixEqualRMS(vnlMatrix3x3_1,vnlMatrix3x3_2,epsilon),"Test for matrix equality with given epsilon and slightly different matrix elements");

  MITK_TEST_END();
}
