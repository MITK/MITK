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
#ifndef __mitkToFImageWriter_h
#define __mitkToFImageWriter_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFImageGrabber.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Writer class for ToF images
  *
  * This writer class allows streaming of ToF data into a file. The .pic file format is used for writing the data.
  * Image information is included in the header of the pic file.
  * Writer can simultaneously save "distance", "intensity" and "amplitude" image.
  * Images can be written as 3D volume (ToFImageType::ToFImageType3D) or temporal image stack (ToFImageType::ToFImageType2DPlusT)
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFImageWriter : public itk::Object
  {
  public:

    ToFImageWriter();

    ~ToFImageWriter();

    mitkClassMacro( ToFImageWriter , itk::Object );

    itkNewMacro( Self );

    itkGetMacro( DistanceImageFileName, std::string );
    itkGetMacro( AmplitudeImageFileName, std::string );
    itkGetMacro( IntensityImageFileName, std::string );
    itkGetMacro( RGBImageFileName, std::string );
    itkGetMacro( Extension, std::string );
    itkGetMacro( ToFCaptureWidth, int );
    itkGetMacro( ToFCaptureHeight, int );
    itkGetMacro( RGBCaptureWidth, int );
    itkGetMacro( RGBCaptureHeight, int );
    itkGetMacro( DistanceImageSelected, bool );
    itkGetMacro( AmplitudeImageSelected, bool );
    itkGetMacro( IntensityImageSelected, bool );
    itkGetMacro( RGBImageSelected, bool );

    itkSetMacro( DistanceImageFileName, std::string );
    itkSetMacro( AmplitudeImageFileName, std::string );
    itkSetMacro( IntensityImageFileName, std::string );
    itkSetMacro( RGBImageFileName, std::string );
    itkSetMacro( Extension, std::string );
    itkSetMacro( ToFCaptureWidth, int );
    itkSetMacro( ToFCaptureHeight, int );
    itkSetMacro( RGBCaptureWidth, int );
    itkSetMacro( RGBCaptureHeight, int );
    itkSetMacro( DistanceImageSelected, bool );
    itkSetMacro( AmplitudeImageSelected, bool );
    itkSetMacro( IntensityImageSelected, bool );
    itkSetMacro( RGBImageSelected, bool );

    enum ToFImageType{ ToFImageType3D, ToFImageType2DPlusT };
    /*!
    \brief Get the type of image to be written
    \return ToF image type: ToFImageType3D (0) or ToFImageType2DPlusT (1)
    */
    ToFImageWriter::ToFImageType GetToFImageType();
    /*!
    \brief Set the type of image to be written
    \param toFImageType type of the ToF image: ToFImageType3D (0) or ToFImageType2DPlusT (1)
    */
    void SetToFImageType(ToFImageWriter::ToFImageType toFImageType);
    /*!
    \brief Open file(s) for writing
    */
    virtual void Open(){};
    /*!
    \brief Close file(s) add .pic header and write
    */
    virtual void Close(){};
    /*!
    \brief Add new data to file.
    */
    virtual void Add(float* distanceFloatData, float* amplitudeFloatData, float* intensityFloatData, unsigned char* rgbData=0){};

  protected:

    /*!
    \brief Checks file name if file extension exists. If not an error message is returned
    */
    void CheckForFileExtension(std::string& fileName);

    // member variables
    std::string m_DistanceImageFileName; ///< file name for saving the distance image
    std::string m_AmplitudeImageFileName; ///< file name for saving the amplitude image
    std::string m_IntensityImageFileName; ///< file name for saving the intensity image
    std::string m_RGBImageFileName; ///< file name for saving the RGB image
    std::string m_Extension; ///< file extension used for saving images

    int m_ToFCaptureWidth; ///< width (x-dimension) of the images to record.
    int m_ToFCaptureHeight; ///< height (y-dimension) of the images to record.
    int m_ToFPixelNumber; ///< number of pixels (widht*height) of the images to record
    int m_ToFImageSizeInBytes; ///< size of the image to save in bytes
    int m_RGBCaptureWidth; ///< width (x-dimension) of the images to record.
    int m_RGBCaptureHeight; ///< height (y-dimension) of the images to record.
    int m_RGBPixelNumber; ///< number of pixels (widht*height) of the images to record
    int m_RGBImageSizeInBytes; ///< size of the image to save in bytes
    int m_NumOfFrames; ///< number of frames written to the image. Used for pic header.
    ToFImageWriter::ToFImageType m_ToFImageType; ///< type of image to be recorded: ToFImageType3D (0) or ToFImageType2DPlusT (1)

    bool m_DistanceImageSelected; ///< flag indicating if distance image should be recorded
    bool m_AmplitudeImageSelected; ///< flag indicating if amplitude image should be recorded
    bool m_IntensityImageSelected; ///< flag indicating if intensity image should be recorded
    bool m_RGBImageSelected; ///< flag indicating if RGB image should be recorded

  private:

  };
} //END mitk namespace
#endif // __mitkToFImageWriter_h
