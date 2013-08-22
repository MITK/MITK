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

mitk::USImageSource::USImageSource()
  : m_OpenCVToMitkFilter(mitk::OpenCVToMitkImageFilter::New()),
    m_MitkToOpenCVFilter(0)
{
  m_OpenCVToMitkFilter->SetCopyBuffer(false);
}

mitk::USImageSource::~USImageSource()
{
}

mitk::USImage::Pointer mitk::USImageSource::GetNextImage()
{
  mitk::Image::Pointer result;

  if ( m_ImageFilter.IsNotNull() )
  {
    cv::Mat image;
    this->GetNextRawImage(image);


    // execute filter if a filter is specified
    if ( m_ImageFilter.IsNotNull() ) { m_ImageFilter->FilterImage(image); }

    // convert to MITK image
    IplImage ipl_img = image;

    this->m_OpenCVToMitkFilter->SetOpenCVImage(&ipl_img);
    this->m_OpenCVToMitkFilter->Update();

    // OpenCVToMitkImageFilter returns a standard mitk::image.
    result = this->m_OpenCVToMitkFilter->GetOutput();

    // clean up
    image.release();
  }
  else
  {
    // mitk image can be received direclty if no filtering is necessary
    this->GetNextRawImage(result);
  }

  if ( result )
  {
    // we then transform standard mitk::Image into an USImage
    return mitk::USImage::New(result);
  }
  else
  {
    //MITK_WARN("mitkUSImageSource") << "Result image is not set.";
    return mitk::USImage::New();
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

  // convert mitk::Image to an OpenCV image
  m_MitkToOpenCVFilter->SetImage(mitkImg);
  IplImage* iplImage = m_MitkToOpenCVFilter->GetOpenCVImage();
  image = iplImage;

  // make sure that IplImage is released
  cvReleaseImage(&iplImage);
}
