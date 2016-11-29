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


#ifndef MITKGalilAPI_H_HEADER_INCLUDED
#define MITKGalilAPI_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"

#include "vector"
#include "MitkPhotoacousticsHardwareExports.h"

#include "gclib.h"
#include "gclibo.h"

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

#include <tinyxml.h>

namespace mitk {

    class MITKPHOTOACOUSTICSHARDWARE_EXPORT GalilMotor : public itk::Object
    {
    public:
      mitkClassMacroItkParent(mitk::GalilMotor, itk::Object);
      itkFactorylessNewMacro(Self);

      virtual bool OpenConnection(std::string configuration);
      virtual bool CloseConnection();

      double GetMinWavelength();
      double GetMaxWavelength();
      double GetCurrentWavelength();

      bool TuneToWavelength(double wavelength, bool isRecalibrating);

      bool FastTuneWavelengths(std::vector<double> wavelengthList);

    protected:
      virtual bool Home();
      void LoadResorceFile(std::string filename, std::string* lines);
      int GetPositionFromWavelength(double wavelength);
      GalilMotor();
      virtual ~GalilMotor();

      int m_ComPort;
      int m_BaudRate;
      std::string m_ErrorMessage;
      GCon m_GalilSystem;
      GReturn m_ReturnCode;

      std::string m_XmlOpoConfiguration;
      double m_MinWavelength;
      double m_MaxWavelength;
      double m_CurrentWavelength;
      double m_WavelengthToStepCalibration[7];
      double m_HomePosition;

    };
} // namespace mitk

#endif /* MITKGalilAPI_H_HEADER_INCLUDED */
