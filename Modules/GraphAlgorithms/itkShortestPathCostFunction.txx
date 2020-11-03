/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __itkShortestPathCostFunction_txx
#define __itkShortestPathCostFunction_txx

#include "itkShortestPathCostFunction.h"

namespace itk
{
  template <class TInputImageType>
  void ShortestPathCostFunction<TInputImageType>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

  template <class TInputImageType>
  void ShortestPathCostFunction<TInputImageType>::SetStartIndex(const typename TInputImageType::IndexType &index)
  {
    for (unsigned int i = 0; i < TInputImageType::ImageDimension; ++i)
    {
      m_StartIndex[i] = index[i];
    }
  }

  template <class TInputImageType>
  void ShortestPathCostFunction<TInputImageType>::SetEndIndex(const typename TInputImageType::IndexType &index)
  {
    for (unsigned int i = 0; i < TInputImageType::ImageDimension; ++i)
    {
      m_EndIndex[i] = index[i];
    }
  }

} // end namespace itk

#endif // __itkShortestPathCostFunction_txx
