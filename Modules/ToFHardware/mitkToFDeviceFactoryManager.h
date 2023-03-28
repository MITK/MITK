/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFDeviceFactoryManager_h
#define mitkToFDeviceFactoryManager_h

#include <MitkToFHardwareExports.h>
#include "mitkToFCameraDevice.h"

//Microservices
#include <usServiceReference.h>

namespace mitk
{
  struct IToFDeviceFactory;

  /**
  * @brief ToFDeviceListener
  *
  * @ingroup ToFHardware
  */
  class MITKTOFHARDWARE_EXPORT ToFDeviceFactoryManager: public itk::Object
  {
  public:

    mitkClassMacroItkParent( ToFDeviceFactoryManager, itk::Object );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    std::vector<std::string> GetRegisteredDeviceFactories();

    std::vector<std::string> GetConnectedDevices();

    ToFCameraDevice* GetInstanceOfDevice(int index);



  protected:

    ToFDeviceFactoryManager();

    ~ToFDeviceFactoryManager() override;

    std::vector<std::string> m_RegisteredFactoryNames;
    std::vector<us::ServiceReference<IToFDeviceFactory> > m_RegisteredFactoryRefs;


  private:

  };
} //END mitk namespace
#endif
