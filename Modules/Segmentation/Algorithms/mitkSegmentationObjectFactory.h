/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationObjectFactory_h
#define mitkSegmentationObjectFactory_h

#include "mitkCoreObjectFactoryBase.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT SegmentationObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(SegmentationObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) Mapper::Pointer
      CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode *node) override;
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
    void RegisterIOFactories();

  protected:
    SegmentationObjectFactory();
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };
}

#endif
