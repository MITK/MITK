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

#include <mitkTransform.h>
#include <mitkTestingMacros.h>
#include <mitkStringFromCvMat.h>
#include <mitkCvMatCompare.h>
#include <mitkCvMatFromVnlMatrix.h>

/**Documentation
 *  test for the class "Transform".
 */
int mitkTransformTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("Transform")

  mitk::Transform::Pointer transform = mitk::Transform::New();
  mitk::Transform* t =  transform;

  // test with homogeneous matrixes
  vnl_matrix_fixed<mitk::ScalarType, 4, 4> I;
  I.set_identity();

  std::cout << "vnl I: " << std::endl << I << std::endl;
  t->SetMatrix( I );

  cv::Mat cvI = t->GetCvMatrix();
  std::string matAsString;
  mitk::StringFromCvMat _StringFromCvMat( &cvI, &matAsString );
  _StringFromCvMat.Update();
  std::cout << "cv identity matrix: " << matAsString << std::endl;

  MITK_TEST_CONDITION_REQUIRED( trace(cvI)
                                == cv::Scalar(4)
                  , "trace(t->GetCvMatrix()) == cv::Scalar(4)" );
  MITK_TEST_CONDITION_REQUIRED( countNonZero(t->GetCvMatrix()) == 4
                  , "countNonZero(t->GetCvMatrix()) == 4" );

  // test 2: 1. create a rotation matrix,convert to quaternion
  // set as rotation vector
  // get it back as vnl rotation matrix
  // convert to cv matrix
  // convert to quaternion (cv::Rodrigues)
  // compare received quaternion with original one (created from rotation matrix)
  cv::Mat cvRotMat = cv::Mat::ones( 3, 3, cv::DataType<mitk::ScalarType>::type );
  cvRotMat.at<mitk::ScalarType>(0,1) = 2;
  cvRotMat.at<mitk::ScalarType>(0,2) = 3;

  cv::Mat cvRotVec;
  cv::Rodrigues( cvRotMat, cvRotVec );
  t->SetRotationVector( cvRotVec );

  vnl_matrix_fixed<mitk::ScalarType, 3, 3> vnlRotMat
      = t->GetVnlRotationMatrix();
  std::cout << "vnl rotation matrix: "
      << vnlRotMat << std::endl;

  vnl_matrix<mitk::ScalarType> rotMat = vnlRotMat.as_matrix();
  cv::Mat cvRotMatReturned;
  mitk::CvMatFromVnlMatrix<mitk::ScalarType>
      _CvMatFromVnlMatrix( &rotMat, &cvRotMatReturned );
  _CvMatFromVnlMatrix.Update();

  _StringFromCvMat.SetMatrix( &cvRotMatReturned );
  _StringFromCvMat.Update();
  std::cout << "cvRotMatReturned: " << matAsString << std::endl;

  cv::Mat cvRotVecReturned;
  cv::Rodrigues( cvRotMatReturned, cvRotVecReturned );

  _StringFromCvMat.SetMatrix( &cvRotVec );
  _StringFromCvMat.Update();
  std::cout << "cvRotVec: " << matAsString << std::endl;
  _StringFromCvMat.SetMatrix( &cvRotVecReturned );
  _StringFromCvMat.Update();
  std::cout << "cvRotVecReturned: " << matAsString << std::endl;

  double epsilon = 0.001;
  bool equals = false;
  mitk::CvMatCompare _CvMatCompare( &cvRotVec, &cvRotVecReturned, &epsilon, &equals );
  _CvMatCompare.Update();
  MITK_TEST_CONDITION( equals,
                         "testing returned rotation vector");

  std::cout << "Transform as string: " << transform << std::endl;

  // always end with this!
  MITK_TEST_END();
}




