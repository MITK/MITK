/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraMITKPlayerController_h
#define mitkToFCameraMITKPlayerController_h

#include <MitkToFHardwareExports.h>
#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImage.h"

#include "itkObject.h"
#include "itkObjectFactory.h"


namespace mitk
{
  /**
  * @brief Controller for playing ToF images saved in NRRD format
  *
  * @ingroup ToFHardware
  */
  class MITKTOFHARDWARE_EXPORT ToFCameraMITKPlayerController : public itk::Object
  {
  public:

    mitkClassMacroItkParent( ToFCameraMITKPlayerController , itk::Object );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

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
    \brief gets the current RGB frame from the input if available
    */
    virtual void GetRgb(unsigned char* rgbArray);
    /*!
    \brief updates the current image frames from input
    */
    virtual void UpdateCamera();

    virtual void SetInputFileName(std::string inputFileName);

    itkGetMacro(PixelNumber, int);
    itkGetMacro(RGBPixelNumber, int);
    itkGetMacro(CaptureWidth, int);
    itkGetMacro(CaptureHeight, int);
    itkGetMacro(RGBCaptureWidth, int);
    itkGetMacro(RGBCaptureHeight, int);
    itkGetMacro( DistanceImageFileName, std::string );
    itkGetMacro( AmplitudeImageFileName, std::string );
    itkGetMacro( IntensityImageFileName, std::string );
    itkGetMacro( RGBImageFileName, std::string );

    itkSetMacro( DistanceImageFileName, std::string );
    itkSetMacro( AmplitudeImageFileName, std::string );
    itkSetMacro( IntensityImageFileName, std::string );
    itkSetMacro( RGBImageFileName, std::string );
    enum ToFImageType{ ToFImageType3D, ToFImageType2DPlusT };

  protected:

    ToFCameraMITKPlayerController();

    ~ToFCameraMITKPlayerController() override;

    int m_PixelNumber; ///< holds the number of pixels contained in the image
    int m_RGBPixelNumber; ///< same for RGB image
    int m_NumberOfBytes; ///< holds the number of bytes contained in the image
    int m_NumberOfRGBBytes; ///< same for RGB image
    int m_CaptureWidth; ///< holds the width of the image
    int m_CaptureHeight; ///< holds the height of the image
    int m_RGBCaptureWidth; ///< same for RGB image which can be different then depth etc.
    int m_RGBCaptureHeight; ///< same for RGB image which can be different then depth etc.
    bool m_ConnectionCheck; ///< flag showing whether the camera is connected (true) or not (false)

    std::string m_InputFileName;
    ToFImageType m_ToFImageType; ///< type of the ToF image to be played: 3D Volume (ToFImageType3D), temporal 2D image stack (ToFImageType2DPlusT)

    Image::Pointer m_DistanceImage;
    Image::Pointer m_AmplitudeImage;
    Image::Pointer m_IntensityImage;
    Image::Pointer m_RGBImage;
    std::vector<bool> m_ImageStatus;

    FILE* m_DistanceInfile; ///< file holding the distance data
    FILE* m_AmplitudeInfile; ///< file holding the amplitude data
    FILE* m_IntensityInfile; ///< file holding the intensity data
    FILE* m_RGBInfile; ///< file holding the rgb data

    float* m_IntensityArray; ///< member holding the current intensity frame
    float* m_DistanceArray; ///< member holding the current distance frame
    float* m_AmplitudeArray; ///< member holding the current amplitude frame
    unsigned char* m_RGBArray; ///< member holding the current rgb frame

    std::string m_DistanceImageFileName; ///< file name of the distance image to be played
    std::string m_AmplitudeImageFileName; ///< file name of the amplitude image to be played
    std::string m_IntensityImageFileName; ///< file name of the intensity image to be played
    std::string m_RGBImageFileName; ///< file name of the rgb image to be played

    long m_PixelStartInFile;
    int m_CurrentFrame;
    int m_NumOfFrames;

  private:

    void AccessData(int frame, Image::Pointer image, float* &data);
    void CleanUp();
  };
} //END mitk namespace
#endif
