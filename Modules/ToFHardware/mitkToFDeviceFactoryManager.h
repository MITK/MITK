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
#ifndef __mitkToFDeviceListener_h
#define __mitkToFDeviceListener_h

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

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    std::vector<std::string> GetRegisteredDeviceFactories();

    std::vector<std::string> GetConnectedDevices();

    ToFCameraDevice* GetInstanceOfDevice(int index);



  protected:

    ToFDeviceFactoryManager();

    ~ToFDeviceFactoryManager();

    std::vector<std::string> m_RegisteredFactoryNames;
    std::vector<us::ServiceReference<IToFDeviceFactory> > m_RegisteredFactoryRefs;


  private:

  };
} //END mitk namespace
#endif
