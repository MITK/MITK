/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkQuaternionAveraging.h"
//#include "bussvrmath/Quaternion.h"
//#include "SphereMean.h"

mitk::QuaternionAveraging::QuaternionAveraging()
{


}

mitk::QuaternionAveraging::~QuaternionAveraging()
{

}

mitk::Quaternion mitk::QuaternionAveraging::CalcAverage(const std::vector<Quaternion>& quaternions, Mode mode)
{
mitk::Quaternion result;
switch (mode)
  {
  case SimpleMean:
  for (std::vector<Quaternion>::size_type i=0; i<quaternions.size(); i++)
    {
    result[0] += quaternions.at(i)[0];
    result[1] += quaternions.at(i)[1];
    result[2] += quaternions.at(i)[2];
    result[3] += quaternions.at(i)[3];
    }
  result[0] /= quaternions.size();
  result[1] /= quaternions.size();
  result[2] /= quaternions.size();
  result[3] /= quaternions.size();
  result.normalize();
  break;

  case Slerp:
  MITK_WARN << "SLERP is not implemented yet!";
  /* To avtivate this code, the vrMath classes from Sam Buss need to be included
  BussQuaternion* bussQuaternions = new BussQuaternion[quaternions.size()];
  double* wheights = new double[quaternions.size()];
  for (int i=0; i < quaternions.size(); i++) {
    bussQuaternions[i] = BussQuaternion(quaternions.at(i).x(), quaternions.at(i).y(), quaternions.at(i).z(), quaternions.at(i).r());
    wheights[i]=1;
  }
  BussQuaternion bussResult = ComputeMeanQuat(5,bussQuaternions, wheights);
  mitk::Quaternion resultSlerp(bussResult.x,bussResult.y,bussResult.z,bussResult.w);
  result = resultSlerp;
  */
  break;
  }

  return result;
}
