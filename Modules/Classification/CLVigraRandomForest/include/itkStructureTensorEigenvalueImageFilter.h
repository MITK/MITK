/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ITKSTRUCTURETENSOREIGENVALUEIMAGEFILTER_H
#define ITKSTRUCTURETENSOREIGENVALUEIMAGEFILTER_H

#include <itkImageToImageFilter.h>

namespace itk
{
  template< class TInputImageType,
  class TOutputImageType = TInputImageType,
  class TMaskImageType = itk::Image<short,3> >
  class StructureTensorEigenvalueImageFilter
    : public itk::ImageToImageFilter<TInputImageType, TOutputImageType>
  {
  public:

    typedef StructureTensorEigenvalueImageFilter< TInputImageType, TOutputImageType > Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter<  TInputImageType, TOutputImageType  > Superclass;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetImageMask(TMaskImageType * maskimage);

    itkSetMacro(InnerScale,double);

    itkGetMacro(InnerScale,double);

    itkSetMacro(OuterScale,double);

    itkGetMacro(OuterScale,double);

  private:

    typename TMaskImageType::Pointer m_ImageMask;
    double m_InnerScale, m_OuterScale;

    void GenerateData() override;
    void GenerateOutputInformation() override;

    StructureTensorEigenvalueImageFilter();
    ~StructureTensorEigenvalueImageFilter() override;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Algorithm/itkStructureTensorEigenvalueImageFilter.cpp"
#endif

#endif
