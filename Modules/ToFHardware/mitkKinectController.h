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
#ifndef __mitkKinectController_h
#define __mitkKinectController_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

#include <XnCppWrapper.h>

namespace mitk
{
  /**
  * @brief Interface to the Kinect camera
  * 
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT KinectController : public itk::Object
  {
  public: 

    mitkClassMacro( KinectController , itk::Object );

    itkNewMacro( Self );

    itkGetConstMacro(CaptureHeight,unsigned int);
    itkGetConstMacro(CaptureWidth,unsigned int);

    /*!
    \brief opens a connection to the Kinect camera.
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool CloseCameraConnection();
    /*!
    \brief updates the camera. The update function of the hardware interface is called only when new data is available
    */
    virtual bool UpdateCamera();
    /*!
    \brief acquire new distance data from the Kinect camera
    \param distances pointer holding the acquired distance array
    */
    void GetDistances(float* distances);

    void GetIntensities(float* intensities);

    void GetAmplitudes(float* amplitudes);

  protected:

    KinectController();

    ~KinectController();

    bool ErrorText(unsigned int error);

    xn::Context m_Context; ///< OpenNI context
    xn::DepthGenerator m_DepthGenerator; ///< Depth generator to access depth image of kinect
    xn::ImageGenerator m_ImageGenerator; ///< Image generator to access RGB image of kinect
    //xn::IRGenerator m_IRGenerator;

    bool m_ConnectionCheck; ///< check if camera is connected or not

    unsigned int m_CaptureWidth; ///< image width
    unsigned int m_CaptureHeight; ///< image height

  private:

  };
} //END mitk namespace
#endif
