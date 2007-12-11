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


#ifndef ITKMATRIXHACK_H_HEADER_INCLUDED_C1EBD0AD
#define ITKMATRIXHACK_H_HEADER_INCLUDED_C1EBD0AD

namespace mitk {

//##Documentation
//## @brief Internal hack to set m_MatrixMTime of
//## itk::MatrixOffsetTransformBase correctly after changing 
//## the matrix. For internal use only.
//##
//## Usage: static_cast object of type itk::MatrixOffsetTransformBase 
//## (or derived from this) to this and call MatrixChanged().
//## Primary reason for this class: ITK 2.2.0 has a bug: 
//## itk::MatrixOffsetTransformBase::SetParameters does not set 
//## m_MatrixMTime thus m_InverseMatrixMTime is the same 
//## as m_MatrixMTime and the inverse is not recalculated.
//## @warning Use with care!
//## @ingroup Geometry
template <class TTransformType>
class ItkMatrixHack : public TTransformType
{
public:
  void MatrixChanged()
  {
#if (ITK_VERSION_MAJOR == 2 && ITK_VERSION_MINOR < 2)
    this->m_MatrixMTime.Modified();
#else
    this->SetVarMatrix(this->GetMatrix());
#endif
  }
};

} // namespace mitk

#endif /* ITKMATRIXHACK_H_HEADER_INCLUDED_C1EBD0AD */
