

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

mitk::Image::Pointer ImageMaskGenerator::GetMask()
{
    if (m_internalMaskImage.IsNull())
    {
        MITK_ERROR << "Mask Image is nullptr";
    }

    if (this->IsUpdateRequired())
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
        this->Modified();
    }

    m_InternalMaskUpdateTime = m_InternalMask->GetMTime();
    return m_InternalMask;
}

bool ImageMaskGenerator::IsUpdateRequired() const
{
    unsigned long thisClassTimeStamp = this->GetMTime();
    unsigned long internalMaskTimeStamp = m_InternalMask->GetMTime();
    unsigned long maskImageTimeStamp = m_internalMaskImage->GetMTime();

    if (thisClassTimeStamp > m_InternalMaskUpdateTime) // inputs have changed
    {
        return true;
    }

    if (m_InternalMaskUpdateTime < maskImageTimeStamp) // mask image has changed outside of this class
    {
        return true;
    }

    if (internalMaskTimeStamp > m_InternalMaskUpdateTime) // internal mask has been changed outside of this class
    {
        return true;
    }

    return false;
}

}






