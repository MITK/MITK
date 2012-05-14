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
#ifndef __mitkToFImageGrabberCreator_h
#define __mitkToFImageGrabberCreator_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFImageGrabber.h"
#include "mitkToFCameraDevice.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Class providing ready-to-use instances of ToFImageGrabbers
  * This singleton can be used to create ToFImageGrabber objects of
  * the supported ToF cameras. The ImageGrabbers are already initialized
  * with the correct device object.
  * When new hardware is added to MITK-ToF they should also be included in
  * this creator class.
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFImageGrabberCreator : public itk::Object 
  {
  public: 

    mitkClassMacro( ToFImageGrabberCreator , itk::Object );

    itkNewMacro( Self );

    /*!
    \brief Get the RenderingManager singleton instance. 
     */
    static ToFImageGrabberCreator* GetInstance();

    /*!
    \brief Create a ImageGrabber object with a PMD CamCube Camera 
    */
    ToFImageGrabber::Pointer GetPMDCamCubeImageGrabber();
    
    /*!
    \brief Create a ImageGrabber object with a PMD raw data CamCube Camera 
    */
    ToFImageGrabber::Pointer GetPMDRawDataCamCubeImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a PMD O3 Camera 
    */
    ToFImageGrabber::Pointer GetPMDO3ImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a PMD CamBoard Camera 
    */
    ToFImageGrabber::Pointer GetPMDCamBoardImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a PMD raw data CamBoard Camera 
    */
    ToFImageGrabber::Pointer GetPMDRawDataCamBoardImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a PMD Player Camera 
    */
    ToFImageGrabber::Pointer GetPMDPlayerImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a PMD MITK Player Camera 
    */
    ToFImageGrabber::Pointer GetPMDMITKPlayerImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a MITK Player Camera 
    */
    ToFImageGrabber::Pointer GetMITKPlayerImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a MESA SR4000 Camera 
    */
    ToFImageGrabber::Pointer GetMESASR4000ImageGrabber();

    /*!
    \brief Create a ImageGrabber object with a Kinect Camera 
    */
    ToFImageGrabber::Pointer GetKinectImageGrabber();

  protected:

    static ToFImageGrabberCreator::Pointer s_Instance; ///< Instance to the singleton ToFImageGrabberCreator. Can be accessed by GetInstance()

    ToFCameraDevice::Pointer m_ToFCameraDevice; ///< ToFCameraDevice currently used in the provided ToFImageGrabber

    ToFImageGrabber::Pointer m_ToFImageGrabber; ///< ToFImageGrabber that will be returned configured with the specific ToFCameraDevice

    ToFImageGrabberCreator();

    ~ToFImageGrabberCreator();
  private:

  };
} //END mitk namespace
#endif
