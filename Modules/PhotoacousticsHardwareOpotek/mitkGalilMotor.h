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
#include "MitkPhotoacousticsHardwareOpotekExports.h"

#include "gclib.h"
#include "gclibo.h"

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

#include <tinyxml.h>

namespace mitk {

    class MITKPHOTOACOUSTICSHARDWAREOPOTEK_EXPORT GalilMotor : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(mitk::GalilMotor, itk::LightObject);
      itkFactorylessNewMacro(Self);

      virtual bool OpenConnection();
      virtual bool CloseConnection();

      bool TuneToWavelength(double wavelength);

      bool FastTuneWavelengths(std::vector<double> wavelengthList);

    protected:
      virtual bool Home();
      void LoadResorceFile(std::string filename, std::string* lines);
      int GetPositionFromWavelength(double wavelength);
      GalilMotor();
      virtual ~GalilMotor();

      int m_ComPort;
      std::string m_ErrorMessage;
      GCon m_GalilSystem;
      GReturn m_ReturnCode;

      std::string m_XmlOpoConfiguration;
      double m_CurrentWavelength;
      double m_WavelengthToStepCalibration[8];

    };
} // namespace mitk

#endif /* MITKGalilAPI_H_HEADER_INCLUDED */
