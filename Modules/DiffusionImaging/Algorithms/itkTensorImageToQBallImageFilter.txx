/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToQBallImageFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_TensorImageToQBallImageFilter_txx_
#define _itk_TensorImageToQBallImageFilter_txx_
#endif

#include "itkTensorImageToQBallImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkOrientationDistributionFunction.h>

namespace itk
{
  template <class TInputScalarType, class TOutputScalarType>
  void
    TensorImageToQBallImageFilter<TInputScalarType, TOutputScalarType>
    ::BeforeThreadedGenerateData()
  {
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
    outImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
    outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
    outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
    outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->Allocate();

    this->SetNumberOfRequiredOutputs (1);
    this->SetNthOutput (0, outImage);
  }

  template <class TInputScalarType, class TOutputScalarType>
  void
    TensorImageToQBallImageFilter<TInputScalarType, TOutputScalarType>
    ::ThreadedGenerateData ( const OutputImageRegionType &outputRegionForThread, int threadId )
  {

    typedef ImageRegionIterator<OutputImageType>      IteratorOutputType;
    typedef ImageRegionConstIterator<InputImageType>  IteratorInputType;

    unsigned long numPixels = outputRegionForThread.GetNumberOfPixels();
    unsigned long step = numPixels/100;
    unsigned long progress = 0;

    IteratorOutputType itOut (this->GetOutput(0), outputRegionForThread);
    IteratorInputType  itIn (this->GetInput(), outputRegionForThread);

    if( threadId==0 )
      this->UpdateProgress (0.0);


    while(!itIn.IsAtEnd())
    {
      if( this->GetAbortGenerateData() )
      {
        throw itk::ProcessAborted(__FILE__,__LINE__);
      }

      InputPixelType T = itIn.Get();

      OutputPixelType out;

      float tensorelems[6] = {
        (float)T[0],
        (float)T[1],
        (float)T[2],
        (float)T[3],
        (float)T[4],
        (float)T[5],
      };
      itk::DiffusionTensor3D<float> tensor(tensorelems);

      itk::OrientationDistributionFunction<TOutputScalarType, QBALL_ODFSIZE> odf;
      odf.InitFromTensor(tensor);
      odf.Normalize();

      for( unsigned int i=0; i<QBALL_ODFSIZE; i++)
          out[i] = odf.GetElement(i);

      itOut.Set(out);

      if( threadId==0 && step>0)
      {
        if( (progress%step)==0 )
        {
          this->UpdateProgress ( double(progress)/double(numPixels) );
        }
      }

      ++progress;
      ++itIn;
      ++itOut;
    }

    if( threadId==0 )
    {
      this->UpdateProgress (1.0);
    }
    MITK_INFO << "one thread finished Q-Ball estimation";
  }
} // end of namespace
