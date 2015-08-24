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

    itkSetMacro(InnerScale,double)
      itkGetMacro(InnerScale,double)

      itkSetMacro(OuterScale,double)
      itkGetMacro(OuterScale,double)

  private:

    typename TMaskImageType::Pointer m_ImageMask;
    double m_InnerScale, m_OuterScale;

    void GenerateData();
    void GenerateOutputInformation();

    StructureTensorEigenvalueImageFilter();
    virtual ~StructureTensorEigenvalueImageFilter();
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Algorithm/itkStructureTensorEigenvalueImageFilter.cpp"
#endif

#endif
