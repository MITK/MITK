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
#include <mitkToFCameraMITKPlayerController.h>
#include <itksys/SystemTools.hxx>

// mitk includes
#include "mitkItkImageFileReader.h"
#include "mitkIOUtil.h"
#include <mitkIpPic.h>
#include "mitkImageReadAccessor.h"

namespace mitk
{

ToFCameraMITKPlayerController::ToFCameraMITKPlayerController() :
  m_PixelNumber(0),
  m_RGBPixelNumber(0),
  m_NumberOfBytes(0),
  m_NumberOfRGBBytes(0),
  m_CaptureWidth(0),
  m_CaptureHeight(0),
  m_RGBCaptureWidth(0),
  m_RGBCaptureHeight(0),
  m_ConnectionCheck(false),
  m_InputFileName(""),
  m_ToFImageType(ToFImageType3D),
  m_DistanceImage(0),
  m_AmplitudeImage(0),
  m_IntensityImage(0),
  m_RGBImage(0),
  m_DistanceInfile(NULL),
  m_AmplitudeInfile(NULL),
  m_IntensityInfile(NULL),
  m_RGBInfile(NULL),
  m_IntensityArray(NULL),
  m_DistanceArray(NULL),
  m_AmplitudeArray(NULL),
  m_RGBArray(NULL),
  m_DistanceImageFileName(""),
  m_AmplitudeImageFileName(""),
  m_IntensityImageFileName(""),
  m_RGBImageFileName(""),
  m_PixelStartInFile(0),
  m_CurrentFrame(-1),
  m_NumOfFrames(0)
{
  m_ImageStatus = std::vector<bool>(4,true);
}

ToFCameraMITKPlayerController::~ToFCameraMITKPlayerController()
{
  this->CleanUp();
}

void ToFCameraMITKPlayerController::CleanUp()
{
  if(m_DistanceImage.IsNotNull())
  {
    m_DistanceImage->ReleaseData();
    m_DistanceImage = NULL;
  }
  if(m_AmplitudeImage.IsNotNull())
  {
    m_AmplitudeImage->ReleaseData();
    m_AmplitudeImage = NULL;
  }
  if(m_IntensityImage.IsNotNull())
  {
    m_IntensityImage->ReleaseData();
    m_IntensityImage = NULL;
  }
  if(m_RGBImage.IsNotNull())
  {
    m_RGBImage->ReleaseData();
    m_RGBImage = NULL;
  }

  delete[] this->m_DistanceArray;
  this->m_DistanceArray = NULL;
  delete[] this->m_AmplitudeArray;
  this->m_AmplitudeArray = NULL;
  delete[] this->m_IntensityArray;
  this->m_IntensityArray = NULL;
  delete[] this->m_RGBArray;
  this->m_RGBArray = NULL;

  this->m_DistanceImageFileName = "";
  this->m_AmplitudeImageFileName = "";
  this->m_IntensityImageFileName = "";
  this->m_RGBImageFileName = "";
}

bool ToFCameraMITKPlayerController::OpenCameraConnection()
{
  if(!this->m_ConnectionCheck)
  {
    // reset the image status before connection
    m_ImageStatus = std::vector<bool>(4,true);
    try
    {
      if (this->m_DistanceImageFileName.empty() &&
          this->m_AmplitudeImageFileName.empty() &&
          this->m_IntensityImageFileName.empty() &&
          this->m_RGBImageFileName.empty())
      {
        throw std::logic_error("No image data file names set");
      }

      if (!this->m_DistanceImageFileName.empty())
      {
        m_DistanceImage = mitk::IOUtil::LoadImage(this->m_DistanceImageFileName);
      }
      else
      {
        MITK_ERROR << "ToF distance image data file empty";
      }
      if (!this->m_AmplitudeImageFileName.empty())
      {
        m_AmplitudeImage = mitk::IOUtil::LoadImage(this->m_AmplitudeImageFileName);
      }
      else
      {
        MITK_WARN << "ToF amplitude image data file empty";
      }
      if (!this->m_IntensityImageFileName.empty())
      {
        m_IntensityImage = mitk::IOUtil::LoadImage(this->m_IntensityImageFileName);
      }
      else
      {
        MITK_WARN << "ToF intensity image data file empty";
      }
      if (!this->m_RGBImageFileName.empty())
      {
        m_RGBImage = mitk::IOUtil::LoadImage(this->m_RGBImageFileName);
      }
      else
      {
        MITK_WARN << "ToF RGB image data file empty";
      }

      // check if the opened files contained data
      if(m_DistanceImage.IsNull())
      {
        m_ImageStatus.at(0) = false;
      }
      if(m_AmplitudeImage.IsNull())
      {
        m_ImageStatus.at(1) = false;
      }
      if(m_IntensityImage.IsNull())
      {
        m_ImageStatus.at(2) = false;
      }
      if(m_RGBImage.IsNull())
      {
        m_ImageStatus.at(3) = false;
      }

      // Check for dimension type
      mitk::Image::Pointer infoImage = NULL;
      if(m_ImageStatus.at(0))
      {
        infoImage = m_DistanceImage;
      }
      else if (m_ImageStatus.at(1))
      {
        infoImage = m_AmplitudeImage;
      }
      else if(m_ImageStatus.at(2))
      {
        infoImage = m_IntensityImage;
      }
      else if(m_ImageStatus.at(3))
      {
        infoImage = m_RGBImage;
      }

      if (infoImage->GetDimension() == 2)
        this->m_ToFImageType = ToFImageType2DPlusT;

      else if (infoImage->GetDimension() == 3)
        this->m_ToFImageType = ToFImageType3D;

      else if (infoImage->GetDimension() == 4)
        this->m_ToFImageType = ToFImageType2DPlusT;

      else
        throw std::logic_error("Error opening ToF data file: Invalid dimension.");

      this->m_CaptureWidth = infoImage->GetDimension(0);
      this->m_CaptureHeight = infoImage->GetDimension(1);
      this->m_PixelNumber = this->m_CaptureWidth*this->m_CaptureHeight;
      this->m_NumberOfBytes = this->m_PixelNumber * sizeof(float);

      if(m_RGBImage)
      {
        m_RGBCaptureWidth = m_RGBImage->GetDimension(0);
        m_RGBCaptureHeight = m_RGBImage->GetDimension(1);
        m_RGBPixelNumber = m_RGBCaptureWidth * m_RGBCaptureHeight;
        m_NumberOfRGBBytes = m_RGBPixelNumber * 3;
      }

      if (this->m_ToFImageType == ToFImageType2DPlusT)
      {
        this->m_NumOfFrames = infoImage->GetDimension(3);
      }
      else
      {
        this->m_NumOfFrames = infoImage->GetDimension(2);
      }

      // allocate buffer
      this->m_DistanceArray = new float[this->m_PixelNumber];
      for(int i=0; i<this->m_PixelNumber; i++) {this->m_DistanceArray[i]=0.0;}
      this->m_AmplitudeArray = new float[this->m_PixelNumber];
      for(int i=0; i<this->m_PixelNumber; i++) {this->m_AmplitudeArray[i]=0.0;}
      this->m_IntensityArray = new float[this->m_PixelNumber];
      for(int i=0; i<this->m_PixelNumber; i++) {this->m_IntensityArray[i]=0.0;}
      this->m_RGBArray = new unsigned char[m_NumberOfRGBBytes];
      for(int i=0; i<m_NumberOfRGBBytes; i++) {this->m_RGBArray[i]=0.0;}

      MITK_INFO << "NumOfFrames: " << this->m_NumOfFrames;

      this->m_ConnectionCheck = true;
      return this->m_ConnectionCheck;
    }
    catch(std::exception& e)
    {
      MITK_ERROR << "Error opening ToF data file " << this->m_InputFileName << " " << e.what();
      throw std::logic_error("Error opening ToF data file");
      return false;
    }
  }
  else
    return this->m_ConnectionCheck;
}

bool ToFCameraMITKPlayerController::CloseCameraConnection()
{
  if (this->m_ConnectionCheck)
  {
    this->CleanUp();
    this->m_ConnectionCheck = false;
    return true;
  }
  return false;
}

void ToFCameraMITKPlayerController::UpdateCamera()
{
  this->m_CurrentFrame++;
  if(this->m_CurrentFrame >= this->m_NumOfFrames)
  {
    this->m_CurrentFrame = 0;
  }

  if(this->m_ImageStatus.at(0))
  {
    this->AccessData(this->m_CurrentFrame, this->m_DistanceImage, this->m_DistanceArray);
  }
  if(this->m_ImageStatus.at(1))
  {
    this->AccessData(this->m_CurrentFrame, this->m_AmplitudeImage, this->m_AmplitudeArray);
  }
  if(this->m_ImageStatus.at(2))
  {
    this->AccessData(this->m_CurrentFrame, this->m_IntensityImage, this->m_IntensityArray);
  }
  if(this->m_ImageStatus.at(3))
  {
    if(!this->m_ToFImageType)
    {
      ImageReadAccessor rgbAcc(m_RGBImage, m_RGBImage->GetSliceData(m_CurrentFrame));
      memcpy(m_RGBArray, rgbAcc.GetData(), m_NumberOfRGBBytes );
    }
    else if(this->m_ToFImageType)
    {
      ImageReadAccessor rgbAcc(m_RGBImage, m_RGBImage->GetVolumeData(m_CurrentFrame));
      memcpy(m_RGBArray, rgbAcc.GetData(), m_NumberOfRGBBytes);
    }
  }
  itksys::SystemTools::Delay(50);
}

void ToFCameraMITKPlayerController::AccessData(int frame, Image::Pointer image, float* &data)
{
  if(!this->m_ToFImageType)
  {
    ImageReadAccessor imgAcc(image, image->GetSliceData(frame));
    memcpy(data, imgAcc.GetData(), this->m_NumberOfBytes );
  }
  else if(this->m_ToFImageType)
  {
    ImageReadAccessor imgAcc(image, image->GetVolumeData(frame));
    memcpy(data, imgAcc.GetData(), this->m_NumberOfBytes);
  }
}

void ToFCameraMITKPlayerController::GetAmplitudes(float* amplitudeArray)
{
  memcpy(amplitudeArray, this->m_AmplitudeArray, this->m_NumberOfBytes);
}

void ToFCameraMITKPlayerController::GetIntensities(float* intensityArray)
{
  memcpy(intensityArray, this->m_IntensityArray, this->m_NumberOfBytes);
}

void ToFCameraMITKPlayerController::GetDistances(float* distanceArray)
{
  memcpy(distanceArray, this->m_DistanceArray, this->m_NumberOfBytes);
}

void ToFCameraMITKPlayerController::GetRgb(unsigned char* rgbArray)
{
  memcpy(rgbArray, this->m_RGBArray, m_NumberOfRGBBytes);
}

void ToFCameraMITKPlayerController::SetInputFileName(std::string inputFileName)
{
  this->m_InputFileName = inputFileName;
}

}
