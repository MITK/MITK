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
#include <mitkToFCameraMITKPlayerController.h>
#include <itksys/SystemTools.hxx>

#include <mitkIpPic.h>

namespace mitk
{

  ToFCameraMITKPlayerController::ToFCameraMITKPlayerController():m_PixelNumber(0),m_NumberOfBytes(0),
    m_CaptureWidth(0),m_CaptureHeight(0),m_ConnectionCheck(false),m_InputFileName(""),
    m_ToFImageType(ToFImageType3D),m_DistanceInfile(NULL),m_AmplitudeInfile(NULL),m_IntensityInfile(NULL),
    m_IntensityArray(NULL),m_DistanceArray(NULL),m_AmplitudeArray(NULL),
    m_DistanceImageFileName(""),m_AmplitudeImageFileName(""),m_IntensityImageFileName(""),
    m_PixelStartInFile(0),m_CurrentFrame(-1),m_NumOfFrames(0)
  {
  }

  ToFCameraMITKPlayerController::~ToFCameraMITKPlayerController()
  {
  }

  bool ToFCameraMITKPlayerController::OpenCameraConnection()
  {
    if(!this->m_ConnectionCheck)
    {
      try
      {
        mitkIpPicDescriptor* pic = NULL;
        mitkIpPicDescriptor* info = NULL;

        pic = mitkIpPicGetHeader(const_cast<char *>(this->m_DistanceImageFileName.c_str()), pic);
        if (pic==NULL)
        {
          MITK_ERROR << "Error opening ToF data file " << this->m_InputFileName;
          return false;
        }
        info = mitkIpPicGetTags(const_cast<char *>(this->m_DistanceImageFileName.c_str()), pic);
        this->m_PixelStartInFile = info->info->pixel_start_in_file;

        if (pic->dim == 2)
        {
          this->m_ToFImageType = ToFImageType2DPlusT;
        }
        else if (pic->dim == 3)
        {
          this->m_ToFImageType = ToFImageType3D;
        }
        else if (pic->dim == 4)
        {
          this->m_ToFImageType = ToFImageType2DPlusT;
        }
        else
        {
          MITK_ERROR << "Error opening ToF data file: Invalid dimension.";
          return false;
        }

        this->m_CaptureWidth = pic->n[0];
        this->m_CaptureHeight = pic->n[1];
        this->m_PixelNumber = this->m_CaptureWidth*this->m_CaptureHeight;
        this->m_NumberOfBytes = this->m_PixelNumber * sizeof(float);
        if (this->m_ToFImageType == ToFImageType2DPlusT)
        {
          this->m_NumOfFrames = pic->n[3];
        }
        else
        {
          this->m_NumOfFrames = pic->n[2];
        }

        // allocate buffer
        this->m_DistanceArray = new float[this->m_PixelNumber];
        for(int i=0; i<this->m_PixelNumber; i++) {this->m_DistanceArray[i]=0.0;}
        this->m_AmplitudeArray = new float[this->m_PixelNumber]; 
        for(int i=0; i<this->m_PixelNumber; i++) {this->m_AmplitudeArray[i]=0.0;}
        this->m_IntensityArray = new float[this->m_PixelNumber];
        for(int i=0; i<this->m_PixelNumber; i++) {this->m_IntensityArray[i]=0.0;}

        OpenPicFileToData(&(this->m_DistanceInfile), this->m_DistanceImageFileName.c_str());
        OpenPicFileToData(&(this->m_AmplitudeInfile), this->m_AmplitudeImageFileName.c_str());
        OpenPicFileToData(&(this->m_IntensityInfile), this->m_IntensityImageFileName.c_str());

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

  void ToFCameraMITKPlayerController::OpenPicFileToData(FILE** outfile, const char *outfileName)
  {
    (*outfile) = fopen( outfileName, "rb" );
    if( (*outfile) == NULL )
    {
      MITK_ERROR << "Error opening ToF data file: file is empty" << outfileName;
      throw std::logic_error("Error opening ToF data file: file is empty");
      return;
    }
    fseek ( (*outfile), this->m_PixelStartInFile, SEEK_SET );
  }

  bool ToFCameraMITKPlayerController::CloseCameraConnection()
  {
    bool returnValue = false;
    if (this->m_ConnectionCheck)
    {
      if(!fclose(this->m_DistanceInfile) && !fclose(this->m_AmplitudeInfile) &&
         !fclose(this->m_IntensityInfile))
      {
        this->m_ConnectionCheck = false;
        returnValue = true;
      }
      else
      {
        this->m_ConnectionCheck = true;
        returnValue = false;
      }
    }
    return returnValue;
  } 

  void ToFCameraMITKPlayerController::UpdateCamera()
  {
    size_t distanceBytesReturn;
    size_t amplitudeBytesReturn;
    size_t intensityBytesReturn;

    this->m_CurrentFrame++;
    if (this->m_CurrentFrame >= this->m_NumOfFrames)
    {
      this->m_CurrentFrame = 0;
      fseek ( this->m_DistanceInfile, this->m_PixelStartInFile, SEEK_SET );
      fseek ( this->m_AmplitudeInfile, this->m_PixelStartInFile, SEEK_SET );
      fseek ( this->m_IntensityInfile, this->m_PixelStartInFile, SEEK_SET );
    }
    distanceBytesReturn = fread( this->m_DistanceArray, this->m_NumberOfBytes, 1, this->m_DistanceInfile );
    amplitudeBytesReturn = fread( this->m_AmplitudeArray, this->m_NumberOfBytes, 1, this->m_AmplitudeInfile );
    intensityBytesReturn = fread( this->m_IntensityArray, this->m_NumberOfBytes, 1, this->m_IntensityInfile );
    itksys::SystemTools::Delay(50);
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

  void ToFCameraMITKPlayerController::SetInputFileName(std::string inputFileName)
  {
    this->m_InputFileName = inputFileName;
  }

}
