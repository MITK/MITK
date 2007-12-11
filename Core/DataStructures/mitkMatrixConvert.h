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


#ifndef MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD
#define MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkGeometry3D.h"
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

  template <class TTransformType1, class TTransformType2>
  void ConvertItkTransform(const TTransformType1* sourceTransform, TTransformType2* destTransform)
  {
    if((sourceTransform==NULL) || (destTransform==NULL))
      return;

    // transfer offset
    const typename TTransformType1::OutputVectorType& sourceOffset =
      sourceTransform->GetOffset();

    typename TTransformType2::OutputVectorType offset;
    offset[0] = sourceOffset[0]; offset[1] = sourceOffset[1]; offset[2] = sourceOffset[2];
    destTransform->SetOffset( offset );

    typename TTransformType1::MatrixType::InternalMatrixType& sourceVnlMatrix =
      const_cast<typename TTransformType1::MatrixType::InternalMatrixType&>(sourceTransform->GetMatrix().GetVnlMatrix());

    //transfer matrix
    typename TTransformType2::MatrixType::InternalMatrixType& destVnlMatrix =
      const_cast<typename TTransformType2::MatrixType::InternalMatrixType&>(destTransform->GetMatrix().GetVnlMatrix());

    for ( int i=0; i < 3; ++i)
      for( int j=0; j < 3; ++j )
        destVnlMatrix[i][j] = sourceVnlMatrix[i][j];    

    // *This* ensures m_MatrixMTime.Modified(), which is therewith not equal to
    // m_InverseMatrixMTime, thus a new inverse will be calculated (when
    // requested).
    static_cast<mitk::ItkMatrixHack<TTransformType2>*>(destTransform)->MatrixChanged();
  }

  template <class TMatrixType>
  void GetRotation(const mitk::Geometry3D * geometry, TMatrixType& itkmatrix)
  {
    const mitk::Vector3D& spacing = geometry->GetSpacing();
    typename mitk::Geometry3D::TransformType::MatrixType::InternalMatrixType& geometryVnlMatrix =
      const_cast<typename mitk::Geometry3D::TransformType::MatrixType::InternalMatrixType&>(geometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix());

    typename TMatrixType::InternalMatrixType& outputVnlMatrix =
      const_cast<typename TMatrixType::InternalMatrixType&>(itkmatrix.GetVnlMatrix());

    for ( int i=0; i < 3; ++i)
      for( int j=0; j < 3; ++j )
        outputVnlMatrix [i][j] = geometryVnlMatrix [i][j] / spacing[j];
  }

  template <class TTransformType>
  void GetWorldToItkPhysicalTransform(const mitk::Geometry3D * geometry, TTransformType* itkTransform)
  {
    if(itkTransform==NULL)
      return;

    // get rotation matrix and offset from Geometry and transfer in TTransformType types
    typename TTransformType::MatrixType rotationMatrix;
    GetRotation(geometry, rotationMatrix);

    const typename mitk::Geometry3D::TransformType::OffsetType& geometryOffset =
      geometry->GetIndexToWorldTransform()->GetOffset();

    vnl_vector<typename TTransformType::MatrixType::ValueType> vnlOffset(3);
    vnlOffset[0] = geometryOffset[0]; vnlOffset[1] = geometryOffset[1]; vnlOffset[2] = geometryOffset[2];

    // do calculations
    typename TTransformType::MatrixType::InternalMatrixType inverseRotationVnlMatrix = rotationMatrix.GetTranspose();

    vnlOffset -= inverseRotationVnlMatrix*vnlOffset;

    typename TTransformType::OutputVectorType offset;//vnl_vector<TTransformType::MatrixType::ValueType> offset;
    offset[0] = vnlOffset[0]; offset[1] = vnlOffset[1]; offset[2] = vnlOffset[2];
    itkTransform->SetOffset( offset );

    // copy in destination itkTransform
    typename TTransformType::MatrixType::InternalMatrixType& destVnlMatrix =
      const_cast<typename TTransformType::MatrixType::InternalMatrixType&>(itkTransform->GetMatrix().GetVnlMatrix());

    for ( int i=0; i < 3; ++i)
      for( int j=0; j < 3; ++j )
        destVnlMatrix[i][j] = inverseRotationVnlMatrix[i][j];    
    // *This* ensures m_MatrixMTime.Modified(), which is therewith not equal to
    // m_InverseMatrixMTime, thus a new inverse will be calculated (when
    // requested).
    static_cast<mitk::ItkMatrixHack<TTransformType>*>(itkTransform)->MatrixChanged();
  }

}

#endif /* MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD */
