/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFImageRecorder.h"
#include <mitkRealTimeClock.h>
#include <itksys/SystemTools.hxx>
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk
{
ToFImageRecorder::ToFImageRecorder()
{
  this->m_ToFCameraDevice = nullptr;
  this->m_NumOfFrames = 1; //lets record one frame per default
  this->m_ToFImageWriter = nullptr;
  this->m_DistanceImageSelected = true; //lets assume a device only has depth data by default
  this->m_AmplitudeImageSelected = false;
  this->m_IntensityImageSelected = false;
  this->m_RGBImageSelected = false;
  this->m_Abort = false;
  this->m_ToFCaptureWidth = 0;
  this->m_ToFCaptureHeight = 0;
  this->m_RGBCaptureWidth = 0;
  this->m_RGBCaptureHeight = 0;
  this->m_FileFormat = ".nrrd"; //lets make nrrd the default
  this->m_ToFPixelNumber = 0;
  this->m_RGBPixelNumber = 0;
  this->m_SourceDataSize = 0;
  this->m_ToFImageType = ToFImageWriter::ToFImageType3D;
  this->m_RecordMode = ToFImageRecorder::PerFrames;
  this->m_DistanceImageFileName = "";
  this->m_AmplitudeImageFileName = "";
  this->m_IntensityImageFileName = "";
  this->m_RGBImageFileName = "";
  this->m_ImageSequence = 0;
  this->m_DistanceArray = nullptr;
  this->m_AmplitudeArray = nullptr;
  this->m_IntensityArray = nullptr;
  this->m_RGBArray = nullptr;
  this->m_SourceDataArray = nullptr;
}

ToFImageRecorder::~ToFImageRecorder()
{
  delete[] m_DistanceArray;
  delete[] m_AmplitudeArray;
  delete[] m_IntensityArray;
  delete[] m_RGBArray;
  delete[] m_SourceDataArray;
}

void ToFImageRecorder::StopRecording()
{

  this->m_AbortMutex.lock();
  this->m_Abort = true;
  this->m_AbortMutex.unlock();

}

void ToFImageRecorder::StartRecording()
{
  if (this->m_ToFCameraDevice.IsNull())
  {
    throw std::invalid_argument("ToFCameraDevice is nullptr.");
    return;
  }
  if (this->m_FileFormat.compare(".csv") == 0)
  {
    this->m_ToFImageWriter = ToFImageCsvWriter::New();
  }
  else if(this->m_FileFormat.compare(".nrrd") == 0)
  {
    this->m_ToFImageWriter = ToFNrrdImageWriter::New();
    this->m_ToFImageWriter->SetExtension(m_FileFormat);
  }
  else
  {
    throw std::logic_error("No file format specified!");
  }

  this->m_RGBCaptureWidth = this->m_ToFCameraDevice->GetRGBCaptureWidth();
  this->m_RGBCaptureHeight = this->m_ToFCameraDevice->GetRGBCaptureHeight();
  this->m_RGBPixelNumber = this->m_RGBCaptureWidth * this->m_RGBCaptureHeight;

  this->m_ToFCaptureWidth = this->m_ToFCameraDevice->GetCaptureWidth();
  this->m_ToFCaptureHeight = this->m_ToFCameraDevice->GetCaptureHeight();
  this->m_ToFPixelNumber = this->m_ToFCaptureWidth * this->m_ToFCaptureHeight;

  this->m_SourceDataSize = this->m_ToFCameraDevice->GetSourceDataSize();

  // allocate buffer
  if(m_IntensityArray == nullptr)
  {
    this->m_IntensityArray = new float[m_ToFPixelNumber];
  }
  if(this->m_DistanceArray == nullptr)
  {
    this->m_DistanceArray = new float[m_ToFPixelNumber];
  }
  if(this->m_AmplitudeArray == nullptr)
  {
    this->m_AmplitudeArray = new float[m_ToFPixelNumber];
  }
  if(this->m_RGBArray == nullptr)
  {
    this->m_RGBArray = new unsigned char[m_RGBPixelNumber*3];
  }
  if(this->m_SourceDataArray == nullptr)
  {
    this->m_SourceDataArray = new char[m_SourceDataSize];
  }

  this->m_ToFImageWriter->SetDistanceImageFileName(this->m_DistanceImageFileName);
  this->m_ToFImageWriter->SetAmplitudeImageFileName(this->m_AmplitudeImageFileName);
  this->m_ToFImageWriter->SetIntensityImageFileName(this->m_IntensityImageFileName);
  this->m_ToFImageWriter->SetRGBImageFileName(this->m_RGBImageFileName);
  this->m_ToFImageWriter->SetRGBCaptureWidth(this->m_RGBCaptureWidth);
  this->m_ToFImageWriter->SetRGBCaptureHeight(this->m_RGBCaptureHeight);
  this->m_ToFImageWriter->SetToFCaptureWidth(this->m_ToFCaptureWidth);
  this->m_ToFImageWriter->SetToFCaptureHeight(this->m_ToFCaptureHeight);
  this->m_ToFImageWriter->SetToFImageType(this->m_ToFImageType);
  this->m_ToFImageWriter->SetDistanceImageSelected(this->m_DistanceImageSelected);
  this->m_ToFImageWriter->SetAmplitudeImageSelected(this->m_AmplitudeImageSelected);
  this->m_ToFImageWriter->SetIntensityImageSelected(this->m_IntensityImageSelected);
  this->m_ToFImageWriter->SetRGBImageSelected(this->m_RGBImageSelected);
  this->m_ToFImageWriter->Open();

  this->m_AbortMutex.lock();
  this->m_Abort = false;
  this->m_AbortMutex.unlock();
  m_Thread = std::thread(&ToFImageRecorder::RecordData, this);
}

void ToFImageRecorder::WaitForThreadBeingTerminated()
{
  if (m_Thread.joinable())
    m_Thread.join();
}

void ToFImageRecorder::RecordData()
{
  ToFCameraDevice::Pointer toFCameraDevice = this->GetCameraDevice();

  mitk::RealTimeClock::Pointer realTimeClock;
  realTimeClock = mitk::RealTimeClock::New();
  int n = 100;
  double t1 = 0;
  double t2 = 0;
  t1 = realTimeClock->GetCurrentStamp();
  bool printStatus = false;
  int requiredImageSequence = 0;
  int numOfFramesRecorded = 0;

  bool abort = false;
  m_AbortMutex.lock();
  abort = m_Abort;
  m_AbortMutex.unlock();
  while ( !abort )
  {
    if ( ((m_RecordMode == ToFImageRecorder::PerFrames) && (numOfFramesRecorded < m_NumOfFrames)) ||
          (m_RecordMode == ToFImageRecorder::Infinite) )
    {

      toFCameraDevice->GetAllImages(m_DistanceArray, m_AmplitudeArray,
                                    m_IntensityArray, m_SourceDataArray, requiredImageSequence, m_ImageSequence, m_RGBArray );

      if (m_ImageSequence >= requiredImageSequence)
      {
        if (m_ImageSequence > requiredImageSequence)
        {
          MITK_INFO << "Problem! required: " << requiredImageSequence << " captured: " << m_ImageSequence;
        }
        requiredImageSequence = m_ImageSequence + 1;
        m_ToFImageWriter->Add(m_DistanceArray, m_AmplitudeArray, m_IntensityArray, m_RGBArray );
        numOfFramesRecorded++;
        if (numOfFramesRecorded % n == 0)
        {
          printStatus = true;
        }
        if (printStatus)
        {
          t2 = realTimeClock->GetCurrentStamp() - t1;
          MITK_INFO << " Framerate (fps): " << n / (t2/1000) << " Sequence: " << m_ImageSequence;
          t1 = realTimeClock->GetCurrentStamp();
          printStatus = false;
        }
      }
      m_AbortMutex.lock();
      abort = m_Abort;
      m_AbortMutex.unlock();
    }
    else
    {
      abort = true;
    }
  }  // end of while loop

  this->InvokeEvent(itk::AbortEvent());

  m_ToFImageWriter->Close();
}

void ToFImageRecorder::SetCameraDevice(ToFCameraDevice* aToFCameraDevice)
{
  this->m_ToFCameraDevice = aToFCameraDevice;
}

ToFCameraDevice* ToFImageRecorder::GetCameraDevice()
{
  return this->m_ToFCameraDevice;
}

ToFImageWriter::ToFImageType ToFImageRecorder::GetToFImageType()
{
  return this->m_ToFImageType;
}

void ToFImageRecorder::SetToFImageType(ToFImageWriter::ToFImageType toFImageType)
{
  this->m_ToFImageType = toFImageType;
}

ToFImageRecorder::RecordMode ToFImageRecorder::GetRecordMode()
{
  return this->m_RecordMode;
}

void ToFImageRecorder::SetRecordMode(ToFImageRecorder::RecordMode recordMode)
{
  this->m_RecordMode = recordMode;
}
}
