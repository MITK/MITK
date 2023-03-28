/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultilabelObjectFactory_h
#define mitkMultilabelObjectFactory_h

#include "mitkCoreObjectFactoryBase.h"

#include <mitkAbstractFileIO.h>

namespace mitk
{
  class MultilabelObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(MultilabelObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) Mapper::Pointer
      CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode *node) override;
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

  protected:
    MultilabelObjectFactory();
    ~MultilabelObjectFactory() override;
    void CreateFileExtensionsMap();
  };
}

#endif
