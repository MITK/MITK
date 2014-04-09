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

#include "mitkUSImageSource.h"
#include "mitkProperties.h"

const char* mitk::USImageSource::IMAGE_PROPERTY_IDENTIFIER = "id_nummer";

mitk::USImageSource::USImageSource()
: m_OpenCVToMitkFilter(mitk::OpenCVToMitkImageFilter::New()),
  m_MitkToOpenCVFilter(0),
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

mitk::Image::Pointer mitk::USImageSource::GetNextImage()
{
  mitk::Image::Pointer result;

  if ( m_ImageFilter.IsNotNull() && ! m_ImageFilter->GetIsEmpty() )
  {
    cv::Mat image;
    this->GetNextRawImage(image);

    if ( ! image.empty() )
    {
      // execute filter if a filter is specified
      if ( m_ImageFilter.IsNotNull() ) { m_ImageFilter->FilterImage(image, m_CurrentImageId); }

      // convert to MITK image
      this->m_OpenCVToMitkFilter->SetOpenCVMat(image);
      this->m_OpenCVToMitkFilter->Update();

      // OpenCVToMitkImageFilter returns a standard mitk::image.
      result = this->m_OpenCVToMitkFilter->GetOutput();
    }
  }
  else
  {
    // mitk image can be received direclty if no filtering is necessary
    this->GetNextRawImage(result);
  }

  if ( result.IsNotNull() )
  {
    result->SetProperty(IMAGE_PROPERTY_IDENTIFIER, mitk::IntProperty::New(m_CurrentImageId));
    m_CurrentImageId++;

    // Everything as expected, return result
    return result;
  }
  else
  {
    //MITK_WARN("mitkUSImageSource") << "Result image is not set.";
    return mitk::Image::New();
  }
}

void mitk::USImageSource::GetNextRawImage( cv::Mat& image )
{
  // create filter object if it does not exist yet
  if ( ! m_MitkToOpenCVFilter )
  {
    m_MitkToOpenCVFilter = mitk::ImageToOpenCVImageFilter::New();
  }

  // get mitk image through virtual method of the subclass
  mitk::Image::Pointer mitkImg;
  this->GetNextRawImage(mitkImg);

  if ( mitkImg.IsNull() || ! mitkImg->IsInitialized() )
  {
    image = cv::Mat();
    return;
  }

  // convert mitk::Image to an OpenCV image
  m_MitkToOpenCVFilter->SetImage(mitkImg);
  image = m_MitkToOpenCVFilter->GetOpenCVMat();
}
