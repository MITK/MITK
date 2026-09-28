/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkOtsuSegmentationFilter.h>
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkAddImageFilter.h>
#include <itkCommand.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

struct paramContainer
{
  paramContainer(unsigned int numThresholds,
                 bool useValley,
                 unsigned int numBins,
                 mitk::Image::Pointer image,
                 mitk::OtsuSegmentationFilter *reportTo)
    : m_NumberOfThresholds(numThresholds),
      m_ValleyEmphasis(useValley),
      m_NumberOfBins(numBins),
      m_Image(image),
      m_ReportTo(reportTo)
  {
  }

  unsigned int m_NumberOfThresholds;
  bool m_ValleyEmphasis;
  unsigned int m_NumberOfBins;
  mitk::Image::Pointer m_Image;
  mitk::OtsuSegmentationFilter *m_ReportTo;
};

template <typename TPixel, unsigned int VImageDimension>
void AccessItkOtsuFilter(const itk::Image<TPixel, VImageDimension> *itkImage, paramContainer params)
{
  typedef itk::Image<TPixel, VImageDimension> itkInputImageType;
  typedef itk::Image<mitk::OtsuSegmentationFilter::OutputPixelType, VImageDimension> itkOutputImageType;
  typedef itk::OtsuMultipleThresholdsImageFilter<itkInputImageType, itkOutputImageType> OtsuFilterType;
  using AddFilterType = itk::AddImageFilter<itkOutputImageType>;

  auto otsuFilter = OtsuFilterType::New();
  otsuFilter->SetNumberOfThresholds(params.m_NumberOfThresholds);
  otsuFilter->SetInput(itkImage);
  otsuFilter->SetValleyEmphasis(params.m_ValleyEmphasis);
  otsuFilter->SetNumberOfHistogramBins(params.m_NumberOfBins);

  // The Otsu filter reports the progress of its internal labelling pass
  // through a ProgressAccumulator. Its histogram and threshold computation
  // report nothing, so the bar stays put until the labelling starts.
  auto forwarder = itk::MemberCommand<mitk::OtsuSegmentationFilter>::New();
  forwarder->SetCallbackFunction(params.m_ReportTo, &mitk::OtsuSegmentationFilter::ForwardProgress);
  otsuFilter->AddObserver(itk::ProgressEvent(), forwarder);

  auto addFilter = AddFilterType::New();
  addFilter->SetInput1(otsuFilter->GetOutput());
  //add 1 to every pixel because otsu also returns 0 as a label
  //which encodes unlabeled in mitk.
  addFilter->SetConstant2(1);
  try
  {
    addFilter->Update();
  }
  catch (...)
  {
    mitkThrow() << "itkOtsuFilter error.";
  }

  mitk::CastToMitkImage<itkOutputImageType>(addFilter->GetOutput(), params.m_Image);
  return;
}

namespace mitk
{
  OtsuSegmentationFilter::OtsuSegmentationFilter()
    : m_NumberOfThresholds(2), m_ValleyEmphasis(false), m_NumberOfBins(128)
  {
  }

  OtsuSegmentationFilter::~OtsuSegmentationFilter() {}

  void OtsuSegmentationFilter::ForwardProgress(itk::Object *caller, const itk::EventObject &)
  {
    if (const auto *source = dynamic_cast<const itk::ProcessObject *>(caller); nullptr != source)
      this->UpdateProgress(source->GetProgress());
  }

  void OtsuSegmentationFilter::GenerateData()
  {
    mitk::Image::ConstPointer mitkImage = GetInput();
    AccessByItk_n(mitkImage,
                  AccessItkOtsuFilter,
                  (paramContainer(m_NumberOfThresholds, m_ValleyEmphasis, m_NumberOfBins, this->GetOutput(), this)));
  }
}
