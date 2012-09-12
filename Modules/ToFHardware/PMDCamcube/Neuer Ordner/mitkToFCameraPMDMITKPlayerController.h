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
#ifndef __mitkToFCameraPMDMITKPlayerController_h
#define __mitkToFCameraPMDMITKPlayerController_h

#include "mitkPMDCamCubeModuleExports.h"
#include "mitkCommon.h"
#include "mitkImage.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface for reading raw data from an MITK-ToF raw data file
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_PMDCAMCUBEMODULE_EXPORT ToFCameraPMDMITKPlayerController : public mitk::ToFCameraPMDController
  {
  public:

    mitkClassMacro( ToFCameraPMDMITKPlayerController , mitk::ToFCameraPMDController );

    itkNewMacro( Self );

    /*!
    \brief opens a connection to the ToF camera
    */
    bool OpenCameraConnection();
    /*!
    \brief closes the connection to the camera
    */
    bool CloseCameraConnection();
    /*!
    \brief calls update on the camera -> a new ToF-image is aquired
    */
    bool UpdateCamera();
    /*!
    \brief returns the raw data from the ToF camera measuring
    */
    bool GetSourceData(char* sourceDataArray);
    /*!
    \brief get the modulation frequency as specified in the PMDDataDescription
    */
    int GetModulationFrequency();
    int SetModulationFrequency(unsigned int modulationFrequency);
    /*!
    \brief get the integration frequency as specified in the PMDDataDescription
    */
    int GetIntegrationTime();
    int SetIntegrationTime(unsigned int integrationTime);

  protected:

    ToFCameraPMDMITKPlayerController();

    ~ToFCameraPMDMITKPlayerController();

    /*!
    \brief read input file and initialize members
    */
    bool ReadFile();
    /*!
    \brief check if current input file is readable MITK file and read it
    */
    bool OpenMITKFile();

  private:

    mitk::Image::Pointer m_MitkImage; ///< member storing image read
    int m_CurrentFrame;
    int m_NumOfFrames;

  };
} //END mitk namespace
#endif
