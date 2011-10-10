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
#ifndef _itk_TensorImageToDiffusionImageFilter_txx_
#define _itk_TensorImageToDiffusionImageFilter_txx_
#endif

#include "itkTensorImageToDiffusionImageFilter.h"
#include "itkTensorToL2NormImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

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

    if( m_GradientList.size()==0 )
    {
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error: gradient list is empty, cannot generate DWI.");
    }

    // create a B0 image by taking the norm of the tensor field * scale:
    typedef itk::TensorToL2NormImageFilter<InputImageType, itk::Image<InputScalarType,3> >
      TensorToL2NormFilterType;

    typename TensorToL2NormFilterType::Pointer myFilter1 = TensorToL2NormFilterType::New();
    myFilter1->SetInput (this->GetInput());

    try
    {
      myFilter1->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e;
      return;
    }

    typename itk::RescaleIntensityImageFilter< itk::Image<InputScalarType,3>, BaselineImageType>::Pointer rescaler=
      itk::RescaleIntensityImageFilter<itk::Image<InputScalarType,3>, BaselineImageType>::New();

    rescaler->SetOutputMinimum ( 0 );
    rescaler->SetOutputMaximum ( 10000 );
    rescaler->SetInput ( myFilter1->GetOutput() );
    try
    {
      rescaler->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e;
      return;
    }

    m_BaselineImage = rescaler->GetOutput();

    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
    outImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
    outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
    outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
    outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetVectorLength(m_GradientList.size()+1);
    outImage->Allocate();

    this->SetNumberOfRequiredOutputs (1);
    this->SetNthOutput (0, outImage);

  }

  template <class TInputScalarType, class TOutputScalarType>
  void
    TensorImageToDiffusionImageFilter<TInputScalarType, TOutputScalarType>
    ::ThreadedGenerateData ( const OutputImageRegionType &outputRegionForThread, int threadId )
  {

    typedef ImageRegionIterator<OutputImageType>      IteratorOutputType;
    typedef ImageRegionConstIterator<InputImageType>  IteratorInputType;
    typedef ImageRegionConstIterator<BaselineImageType>  IteratorBaselineType;

    unsigned long numPixels = outputRegionForThread.GetNumberOfPixels();
    unsigned long step = numPixels/100;
    unsigned long progress = 0;

    IteratorOutputType itOut (this->GetOutput(0), outputRegionForThread);
    IteratorInputType  itIn (this->GetInput(), outputRegionForThread);
    IteratorBaselineType itB0 (m_BaselineImage, outputRegionForThread);

    if( threadId==0 )
    {
      this->UpdateProgress (0.0);
    }


    while(!itIn.IsAtEnd())
    {

      if( this->GetAbortGenerateData() )
      {
        throw itk::ProcessAborted(__FILE__,__LINE__);
      }

      InputPixelType T = itIn.Get();

      BaselinePixelType b0 = itB0.Get();

      OutputPixelType out;
      out.SetSize(m_GradientList.size()+1);
      out.Fill(0);

      if( b0 > 0)
      {
        for( unsigned int i=0; i<m_GradientList.size(); i++)
        {

          GradientType g = m_GradientList[i];

          InputPixelType S;
          S[0] = g[0]*g[0];
          S[1] = g[1]*g[0];
          S[2] = g[2]*g[0];
          S[3] = g[1]*g[1];
          S[4] = g[2]*g[1];
          S[5] = g[2]*g[2];

          double res =
              T[0]*S[0] + T[1]*S[1] + T[2]*S[2] +
              T[1]*S[1] + T[3]*S[3] + T[4]*S[4] +
              T[2]*S[2] + T[4]*S[4] + T[5]*S[5];

          // check for corrupted tensor
          if (res>=0)
            out[i] = static_cast<OutputScalarType>( 1.0*b0*exp ( -1.0 * m_BValue * res ) );
        }
      }

      out[m_GradientList.size()] = b0;

      itOut.Set(out);

      if( threadId==0 && step>0)
      {
        if( (progress%step)==0 )
        {
          this->UpdateProgress ( double(progress)/double(numPixels) );
        }
      }

      ++progress;
      ++itB0;
      ++itIn;
      ++itOut;

    }

    if( threadId==0 )
    {
      this->UpdateProgress (1.0);
    }
  }



} // end of namespace
