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
#ifndef __mitkToFCameraPMDModuleActivator_h
#define __mitkToFCameraPMDModuleActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"
#include "mitkToFConfig.h"
#include "mitkToFPMDConfig.h"


//The follwing heades only get implemented if the respective Devices are activated through Cmake
#ifdef MITK_USE_TOF_PMDCAMCUBE
#include "mitkToFCameraPMDCamCubeDeviceFactory.h"
#include "mitkToFCameraPMDRawDataCamCubeDeviceFactory.h"
#endif

#ifdef MITK_USE_TOF_PMDO3
#include "mitkToFCameraPMDO3DeviceFactory.h"
#endif

#ifdef MITK_USE_TOF_PMDCAMBOARD
#include "mitkToFCameraPMDCamBoardDeviceFactory.h"
#include "mitkToFCameraPMDRawDataCamBoardDeviceFactory.h"
#endif

#include "mitkToFCameraPMDPlayerDeviceFactory.h"



//Implementing a Device on start
#include "mitkAbstractToFDeviceFactory.h"


/*
  * This is the module activator for the "mitkPMDModule". It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class PMDModuleActivator : public ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {
    //The PMD CamBoard-Factory gets activated as soon as the user toggels one the PMD-parameter in Cmake to on
      #ifdef MITK_USE_TOF_PMDCAMCUBE
       //Implementing Cam Cube
        ToFCameraPMDCamCubeDeviceFactory* toFCameraPMDCamCubeDeviceFactory = new ToFCameraPMDCamCubeDeviceFactory();
        ServiceProperties camCubeFactoryProps;
        camCubeFactoryProps["ToFFactoryName"] = toFCameraPMDCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamCubeDeviceFactory,camCubeFactoryProps);

        toFCameraPMDCamCubeDeviceFactory->ConnectToFDevice();

       //Implementing PMD Raw Data Cam Cube DeviceFactory
        ToFCameraPMDRawDataCamCubeDeviceFactory* toFCameraPMDRawDataCamCubeDeviceFactory = new ToFCameraPMDRawDataCamCubeDeviceFactory();
        ServiceProperties rawCamCubeFactoryProps;
        rawCamCubeFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamCubeDeviceFactory,rawCamCubeFactoryProps);

        toFCameraPMDRawDataCamCubeDeviceFactory->ConnectToFDevice();

        m_Factories.push_back(toFCameraPMDCamCubeDeviceFactory);
        m_Factories.push_back(toFCameraPMDRawDataCamCubeDeviceFactory);
      #endif

    //The PMD O3-Factory gets activated as soon as the user toggels one the PMD-parameter in Cmake to on
      #ifdef MITK_USE_TOF_PMDO3
       //Implementing PMD O3D DeviceFactory
        ToFCameraPMDO3DeviceFactory* toFCameraPMDO3DeviceFactory = new ToFCameraPMDO3DeviceFactory();
        ServiceProperties o3FactoryProps;
        o3FactoryProps["ToFFactoryName"] = toFCameraPMDO3DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDO3DeviceFactory,o3FactoryProps);

        toFCameraPMDO3DeviceFactory->ConnectToFDevice();

        m_Factories.push_back(toFCameraPMDO3DeviceFactory);
      #endif

    //The PMD CamBoard-Factory gets activated as soon as the user toggels one the PMD-parameter in Cmake to on
      #ifdef MITK_USE_TOF_PMDCAMBOARD
       //Implementing CamBoardDeviceFactory
        ToFCameraPMDCamBoardDeviceFactory* toFCameraPMDCamBoardDeviceFactory = new ToFCameraPMDCamBoardDeviceFactory();
        ServiceProperties camBoardFactoryProps;
        camBoardFactoryProps["ToFFactoryName"] = toFCameraPMDCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamBoardDeviceFactory, camBoardFactoryProps);

        toFCameraPMDCamBoardDeviceFactory->ConnectToFDevice();


       //Implementing PMD Raw Data Cam Board DeviceFactory
        ToFCameraPMDRawDataCamBoardDeviceFactory* toFCameraPMDRawDataCamBoardDeviceFactory = new ToFCameraPMDRawDataCamBoardDeviceFactory();
        ServiceProperties rawCamBoardFactoryProps;
        rawCamBoardFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamBoardDeviceFactory,rawCamBoardFactoryProps);

        toFCameraPMDRawDataCamBoardDeviceFactory->ConnectToFDevice();

        m_Factories.push_back(toFCameraPMDCamBoardDeviceFactory);
        m_Factories.push_back(toFCameraPMDRawDataCamBoardDeviceFactory);
      #endif

      //The PMD Player Device Factory gets activated as soon as the user toggels one of the PMD-parameters in Cmake to on
        //Registrating the PMD Player DeviceFactory
        ToFCameraPMDPlayerDeviceFactory* toFCameraPMDPlayerDeviceFactory = new ToFCameraPMDPlayerDeviceFactory();
        ServiceProperties pMDPlayerFactoryProps;
        pMDPlayerFactoryProps["ToFFactoryName"] = toFCameraPMDPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDPlayerDeviceFactory,pMDPlayerFactoryProps);

        toFCameraPMDPlayerDeviceFactory->ConnectToFDevice();

        m_Factories.push_back(toFCameraPMDPlayerDeviceFactory);

    }

    void Unload(mitk::ModuleContext* )
    {
    }

    ~PMDModuleActivator()
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
US_EXPORT_MODULE_ACTIVATOR(mitkPMDModule, mitk::PMDModuleActivator)
#endif
