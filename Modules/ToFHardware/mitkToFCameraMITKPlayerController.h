/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFCameraMITKPlayerController_h
#define __mitkToFCameraMITKPlayerController_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImage.h"

#include "itkObject.h"
#include "itkObjectFactory.h"


namespace mitk
{
  /**
  * @brief Controller for playing ToF images saved in MITK (.pic) format
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraMITKPlayerController : public itk::Object
  {
  public: 

    mitkClassMacro( ToFCameraMITKPlayerController , itk::Object );

    itkNewMacro( Self );

    /*!
    \brief opens a connection to the ToF camera
    \return Returns 'true' if connection could be established and 'false' otherwise
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief closes the connection to the camera
    \return Returns 'true' if connection could be closed (i.e. only if a connection was established before) and 'false' otherwise
    */
    virtual bool CloseCameraConnection();
    /*!
    \brief gets the current amplitude frame from the input
    These values can be used to determine the quality of the distance values. The higher the amplitude value, the higher the accuracy of the according distance value
    */
    virtual void GetAmplitudes(float* amplitudeArray);
    /*!
    \brief gets the current intensity frame from the input as a greyscale image
    */
    virtual void GetIntensities(float* intensityArray);
    /*!
    \brief gets the current distance frame from the inpug measuring the distance between the camera and the different object points in millimeters
    */
    virtual void GetDistances(float* distanceArray);
    /*!
    \brief updates the current image frames from input
    */
    virtual void UpdateCamera();

    virtual void SetInputFileName(std::string inputFileName);

    itkGetMacro(CaptureWidth, int);
    itkGetMacro(CaptureHeight, int);
    itkGetMacro( DistanceImageFileName, std::string );
    itkGetMacro( AmplitudeImageFileName, std::string );
    itkGetMacro( IntensityImageFileName, std::string );

    itkSetMacro( DistanceImageFileName, std::string );
    itkSetMacro( AmplitudeImageFileName, std::string );
    itkSetMacro( IntensityImageFileName, std::string );
    enum ToFImageType{ ToFImageType3D, ToFImageType2DPlusT };

  protected:

    ToFCameraMITKPlayerController();

    ~ToFCameraMITKPlayerController();

    int m_PixelNumber; ///< holds the number of pixels contained in the image
    int m_NumberOfBytes; ///< holds the number of bytes contained in the image
    int m_CaptureWidth; ///< holds the width of the image
    int m_CaptureHeight; ///< holds the height of the image
    bool m_ConnectionCheck; ///< flag showing whether the camera is connected (true) or not (false)

    std::string m_InputFileName;
    std::string m_Extension;

    ToFImageType m_ToFImageType; ///< type of the ToF image to be played: 3D Volume (ToFImageType3D), temporal 2D image stack (ToFImageType2DPlusT)

    Image::Pointer m_DistanceImage;
    Image::Pointer m_AmplitudeImage;
    Image::Pointer m_IntensityImage;
    std::vector<bool> m_ImageStatus;

    FILE* m_DistanceInfile; ///< file holding the distance data
    FILE* m_AmplitudeInfile; ///< file holding the amplitude data
    FILE* m_IntensityInfile; ///< file holding the intensity data

    float* m_IntensityArray; ///< member holding the current intensity frame
    float* m_DistanceArray; ///< member holding the current distance frame
    float* m_AmplitudeArray; ///< member holding the current amplitude frame

    std::string m_DistanceImageFileName; ///< file name of the distance image to be played
    std::string m_AmplitudeImageFileName; ///< file name of the amplitude image to be played
    std::string m_IntensityImageFileName; ///< file name of the intensity image to be played

    long m_PixelStartInFile;
    int m_CurrentFrame;
    int m_NumOfFrames;

  private:

    //void OpenPicFileToData(FILE** outfile, const char *outfileName);
    void AccessData(int frame, Image::Pointer image, float* &data);
    bool CheckCurrentFileType();
    void OpenNrrdImageFile( const std::string outfileName, Image::Pointer &image);
    void OpenPicImageFile( const std::string outfileName, Image::Pointer &image);
    void CleanUp();
  };
} //END mitk namespace
#endif
