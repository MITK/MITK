/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkStaticIGTHelperFunctions.h>
#include <itkVector.h>

double mitk::StaticIGTHelperFunctions::GetAngleBetweenTwoQuaterions(mitk::Quaternion a, mitk::Quaternion b, itk::Vector<double,3> rotationVector)
  {
  double returnValue;

  itk::Vector<double,3> point; //caution 5D-Tools: correct vector along the tool axis is needed
  point[0] = rotationVector[0];
  point[1] = rotationVector[1];
  point[2] = rotationVector[2];

  //Quaternions used for rotations should alway be normalized, so just to be safe:
  a.normalize();
  b.normalize();

  itk::Matrix<double,3,3> rotMatrixA;
  for(int i=0; i<3; i++) for(int j=0; j<3; j++) rotMatrixA[i][j] = a.rotation_matrix_transpose().transpose()[i][j];

  itk::Matrix<double,3,3> rotMatrixB;
  for(int i=0; i<3; i++) for(int j=0; j<3; j++) rotMatrixB[i][j] = b.rotation_matrix_transpose().transpose()[i][j];

  itk::Vector<double,3> pt1 = rotMatrixA * point;
  itk::Vector<double,3> pt2 = rotMatrixB * point;

  returnValue = (pt1[0]*pt2[0]+pt1[1]*pt2[1]+pt1[2]*pt2[2]) / ( sqrt(pow(pt1[0],2.0)+pow(pt1[1],2.0)+pow(pt1[2],2.0)) * sqrt(pow(pt2[0],2.0)+pow(pt2[1],2.0)+pow(pt2[2],2.0)));
  returnValue = acos(returnValue) * 57.296; //57,296 = 180/Pi ; conversion to degrees

  return returnValue;
  }

double mitk::StaticIGTHelperFunctions::GetAngleBetweenTwoQuaterions(mitk::Quaternion a, mitk::Quaternion b)
{
  //formula returnValue = 2 * acos ( a <dotproduct> b )
  //(+ normalization because we need unit quaternions)
  //derivation from here: https://fgiesen.wordpress.com/2013/01/07/small-note-on-quaternion-distance-metrics/
  double returnValue = ((a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]) / (sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2] + a[3] * a[3])*sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2] + b[3] * b[3])));
  returnValue = 2 * acos(returnValue);
  return returnValue;
}

itk::Matrix<double,3,3> mitk::StaticIGTHelperFunctions::ConvertEulerAnglesToRotationMatrix(double alpha, double beta, double gamma)
{
    double PI = 3.141592653589793;
    alpha = alpha * PI / 180;
    beta = beta * PI / 180;
    gamma = gamma * PI / 180;

    //convert angles to matrix:
    itk::Matrix<double,3,3> matrix;

    /* x-Konvention (Z, X, Z)
    matrix[0][0] = cos(alpha) * cos(gamma) - sin(alpha) * cos(beta) * sin(gamma);
    matrix[0][1] = -cos(alpha) * sin(gamma)- sin(alpha) * cos(beta) * cos(gamma);
    matrix[0][2] = sin(alpha) * sin(beta);

    matrix[1][0] = sin(alpha) * cos(gamma) + cos(alpha) * cos(beta) * sin(gamma);
    matrix[1][1] = cos(alpha) * cos(beta) * cos(gamma) - sin(alpha) * sin(gamma);
    matrix[1][2] = -cos(alpha) * sin(beta);

    matrix[2][0] = sin(beta) * sin(gamma);
    matrix[2][1] = sin(beta) * cos(gamma);
    matrix[2][2] = cos(beta);
    */

    //Luftfahrtnorm (DIN 9300) (Yaw-Pitch-Roll, Z, Y, X)
    matrix[0][0] = cos(beta) * cos(alpha);
    matrix[0][1] = cos(beta) * sin(alpha);
    matrix[0][2] = -sin(beta);

    matrix[1][0] = sin(gamma) * sin(beta) * cos(alpha) - cos(gamma) * sin(alpha) ;
    matrix[1][1] = sin(gamma) * sin(beta) * sin(alpha) + cos(gamma) * cos(alpha);
    matrix[1][2] = sin(gamma) * cos(beta);

    matrix[2][0] = cos(gamma) * sin(beta) * cos(alpha) + sin(gamma) * sin(alpha);
    matrix[2][1] = cos(gamma) * sin(beta) * sin(alpha) - sin(gamma) * cos(alpha);
    matrix[2][2] = cos(gamma) * cos(beta);

    return matrix;
}

double mitk::StaticIGTHelperFunctions::ComputeFRE(mitk::PointSet::Pointer imageFiducials, mitk::PointSet::Pointer realWorldFiducials, vtkSmartPointer<vtkLandmarkTransform> transform)
{
  if (imageFiducials->GetSize() != realWorldFiducials->GetSize())
  {
    MITK_WARN << "Cannot compute FRE, got different numbers of points (1: " << imageFiducials->GetSize() << " /2: " << realWorldFiducials->GetSize() << ")";
    return -1;
  }
  double FRE = 0;
  for (int i = 0; i < imageFiducials->GetSize(); i++)
  {
    itk::Point<double> current_image_fiducial_point = imageFiducials->GetPoint(i);
    if (transform != nullptr)
    {
      current_image_fiducial_point = transform->TransformPoint(imageFiducials->GetPoint(i)[0], imageFiducials->GetPoint(i)[1], imageFiducials->GetPoint(i)[2]);
    }
    double cur_error_squared = current_image_fiducial_point.SquaredEuclideanDistanceTo(realWorldFiducials->GetPoint(i));
    FRE += cur_error_squared;
  }

  FRE = sqrt(FRE / (double)imageFiducials->GetSize());

  return FRE;
}
