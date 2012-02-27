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
#ifndef _itk_ResidualImageFilter_txx_
#define _itk_ResidualImageFilter_txx_
#endif

#include "itkResidualImageFilter.h"
#include <mitkCommon.h>

namespace itk
{




  template <class TInputScalarType, class TOutputScalarType>
  void
  ResidualImageFilter<TInputScalarType, TOutputScalarType>
  ::GenerateData()
  {
    typename InputImageType::SizeType size = this->GetInput()->GetLargestPossibleRegion().GetSize();
    typename InputImageType::SizeType size2 = m_SecondDiffusionImage->GetLargestPossibleRegion().GetSize();

    if(size != size2)
    {
      MITK_ERROR << "Sizes do not match";
      return;
    }

    // Initialize output image
    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    outputImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
    outputImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
    outputImage->Allocate();
    outputImage->FillBuffer(0.0);

    for(int x=0; x<size[0]; x++)
    {
      for(int y=0; y<size[1]; y++)
      {
        for(int z=0; z<size[2]; z++)
        {
          itk::Index<3> ix;
          ix[0] = x;
          ix[1] = y;
          ix[2] = z;

          typename InputImageType::PixelType p1 = this->GetInput()->GetPixel(ix);
          typename InputImageType::PixelType p2 = m_SecondDiffusionImage->GetPixel(ix);



          if(p1.GetSize() != p2.GetSize())
          {
            MITK_ERROR << "Vector sizes do not match";
            return;
          }

          float res = 0;

          for(int i = 0; i<p1.GetSize(); i++)
          {
            float val1 = (float)p1.GetElement(i);
            float val2 = (float)p2.GetElement(i);

            res += abs(val1-val2);
          }


          res = res/p1.GetSize();

          outputImage->SetPixel(ix, res);

        }
      }
    }

   // Superclass::SetNthInput(0, outputImage);



  }






} // end of namespace
