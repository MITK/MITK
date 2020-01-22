/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDPlayerController_h
#define __mitkToFCameraPMDPlayerController_h

#include <MitkPMDExports.h>
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
  class MITKPMD_EXPORT ToFCameraPMDPlayerController : public ToFCameraPMDController
  {
  public:

    mitkClassMacro( ToFCameraPMDPlayerController , ToFCameraPMDController );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

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

    /*
    \brief Transform method used for phyical camera devices. For the player no transform is needed.
    */
    virtual void TransformCameraOutput(float* in, float* out, bool isDist){};

    ToFCameraPMDPlayerController();

    ~ToFCameraPMDPlayerController();

    std::string m_PMDFileName;  ///< File name of the pmd data stream

  private:
    unsigned int m_NumOfFrames; ///< Number of frames

  };
} //END mitk namespace
#endif
