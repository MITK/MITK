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

#include "itkCommand.h"

#include "mitkTimeFramesRegistrationHelper.h"
#include <mitkImageTimeSelector.h>
#include <mitkImageReadAccessor.h>

#include <mitkMaskedAlgorithmHelper.h>
#include <mitkAlgorithmHelper.h>

mitk::Image::Pointer
mitk::TimeFramesRegistrationHelper::GetFrameImage(const mitk::Image* image,
    mitk::TimePointType timePoint) const
{
  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(timePoint);
  imageTimeSelector->UpdateLargestPossibleRegion();

  mitk::Image::Pointer frameImage = imageTimeSelector->GetOutput();
  return frameImage;
};

void
mitk::TimeFramesRegistrationHelper::Generate()
{
  CheckValidInputs();

  //prepare processing
  mitk::Image::Pointer targetFrame = GetFrameImage(this->m_4DImage, 0);

  this->m_Registered4DImage = this->m_4DImage->Clone();

  Image::ConstPointer mask;

  if (m_TargetMask.IsNotNull())
  {
    if (m_TargetMask->GetTimeSteps() > 1)
    {
      mask = GetFrameImage(m_TargetMask, 0);
    }
    else
    {
      mask = m_TargetMask;
    }
  }

  double progressDelta = 1.0 / ((this->m_4DImage->GetTimeSteps() - 1) * 3.0);
  m_Progress = 0.0;

  //process the frames
  for (unsigned int i = 1; i < this->m_4DImage->GetTimeSteps(); ++i)
  {
    Image::Pointer movingFrame = GetFrameImage(this->m_4DImage, i);
    Image::Pointer mappedFrame;

    IgnoreListType::iterator finding = std::find(m_IgnoreList.begin(), m_IgnoreList.end(), i);


    if (finding == m_IgnoreList.end())
    {
      //frame should be processed
      RegistrationPointer reg = DoFrameRegistration(movingFrame, targetFrame, mask);

      m_Progress += progressDelta;
      this->InvokeEvent(::mitk::FrameRegistrationEvent(nullptr,
                        "Registred frame #" +::map::core::convert::toStr(i)));

      mappedFrame = DoFrameMapping(movingFrame, reg, targetFrame);

      m_Progress += progressDelta;
      this->InvokeEvent(::mitk::FrameMappingEvent(nullptr,
                        "Mapped frame #" + ::map::core::convert::toStr(i)));

      mitk::ImageReadAccessor accessor(mappedFrame, mappedFrame->GetVolumeData(0, 0, nullptr,
                                       mitk::Image::ReferenceMemory));


      this->m_Registered4DImage->SetVolume(accessor.GetData(), i);
      this->m_Registered4DImage->GetTimeGeometry()->SetTimeStepGeometry(mappedFrame->GetGeometry(), i);

      m_Progress += progressDelta;
    }
    else
    {
      m_Progress += 3 * progressDelta;
    }

    this->InvokeEvent(::itk::ProgressEvent());

  }

};

mitk::Image::Pointer
mitk::TimeFramesRegistrationHelper::GetRegisteredImage()
{
  if (this->HasOutdatedResult())
  {
    Generate();
  }

  return m_Registered4DImage;
};

void
mitk::TimeFramesRegistrationHelper::
SetIgnoreList(const IgnoreListType& il)
{
  m_IgnoreList = il;
  this->Modified();
}

void
mitk::TimeFramesRegistrationHelper::ClearIgnoreList()
{
  m_IgnoreList.clear();
  this->Modified();
};


mitk::TimeFramesRegistrationHelper::RegistrationPointer
mitk::TimeFramesRegistrationHelper::DoFrameRegistration(const mitk::Image* movingFrame,
    const mitk::Image* targetFrame, const mitk::Image* targetMask) const
{
  mitk::MITKAlgorithmHelper algHelper(m_Algorithm);
  algHelper.SetAllowImageCasting(true);
  algHelper.SetData(movingFrame, targetFrame);

  if (targetMask)
  {
    mitk::MaskedAlgorithmHelper maskHelper(m_Algorithm);
    maskHelper.SetMasks(nullptr, targetMask);
  }

  return algHelper.GetRegistration();
};

mitk::Image::Pointer mitk::TimeFramesRegistrationHelper::DoFrameMapping(
  const mitk::Image* movingFrame, const RegistrationType* reg, const mitk::Image* targetFrame) const
{
  return mitk::ImageMappingHelper::map(movingFrame, reg, !m_AllowUndefPixels, m_PaddingValue,
                                      targetFrame->GetGeometry(), !m_AllowUnregPixels, m_ErrorValue, m_InterpolatorType);
};

bool
mitk::TimeFramesRegistrationHelper::HasOutdatedResult() const
{
  if (m_Registered4DImage.IsNull())
  {
    return true;
  }

  bool result = false;

  if (m_Registered4DImage->GetMTime() > this->GetMTime())
  {
    result = true;
  }


  if (m_Algorithm.IsNotNull())
  {
    if (m_Algorithm->GetMTime() > this->GetMTime())
    {
      result = true;
    }
  }

  if (m_4DImage.IsNotNull())
  {
    if (m_4DImage->GetMTime() > this->GetMTime())
    {
      result = true;
    }
  }

  if (m_TargetMask.IsNotNull())
  {
    if (m_TargetMask->GetMTime() > this->GetMTime())
    {
      result = true;
    }
  }

  return result;
};

void
mitk::TimeFramesRegistrationHelper::CheckValidInputs() const
{
  if (m_4DImage.IsNull())
  {
    mitkThrow() << "Cannot register image. Input 4D image is not set.";
  }

  if (m_Algorithm.IsNull())
  {
    mitkThrow() << "Cannot register image. Algorithm is not set.";
  }

  if (m_4DImage->GetTimeSteps() <= 1)
  {
    mitkThrow() << "Cannot register image. Input 4D image must have 2 or more time steps.";
  }

  for (IgnoreListType::const_iterator pos = this->m_IgnoreList.begin();
       pos != this->m_IgnoreList.end(); ++pos)
  {
    if (*pos >= m_4DImage->GetTimeSteps())
    {
      mitkThrow() <<
                  "Cannot register image. Ignore list containes at least one inexistant frame. Invalid frame index: "
                  << *pos;
    }
  }
};

double
mitk::TimeFramesRegistrationHelper::GetProgress() const
{
  return m_Progress;
};
