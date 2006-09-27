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


#ifndef MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD
#define MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkItkMatrixHack.h"
#include <vtkMatrix4x4.h>

namespace mitk
{
  template <class TTransformType>
  void TransferVtkMatrixToItkTransform(const vtkMatrix4x4* vtkmatrix, TTransformType * itkTransform)
  {
  if(itkTransform==NULL)
    return;

  typename TTransformType::MatrixType::InternalMatrixType& vnlMatrix =
    const_cast<typename TTransformType::MatrixType::InternalMatrixType&>(itkTransform->GetMatrix().GetVnlMatrix());

  for ( int i=0; i < 3; ++i)
    for( int j=0; j < 3; ++j )
      vnlMatrix[i][j] = vtkmatrix->GetElement( i, j );    
  // *This* ensures m_MatrixMTime.Modified(), which is therewith not equal to
  // m_InverseMatrixMTime, thus a new inverse will be calculated (when
  // requested).
  static_cast<mitk::ItkMatrixHack<TTransformType>*>(itkTransform)->MatrixChanged();

  typename TTransformType::OffsetType offset;
  offset[0] = vtkmatrix->GetElement( 0, 3 );
  offset[1] = vtkmatrix->GetElement( 1, 3 );
  offset[2] = vtkmatrix->GetElement( 2, 3 );
  itkTransform->SetOffset( offset );
  }

  template <class TTransformType>
  void TransferItkTransformToVtkMatrix(const TTransformType * itkTransform, vtkMatrix4x4* vtkmatrix)
  {
    int i,j;
    for(i=0;i<3;++i)
      for(j=0;j<3;++j)
        vtkmatrix->SetElement(i, j, itkTransform->GetMatrix().GetVnlMatrix().get(i, j));
    for(i=0;i<3;++i)
      vtkmatrix->SetElement(i, 3, itkTransform->GetOffset()[i]);
    vtkmatrix->Modified();
  }
}

#endif /* MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD */
