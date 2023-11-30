/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIObjectFactory_h
#define mitkROIObjectFactory_h

#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  class ROIObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(ROIObjectFactory, CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)

    Mapper::Pointer CreateMapper(DataNode* node, MapperSlotId slotId) override;
    void SetDefaultProperties(DataNode *node) override;
    std::string GetFileExtensions() override;
    MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    MultimapType GetSaveFileExtensionsMap() override;

  protected:
    ROIObjectFactory();
    ~ROIObjectFactory() override;
  };
}

#endif
