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

    itkSetMacro(Sigma,double)
      itkGetMacro(Sigma,double)

  private:

    typename TMaskImageType::Pointer m_ImageMask;
    double m_Sigma;

    void GenerateData();
    void GenerateOutputInformation();

    HessianMatrixEigenvalueImageFilter();
    virtual ~HessianMatrixEigenvalueImageFilter();
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Algorithm/itkHessianMatrixEigenvalueImageFilter.cpp"
#endif

#endif
