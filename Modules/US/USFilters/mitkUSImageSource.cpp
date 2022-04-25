/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSImageSource.h"
#include "mitkProperties.h"

const char* mitk::USImageSource::IMAGE_PROPERTY_IDENTIFIER = "id_nummer";

mitk::USImageSource::USImageSource()
  : m_OpenCVToMitkFilter(mitk::OpenCVToMitkImageFilter::New()),
  m_MitkToOpenCVFilter(nullptr),
  m_ImageFilter(mitk::BasicCombinationOpenCVImageFilter::New()),
  m_CurrentImageId(0)
{
}

mitk::USImageSource::~USImageSource()
{
}

void mitk::USImageSource::PushFilter(AbstractOpenCVImageFilter::Pointer filter)
{
  m_ImageFilter->PushFilter(filter);
}

bool mitk::USImageSource::RemoveFilter(AbstractOpenCVImageFilter::Pointer filter)
{
  return m_ImageFilter->RemoveFilter(filter);
}

bool mitk::USImageSource::GetIsFilterInThePipeline(AbstractOpenCVImageFilter::Pointer filter)
{
  return m_ImageFilter->GetIsFilterOnTheList(filter);
}

std::vector<mitk::Image::Pointer> mitk::USImageSource::GetNextImage()
{
  std::vector<mitk::Image::Pointer> result;

  // Apply OpenCV based filters beforehand
  if (m_ImageFilter.IsNotNull() && !m_ImageFilter->GetIsEmpty())
  {
    std::vector<cv::Mat> imageVector;
    GetNextRawImage(imageVector);
    if(result.size() != imageVector.size())
      result.resize(imageVector.size());

    for (size_t i = 0; i < imageVector.size(); ++i)
    {
      if (!imageVector[i].empty())
      {
        m_ImageFilterMutex.lock();
        m_ImageFilter->FilterImage(imageVector[i], m_CurrentImageId);
        m_ImageFilterMutex.unlock();

        // convert to MITK image
        this->m_OpenCVToMitkFilter->SetOpenCVMat(imageVector[i]);
        this->m_OpenCVToMitkFilter->Update();

        // OpenCVToMitkImageFilter returns a standard mitk::image.
        result[i] = this->m_OpenCVToMitkFilter->GetOutput();
      }
    }
  }
  else
  {
    this->GetNextRawImage(result);
  }

  for (size_t i = 0; i < result.size(); ++i)
  {
    if (result[i].IsNotNull())
    {
      result[i]->SetProperty(IMAGE_PROPERTY_IDENTIFIER, mitk::IntProperty::New(m_CurrentImageId));
    }
    else
    {
      result[i] = mitk::Image::New();
    }
  }
  m_CurrentImageId++;

  return result;
}

void mitk::USImageSource::GetNextRawImage(std::vector<cv::Mat>& imageVector)
{
  // create filter object if it does not exist yet
  if (!m_MitkToOpenCVFilter)
  {
    m_MitkToOpenCVFilter = mitk::ImageToOpenCVImageFilter::New();
  }

  // get mitk image through virtual method of the subclass
  std::vector<mitk::Image::Pointer> mitkImg;
  this->GetNextRawImage(mitkImg);

  for (unsigned int i = 0; i < mitkImg.size(); ++i)
  {
    if (mitkImg[i].IsNull() || !mitkImg[i]->IsInitialized())
    {
      imageVector[i] = cv::Mat();
    }
    else
    {
      // convert mitk::Image to an OpenCV image
      m_MitkToOpenCVFilter->SetImage(mitkImg[i]);
      imageVector[i] = m_MitkToOpenCVFilter->GetOpenCVMat();
    }
  }
}
