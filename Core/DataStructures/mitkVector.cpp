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

#include "mitkVector.h"
#include "mitkItkMatrixHack.h"
#include <vtkSystemIncludes.h>
#include <vtkMatrix4x4.h>

const mitk::ScalarType mitk::eps     = vnl_math::float_eps*100;
const mitk::ScalarType mitk::sqrteps = vnl_math::float_sqrteps;
extern const double mitk::large      = VTK_LARGE_FLOAT;

void mitk::TransferVtkMatrixToItkTransform(const vtkMatrix4x4* vtkmatrix, mitk::AffineTransform3D* itkTransform)
{
  if(itkTransform==NULL)
    return;
  
  itk::Matrix<mitk::ScalarType,3,3>::InternalMatrixType& vnlMatrix = 
    const_cast<itk::Matrix<mitk::ScalarType,3,3>::InternalMatrixType&>(itkTransform->GetMatrix().GetVnlMatrix());

  for ( int i=0; i < 3; ++i)
    for( int j=0; j < 3; ++j )
      vnlMatrix[i][j] = vtkmatrix->GetElement( i, j );      

  // *This* ensures m_MatrixMTime.Modified(), which is therewith not equal to 
  // m_InverseMatrixMTime, thus a new inverse will be calculated (when
  // requested).
  static_cast<mitk::ItkMatrixHack*>(itkTransform)->MatrixChanged();

  itk::AffineTransform<mitk::ScalarType>::OffsetType offset;
  offset[0] = vtkmatrix->GetElement( 0, 3 );
  offset[1] = vtkmatrix->GetElement( 1, 3 );
  offset[2] = vtkmatrix->GetElement( 2, 3 );
  itkTransform->SetOffset( offset );
}

void mitk::TransferItkTransformToVtkMatrix(const mitk::AffineTransform3D* itkTransform, vtkMatrix4x4* vtkmatrix)
{
  int i,j;
  for(i=0;i<3;++i)
    for(j=0;j<3;++j)
      vtkmatrix->SetElement(i, j, itkTransform->GetMatrix().GetVnlMatrix().get(i, j));
  for(i=0;i<3;++i)
    vtkmatrix->SetElement(i, 3, itkTransform->GetOffset()[i]);
  vtkmatrix->Modified();
}
