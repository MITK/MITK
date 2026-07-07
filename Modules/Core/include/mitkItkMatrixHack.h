/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkItkMatrixHack_h
#define mitkItkMatrixHack_h

namespace mitk
{
  /**
   * \brief Internal hack to set m_MatrixMTime of itk::MatrixOffsetTransformBase
   *        correctly after changing the matrix. For internal use only.
   *
   * Usage: static_cast an object of type itk::MatrixOffsetTransformBase
   * (or derived from it) to this class and call MatrixChanged().
   * itk::MatrixOffsetTransformBase::SetParameters does not set
   * m_MatrixMTime, thus m_InverseMatrixMTime is the same
   * as m_MatrixMTime and the inverse is not recalculated.
   *
   * \tparam TTransformType The itk transform type to hack.
   *
   * \warning Use with care!
   * \ingroup Geometry
   */
  template <class TTransformType>
  class ItkMatrixHack : public TTransformType
  {
  public:
    /** \brief Triggers a recalculation of the inverse matrix by calling SetVarMatrix. */
    void MatrixChanged() { this->SetVarMatrix(this->GetMatrix()); }
  };

} // namespace mitk

#endif
