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

#ifndef mitkSimulationObjectFactory_h
#define mitkSimulationObjectFactory_h

#include "mitkSimulationIOFactory.h"
#include "mitkSimulationWriterFactory.h"
#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  class MitkSimulation_EXPORT SimulationObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(SimulationObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);

    Mapper::Pointer CreateMapper(DataNode* node, MapperSlotId slotId);
    const char* GetDescription() const;
    const char* GetFileExtensions();
    MultimapType GetFileExtensionsMap();
    const char* GetITKSourceVersion() const;
    const char* GetSaveFileExtensions();
    MultimapType GetSaveFileExtensionsMap();
    void SetDefaultProperties(DataNode* node);

  private:
    SimulationObjectFactory();
    ~SimulationObjectFactory();

    SimulationIOFactory::Pointer m_SimulationIOFactory;
    SimulationWriterFactory::Pointer m_SimulationWriterFactory;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };

  MitkSimulation_EXPORT void RegisterSimulationObjectFactory();
}

#endif
