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

#ifndef mitkSimulationWriterFactory_h
#define mitkSimulationWriterFactory_h

#include <itkObjectFactoryBase.h>
#include <mitkCommon.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MitkSimulation_EXPORT SimulationWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacro(SimulationWriterFactory, itk::ObjectFactoryBase);
    itkFactorylessNewMacro(Self);

    const char* GetITKSourceVersion() const;
    const char* GetDescription() const;

  protected:
    SimulationWriterFactory();
    ~SimulationWriterFactory();
  };
}

#endif
