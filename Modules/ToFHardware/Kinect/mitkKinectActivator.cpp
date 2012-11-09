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
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkKinectDeviceFactory.h"

#include "mitkAbstractToFDeviceFactory.h"

/*
  * This is the module activator for the "mitkKinectModule" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class KinectActivator : public ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {
        //Implementing KinectDeviceFactory
        KinectDeviceFactory* kinectFactory = new KinectDeviceFactory();
        ServiceProperties kinectFactoryProps;
        kinectFactoryProps["ToFFactoryName"] =kinectFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(kinectFactory, kinectFactoryProps);
        kinectFactory->ConnectToFDevice();

        m_Factories.push_back( kinectFactory );
    }

    void Unload(mitk::ModuleContext* )
    {
    }

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

    std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitkKinectModule, mitk::KinectActivator)
#endif
