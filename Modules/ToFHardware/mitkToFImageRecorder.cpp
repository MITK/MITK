/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFImageRecorder.h"
#include <mitkRealTimeClock.h>
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk
{
ToFImageRecorder::ToFImageRecorder()
{
  this->m_ToFCameraDevice = nullptr;
  this->m_MultiThreader = itk::MultiThreader::New();
  this->m_AbortMutex = itk::FastMutexLock::New();
  this->m_ThreadID = 0;
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

  this->m_AbortMutex->Lock();
  this->m_Abort = true;
  this->m_AbortMutex->Unlock();

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

  this->m_AbortMutex->Lock();
  this->m_Abort = false;
  this->m_AbortMutex->Unlock();
  this->m_ThreadID = this->m_MultiThreader->SpawnThread(this->RecordData, this);
}

void ToFImageRecorder::WaitForThreadBeingTerminated()
{
  this->m_MultiThreader->TerminateThread(this->m_ThreadID);
}

ITK_THREAD_RETURN_TYPE ToFImageRecorder::RecordData(void* pInfoStruct)
{
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  ToFImageRecorder* toFImageRecorder = (ToFImageRecorder*)pInfo->UserData;
  if (toFImageRecorder!=nullptr)
  {

    ToFCameraDevice::Pointer toFCameraDevice = toFImageRecorder->GetCameraDevice();

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
    toFImageRecorder->m_AbortMutex->Lock();
    abort = toFImageRecorder->m_Abort;
    toFImageRecorder->m_AbortMutex->Unlock();
    while ( !abort )
    {
      if ( ((toFImageRecorder->m_RecordMode == ToFImageRecorder::PerFrames) && (numOfFramesRecorded < toFImageRecorder->m_NumOfFrames)) ||
           (toFImageRecorder->m_RecordMode == ToFImageRecorder::Infinite) )
      {

        toFCameraDevice->GetAllImages(toFImageRecorder->m_DistanceArray, toFImageRecorder->m_AmplitudeArray,
                                      toFImageRecorder->m_IntensityArray, toFImageRecorder->m_SourceDataArray, requiredImageSequence, toFImageRecorder->m_ImageSequence, toFImageRecorder->m_RGBArray );

        if (toFImageRecorder->m_ImageSequence >= requiredImageSequence)
        {
          if (toFImageRecorder->m_ImageSequence > requiredImageSequence)
          {
            MITK_INFO << "Problem! required: " << requiredImageSequence << " captured: " << toFImageRecorder->m_ImageSequence;
          }
          requiredImageSequence = toFImageRecorder->m_ImageSequence + 1;
          toFImageRecorder->m_ToFImageWriter->Add( toFImageRecorder->m_DistanceArray,
                                                   toFImageRecorder->m_AmplitudeArray, toFImageRecorder->m_IntensityArray, toFImageRecorder->m_RGBArray );
          numOfFramesRecorded++;
          if (numOfFramesRecorded % n == 0)
          {
            printStatus = true;
          }
          if (printStatus)
          {
            t2 = realTimeClock->GetCurrentStamp() - t1;
            MITK_INFO << " Framerate (fps): " << n / (t2/1000) << " Sequence: " << toFImageRecorder->m_ImageSequence;
            t1 = realTimeClock->GetCurrentStamp();
            printStatus = false;
          }
        }
        toFImageRecorder->m_AbortMutex->Lock();
        abort = toFImageRecorder->m_Abort;
        toFImageRecorder->m_AbortMutex->Unlock();
      }
      else
      {
        abort = true;
      }
    }  // end of while loop

    toFImageRecorder->InvokeEvent(itk::AbortEvent());

    toFImageRecorder->m_ToFImageWriter->Close();
  }
  return ITK_THREAD_RETURN_VALUE;
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
