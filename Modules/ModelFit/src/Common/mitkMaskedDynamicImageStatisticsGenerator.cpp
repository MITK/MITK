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

#include "mitkMaskedDynamicImageStatisticsGenerator.h"

#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "itkMaskedNaryStatisticsImageFilter.h"

mitk::MaskedDynamicImageStatisticsGenerator::MaskedDynamicImageStatisticsGenerator()
{
  m_Mask = NULL;
  m_DynamicImage = NULL;
};

mitk::MaskedDynamicImageStatisticsGenerator::~MaskedDynamicImageStatisticsGenerator(){};

const mitk::MaskedDynamicImageStatisticsGenerator::ResultType& mitk::MaskedDynamicImageStatisticsGenerator::GetMaximum()
{
  if(this->HasOutdatedResults())
  {
    CheckValidInputs();
    Generate();
  }
  return m_Maximum;
};

const mitk::MaskedDynamicImageStatisticsGenerator::ResultType& mitk::MaskedDynamicImageStatisticsGenerator::GetMinimum()
{
  if(this->HasOutdatedResults())
  {
    CheckValidInputs();
    Generate();
  }
  return m_Minimum;
};

const mitk::MaskedDynamicImageStatisticsGenerator::ResultType& mitk::MaskedDynamicImageStatisticsGenerator::GetMean()
{
  if(this->HasOutdatedResults())
  {
    CheckValidInputs();
    Generate();
  }
  return m_Mean;
};

const mitk::MaskedDynamicImageStatisticsGenerator::ResultType& mitk::MaskedDynamicImageStatisticsGenerator::GetSigma()
{
  if(this->HasOutdatedResults())
  {
    CheckValidInputs();
    Generate();
  }
  return m_Sigma;
};

const mitk::MaskedDynamicImageStatisticsGenerator::ResultType& mitk::MaskedDynamicImageStatisticsGenerator::GetVariance()
{
  if(this->HasOutdatedResults())
  {
    CheckValidInputs();
    Generate();
  }
  return m_Variance;
};

const mitk::MaskedDynamicImageStatisticsGenerator::ResultType& mitk::MaskedDynamicImageStatisticsGenerator::GetSum()
{
  if(this->HasOutdatedResults())
  {
    CheckValidInputs();
    Generate();
  }
  return m_Sum;
};

template <typename TPixel, unsigned int VDim>
void mitk::MaskedDynamicImageStatisticsGenerator::DoCalculateStatistics(const itk::Image<TPixel, VDim>* /*image*/)
{
  typedef itk::Image<TPixel, VDim-1> InputFrameImageType;
  typedef itk::MaskedNaryStatisticsImageFilter<InputFrameImageType, InternalMaskType> FilterType;

  typename FilterType::Pointer statFilter = FilterType::New();

  //add the time frames to the fit filter
  unsigned int timeSteps = this->m_DynamicImage->GetTimeSteps();
  std::vector<Image::Pointer> frameCache;
  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(this->m_DynamicImage);
  for (unsigned int i = 0; i < timeSteps; ++i)
  {
    typename InputFrameImageType::Pointer frameImage;
    imageTimeSelector->SetTimeNr(i);
    imageTimeSelector->UpdateLargestPossibleRegion();

    Image::Pointer frameMITKImage = imageTimeSelector->GetOutput();
    frameCache.push_back(frameMITKImage);
    mitk::CastToItkImage(frameMITKImage, frameImage);
    statFilter->SetInput(i,frameImage);
  }

  if (this->m_InternalMask.IsNotNull())
  {
    statFilter->SetMask(this->m_InternalMask);
  }

  statFilter->Update();

  m_Maximum.SetSize(timeSteps);
  m_Minimum.SetSize(timeSteps);
  m_Mean.SetSize(timeSteps);
  m_Sigma.SetSize(timeSteps);
  m_Variance.SetSize(timeSteps);
  m_Sum.SetSize(timeSteps);

  for (unsigned int i = 0; i < timeSteps; ++i)
  {
    m_Maximum.SetElement(i,statFilter->GetMaximum()[i]);
    m_Minimum.SetElement(i,statFilter->GetMinimum()[i]);
    m_Mean.SetElement(i,statFilter->GetMean()[i]);
    m_Sigma.SetElement(i,statFilter->GetSigma()[i]);
    m_Variance.SetElement(i,statFilter->GetVariance()[i]);
    m_Sum.SetElement(i,statFilter->GetSum()[i]);
  }

  this->m_GenerationTimeStamp.Modified();
}

void mitk::MaskedDynamicImageStatisticsGenerator::Generate()
{
  if(this->m_Mask.IsNotNull())
  {
    InternalMaskType::Pointer castedMask;
    CastToItkImage<InternalMaskType>(m_Mask, castedMask);
    if (castedMask.IsNull())
    {
      mitkThrow() << "Dynamic cast of mask went wrong. Internal Mask is NULL. Image statistics cannot be generated.";
    }

    this->m_InternalMask = castedMask;
  }
  else
  {
    this->m_InternalMask = NULL;
  }

  AccessFixedDimensionByItk(m_DynamicImage, mitk::MaskedDynamicImageStatisticsGenerator::DoCalculateStatistics, 4);
}

void
  mitk::MaskedDynamicImageStatisticsGenerator::CheckValidInputs() const
{

  if (m_DynamicImage.IsNull())
  {
    mitkThrow() << "Cannot generate statistics. Input dynamic image is not set.";
  }
}

bool
  mitk::MaskedDynamicImageStatisticsGenerator::HasOutdatedResults() const
{
  bool result = this->GetMTime() > this->m_GenerationTimeStamp;

  if (m_DynamicImage.IsNotNull())
  {
    if (m_DynamicImage->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }

  if (m_Mask.IsNotNull())
  {
    if (m_Mask->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }
  return result;

};
