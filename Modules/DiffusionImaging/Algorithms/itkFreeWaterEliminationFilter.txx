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
#define _itk_FreeWaterEliminationFilter_txx_
#endif

#include "itkFreeWaterEliminationFilter.h"


namespace itk
{


  template <class TDiffusionPixelType, class TTensorPixelType>
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::FreeWaterEliminationFilter()
  {

  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::BeforeThreadedGenerateData()
  {



  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::ThreadedGenerateData ( const OutputImageRegionType &outputRegionForThread, int threadId )
  {


  }



  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::SetGradientImage( GradientDirectionContainerType *gradientDirection,
                          const GradientImagesType *gradientImage )
  {
    // Make sure crazy users did not call both AddGradientImage and
    // SetGradientImage
    if( m_GradientImageTypeEnumeration == GradientIsInManyImages )
      {
      itkExceptionMacro( << "Cannot call both methods:"
      << "AddGradientImage and SetGradientImage. Please call only one of them.");
      }

    this->m_GradientDirectionContainer = gradientDirection;

    unsigned int numImages = gradientDirection->Size();
    this->m_NumberOfBaselineImages = 0;
    for(GradientDirectionContainerType::Iterator it = this->m_GradientDirectionContainer->Begin();
        it != this->m_GradientDirectionContainer->End(); it++)
      {
      if(it.Value().one_norm() <= 0.0)
        {
        this->m_NumberOfBaselineImages++;
        }
      else // Normalize non-zero gradient directions
        {
        it.Value() = it.Value() / it.Value().two_norm();
        }
      }

    this->m_NumberOfGradientDirections = numImages - this->m_NumberOfBaselineImages;

    // ensure that the gradient image we received has as many components as
    // the number of gradient directions
    if( gradientImage->GetVectorLength() != this->m_NumberOfBaselineImages + this->m_NumberOfGradientDirections )
      {
      itkExceptionMacro( << this->m_NumberOfGradientDirections << " gradients + " << this->m_NumberOfBaselineImages
                         << "baselines = " << this->m_NumberOfGradientDirections + this->m_NumberOfBaselineImages
                         << " directions specified but image has " << gradientImage->GetVectorLength()
        << " components.");
      }

    this->ProcessObject::SetNthInput( 0,
        const_cast< GradientImagesType* >(gradientImage) );
    m_GradientImageTypeEnumeration = GradientIsInASingleImage;
  }



} // end of namespace
