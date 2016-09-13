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
      
      GalilMotor();
      virtual ~GalilMotor();

      char* m_SomeFilePath;
      int m_ComPort;
      std::string m_ErrorMessage;
      GCon m_GalilSystem;
      GReturn m_ReturnCode;

      double m_CurrentWavelength;
    };
} // namespace mitk

#endif /* MITKGalilAPI_H_HEADER_INCLUDED */
