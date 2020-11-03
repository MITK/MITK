/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

    void ThreadedGenerateData(const typename Superclass::OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;
    void BeforeThreadedGenerateData() override;
    //  void GenerateOutputInformation();

    vnl_vector<double> GetCenterOfMass(const TMaskImageType * maskImage);

    LineHistogramBasedMassImageFilter();
    ~LineHistogramBasedMassImageFilter() override;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Features/itkLineHistogramBasedMassImageFilter.cpp"
#endif

#endif
