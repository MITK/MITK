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
#include "mitkPicFileReader.h"
#include <mitkIpPic.h>

namespace mitk
{

  ToFCameraMITKPlayerController::ToFCameraMITKPlayerController() :
    m_PixelNumber(0),
    m_NumberOfBytes(0),
    m_CaptureWidth(0),
    m_CaptureHeight(0),
    m_ConnectionCheck(false),
    m_InputFileName(""),
    m_Extension(""),
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
  }

 bool ToFCameraMITKPlayerController::CheckCurrentFileType()
  {
    if(!this->m_DistanceImageFileName.empty())
    {
      if(ItkImageFileReader::CanReadFile(m_DistanceImageFileName,"",".nrrd"))
      {
        m_Extension = ".nrrd";
        return true;
      }
      else if (PicFileReader::CanReadFile(m_DistanceImageFileName,"",".pic"))
      {
        m_Extension = ".pic";
        return true;
      }
    }

    if(!this->m_AmplitudeImageFileName.empty())
    {
      if(ItkImageFileReader::CanReadFile(m_AmplitudeImageFileName,"",".nrrd"))
      {
        m_Extension = ".nrrd";
        return true;
      }
      else if (PicFileReader::CanReadFile(m_AmplitudeImageFileName,"",".pic"))
      {
        m_Extension = ".pic";
        return true;
      }
    }

    if(!this->m_IntensityImageFileName.empty())
    {
      if(ItkImageFileReader::CanReadFile(m_IntensityImageFileName,"",".nrrd"))
      {
        m_Extension = ".nrrd";
        return true;
      }
      else if (PicFileReader::CanReadFile(m_IntensityImageFileName,"",".pic"))
      {
        m_Extension = ".pic";
        return true;
      }
    }
    if(!this->m_RGBImageFileName.empty())
    {
      if(ItkImageFileReader::CanReadFile(m_RGBImageFileName,"",".nrrd"))
      {
        m_Extension = ".nrrd";
        return true;
      }
      else if (PicFileReader::CanReadFile(m_RGBImageFileName,"",".pic"))
      {
        m_Extension = ".pic";
        return true;
      }
    }
    return false;
  }

  bool ToFCameraMITKPlayerController::OpenCameraConnection()
  {
    if(!this->m_ConnectionCheck)
    {
      try
      {
        // Check for file type, only .nrrd and .pic files are supported!
        if( this->CheckCurrentFileType())
        {
          if(m_Extension == ".nrrd")
          {
            this->OpenNrrdImageFile(this->m_DistanceImageFileName, m_DistanceImage);
            this->OpenNrrdImageFile(this->m_AmplitudeImageFileName, m_AmplitudeImage);
            this->OpenNrrdImageFile(this->m_IntensityImageFileName, m_IntensityImage);
            this->OpenNrrdImageFile(this->m_RGBImageFileName, m_RGBImage);
          }
          else if(m_Extension == ".pic")
          {
            this->OpenPicImageFile(this->m_DistanceImageFileName, m_DistanceImage);
            this->OpenPicImageFile(this->m_AmplitudeImageFileName, m_AmplitudeImage);
            this->OpenPicImageFile(this->m_IntensityImageFileName, m_IntensityImage);
            this->OpenPicImageFile(this->m_RGBImageFileName, m_RGBImage);
          }
        }
        else
        {
          throw std::logic_error("Please check image type, currently only .nrrd files are supported (.pic files are deprecated!)");
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
        this->m_RGBArray = new unsigned char[this->m_PixelNumber*3];
        for(int i=0; i<this->m_PixelNumber*3; i++) {this->m_RGBArray[i]=0.0;}

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

  void ToFCameraMITKPlayerController::OpenNrrdImageFile( const std::string outfileName, Image::Pointer &image)
  {
    if(!outfileName.empty())
    {
      if(image.IsNotNull())
      {
        image->ReleaseData();
        image = NULL;
      }
      ItkImageFileReader::Pointer nrrdReader = ItkImageFileReader::New();
      nrrdReader->SetFileName(outfileName);
      nrrdReader->Update();
      image = nrrdReader->GetOutput()->Clone();
    }
    else
    {
      MITK_ERROR << "Error opening ToF data file " << outfileName;
    }

  }

  void ToFCameraMITKPlayerController::OpenPicImageFile( const std::string outfileName, Image::Pointer &image)
  {
    if(!outfileName.empty())
    {
      if(image.IsNotNull())
      {
        image->ReleaseData();
        image = NULL;
      }
      PicFileReader::Pointer picReader = PicFileReader::New();
      picReader->SetFileName(outfileName);
      picReader->Update();
      image = picReader->GetOutput()->Clone();
    }
    else
    {
      MITK_ERROR << "Error opening ToF data file 2" << outfileName;
    }
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
        memcpy(m_RGBArray, m_RGBImage->GetSliceData(m_CurrentFrame)->GetData(),m_PixelNumber * sizeof(unsigned char)*3 );
      }
      else if(this->m_ToFImageType)
      {
        memcpy(m_RGBArray, m_RGBImage->GetVolumeData(m_CurrentFrame)->GetData(), m_PixelNumber * sizeof(unsigned char)*3);
      }
    }
    itksys::SystemTools::Delay(50);
  }

  void ToFCameraMITKPlayerController::AccessData(int frame, Image::Pointer image, float* &data)
  {
    if(!this->m_ToFImageType)
    {
      memcpy(data, image->GetSliceData(frame)->GetData(),this->m_NumberOfBytes );
    }
    else if(this->m_ToFImageType)
    {
      memcpy(data, image->GetVolumeData(frame)->GetData(), this->m_NumberOfBytes);
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
    memcpy(rgbArray, this->m_RGBArray, m_PixelNumber * sizeof(unsigned char)*3);
  }

  void ToFCameraMITKPlayerController::SetInputFileName(std::string inputFileName)
  {
    this->m_InputFileName = inputFileName;
  }

}
