/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkToFImageRecorder.h"
#include "mitkRealTimeClock.h"
#include "itkMultiThreader.h"
#include <itksys/SystemTools.hxx>
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk
{
  ToFImageRecorder::ToFImageRecorder()
  {
    this->m_ToFCameraDevice = NULL;
    this->m_MultiThreader = itk::MultiThreader::New();
    this->m_AbortMutex = itk::FastMutexLock::New();
    this->m_ThreadID = 0;
    this->m_NumOfFrames = 0;
    this->m_ToFImageWriter = NULL;
    this->m_DistanceImageSelected = true;
    this->m_AmplitudeImageSelected = true;
    this->m_IntensityImageSelected = true;
    this->m_Abort = true;
    this->m_CaptureWidth = 0;
    this->m_CaptureHeight = 0;
    this->m_FileFormat = "";
    this->m_PixelNumber = 0;
    this->m_SourceDataSize = 0;
    this->m_ToFImageType = ToFImageWriter::ToFImageType3D;
    this->m_RecordMode = ToFImageRecorder::PerFrames;
    this->m_DistanceImageFileName = ""; 
    this->m_AmplitudeImageFileName = "";
    this->m_IntensityImageFileName = "";
    this->m_ImageSequence = 0;
    this->m_DistanceArray = NULL;
    this->m_AmplitudeArray = NULL;
    this->m_IntensityArray = NULL;
    this->m_SourceDataArray = NULL;
  }

  ToFImageRecorder::~ToFImageRecorder()
  {
    delete[] m_DistanceArray;
    delete[] m_AmplitudeArray;
    delete[] m_IntensityArray;
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
      throw std::invalid_argument("ToFCameraDevice is NULL.");
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
    else if(this->m_FileFormat.compare(".pic") == 0)
    {
      this->m_ToFImageWriter = ToFPicImageWriter::New();
      this->m_ToFImageWriter->SetExtension(m_FileFormat);
    }
    else
    {
      throw std::logic_error("No file format specified!");
    }

    this->m_CaptureWidth = this->m_ToFCameraDevice->GetCaptureWidth();
    this->m_CaptureHeight = this->m_ToFCameraDevice->GetCaptureHeight();
    this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;
    this->m_SourceDataSize = this->m_ToFCameraDevice->GetSourceDataSize();

    // allocate buffer
    if(m_IntensityArray == NULL)
    {
      this->m_IntensityArray = new float[m_PixelNumber];
    }
    if(this->m_DistanceArray == NULL)
    {
      this->m_DistanceArray = new float[m_PixelNumber];
    }
    if(this->m_AmplitudeArray == NULL)
    {
      this->m_AmplitudeArray = new float[m_PixelNumber];
    }
    if(this->m_SourceDataArray == NULL)
    {
      this->m_SourceDataArray = new char[m_SourceDataSize];
    }

    this->m_ToFImageWriter->SetDistanceImageFileName(this->m_DistanceImageFileName);
    this->m_ToFImageWriter->SetAmplitudeImageFileName(this->m_AmplitudeImageFileName);
    this->m_ToFImageWriter->SetIntensityImageFileName(this->m_IntensityImageFileName);
    this->m_ToFImageWriter->SetCaptureWidth(this->m_CaptureWidth);
    this->m_ToFImageWriter->SetCaptureHeight(this->m_CaptureHeight);
    this->m_ToFImageWriter->SetToFImageType(this->m_ToFImageType);
    this->m_ToFImageWriter->SetDistanceImageSelected(this->m_DistanceImageSelected);
    this->m_ToFImageWriter->SetAmplitudeImageSelected(this->m_AmplitudeImageSelected);
    this->m_ToFImageWriter->SetIntensityImageSelected(this->m_IntensityImageSelected);
    this->m_ToFImageWriter->Open();

    this->m_AbortMutex->Lock();
    this->m_Abort = false;
    this->m_AbortMutex->Unlock();
    this->m_ThreadID = this->m_MultiThreader->SpawnThread(this->RecordData, this);
  }

  ITK_THREAD_RETURN_TYPE ToFImageRecorder::RecordData(void* pInfoStruct)
  {
    struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
    if (pInfo == NULL)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    if (pInfo->UserData == NULL)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    ToFImageRecorder* toFImageRecorder = (ToFImageRecorder*)pInfo->UserData;
    if (toFImageRecorder!=NULL)
    {

      ToFCameraDevice::Pointer toFCameraDevice = toFImageRecorder->GetCameraDevice();

      mitk::RealTimeClock::Pointer realTimeClock;
      realTimeClock = mitk::RealTimeClock::New();
      int n = 100; 
      double t1 = 0;
      double t2 = 0;
      t1 = realTimeClock->GetCurrentStamp();
      bool overflow = false;
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
            toFImageRecorder->m_IntensityArray, toFImageRecorder->m_SourceDataArray, requiredImageSequence, toFImageRecorder->m_ImageSequence );

          if (toFImageRecorder->m_ImageSequence >= requiredImageSequence)
          {
            if (toFImageRecorder->m_ImageSequence > requiredImageSequence)
            {
              MITK_INFO << "Problem! required: " << requiredImageSequence << " captured: " << toFImageRecorder->m_ImageSequence;
            }
            requiredImageSequence = toFImageRecorder->m_ImageSequence + 1;
            toFImageRecorder->m_ToFImageWriter->Add( toFImageRecorder->m_DistanceArray, 
              toFImageRecorder->m_AmplitudeArray, toFImageRecorder->m_IntensityArray );
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
