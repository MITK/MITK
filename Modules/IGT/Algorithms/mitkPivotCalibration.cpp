/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPivotCalibration.h"
#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"
#include <vtkMatrix4x4.h>

mitk::PivotCalibration::PivotCalibration() : m_NavigationDatas(std::vector<mitk::NavigationData::Pointer>()), m_ResultPivotPoint(mitk::Point3D(0.0))
{


}

mitk::PivotCalibration::~PivotCalibration()
{

}

void mitk::PivotCalibration::AddNavigationData(mitk::NavigationData::Pointer data)
{
  m_NavigationDatas.push_back(data);
}

bool mitk::PivotCalibration::ComputePivotResult()
{
  return ComputePivotPoint();
}

bool mitk::PivotCalibration::ComputePivotPoint()
{
  double defaultThreshold = 1e-1;

  std::vector<mitk::NavigationData::Pointer> _CheckedTransforms;
  for (size_t i = 0; i < m_NavigationDatas.size(); ++i)
  {
    if (!m_NavigationDatas.at(i)->IsDataValid())
    {
      MITK_WARN << "Skipping invalid transform " << i << ".";
      continue;
    }
    _CheckedTransforms.push_back(m_NavigationDatas.at(i));
  }

  if (_CheckedTransforms.empty())
  {
    MITK_WARN << "Checked Transforms are empty";
    return false;
  }

  unsigned int rows = 3 * _CheckedTransforms.size();
  unsigned int columns = 6;

  vnl_matrix< double > A(rows, columns), minusI(3, 3, 0), R(3, 3);
  vnl_vector< double > b(rows), x(columns), t(3);

  minusI(0, 0) = -1;
  minusI(1, 1) = -1;
  minusI(2, 2) = -1;

  //do the computation and set the internal variables
  unsigned int currentRow = 0;


  for (size_t i = 0; i < _CheckedTransforms.size(); ++i)
  {
    t = _CheckedTransforms.at(i)->GetPosition().GetVnlVector();// t = the current position of the tracked sensor
    t *= -1;
    b.update(t, currentRow); //b = combines the position for each collected transform in one column vector
    R = _CheckedTransforms.at(i)->GetOrientation().rotation_matrix_transpose().transpose(); // R = the current rotation of the tracked sensor, *rotation_matrix_transpose().transpose() is used to obtain original matrix
    A.update(R, currentRow, 0); //A = the matrix which stores the rotations for each collected transform and -I
    A.update(minusI, currentRow, 3);
    currentRow += 3;
  }
  vnl_svd<double> svdA(A); //The singular value decomposition of matrix A
  svdA.zero_out_absolute(defaultThreshold);

  //there is a solution only if rank(A)=6 (columns are linearly
  //independent)
  if (svdA.rank() < 6)
  {
    MITK_WARN << "svdA.rank() < 6";
    return false;
  }
  else
  {
    x = svdA.solve(b); //x = the resulting pivot point

    m_ResultRMSError = (A * x - b).rms();  //the root mean sqaure error of the computation

    //sets the Pivot Point
    m_ResultPivotPoint[0] = x[0];
    m_ResultPivotPoint[1] = x[1];
    m_ResultPivotPoint[2] = x[2];


  }
  return true;

}
