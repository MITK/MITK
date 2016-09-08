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


#ifndef MITKOpotekLaser_H_HEADER_INCLUDED
#define MITKOpotekLaser_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"

#include "MitkPhotoacousticsHardwareOpotekExports.h"

namespace mitk {

    class MITKPHOTOACOUSTICSHARDWAREOPOTEK_EXPORT OpotekLaser : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(mitk::OpotekLaser, itk::LightObject);
      itkFactorylessNewMacro(Self);

      virtual void SetConfigurationPath(std::string configurationFilePath);
      virtual bool Initialize();
      virtual bool ResetAndRelease();
      virtual bool TuneToWavelength(int wavelengthInNanoMeterTenths);

      virtual bool StartFlashing();
      virtual bool StopFlashing();

      virtual bool StartQswitching();
      virtual bool StopQswitching();
      virtual bool IsFlashing();
      virtual bool IsEmitting();
      virtual int GetMinWavelength();
      virtual int GetMaxWavelength();
      virtual int GetWavelength();

    protected:

      std::map<int, std::string> m_ErrorMessages;

      OpotekLaser();
      virtual ~OpotekLaser();

      char* m_ConfigurationFilePath;
      int m_MotorComPort;
      int m_PumpLaserComPort;
      int m_ErrorCode;

      double m_WavelengthMax;
      double m_WavelengthMin;
      int m_CurrentWavelengthInNmTenths;

      bool m_IsFlashing;
      bool m_IsEmitting;
    };
} // namespace mitk

#endif /* MITKOpotekLaser_H_HEADER_INCLUDED */
