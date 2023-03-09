/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFImageWriter_h
#define mitkToFImageWriter_h

#include <MitkToFHardwareExports.h>
#include "mitkCommon.h"
#include "mitkToFImageGrabber.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Writer class for ToF images
  *
  * This writer class allows streaming of ToF data into a file.
  * Image information is included in the header of the nrrd file.
  * Writer can simultaneously save "distance", "intensity" and "amplitude" image.
  * Images can be written as 3D volume (ToFImageType::ToFImageType3D) or temporal image stack (ToFImageType::ToFImageType2DPlusT)
  *
  * @ingroup ToFHardware
  */
  class MITKTOFHARDWARE_EXPORT ToFImageWriter : public itk::Object
  {
  public:

    ToFImageWriter();

    ~ToFImageWriter() override;

    mitkClassMacroItkParent( ToFImageWriter , itk::Object );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

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
    \brief Close file(s) add header and write
    */
    virtual void Close(){};
    /*!
    \brief Add new data to file.
    */
    virtual void Add(float* /*distanceFloatData*/, float* /*amplitudeFloatData*/, float* /*intensityFloatData*/, unsigned char* /*rgbData*/=nullptr){};

  protected:

    /*!
    \brief Checks file name if file extension exists. If not an error message is returned
    */
    void CheckForFileExtension(std::string& fileName);

    // member variables
    std::string m_Extension; ///< file extension used for saving images
    std::string m_DistanceImageFileName; ///< file name for saving the distance image
    std::string m_AmplitudeImageFileName; ///< file name for saving the amplitude image
    std::string m_IntensityImageFileName; ///< file name for saving the intensity image
    std::string m_RGBImageFileName; ///< file name for saving the RGB image

    int m_NumOfFrames; ///< number of frames written to the image. Used for pic header.
    bool m_DistanceImageSelected; ///< flag indicating if distance image should be recorded
    bool m_AmplitudeImageSelected; ///< flag indicating if amplitude image should be recorded
    bool m_IntensityImageSelected; ///< flag indicating if intensity image should be recorded
    bool m_RGBImageSelected; ///< flag indicating if RGB image should be recorded
    int m_ToFCaptureWidth; ///< width (x-dimension) of the images to record.
    int m_ToFCaptureHeight; ///< height (y-dimension) of the images to record.
    int m_RGBCaptureWidth; ///< width (x-dimension) of the images to record.
    int m_RGBCaptureHeight; ///< height (y-dimension) of the images to record.
    int m_ToFPixelNumber; ///< number of pixels (widht*height) of the images to record
    int m_ToFImageSizeInBytes; ///< size of the image to save in bytes
    int m_RGBPixelNumber; ///< number of pixels (widht*height) of the images to record
    int m_RGBImageSizeInBytes; ///< size of the image to save in bytes
    ToFImageWriter::ToFImageType m_ToFImageType; ///< type of image to be recorded: ToFImageType3D (0) or ToFImageType2DPlusT (1)
  };
} //END mitk namespace
#endif
