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
#include "mitkToFCameraPMDCamBoardDevice.h"
#include "mitkToFCameraPMDCamBoardController.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"


namespace mitk
{
  ToFCameraPMDCamBoardDevice::ToFCameraPMDCamBoardDevice()
  {
    m_Controller = ToFCameraPMDCamBoardController::New();
  }

  ToFCameraPMDCamBoardDevice::~ToFCameraPMDCamBoardDevice()
  {
  }

  void ToFCameraPMDCamBoardDevice::SetRegionOfInterest(unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height)
  {
    if (m_Controller.IsNotNull())
    {
      dynamic_cast<ToFCameraPMDCamBoardController*>(m_Controller.GetPointer())->SetRegionOfInterest(leftUpperCornerX,leftUpperCornerY,width,height);
    }
  }

  void ToFCameraPMDCamBoardDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraPMDCamBoardController::Pointer myController = dynamic_cast<mitk::ToFCameraPMDCamBoardController*>(this->m_Controller.GetPointer());

    bool boolValue = false;
    GetBoolProperty(propertyKey, boolValue);
    if (strcmp(propertyKey, "SetFPNCalibration") == 0)
    {
      myController->SetFPNCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetFPPNCalibration") == 0)
    {
      myController->SetFPPNCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetLinearityCalibration") == 0)
    {
      myController->SetLinearityCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetLensCalibration") == 0)
    {
      myController->SetLensCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetExposureMode") == 0)
    {
      if (boolValue)
      {
        myController->SetExposureMode(1);
      }
      else
      {
        myController->SetExposureMode(0);
      }
    }
  }
  void ToFCameraPMDCamBoardDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
    int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray)
  {
    if (m_CameraActive)
    {
      // 1) copy the image buffer
      // 2) convert the distance values from m to mm

      // check for empty buffer
      if (this->m_ImageSequence < 0)
      {
        // buffer empty
        MITK_INFO << "Buffer empty!! ";
        capturedImageSequence = this->m_ImageSequence;
        return;
      }
      // determine position of image in buffer
      int pos = 0;
      if ((requiredImageSequence < 0) || (requiredImageSequence > this->m_ImageSequence))
      {
        capturedImageSequence = this->m_ImageSequence;
        pos = this->m_CurrentPos;
        //MITK_INFO << "Required image not found! Required: " << requiredImageSequence << " delivered/current: " << this->m_ImageSequence;
      }
      else if (requiredImageSequence <= this->m_ImageSequence - this->m_BufferSize)
      {
        capturedImageSequence = (this->m_ImageSequence - this->m_BufferSize) + 1;
        pos = (this->m_CurrentPos + 1) % this->m_BufferSize;
        //MITK_INFO << "Out of buffer! Required: " << requiredImageSequence << " delivered: " << capturedImageSequence << " current: " << this->m_ImageSequence;
      }
      else // (requiredImageSequence > this->m_ImageSequence - this->m_BufferSize) && (requiredImageSequence <= this->m_ImageSequence)
      {
        capturedImageSequence = requiredImageSequence;
        pos = (this->m_CurrentPos + (10-(this->m_ImageSequence - requiredImageSequence))) % this->m_BufferSize;
      }

      m_ImageMutex->Lock();
      this->m_Controller->GetDistances(this->m_SourceDataBuffer[pos], distanceArray);
      this->m_Controller->GetAmplitudes(this->m_SourceDataBuffer[pos], amplitudeArray);
      this->m_Controller->GetIntensities(this->m_SourceDataBuffer[pos], intensityArray);
      memcpy(sourceDataArray, this->m_SourceDataBuffer[this->m_CurrentPos], this->m_SourceDataSize);
      m_ImageMutex->Unlock();

    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

}
