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

#ifndef ITKLINEHISTOGRAMBASEDMASSIMAGEFILTER_H
#define ITKLINEHISTOGRAMBASEDMASSIMAGEFILTER_H

#include <itkImageToImageFilter.h>

namespace itk
{
  template< class TInputImageType, class TOutputImageType = TInputImageType, class TMaskImageType = itk::Image<short,3> >
  class LineHistogramBasedMassImageFilter
    : public itk::ImageToImageFilter<TInputImageType, TOutputImageType>
  {
  public:

    typedef LineHistogramBasedMassImageFilter< TInputImageType, TOutputImageType > Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter<  TInputImageType, TOutputImageType  > Superclass;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetImageMask(TMaskImageType * maskimage);
    void SetBinaryContour(TMaskImageType * contouriamge);

  private:

    typename TMaskImageType::Pointer m_ImageMask;
    typename TMaskImageType::Pointer m_BinaryContour;
    vnl_vector<double> m_CenterOfMask;

    void ThreadedGenerateData(const typename Superclass::OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);
    void BeforeThreadedGenerateData();
    //  void GenerateOutputInformation();

    vnl_vector<double> GetCenterOfMass(const TMaskImageType * maskImage);

    LineHistogramBasedMassImageFilter();
    virtual ~LineHistogramBasedMassImageFilter();
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Features/itkLineHistogramBasedMassImageFilter.cpp"
#endif

#endif
