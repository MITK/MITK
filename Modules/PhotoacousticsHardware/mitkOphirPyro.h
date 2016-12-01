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


#ifndef MITKOPHIRPYRO_H_HEADER_INCLUDED
#define MITKOPHIRPYRO_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"

#include "vector"
#include "MitkPhotoacousticsHardwareExports.h"

#include "OphirPyroWrapper.h"

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

namespace mitk {

  class MITKPHOTOACOUSTICSHARDWARE_EXPORT OphirPyro : public itk::Object
  {
  public:
    mitkClassMacroItkParent(mitk::OphirPyro, itk::Object);
    itkFactorylessNewMacro(Self);

    virtual bool OpenConnection();
    virtual bool CloseConnection();
    virtual bool StartDataAcquisition();
    virtual bool StopDataAcquisition();
    unsigned int GetDataFromSensor();
    void GetDataFromSensorThread();
    void SaveData();

    virtual double LookupCurrentPulseEnergy();
    virtual double GetNextPulseEnergy();
    virtual double LookupCurrentPulseEnergy(double* timestamp, int* status);
    virtual double GetNextPulseEnergy(double* timestamp, int* status);

    virtual double GetClosestEnergyInmJ(long long ImageTimeStamp, double interval=20);
    virtual double GetNextEnergyInmJ(long long ImageTimeStamp, double interval = 20);
    virtual void SetSyncDelay(long long FirstImageTimeStamp);
    virtual bool IsSyncDelaySet();


  protected:
    OphirPyro();
    virtual                       ~OphirPyro();
    void                          SaveCsvData();
    OphirPyroWrapper              ophirAPI;
    char*                         m_SerialNumber;
    int                           m_DeviceHandle;
    bool                          m_Connected;
    bool                          m_Streaming;
    std::vector<double>           m_PulseEnergy;
    std::vector<double>           m_PulseTime;
    std::vector<int>              m_PulseStatus;
    std::vector<long long>        m_TimeStamps;

    std::vector<double>           m_PulseEnergySaved;
    std::vector<double>           m_PulseTimeSaved;
    std::vector<int>              m_PulseStatusSaved;
    std::vector<long long>        m_TimeStampsSaved;

    double                        m_CurrentWavelength;
    double                        m_CurrentEnergyRange;
    long long                     m_ImagePyroDelay;
    float                         m_EnergyMultiplicator;

    std::thread m_GetDataThread;
  };
} // namespace mitk

#endif /* MITKOPHIRPYRO_H_HEADER_INCLUDED */
