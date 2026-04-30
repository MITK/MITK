/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMatrixConvert_h
#define mitkMatrixConvert_h

#include <mitkBaseGeometry.h>
#include <mitkItkMatrixHack.h>
#include <vtkMatrix4x4.h>

namespace mitk
{
  /**
   * \brief Transfer a VTK 4x4 matrix into an ITK transform.
   *
   * Copies the 3x3 rotation submatrix and the translation offset from
   * a vtkMatrix4x4 into the given ITK transform. The transform's internal
   * matrix modification time is updated so that the inverse will be
   * recomputed on the next request.
   *
   * \tparam TTransformType The ITK transform type (e.g. AffineTransform3D).
   * \param vtkmatrix The source VTK 4x4 matrix.
   * \param itkTransform The destination ITK transform. If nullptr, the function returns immediately.
   *
   * \sa TransferItkTransformToVtkMatrix
   * \sa ItkMatrixHack
   */
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

  /**
   * \brief Transfer an ITK transform into a VTK 4x4 matrix.
   *
   * Copies the 3x3 rotation submatrix and the translation offset from
   * the given ITK transform into a vtkMatrix4x4. The bottom row is set
   * to [0, 0, 0, 1].
   *
   * \tparam TTransformType The ITK transform type (e.g. AffineTransform3D).
   * \param itkTransform The source ITK transform.
   * \param vtkmatrix The destination VTK 4x4 matrix.
   *
   * \sa TransferVtkMatrixToItkTransform
   */
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

  /**
   * \brief Convert between two ITK transform types by copying matrix and offset.
   *
   * \tparam TTransformType1 The source ITK transform type.
   * \tparam TTransformType2 The destination ITK transform type.
   * \param sourceTransform The source transform. If nullptr, the function returns immediately.
   * \param destTransform The destination transform. If nullptr, the function returns immediately.
   *
   * \sa TransferVtkMatrixToItkTransform
   * \sa TransferItkTransformToVtkMatrix
   */
  template <class TTransformType1, class TTransformType2>
  void ConvertItkTransform(const TTransformType1 *sourceTransform, TTransformType2 *destTransform)
  {
    if ((sourceTransform == nullptr) || (destTransform == nullptr))
      return;

    destTransform->SetMatrix(sourceTransform->GetMatrix());
    destTransform->SetOffset(sourceTransform->GetOffset());
  }

  /**
   * \brief Extract the rotation matrix from a geometry, removing spacing.
   *
   * Divides each column of the geometry's IndexToWorld matrix by the
   * corresponding spacing component to obtain a pure rotation matrix.
   *
   * \tparam TMatrixType The ITK matrix type for the output.
   * \param geometry The source geometry from which to extract the rotation.
   * \param itkmatrix The output matrix that receives the rotation.
   *
   * \sa GetWorldToItkPhysicalTransform
   */
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

  /**
   * \brief Compute the world-to-ITK-physical-space transform from a geometry.
   *
   * Extracts the rotation from the geometry (with spacing removed), computes
   * its inverse (transpose), and combines it with the geometry's offset to
   * produce a transform from MITK world coordinates to ITK physical coordinates.
   *
   * \tparam TTransformType The ITK transform type (e.g. AffineTransform3D).
   * \param geometry The source geometry.
   * \param itkTransform The output transform. If nullptr, the function returns immediately.
   *
   * \sa GetRotation
   * \sa TransferVtkMatrixToItkTransform
   */
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

#endif
