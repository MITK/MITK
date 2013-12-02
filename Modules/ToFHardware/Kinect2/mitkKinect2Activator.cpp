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
#ifndef __mitkKinect2Activator_h
#define __mitkKinect2Activator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkKinect2DeviceFactory.h"

#include "mitkAbstractToFDeviceFactory.h"

/*
  * This is the module activator for the "mitkKinect2Module" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class Kinect2Activator : public us::ModuleActivator {
public:

    void Load(us::ModuleContext* context)
    {
        //Implementing KinectDeviceFactory
        Kinect2DeviceFactory* kinect2Factory = new Kinect2DeviceFactory();
        us::ServiceProperties kinect2FactoryProps;
        kinect2FactoryProps["ToFFactoryName"] =kinect2Factory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(kinect2Factory, kinect2FactoryProps);
        kinect2Factory->ConnectToFDevice();

        m_Factories.push_back( kinect2Factory );
    }

    void Unload(us::ModuleContext* )
    {
    }

    ~Kinect2Activator()
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
US_EXPORT_MODULE_ACTIVATOR(mitkKinect2Module, mitk::Kinect2Activator)
#endif
