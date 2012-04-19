/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToDiffusionImageFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_FreeWaterEliminationFilter_txx_
#define _itk_TensorReconstructionWithEigenvalueCorrectioFiltern_txx_
#endif
#include "itkImageRegionConstIterator.h"
#include <math.h>
#include <mitkImageWriter.h>

namespace itk
{


  template <class TDiffusionPixelType, class TTensorPixelType>
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::FreeWaterEliminationFilter()
  {
    m_B0Threshold = 50.0;
  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::GenerateData ( )
  {
    std::cout << "eliminate free water" << std::endl;


  }
  
  


   
} // end of namespace



  

  

