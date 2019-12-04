/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFCameraPMDRawDataCamBoardDevice.h"
#include "mitkToFCameraPMDCamBoardController.h"

// vtk includes
#include "vtkSmartPointer.h"

namespace mitk
{
  ToFCameraPMDRawDataCamBoardDevice::ToFCameraPMDRawDataCamBoardDevice()
  {
    m_Controller = ToFCameraPMDCamBoardController::New();
  }

  ToFCameraPMDRawDataCamBoardDevice::~ToFCameraPMDRawDataCamBoardDevice()
  {
  }

  bool ToFCameraPMDRawDataCamBoardDevice::ConnectCamera()
  {
    bool ok = Superclass::ConnectCamera();
    if(ok)
    {
      this->m_OriginControllerHeight = static_cast<ToFCameraPMDCamBoardController*>(m_Controller.GetPointer())->GetCaptureHeight();
      this->m_OriginControllerWidth = static_cast<ToFCameraPMDCamBoardController*>(m_Controller.GetPointer())->GetCaptureWidth();
      this->m_PixelNumber = m_OriginControllerHeight* m_OriginControllerWidth;
      this->AllocatePixelArrays();
      this->AllocateSourceData();
    }
    return ok;
  }

  void ToFCameraPMDRawDataCamBoardDevice::GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray )
  {
    int i = 0;
    //unsigned int channelDataPosition = 0;
    unsigned int channelSize = (this->m_OriginControllerHeight*this->m_OriginControllerWidth*2);
    this->SetChannelSize(channelSize);
    signed short* channel1;
    signed short* channel2;
    signed short* channel3;
    signed short* channel4;

    vtkSmartPointer<vtkShortArray> cvtkChannelArray = vtkSmartPointer<vtkShortArray>::New();
    cvtkChannelArray->SetNumberOfComponents(channelSize);
    cvtkChannelArray->SetNumberOfTuples(4);
    cvtkChannelArray->Allocate(1);

    channel1 = sourceData;
    cvtkChannelArray->InsertTupleValue(0,channel1);
    sourceData += channelSize;
    channel2 = sourceData;
    cvtkChannelArray->InsertTupleValue(1,channel2);
    sourceData += channelSize;
    channel3 = sourceData;
    cvtkChannelArray->InsertTupleValue(2,channel3);
    sourceData += channelSize;
    channel4 = sourceData;
    cvtkChannelArray->InsertTupleValue(3,channel4);
    vtkChannelArray->DeepCopy(cvtkChannelArray);
  }

  void ToFCameraPMDRawDataCamBoardDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraPMDRawDataDevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);
  }

  void ToFCameraPMDRawDataCamBoardDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
  {
    if (m_CameraActive)
    {
      this->ResizeOutputImage(m_AmplitudeArray,amplitudeArray);
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataCamBoardDevice::GetIntensities(float* intensityArray, int& imageSequence)
  {
    if (m_CameraActive)
    {
      this->ResizeOutputImage(m_IntensityArray, intensityArray);
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataCamBoardDevice::GetDistances(float* distanceArray, int& imageSequence)
  {
    if (m_CameraActive)
    {
      this->ResizeOutputImage(m_DistanceArray, distanceArray);
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataCamBoardDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
    int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray)
  {
    if (m_CameraActive)
    {
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

      this->ResizeOutputImage(m_DistanceArray, distanceArray);
      this->ResizeOutputImage(m_AmplitudeArray, amplitudeArray);
      this->ResizeOutputImage(m_IntensityArray, intensityArray);
      memcpy(sourceDataArray, this->m_SourceDataBuffer[this->m_CurrentPos], this->m_SourceDataSize);
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataCamBoardDevice::ResizeOutputImage(float* in, float* out)
  {
    vnl_matrix<float> inMat = vnl_matrix<float>(m_OriginControllerHeight, m_OriginControllerWidth);
    inMat.copy_in(in);
    vnl_matrix<float> outMat = vnl_matrix<float>(m_CaptureHeight,m_CaptureWidth);
    vnl_matrix<float> temp = vnl_matrix<float>(m_CaptureHeight,m_CaptureWidth);
    temp = inMat.extract(m_CaptureHeight,m_CaptureWidth, 0, 4);
    outMat = temp.transpose();    // rotate the image data
    outMat.flipud();              // flip image upside down
    outMat.copy_out(out);
    inMat.clear();                // clean data
    outMat.clear();
    temp.clear();
  }
}
