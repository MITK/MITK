/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDO3Controller_h
#define __mitkToFCameraPMDO3Controller_h

#include <MitkPMDExports.h>
#include "mitkCommon.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Time-of-Flight (ToF) camera PMD O3
  * Connection is established over Ethernet connection. IP address must be specified.
  *
  * @ingroup ToFHardware
  */
  class MITKPMD_EXPORT ToFCameraPMDO3Controller : public mitk::ToFCameraPMDController
  {
  public:

    mitkClassMacro( ToFCameraPMDO3Controller , mitk::ToFCameraPMDController );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /*!
    \brief opens a connection to the PMD O3 ToF camera
    */
    virtual bool OpenCameraConnection();

  protected:

    ToFCameraPMDO3Controller();

    ~ToFCameraPMDO3Controller();

    /*
    \brief method that transforms camera input such that it is ordered correctly.
    */
    void TransformCameraOutput(float* in, float* out, bool isDist);

  private:

    char *m_IPAddress; ///< holds the ip adress the O3 camera is connected to
  };
} //END mitk namespace
#endif
