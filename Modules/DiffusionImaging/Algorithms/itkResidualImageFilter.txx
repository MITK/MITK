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


namespace itk
{

  //template <class TInputScalarType, class TOutputScalarType>
  //void
  //  TensorImageToDiffusionImageFilter<TInputScalarType, TOutputScalarType>
  //  ::GenerateData()
  //{
  //  // Call a method that can be overriden by a subclass to allocate
  //  // memory for the filter's outputs
  //  this->AllocateOutputs();

  //  // Call a method that can be overridden by a subclass to perform
  //  // some calculations prior to splitting the main computations into
  //  // separate threads
  //  this->BeforeThreadedGenerateData();

  //  // Set up the multithreaded processing
  //  ThreadStruct str;
  //  str.Filter = this;

  //  this->GetMultiThreader()->SetNumberOfThreads(this->GetNumberOfThreads());
  //  this->GetMultiThreader()->SetSingleMethod(this->ThreaderCallback, &str);

  //  // multithread the execution
  //  this->GetMultiThreader()->SingleMethodExecute();

  //  // Call a method that can be overridden by a subclass to perform
  //  // some calculations after all the threads have completed
  //  this->AfterThreadedGenerateData();

  //}

  template <class TInputScalarType, class TOutputScalarType>
  void
    TensorImageToDiffusionImageFilter<TInputScalarType, TOutputScalarType>
    ::BeforeThreadedGenerateData()
  {



  }

  template <class TInputScalarType, class TOutputScalarType>
  void
    TensorImageToDiffusionImageFilter<TInputScalarType, TOutputScalarType>
    ::ThreadedGenerateData ( const OutputImageRegionType &outputRegionForThread, int threadId )
  {


  }



} // end of namespace
