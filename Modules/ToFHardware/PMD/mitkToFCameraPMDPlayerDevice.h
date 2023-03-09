/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraPMDPlayerDevice_h
#define mitkToFCameraPMDPlayerDevice_h

#include <MitkPMDExports.h>
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDDevice.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Device class representing a player for PMD data
  *
  *
  * @ingroup ToFHardware
  */
  class MITKPMD_EXPORT ToFCameraPMDPlayerDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDPlayerDevice , ToFCameraPMDDevice );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty(const char* propertyKey, BaseProperty* propertyValue);

  protected:

    ToFCameraPMDPlayerDevice();

    ~ToFCameraPMDPlayerDevice();
    std::string m_InputFileName; ///< input file name for data stored in an .pmd file

  private:

  };
} //END mitk namespace
#endif
