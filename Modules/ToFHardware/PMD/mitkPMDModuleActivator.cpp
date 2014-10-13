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
#include <usModuleContext.h>
#include <usModuleActivator.h>

//ToF
#include "mitkIToFDeviceFactory.h"
#include "mitkToFConfig.h"
#include "mitkToFPMDConfig.h"
#include "mitkToFCameraPMDPlayerDeviceFactory.h"
#include "mitkAbstractToFDeviceFactory.h"

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

namespace mitk
{
/**
 * @brief The PMDModuleActivator class This is the module activator for the "PMDModule"
 * module. It registers services like the IToFDeviceFactory.
 */
class PMDModuleActivator : public us::ModuleActivator {
public:

  /**
  * @brief Load This method is automatically called, when the PMDModule
  * is activated. It will automatically register factories of activated PMD devices.
  */
  void Load(us::ModuleContext* context)
  {
    //The PMD CamBoard-Factory gets activated as soon as the user toggels one the PMD-parameter in Cmake to on
#ifdef MITK_USE_TOF_PMDCAMCUBE
    //Register Cam Cube Factory
    ToFCameraPMDCamCubeDeviceFactory* toFCameraPMDCamCubeDeviceFactory = new ToFCameraPMDCamCubeDeviceFactory();
    us::ServiceProperties camCubeFactoryProps;
    camCubeFactoryProps["ToFFactoryName"] = toFCameraPMDCamCubeDeviceFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamCubeDeviceFactory,camCubeFactoryProps);

    toFCameraPMDCamCubeDeviceFactory->ConnectToFDevice();

    //Register PMD Raw Data Cam Cube DeviceFactory
    ToFCameraPMDRawDataCamCubeDeviceFactory* toFCameraPMDRawDataCamCubeDeviceFactory = new ToFCameraPMDRawDataCamCubeDeviceFactory();
    us::ServiceProperties rawCamCubeFactoryProps;
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
    us::ServiceProperties o3FactoryProps;
    o3FactoryProps["ToFFactoryName"] = toFCameraPMDO3DeviceFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(toFCameraPMDO3DeviceFactory,o3FactoryProps);

    toFCameraPMDO3DeviceFactory->ConnectToFDevice();

    m_Factories.push_back(toFCameraPMDO3DeviceFactory);
#endif

    //The PMD CamBoard-Factory gets activated as soon as the user toggels one the PMD-parameter in Cmake to on
#ifdef MITK_USE_TOF_PMDCAMBOARD
    //Implementing CamBoardDeviceFactory
    ToFCameraPMDCamBoardDeviceFactory* toFCameraPMDCamBoardDeviceFactory = new ToFCameraPMDCamBoardDeviceFactory();
    us::ServiceProperties camBoardFactoryProps;
    camBoardFactoryProps["ToFFactoryName"] = toFCameraPMDCamBoardDeviceFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamBoardDeviceFactory, camBoardFactoryProps);

    toFCameraPMDCamBoardDeviceFactory->ConnectToFDevice();


    //Implementing PMD Raw Data Cam Board DeviceFactory
    ToFCameraPMDRawDataCamBoardDeviceFactory* toFCameraPMDRawDataCamBoardDeviceFactory = new ToFCameraPMDRawDataCamBoardDeviceFactory();
    us::ServiceProperties rawCamBoardFactoryProps;
    rawCamBoardFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamBoardDeviceFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamBoardDeviceFactory,rawCamBoardFactoryProps);

    toFCameraPMDRawDataCamBoardDeviceFactory->ConnectToFDevice();

    m_Factories.push_back(toFCameraPMDCamBoardDeviceFactory);
    m_Factories.push_back(toFCameraPMDRawDataCamBoardDeviceFactory);
#endif

    //The PMD Player Device Factory gets activated as soon as the user toggels one of the PMD-parameters in Cmake to on
    //Registrating the PMD Player DeviceFactory
    ToFCameraPMDPlayerDeviceFactory* toFCameraPMDPlayerDeviceFactory = new ToFCameraPMDPlayerDeviceFactory();
    us::ServiceProperties pMDPlayerFactoryProps;
    pMDPlayerFactoryProps["ToFFactoryName"] = toFCameraPMDPlayerDeviceFactory->GetFactoryName();
    context->RegisterService<IToFDeviceFactory>(toFCameraPMDPlayerDeviceFactory,pMDPlayerFactoryProps);

    toFCameraPMDPlayerDeviceFactory->ConnectToFDevice();

    m_Factories.push_back(toFCameraPMDPlayerDeviceFactory);

  }

  /**
    * @brief Unload called when the module is unloaded.
    */
  void Unload(us::ModuleContext* )
  {
  }

  /**
    @brief Default destructor which deletes all registered factories.
    */
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
  /**
    * @brief m_Factories List of all registered factories.
    */
  std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitk::PMDModuleActivator)
#endif
