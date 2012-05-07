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
    itkGetConstMacro(UseIR,bool);

    itkSetMacro(UseIR,bool);

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
    \param distances pointer to memory location where distances should be stored
    */
    void GetDistances(float* distances);
    void GetAmplitudes(float* amplitudes);
    void GetIntensities(float* intensities);
    /*!
    \brief acquire new rgb data from the Kinect camera
    \parama rgb pointer to memory location where rgb information should be stored
    */
    void GetRgb(unsigned char* rgb);
    /*!
    \brief convenience method for faster access to distance and rgb data
    \param distances pointer to memory location where distances should be stored
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetAllData(float* distances, float* amplitudes, unsigned char* rgb);

  protected:

    KinectController();

    ~KinectController();

    bool ErrorText(unsigned int error);

    xn::Context m_Context; ///< OpenNI context
    xn::DepthGenerator m_DepthGenerator; ///< Depth generator to access depth image of kinect
    xn::ImageGenerator m_ImageGenerator; ///< Image generator to access RGB image of kinect
    xn::IRGenerator m_IRGenerator; ///< IR generator to access IR image of kinect
    //xn::IRGenerator m_IRGenerator;

    bool m_ConnectionCheck; ///< check if camera is connected or not

    bool m_UseIR; ///< flag indicating whether IR image is used or not

    unsigned int m_CaptureWidth; ///< image width
    unsigned int m_CaptureHeight; ///< image height

  private:

  };
} //END mitk namespace
#endif
