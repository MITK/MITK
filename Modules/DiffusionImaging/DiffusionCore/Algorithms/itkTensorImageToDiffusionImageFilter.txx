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
   2
   3 Program:   Tensor ToolKit - TTK
   4 Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToDiffusionImageFilter.txx $
   5 Language:  C++
   6 Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
   7 Version:   $Revision: 68 $
   8
   9 Copyright (c) INRIA 2010. All rights reserved.
  10 See LICENSE.txt for details.
  11
  12 This software is distributed WITHOUT ANY WARRANTY; without even
  13 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  14 PURPOSE.  See the above copyright notices for more information.
  15
  16 =========================================================================*/
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

template <class TInputScalarType, class TOutputScalarType>
void
TensorImageToDiffusionImageFilter<TInputScalarType, TOutputScalarType>
::BeforeThreadedGenerateData()
{

  if( m_GradientList->Size()==0 )
  {
    throw itk::ExceptionObject (__FILE__,__LINE__,"Error: gradient list is empty, cannot generate DWI.");
  }

  if( m_BaselineImage.IsNull() )
  {
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

    rescaler->SetOutputMinimum ( m_Min );
    rescaler->SetOutputMaximum ( m_Max );
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
  }

  typename OutputImageType::Pointer outImage = OutputImageType::New();
  outImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
  outImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength(m_GradientList->Size());
  outImage->Allocate();

  this->SetNumberOfRequiredOutputs (1);
  this->SetNthOutput (0, outImage);

}

template <class TInputScalarType, class TOutputScalarType>
void TensorImageToDiffusionImageFilter<TInputScalarType, TOutputScalarType>
::ThreadedGenerateData (const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId )
{
  typedef ImageRegionIterator<OutputImageType>      IteratorOutputType;
  typedef ImageRegionConstIterator<InputImageType>  IteratorInputType;
  typedef ImageRegionConstIterator<BaselineImageType>  IteratorBaselineType;

  unsigned long numPixels = outputRegionForThread.GetNumberOfPixels();
  unsigned long step = numPixels/100;
  unsigned long progress = 0;

  IteratorOutputType itOut (this->GetOutput(), outputRegionForThread);
  IteratorInputType  itIn (this->GetInput(), outputRegionForThread);
  IteratorBaselineType itB0 (m_BaselineImage, outputRegionForThread);

  typedef ImageRegionConstIterator< MaskImageType >   IteratorMaskImageType;
  IteratorMaskImageType itMask;

  if( m_MaskImage.IsNotNull() )
  {
    itMask = IteratorMaskImageType( m_MaskImage, outputRegionForThread);
    itMask.GoToBegin();
  }

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
    out.SetSize(m_GradientList->Size());
    out.Fill(0);

    short maskvalue = 1;
    if( m_MaskImage.IsNotNull() )
    {
      maskvalue = itMask.Get();
      ++itMask;
    }

    std::vector<unsigned int> b0_indices;
    if( b0 > 0)
    {
      for( unsigned int i=0; i<m_GradientList->Size(); i++)
      {
        GradientType g = m_GradientList->at(i);

        // normalize vector so the following computations work
        const double twonorm = g.two_norm();
        if( twonorm < vnl_math::eps )
        {
          b0_indices.push_back(i);
          continue;
        }

        GradientType gn = g.normalize();

        InputPixelType S;
        S[0] = gn[0]*gn[0];
        S[1] = gn[1]*gn[0];
        S[2] = gn[2]*gn[0];
        S[3] = gn[1]*gn[1];
        S[4] = gn[2]*gn[1];
        S[5] = gn[2]*gn[2];

        const double res =
                T[0]*S[0] +
            2 * T[1]*S[1] +     T[3]*S[3] +
            2 * T[2]*S[2] + 2 * T[4]*S[4] + T[5]*S[5];

        // check for corrupted tensor
        if (res>=0)
        {
          // estimate the bvalue from the base value and the norm of the gradient
          //   - because of this estimation the vector have to be normalized beforehand
          //     otherwise the modelled signal is wrong ( i.e. not scaled properly )
          const double bval = m_BValue * twonorm * twonorm;
          out[i] = static_cast<OutputScalarType>( maskvalue * 1.0 * b0 * exp ( -1.0 * bval * res ) );
        }
      }
    }

    for(unsigned int idx = 0; idx < b0_indices.size(); idx++ )
    {
      out[b0_indices.at(idx)] = b0;
    }


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
