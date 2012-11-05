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
#ifndef __mitkToFCameraPMDPlayerDevice_h
#define __mitkToFCameraPMDPlayerDevice_h

#include "mitkPMDModuleExports.h"
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
  class MITK_PMDMODULE_EXPORT ToFCameraPMDPlayerDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDPlayerDevice , ToFCameraPMDDevice );

    itkNewMacro( Self );


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
