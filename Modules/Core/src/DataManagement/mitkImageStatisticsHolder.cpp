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
#include "mitkImageStatisticsHolder.h"

#include "mitkHistogramGenerator.h"
#include <mitkProperties.h>

mitk::ImageStatisticsHolder::ImageStatisticsHolder(mitk::Image *image)
  : m_Image(image)
{
  m_CountOfMinValuedVoxels.resize(1, 0);
  m_CountOfMaxValuedVoxels.resize(1, 0);
  m_ScalarMin.resize(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.resize(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.resize(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.resize(1, itk::NumericTraits<ScalarType>::NonpositiveMin());

  mitk::HistogramGenerator::Pointer generator = mitk::HistogramGenerator::New();
  m_HistogramGeneratorObject = generator;
}

mitk::ImageStatisticsHolder::~ImageStatisticsHolder()
{
  m_HistogramGeneratorObject = nullptr;
}

const mitk::ImageStatisticsHolder::HistogramType *mitk::ImageStatisticsHolder::GetScalarHistogram(
  int t, unsigned int /*component*/)
{
  mitk::ImageTimeSelector *timeSelector = this->GetTimeSelector();
  if (timeSelector != nullptr)
  {
    timeSelector->SetTimeNr(t);
    timeSelector->UpdateLargestPossibleRegion();

    auto *generator =
      static_cast<mitk::HistogramGenerator *>(m_HistogramGeneratorObject.GetPointer());
    generator->SetImage(timeSelector->GetOutput());
    generator->ComputeHistogram();
    return static_cast<const mitk::ImageStatisticsHolder::HistogramType *>(generator->GetHistogram());
  }
  return nullptr;
}

bool mitk::ImageStatisticsHolder::IsValidTimeStep(int t) const
{
  return m_Image->IsValidTimeStep(t);
}

mitk::ImageTimeSelector::Pointer mitk::ImageStatisticsHolder::GetTimeSelector()
{
  ImageTimeSelector::Pointer timeSelector =
    ImageTimeSelector::New();
  timeSelector->SetInput(m_Image);

  return timeSelector;
}

void mitk::ImageStatisticsHolder::Expand(unsigned int timeSteps)
{
  if (!m_Image->IsValidTimeStep(timeSteps - 1))
    return;

  // The BaseData needs to be expanded, call the mitk::Image::Expand() method
  m_Image->Expand(timeSteps);

  if (timeSteps > m_ScalarMin.size())
  {
    m_ScalarMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_ScalarMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_Scalar2ndMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_Scalar2ndMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_CountOfMinValuedVoxels.resize(timeSteps, 0);
    m_CountOfMaxValuedVoxels.resize(timeSteps, 0);
  }
}

void mitk::ImageStatisticsHolder::ResetImageStatistics()
{
  m_ScalarMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_CountOfMinValuedVoxels.assign(1, 0);
  m_CountOfMaxValuedVoxels.assign(1, 0);
}

#include "mitkImageAccessByItk.h"

//#define BOUNDINGOBJECT_IGNORE

template <typename ItkImageType>
void mitk::_ComputeExtremaInItkImage(const ItkImageType *itkImage, mitk::ImageStatisticsHolder *statisticsHolder, int t)
{
  typename ItkImageType::RegionType region;
  region = itkImage->GetBufferedRegion();
  if (region.Crop(itkImage->GetRequestedRegion()) == false)
    return;
  if (region != itkImage->GetRequestedRegion())
    return;

  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, region);
  typedef typename ItkImageType::PixelType TPixel;
  TPixel value = 0;

  if (statisticsHolder == nullptr || !statisticsHolder->IsValidTimeStep(t))
    return;
  statisticsHolder->Expand(t + 1); // make sure we have initialized all arrays
  statisticsHolder->m_CountOfMinValuedVoxels[t] = 0;
  statisticsHolder->m_CountOfMaxValuedVoxels[t] = 0;

  statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMin[t] = itk::NumericTraits<ScalarType>::max();
  statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_ScalarMax[t] =
    itk::NumericTraits<ScalarType>::NonpositiveMin();

  while (!it.IsAtEnd())
  {
    value = it.Get();
#ifdef BOUNDINGOBJECT_IGNORE
    if (value > -32765)
    {
#endif
      // update min
      if (value < statisticsHolder->m_ScalarMin[t])
      {
        statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMin[t];
        statisticsHolder->m_ScalarMin[t] = value;
        statisticsHolder->m_CountOfMinValuedVoxels[t] = 1;
      }
      else if (value == statisticsHolder->m_ScalarMin[t])
      {
        ++statisticsHolder->m_CountOfMinValuedVoxels[t];
      }
      else if (value < statisticsHolder->m_Scalar2ndMin[t])
      {
        statisticsHolder->m_Scalar2ndMin[t] = value;
      }

      // update max
      if (value > statisticsHolder->m_ScalarMax[t])
      {
        statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_ScalarMax[t];
        statisticsHolder->m_ScalarMax[t] = value;
        statisticsHolder->m_CountOfMaxValuedVoxels[t] = 1;
      }
      else if (value == statisticsHolder->m_ScalarMax[t])
      {
        ++statisticsHolder->m_CountOfMaxValuedVoxels[t];
      }
      else if (value > statisticsHolder->m_Scalar2ndMax[t])
      {
        statisticsHolder->m_Scalar2ndMax[t] = value;
      }
#ifdef BOUNDINGOBJECT_IGNORE
    }
#endif

    ++it;
  }

  //// guard for wrong 2dMin/Max on single constant value images
  if (statisticsHolder->m_ScalarMax[t] == statisticsHolder->m_ScalarMin[t])
  {
    statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMax[t];
  }
  statisticsHolder->m_LastRecomputeTimeStamp.Modified();
}

template <typename ItkImageType>
void mitk::_ComputeExtremaInItkVectorImage(const ItkImageType *itkImage,
                                           mitk::ImageStatisticsHolder *statisticsHolder,
                                           int t,
                                           unsigned int component)
{
  typename ItkImageType::RegionType region;
  region = itkImage->GetBufferedRegion();
  if (region.Crop(itkImage->GetRequestedRegion()) == false)
    return;
  if (region != itkImage->GetRequestedRegion())
    return;

  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, region);

  if (statisticsHolder == nullptr || !statisticsHolder->IsValidTimeStep(t))
    return;
  statisticsHolder->Expand(t + 1); // make sure we have initialized all arrays
  statisticsHolder->m_CountOfMinValuedVoxels[t] = 0;
  statisticsHolder->m_CountOfMaxValuedVoxels[t] = 0;

  statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMin[t] = itk::NumericTraits<ScalarType>::max();
  statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_ScalarMax[t] =
    itk::NumericTraits<ScalarType>::NonpositiveMin();

  while (!it.IsAtEnd())
  {
    double value = it.Get()[component];
#ifdef BOUNDINGOBJECT_IGNORE
    if (value > -32765)
    {
#endif
      // update min
      if (value < statisticsHolder->m_ScalarMin[t])
      {
        statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMin[t];
        statisticsHolder->m_ScalarMin[t] = value;
        statisticsHolder->m_CountOfMinValuedVoxels[t] = 1;
      }
      else if (value == statisticsHolder->m_ScalarMin[t])
      {
        ++statisticsHolder->m_CountOfMinValuedVoxels[t];
      }
      else if (value < statisticsHolder->m_Scalar2ndMin[t])
      {
        statisticsHolder->m_Scalar2ndMin[t] = value;
      }

      // update max
      if (value > statisticsHolder->m_ScalarMax[t])
      {
        statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_ScalarMax[t];
        statisticsHolder->m_ScalarMax[t] = value;
        statisticsHolder->m_CountOfMaxValuedVoxels[t] = 1;
      }
      else if (value == statisticsHolder->m_ScalarMax[t])
      {
        ++statisticsHolder->m_CountOfMaxValuedVoxels[t];
      }
      else if (value > statisticsHolder->m_Scalar2ndMax[t])
      {
        statisticsHolder->m_Scalar2ndMax[t] = value;
      }
#ifdef BOUNDINGOBJECT_IGNORE
    }
#endif

    ++it;
  }

  //// guard for wrong 2dMin/Max on single constant value images
  if (statisticsHolder->m_ScalarMax[t] == statisticsHolder->m_ScalarMin[t])
  {
    statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMax[t];
  }
  statisticsHolder->m_LastRecomputeTimeStamp.Modified();
}

void mitk::ImageStatisticsHolder::ComputeImageStatistics(int t, unsigned int component)
{
  // timestep valid?
  if (!m_Image->IsValidTimeStep(t))
    return;

  // image modified?
  if (this->m_Image->GetMTime() > m_LastRecomputeTimeStamp.GetMTime())
    this->ResetImageStatistics();

  Expand(t + 1);

  // do we have valid information already?
  if (m_ScalarMin[t] != itk::NumericTraits<ScalarType>::max() ||
      m_Scalar2ndMin[t] != itk::NumericTraits<ScalarType>::max())
    return; // Values already calculated before...

  // used to avoid statistics calculation on Odf images. property will be replaced as soons as bug 17928 is merged and
  // the diffusion image refactoring is complete.
  mitk::BoolProperty *isSh = dynamic_cast<mitk::BoolProperty *>(m_Image->GetProperty("IsShImage").GetPointer());
  mitk::BoolProperty *isOdf = dynamic_cast<mitk::BoolProperty *>(m_Image->GetProperty("IsOdfImage").GetPointer());
  const mitk::PixelType pType = m_Image->GetPixelType(0);
  if (pType.GetNumberOfComponents() == 1 && (pType.GetPixelType() != itk::ImageIOBase::UNKNOWNPIXELTYPE) &&
      (pType.GetPixelType() != itk::ImageIOBase::VECTOR))
  {
    // recompute
    mitk::ImageTimeSelector::Pointer timeSelector = this->GetTimeSelector();
    if (timeSelector.IsNotNull())
    {
      timeSelector->SetTimeNr(t);
      timeSelector->UpdateLargestPossibleRegion();
      const mitk::Image *image = timeSelector->GetOutput();
      AccessByItk_2(image, _ComputeExtremaInItkImage, this, t);
    }
  }
  else if (pType.GetPixelType() == itk::ImageIOBase::VECTOR &&
           (!isOdf || !isOdf->GetValue()) && (!isSh || !isSh->GetValue())) // we have a vector image
  {
    // recompute
    mitk::ImageTimeSelector::Pointer timeSelector = this->GetTimeSelector();
    if (timeSelector.IsNotNull())
    {
      timeSelector->SetTimeNr(t);
      timeSelector->UpdateLargestPossibleRegion();
      const mitk::Image *image = timeSelector->GetOutput();
      AccessVectorPixelTypeByItk_n(image, _ComputeExtremaInItkVectorImage, (this, t, component));
    }
  }
  else
  {
    m_ScalarMin[t] = 0;
    m_ScalarMax[t] = 255;
    m_Scalar2ndMin[t] = 0;
    m_Scalar2ndMax[t] = 255;
  }
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValueMin(int t, unsigned int component)
{
  ComputeImageStatistics(t, component);
  return m_ScalarMin[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValueMax(int t, unsigned int component)
{
  ComputeImageStatistics(t, component);
  return m_ScalarMax[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValue2ndMin(int t, unsigned int component)
{
  ComputeImageStatistics(t, component);
  return m_Scalar2ndMin[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValue2ndMax(int t, unsigned int component)
{
  ComputeImageStatistics(t, component);
  return m_Scalar2ndMax[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetCountOfMinValuedVoxels(int t, unsigned int component)
{
  ComputeImageStatistics(t, component);
  return m_CountOfMinValuedVoxels[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetCountOfMaxValuedVoxels(int t, unsigned int component)
{
  ComputeImageStatistics(t, component);
  return m_CountOfMaxValuedVoxels[t];
}
