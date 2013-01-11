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
/*=========================================================================

  Program:   Tensor ToolKit - TTK
  Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorToL2NormImageFilter.txx $
  Language:  C++
  Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
  Version:   $Revision: 68 $

  Copyright (c) INRIA 2010. All rights reserved.
  See LICENSE.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_TensorToL2NormImageFilter_txx_
#define _itk_TensorToL2NormImageFilter_txx_
#endif

#include "itkTensorToL2NormImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

namespace itk
{

  template <class TInputImage, class TOutputImage>
  void
  TensorToL2NormImageFilter<TInputImage, TOutputImage>
  ::ThreadedGenerateData ( const OutputImageRegionType &outputRegionForThread, int threadId )
  {
    typedef ImageRegionIterator<OutputImageType>      IteratorOutputType;
    typedef ImageRegionConstIterator<InputImageType>  IteratorInputType;

    unsigned long numPixels = outputRegionForThread.GetNumberOfPixels();
    unsigned long step = numPixels/100;
    unsigned long progress = 0;

    IteratorOutputType itOut(this->GetOutput(), outputRegionForThread);
    IteratorInputType  itIn(this->GetInput(), outputRegionForThread);

    if( threadId==0 )
      this->UpdateProgress (0.0);

    while(!itOut.IsAtEnd())
    {
      if( this->GetAbortGenerateData() )
        throw itk::ProcessAborted(__FILE__,__LINE__);


      OutputPixelType out = static_cast<OutputPixelType>( 0.0 ); // be careful, overload in MedINRIA

      InputPixelType T = itIn.Get();

      if ( !(T[0]==0 && T[1]==0 && T[2]==0 && T[3]==0 && T[4]==0 && T[5]==0) )
      {
        double sum = T[0]*T[0] + T[3]*T[3] + T[5]*T[5]
          + T[1]*T[2]*2.0 + T[2]*T[4]*2.0 + T[1]*T[4]*2.0;
        out = static_cast<OutputPixelType>( vcl_sqrt( sum ));
      }

      if( threadId==0 && step>0)
      {
        if( (progress%step)==0 )
          this->UpdateProgress ( double(progress)/double(numPixels) );
      }


      itOut.Set (out);
      ++progress;
      ++itOut;
      ++itIn;

    }

    if( threadId==0 )
      this->UpdateProgress (1.0);

  }



} // end of namespace
