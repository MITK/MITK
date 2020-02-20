/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ITKHESSIANMATRIXEIGENVALUEIMAGEFILTER_H
#define ITKHESSIANMATRIXEIGENVALUEIMAGEFILTER_H

#include <itkImageToImageFilter.h>

namespace itk
{
  template< class TInputImageType, class TOutputImageType = TInputImageType, class TMaskImageType = itk::Image<short,3> >
  class HessianMatrixEigenvalueImageFilter
    : public itk::ImageToImageFilter<TInputImageType, TOutputImageType>
  {
  public:

    typedef HessianMatrixEigenvalueImageFilter< TInputImageType, TOutputImageType > Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter<  TInputImageType, TOutputImageType  > Superclass;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetImageMask(TMaskImageType * maskimage);

    itkSetMacro(Sigma,double);

    itkGetMacro(Sigma,double);

  private:

    typename TMaskImageType::Pointer m_ImageMask;
    double m_Sigma;

    void GenerateData() override;
    void GenerateOutputInformation() override;

    HessianMatrixEigenvalueImageFilter();
    ~HessianMatrixEigenvalueImageFilter() override;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Algorithm/itkHessianMatrixEigenvalueImageFilter.cpp"
#endif

#endif
