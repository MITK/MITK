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
    virtual double LookupCurrentPulseEnergy();
    virtual double GetNextPulseEnergy();
    virtual double LookupCurrentPulseEnergy(double* timestamp, int* status);
    virtual double GetNextPulseEnergy(double* timestamp, int* status);

  protected:
    OphirPyro();
    virtual ~OphirPyro();
    OphirPyroWrapper ophirAPI;
    char* m_SerialNumber;
    int m_DeviceHandle;
    bool m_Connected;
    std::vector<double> m_PulseEnergy;
    std::vector<double> m_PulseTime;
    std::vector<int> m_PulseStatus;
    double m_CurrentWavelength;
    double m_CurrentEnergyRange;
  };
} // namespace mitk

#endif /* MITKOPHIRPYRO_H_HEADER_INCLUDED */
