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

#ifndef MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD
#define MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkBaseGeometry.h"
#include "mitkItkMatrixHack.h"
#include <vtkMatrix4x4.h>

namespace mitk
{
  template <class TTransformType>
  void TransferVtkMatrixToItkTransform(const vtkMatrix4x4 *vtkmatrix, TTransformType *itkTransform)
  {
    if (itkTransform == nullptr)
      return;

    typename TTransformType::MatrixType::InternalMatrixType &vnlMatrix =
      const_cast<typename TTransformType::MatrixType::InternalMatrixType &>(itkTransform->GetMatrix().GetVnlMatrix());

    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        vnlMatrix[i][j] = vtkmatrix->GetElement(i, j);
    // *This* ensures m_MatrixMTime.Modified(), which is therewith not equal to
    // m_InverseMatrixMTime, thus a new inverse will be calculated (when
    // requested).
    static_cast<mitk::ItkMatrixHack<TTransformType> *>(itkTransform)->MatrixChanged();

    typename TTransformType::OffsetType offset;
    offset[0] = vtkmatrix->GetElement(0, 3);
    offset[1] = vtkmatrix->GetElement(1, 3);
    offset[2] = vtkmatrix->GetElement(2, 3);
    itkTransform->SetOffset(offset);
  }

  template <class TTransformType>
  void TransferItkTransformToVtkMatrix(const TTransformType *itkTransform, vtkMatrix4x4 *vtkmatrix)
  {
    int i, j;
    for (i = 0; i < 3; ++i)
      for (j = 0; j < 3; ++j)
        vtkmatrix->SetElement(i, j, itkTransform->GetMatrix().GetVnlMatrix().get(i, j));
    for (i = 0; i < 3; ++i)
      vtkmatrix->SetElement(i, 3, itkTransform->GetOffset()[i]);
    for (i = 0; i < 3; ++i)
      vtkmatrix->SetElement(3, i, 0.0);
    vtkmatrix->SetElement(3, 3, 1);
  }

  template <class TTransformType1, class TTransformType2>
  void ConvertItkTransform(const TTransformType1 *sourceTransform, TTransformType2 *destTransform)
  {
    if ((sourceTransform == nullptr) || (destTransform == nullptr))
      return;

    destTransform->SetMatrix(sourceTransform->GetMatrix());
    destTransform->SetOffset(sourceTransform->GetOffset());
  }

  template <class TMatrixType>
  void GetRotation(const mitk::BaseGeometry *geometry, TMatrixType &itkmatrix)
  {
    const mitk::Vector3D &spacing = geometry->GetSpacing();
    auto &geometryVnlMatrix = geometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();

    typename TMatrixType::InternalMatrixType &outputVnlMatrix =
      itkmatrix.GetVnlMatrix();

    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        outputVnlMatrix[i][j] = geometryVnlMatrix[i][j] / spacing[j];
  }

  template <class TTransformType>
  void GetWorldToItkPhysicalTransform(const mitk::BaseGeometry *geometry, TTransformType *itkTransform)
  {
    if (itkTransform == nullptr)
      return;

    // get rotation matrix and offset from Geometry and transfer in TTransformType types
    typename TTransformType::MatrixType rotationMatrix;
    GetRotation(geometry, rotationMatrix);

    const typename mitk::BaseGeometry::TransformType::OffsetType &geometryOffset =
      geometry->GetIndexToWorldTransform()->GetOffset();

    vnl_vector<typename TTransformType::MatrixType::ValueType> vnlOffset(3);
    vnlOffset[0] = geometryOffset[0];
    vnlOffset[1] = geometryOffset[1];
    vnlOffset[2] = geometryOffset[2];

    // do calculations
    typename TTransformType::MatrixType::InternalMatrixType inverseRotationVnlMatrix = rotationMatrix.GetTranspose();

    vnlOffset -= inverseRotationVnlMatrix * vnlOffset;

    typename TTransformType::OutputVectorType offset; // vnl_vector<TTransformType::MatrixType::ValueType> offset;
    offset[0] = vnlOffset[0];
    offset[1] = vnlOffset[1];
    offset[2] = vnlOffset[2];
    itkTransform->SetOffset(offset);

    // copy in destination itkTransform
    typename TTransformType::MatrixType::InternalMatrixType &destVnlMatrix =
      itkTransform->GetMatrix().GetVnlMatrix();

    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        destVnlMatrix[i][j] = inverseRotationVnlMatrix[i][j];
    // *This* ensures m_MatrixMTime.Modified(), which is therewith not equal to
    // m_InverseMatrixMTime, thus a new inverse will be calculated (when
    // requested).
    static_cast<mitk::ItkMatrixHack<TTransformType> *>(itkTransform)->MatrixChanged();
  }
}

#endif /* MITKMATRIXCONVERT_H_HEADER_INCLUDED_C1EBD0AD */
