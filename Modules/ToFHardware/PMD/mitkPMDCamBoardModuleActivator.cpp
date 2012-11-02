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
#ifndef __mitkToFCameraPMDCamBoardActivator_h
#define __mitkToFCameraPMDCamBoardActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkToFCameraPMDCamBoardDeviceFactory.h"
#include "mitkToFCameraPMDRawDataCamBoardDeviceFactory.h"

#include "mitkToFCameraPMDPlayerDeviceFactory.h"

//Implementing a Device on start
#include "mitkAbstractToFDeviceFactory.h"


/*
  * This is the module activator for the "mitkPMDCamBoardModule" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class PMDCamBoardModuleActivator : public ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {


        //Implementing CamBoardDeviceFactory
        ToFCameraPMDCamBoardDeviceFactory* toFCameraPMDCamBoardDeviceFactory = new ToFCameraPMDCamBoardDeviceFactory();
        ServiceProperties camBoardFactoryProps;
        camBoardFactoryProps["ToFFactoryName"] = toFCameraPMDCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamBoardDeviceFactory, camBoardFactoryProps);

        toFCameraPMDCamBoardDeviceFactory->ConnectToFDevice();


        ////Implementing PMD Raw Data Cam Board DeviceFactory
        //ToFCameraPMDRawDataCamBoardDeviceFactory* toFCameraPMDRawDataCamBoardDeviceFactory = new ToFCameraPMDRawDataCamBoardDeviceFactory();
        //ServiceProperties rawCamBoardFactoryProps;
        //rawCamBoardFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamBoardDeviceFactory->GetFactoryName();
        //context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamBoardDeviceFactory,rawCamBoardFactoryProps);

        //toFCameraPMDRawDataCamBoardDeviceFactory->ConnectToFDevice();





        m_Factories.push_back(toFCameraPMDCamBoardDeviceFactory);
        //m_Factories.push_back(toFCameraPMDRawDataCamBoardDeviceFactory);
    }

    void Unload(mitk::ModuleContext* )
    {
    }

    ~PMDCamBoardModuleActivator()
    {
        //todo iterieren über liste m_Factories und löschen
        if(m_Factories.size() > 0)
        {
            for(std::list< IToFDeviceFactory* >::iterator it = m_Factories.begin(); it != m_Factories.end(); ++it)
            {
                delete (*it); //todo wie genau löschen?
            }
        }
    }

private:

    std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitkPMDCamBoardModule, mitk::PMDCamBoardModuleActivator)
#endif
