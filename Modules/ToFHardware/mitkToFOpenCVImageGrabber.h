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
#ifndef __mitkToFOpenCVImageGrabber_h
#define __mitkToFOpenCVImageGrabber_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkOpenCVImageSource.h"
#include "mitkToFImageGrabber.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief TofImageGrabber class providing OpenCV images
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFOpenCVImageGrabber : public mitk::OpenCVImageSource
  {
  public:

    ToFOpenCVImageGrabber();

    ~ToFOpenCVImageGrabber();

    mitkClassMacro( ToFOpenCVImageGrabber , OpenCVImageSource );

    itkNewMacro( Self );

    /*!
    \brief Get current ToF image. Specify image you want to grab with SetImageType()
    */
    cv::Mat GetImage();

    /*!
    \brief set type of image you want to grab.
    0: Distance image (Default)
    1: Amplitude image
    2: Intensity image
    */
    void SetImageType(unsigned int imageType);
    /*!
    \brief set the depth of the image. Some functions of OpenCV do not support IPL_DEPTH_32F.
    Warning: changing from default results in a mapping of the pixel value through a lookup table
    IPL_DEPTH_1U     1
    IPL_DEPTH_8U     8
    IPL_DEPTH_16U   16
    IPL_DEPTH_32F   32 (Default)
    */
    void SetImageDepth(unsigned int imageDepth);
    /*!
    \brief set the ImageGrabber used for fetching image data from the camera
    */
    void SetToFImageGrabber(mitk::ToFImageGrabber::Pointer imageGrabber);
    /*!
    \brief get the ImageGrabber used for fetching image data from the camera
    */
    mitk::ToFImageGrabber::Pointer GetToFImageGrabber();

    void StartCapturing();
    void StopCapturing();


  protected:
    /*!
    \brief map scalars through lookup table
    \param image current MITK image
    */
    void MapScalars(mitk::Image::Pointer mitkImage, IplImage* openCVImage);

    mitk::ToFImageGrabber::Pointer m_ImageGrabber; ///< ImageGrabber used for fetching ToF image data from the camera
    unsigned int m_ImageType; ///< type of image currently supplied by this image source
    /*!
    \brief image depth currently used by this image source.
    Warning: Changing from default (IPL_DEPTH_32F) results in a mapping of the pixel value through a lookup table
    */
    unsigned int m_ImageDepth;
    IplImage* m_CurrentOpenCVIntensityImage; ///< OpenCV image holding the current intensity data
    IplImage* m_CurrentOpenCVAmplitudeImage; ///< OpenCV image holding the current amplitude data
    IplImage* m_CurrentOpenCVDistanceImage; ///< OpenCV image holding the current distance data

  private:

  };
} //END mitk namespace
#endif
