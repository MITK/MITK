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
#ifndef __mitkToFCameraPMDPlayerController_h
#define __mitkToFCameraPMDPlayerController_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to read ToF data from a PMD file
  * 
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDPlayerController : public mitk::ToFCameraPMDController
  {
  public: 

    mitkClassMacro( ToFCameraPMDPlayerController , mitk::ToFCameraPMDController );

    itkNewMacro( Self );

    itkSetMacro(PMDFileName, std::string);

    itkGetMacro(PMDFileName, std::string);

    /*!
    \brief opens a connection to the ToF camera
    */
    bool OpenCameraConnection();
    /*!
    \brief convenience method setting the modulation frequency in the PMDDataDescription
    \param modulationFrequency modulation frequency
    \return currently set modulation frequency in PMDDataDescription
    */
    int SetModulationFrequency(unsigned int modulationFrequency);
    /*!
    \brief convenience method setting the integration time in the PMDDataDescription
    \param integrationTime integration time
    \return currently set integration time in PMDDataDescription
    */
    int SetIntegrationTime(unsigned int integrationTime);

  protected:

    ToFCameraPMDPlayerController();

    ~ToFCameraPMDPlayerController();

    std::string m_PMDFileName;  ///< File name of the pmd data stream

  private:
    unsigned int m_NumOfFrames; ///< Number of frames

  };
} //END mitk namespace
#endif
