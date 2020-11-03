/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ITKMATRIXHACK_H_HEADER_INCLUDED_C1EBD0AD
#define ITKMATRIXHACK_H_HEADER_INCLUDED_C1EBD0AD

namespace mitk
{
  //##Documentation
  //## @brief Internal hack to set m_MatrixMTime of
  //## itk::MatrixOffsetTransformBase correctly after changing
  //## the matrix. For internal use only.
  //##
  //## Usage: static_cast object of type itk::MatrixOffsetTransformBase
  //## (or derived from this) to this and call MatrixChanged().
  //## itk::MatrixOffsetTransformBase::SetParameters does not set
  //## m_MatrixMTime thus m_InverseMatrixMTime is the same
  //## as m_MatrixMTime and the inverse is not recalculated.
  //## @warning Use with care!
  //## @ingroup Geometry
  template <class TTransformType>
  class ItkMatrixHack : public TTransformType
  {
  public:
    void MatrixChanged() { this->SetVarMatrix(this->GetMatrix()); }
  };

} // namespace mitk

#endif /* ITKMATRIXHACK_H_HEADER_INCLUDED_C1EBD0AD */
