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

void ImageMaskGenerator::SetImageMask(Image::Pointer maskImage)
{
    if (m_internalMaskImage != maskImage)
    {
        m_internalMaskImage = maskImage;
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
    unsigned int timeStepForExtraction;

    if (m_TimeStep >= m_internalMaskImage->GetTimeSteps())
    {
        MITK_WARN << "Warning: time step > number of time steps in mask image, using last time step";
        timeStepForExtraction = m_internalMaskImage->GetTimeSteps() - 1;
    }
    else
    {
        timeStepForExtraction = m_TimeStep;
    }
    ImageTimeSelector::Pointer imageTimeSelector = ImageTimeSelector::New();
    imageTimeSelector->SetInput(m_internalMaskImage);
    imageTimeSelector->SetTimeNr(timeStepForExtraction);
    imageTimeSelector->UpdateLargestPossibleRegion();

    m_InternalMask = mitk::Image::New();
    m_InternalMask = imageTimeSelector->GetOutput();
}


mitk::Image::Pointer ImageMaskGenerator::GetMask()
{
    if (m_internalMaskImage.IsNull())
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
    unsigned long maskImageTimeStamp = m_internalMaskImage->GetMTime();

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






