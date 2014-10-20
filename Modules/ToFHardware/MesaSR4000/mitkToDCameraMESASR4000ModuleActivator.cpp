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
#ifndef __mitkMESASR4000Activator_h
#define __mitkMESASR4000Activator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "usModuleContext.h"
#include <usModuleActivator.h>

//ToF
#include "mitkIToFDeviceFactory.h"
#include "mitkToFConfig.h"
#include "mitkToFCameraMESASR4000DeviceFactory.h"

namespace mitk
{
/**
 * @brief The MESASR4000ModuleActivator class This is the module activator for the "MESASR4000Module"
 * module. It registers services like the IToFDeviceFactory.
 */
class MESASR4000ModuleActivator : public us::ModuleActivator {
public:

  /**
  * @brief Load This method is automatically called, when the MESASR4000Module
  * is activated. It will automatically register a ToFCameraMESASR4000DeviceFactory.
  */
  void Load(us::ModuleContext* context)
  {
    //Implementing MESASR4000DeviceFactory
    ToFCameraMESASR4000DeviceFactory* toFCameraMESASR4000DeviceFactory = new ToFCameraMESASR4000DeviceFactory();
    us::ServiceProperties mitkMESASR4000FactoryProps;
    mitkMESASR4000FactoryProps["ToFFactoryName"] = toFCameraMESASR4000DeviceFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(toFCameraMESASR4000DeviceFactory, mitkMESASR4000FactoryProps);

    toFCameraMESASR4000DeviceFactory->ConnectToFDevice();

    m_Factories.push_back(toFCameraMESASR4000DeviceFactory);
  }

  /**
    * @brief Unload called when the module is unloaded.
    */
  void Unload(us::ModuleContext* )
  {
  }

  /**
    * @brief Default destructor which deletes all registered factories.
    */
  ~MESASR4000ModuleActivator()
  {
    if(m_Factories.size() > 0)
    {
      for(std::list< IToFDeviceFactory* >::iterator it = m_Factories.begin(); it != m_Factories.end(); ++it)
      {
        delete (*it);
      }
    }
  }

private:
  /**
    * @brief m_Factories List of all registered factories.
    */
  std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitk::MESASR4000ModuleActivator)
#endif
