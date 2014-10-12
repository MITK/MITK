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
#ifndef __mitkKinectV2Activator_h
#define __mitkKinectV2Activator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkKinectV2DeviceFactory.h"

#include "mitkAbstractToFDeviceFactory.h"

/*
  * This is the module activator for the "mitkKinectV2Module" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class KinectV2Activator : public us::ModuleActivator {
public:

    void Load(us::ModuleContext* context)
    {
        //Implementing KinectDeviceFactory
        KinectV2DeviceFactory* KinectV2Factory = new KinectV2DeviceFactory();
        us::ServiceProperties KinectV2FactoryProps;
        KinectV2FactoryProps["ToFFactoryName"] =KinectV2Factory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(KinectV2Factory, KinectV2FactoryProps);
        KinectV2Factory->ConnectToFDevice();

        m_Factories.push_back( KinectV2Factory );
    }

    void Unload(us::ModuleContext* )
    {
    }

    ~KinectV2Activator()
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

    std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitk::KinectV2Activator)
#endif
