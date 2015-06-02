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
#include <mitkCommon.h>
#include <itkMatrix.h>
#include <mitkQuaternion.h>
#include "MitkIGTBaseExports.h"

namespace mitk {
  class MITKIGTBASE_EXPORT StaticIGTHelperFunctions
  {
  public:
  /** Computes the angle in the plane perpendicular to the rotation axis of the two quaterions.
   *  Therefore, a vector is rotated with the difference of both rotations and the angle is computed.
   *  In some cases you might want to defice this vector e.g., if working with 5D tools. By default
   *  the vector is defined along the Z-axis which works for Aurora 5D tools.
   *  @return Returns the angle in degrees.
   **/
  static double GetAngleBetweenTwoQuaterions(mitk::Quaternion a, mitk::Quaternion b, itk::Vector<double,3> rotationVector);

  /** Computes the angle in the plane perpendicular to the rotation axis of the two quaterions.
   *  Therefore, a vector is rotated with the difference of both rotations and the angle is computed.
   *  In some cases you might want to defice this vector e.g., if working with 5D tools. By default
   *  the vector is defined along the Z-axis which works for Aurora 5D tools.
   *  @return Returns the angle in degrees.
   **/
  static double GetAngleBetweenTwoQuaterions(mitk::Quaternion a, mitk::Quaternion b);

  /** Converts euler angles (in degrees) to a rotation matrix. */
  static itk::Matrix<double,3,3> ConvertEulerAnglesToRotationMatrix(double alpha, double beta, double gamma);

  };
}
