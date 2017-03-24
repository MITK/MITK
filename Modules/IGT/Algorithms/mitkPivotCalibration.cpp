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

#include "mitkPivotCalibration.h"
#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"

mitk::PivotCalibration::PivotCalibration() : m_NavigationDatas(std::vector<mitk::NavigationData::Pointer>()), m_ResultPivotRotation(mitk::Quaternion(0,0,0,1))
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
  for( size_t i = 0; i < m_NavigationDatas.size(); ++i )
    {
      if( !m_NavigationDatas.at(i)->IsDataValid() )
      {
        MITK_WARN << "Skipping invalid transform " << i << ".";
        continue;
      }
      _CheckedTransforms.push_back( m_NavigationDatas.at(i) );
    }

  if(_CheckedTransforms.empty())
    {
      MITK_WARN << "Checked Transforms are empty";
      return false;
    }

  unsigned int rows = 3*_CheckedTransforms.size();
  unsigned int columns = 6;

  vnl_matrix< double > A( rows, columns ), minusI( 3, 3, 0 ), R( 3, 3 );
  vnl_vector< double > b(rows), x(columns), t(3);

  minusI( 0, 0 ) = -1;
  minusI( 1, 1 ) = -1;
  minusI( 2, 2 ) = -1;

  //do the computation and set the internal variables
  unsigned int currentRow = 0;
  for( size_t i = 0; i < _CheckedTransforms.size(); ++i )
  {
    if( !_CheckedTransforms.at(i)->IsDataValid() )
    {
      MITK_WARN << "skipping invalid transform " << i << ".";
      continue;
    }
    t = _CheckedTransforms.at(i)->GetPosition().Get_vnl_vector();
    t *= -1;
    b.update( t, currentRow );
    R = _CheckedTransforms.at(i)->GetOrientation().rotation_matrix_transpose();
    A.update( R, currentRow, 0 );
    A.update( minusI, currentRow, 3 );
    currentRow += 3;
  }
  vnl_svd<double> svdA(A);
  svdA.zero_out_absolute( defaultThreshold );

  //there is a solution only if rank(A)=6 (columns are linearly
  //independent)
  if( svdA.rank() < 6 )
  {
    MITK_WARN << "svdA.rank() < 6";
    return false;
  }
  else
  {
    x = svdA.solve( b );

    m_ResultRMSError = ( A * x - b ).rms();

    //set the transformation
    m_ResultPivotPoint[0] = x[0];
    m_ResultPivotPoint[1] = x[1];
    m_ResultPivotPoint[2] = x[2];

  }
  return true;

}
