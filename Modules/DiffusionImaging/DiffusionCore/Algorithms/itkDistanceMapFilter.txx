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
#ifndef _itkDistanceMapFilter_txx
#define _itkDistanceMapFilter_txx

#include "itkDistanceMapFilter.h"

#include "mitkProgressBar.h"
#include <itkSignedMaurerDistanceMapImageFilter.h>


namespace itk
{

  template< class TInputImage, class TOutputImage >
  DistanceMapFilter<TInputImage, TOutputImage>::DistanceMapFilter()
  {

  }

  template< class TInputImage, class TOutputImage >
  DistanceMapFilter<TInputImage, TOutputImage>::~DistanceMapFilter()
  {


  }




  template< class TInputImage, class TOutputImage >
  void DistanceMapFilter<TInputImage, TOutputImage>::GenerateData()
  {
    //----------------------------------------------------------------------//
    //  Progress bar                                                        //
    //----------------------------------------------------------------------//
    //mitk::ProgressBar::GetInstance()->AddStepsToDo( 3 );



    typedef itk::SignedMaurerDistanceMapImageFilter<InputImageType, OutputImageType> DistanceFilterType;
    typename DistanceFilterType::Pointer dFilter = DistanceFilterType::New();
    dFilter->SetInput(this->GetInput());
    dFilter->SetUseImageSpacing(true);
    dFilter->SetSquaredDistance(false);
    dFilter->SetInsideIsPositive(true);
    dFilter->Update();

    typename OutputImageType::Pointer outputImg = dFilter->GetOutput();

    typedef itk::ImageRegionIterator<OutputImageType> ImageIteratorType;

    ImageIteratorType outIt(outputImg, outputImg->GetRequestedRegion());
    outIt.GoToBegin();
    while(!outIt.IsAtEnd())
    {
      typename OutputImageType::PixelType p = outIt.Get();
      p *= -1;
      outIt.Set(p);
      ++outIt;
    }

    Superclass::SetNthOutput( 0, outputImg );

  }





}
#endif // _itkDistanceMapFilter_txx
