/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageMaskGenerator.h>
#include <mitkImageTimeSelector.h>
#include <stdexcept>

namespace mitk {

void ImageMaskGenerator::SetImageMask(const Image* maskImage)
{
    if (m_InternalMaskImage != maskImage)
    {
        m_InternalMaskImage = maskImage;
        this->Modified();
    }
}

void ImageMaskGenerator::SetTimeStep(unsigned int timeStep)
{
    if (timeStep != m_TimeStep)
    {
        m_TimeStep = timeStep;
        UpdateInternalMask();
    }
}

void ImageMaskGenerator::UpdateInternalMask()
{
  if (this->m_inputImage.IsNull())
  {
    mitkThrow() << "Cannot update internal mask. Input image is not set.";
  }

  const auto timeGeo = this->m_inputImage->GetTimeGeometry();
  if (!timeGeo->IsValidTimeStep(this->m_TimeStep))
  {
    mitkThrow() << "Cannot update internal mask. Time step selected that is not supported by input image.";
  }

  auto timePoint = this->m_inputImage->GetTimeGeometry()->TimeStepToTimePoint(this->m_TimeStep);
  m_InternalMask = SelectImageByTimePoint(m_InternalMaskImage, timePoint);

  if (m_InternalMask.IsNull())
  {
      MITK_WARN << "Warning: time step > number of time steps in mask image, using last time step";
      m_InternalMask = SelectImageByTimeStep(m_InternalMaskImage, m_InternalMaskImage->GetTimeSteps()-1);
  }
}


mitk::Image::ConstPointer ImageMaskGenerator::GetMask()
{
    if (m_InternalMaskImage.IsNull())
    {
        MITK_ERROR << "Mask Image is nullptr";
    }
    if (IsUpdateRequired())
    {
        UpdateInternalMask();
    }

    return m_InternalMask;
}

bool ImageMaskGenerator::IsUpdateRequired() const
{
    unsigned long internalMaskTimeStamp = m_InternalMask->GetMTime();
    unsigned long maskImageTimeStamp = m_InternalMaskImage->GetMTime();

    if (maskImageTimeStamp > internalMaskTimeStamp) // inputs have changed
    {
        return true;
    }

    if (this->GetMTime() > maskImageTimeStamp) // input has changed
    {
        return true;
    }

    return false;
}

}






