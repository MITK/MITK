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
#ifndef __mitkKinectActivator_h
#define __mitkKinectActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

//ToF
#include "mitkIToFDeviceFactory.h"
#include "mitkToFConfig.h"
#include "mitkKinectDeviceFactory.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
/**
 * @brief The KinectActivator class This is the module activator for the "mitkKinectModule"
 * module. It registers services like the IToFDeviceFactory.
 */
class KinectActivator : public us::ModuleActivator{
public:
  /**
  * @brief Load This method is automatically called, when the kinect module
  * is activated. It will automatically register a KinectDeviceFactory.
  */
  void Load(us::ModuleContext* context)
  {
    //Implementing KinectDeviceFactory
    KinectDeviceFactory* kinectFactory = new KinectDeviceFactory();
    us::ServiceProperties kinectFactoryProps;
    kinectFactoryProps["ToFFactoryName"] =kinectFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(kinectFactory, kinectFactoryProps);
    kinectFactory->ConnectToFDevice();

    m_Factories.push_back( kinectFactory );
  }

  /**
  * @brief Unload called when the module is unloaded.
  */
  void Unload(us::ModuleContext* )
  {
  }

  /**
  @brief Default destructor which deletes all registered factories.
  Usually, there should never be more than one.
  */
  ~KinectActivator()
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
US_EXPORT_MODULE_ACTIVATOR(mitk::KinectActivator)
#endif
