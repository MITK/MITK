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

#include <mitkCoreObjectFactoryBase.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MITKSIMULATION_EXPORT SimulationObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(SimulationObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);

    Mapper::Pointer CreateMapper(DataNode* node, MapperSlotId slotId) override;
    const char* GetDescription() const override;
    const char* GetFileExtensions() override;
    MultimapType GetFileExtensionsMap() override;
    const char* GetSaveFileExtensions() override;
    MultimapType GetSaveFileExtensionsMap() override;
    void SetDefaultProperties(DataNode* node) override;

  private:
    SimulationObjectFactory();
    ~SimulationObjectFactory();
  };

  MITKSIMULATION_EXPORT void RegisterSimulationObjectFactory();
}

#endif
