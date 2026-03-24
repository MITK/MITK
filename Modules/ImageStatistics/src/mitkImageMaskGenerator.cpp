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

void ImageMaskGenerator::UpdateInternalMask()
{
  if (this->m_ImageMask.IsNull())
  {
    mitkThrow() << "Cannot update internal mask. Input image is not set.";
  }

  const auto timeGeo = this->m_InputImage->GetTimeGeometry();
  if (!timeGeo->IsValidTimePoint(this->m_TimePoint))
  {
    mitkThrow() << "Cannot update internal mask. Time step selected that is not supported by input image.";
  }

  m_InternalMask = SelectImageByTimePoint(m_ImageMask, m_TimePoint);

  if (m_InternalMask.IsNull())
  {
      MITK_WARN << "Warning: time step > number of time steps in mask image, using last time step";
      m_InternalMask = SelectImageByTimeStep(m_ImageMask, m_ImageMask->GetTimeSteps()-1);
  }
}

unsigned int ImageMaskGenerator::GetNumberOfMasks() const
{
  return 1;
}

mitk::Image::ConstPointer ImageMaskGenerator::DoGetMask(unsigned int)
{
    if (m_ImageMask.IsNull())
    {
        mitkThrow() << "Mask Image is nullptr";
    }
    if (IsUpdateRequired())
    {
        UpdateInternalMask();
    }

    return m_InternalMask;
}

bool ImageMaskGenerator::IsUpdateRequired() const
{
    unsigned long internalMaskTimeStamp = m_InternalMask.IsNull() ? 0 : m_InternalMask->GetMTime();
    unsigned long maskImageTimeStamp = m_ImageMask->GetMTime();

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






