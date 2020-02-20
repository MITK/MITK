/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/

#ifndef __itkImageToPathFilter_txx
#define __itkImageToPathFilter_txx

#include "itkImageToPathFilter.h"

namespace itk
{
  /**
   *
   */
  template <class TInputImage, class TOutputPath>
  ImageToPathFilter<TInputImage, TOutputPath>::ImageToPathFilter()
  {
    // Modify superclass default values, can be overridden by subclasses
    this->SetNumberOfRequiredInputs(1);
  }

  /**
   *
   */
  template <class TInputImage, class TOutputPath>
  ImageToPathFilter<TInputImage, TOutputPath>::~ImageToPathFilter()
  {
  }

  /**
   *
   */
  template <class TInputImage, class TOutputPath>
  void ImageToPathFilter<TInputImage, TOutputPath>::SetInput(const InputImageType *input)
  {
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput(0, const_cast<InputImageType *>(input));
  }

  /**
   * Connect one of the operands for pixel-wise addition
   */
  template <class TInputImage, class TOutputPath>
  void ImageToPathFilter<TInputImage, TOutputPath>::SetInput(unsigned int index, const TInputImage *image)
  {
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput(index, const_cast<TInputImage *>(image));
  }

  /**
   *
   */
  template <class TInputImage, class TOutputPath>
  const typename ImageToPathFilter<TInputImage, TOutputPath>::InputImageType *
    ImageToPathFilter<TInputImage, TOutputPath>::GetInput(void)
  {
    if (this->GetNumberOfInputs() < 1)
    {
      return nullptr;
    }

    return static_cast<const TInputImage *>(this->ProcessObject::GetInput(0));
  }

  /**
   *
   */
  template <class TInputImage, class TOutputPath>
  const typename ImageToPathFilter<TInputImage, TOutputPath>::InputImageType *
    ImageToPathFilter<TInputImage, TOutputPath>::GetInput(unsigned int idx)
  {
    return static_cast<const TInputImage *>(this->ProcessObject::GetInput(idx));
  }

  template <class TInputImage, class TOutputPath>
  void ImageToPathFilter<TInputImage, TOutputPath>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

} // end namespace itk

#endif
