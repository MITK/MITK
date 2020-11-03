/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeObjectFactory_h
#define mitkBoundingShapeObjectFactory_h

#include <MitkBoundingShapeExports.h>
#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(BoundingShapeObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      Mapper::Pointer CreateMapper(DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(DataNode *node) override;
    std::string GetFileExtensions() override;
    CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
    const char *GetDescription() const override;

  protected:
    BoundingShapeObjectFactory();
    ~BoundingShapeObjectFactory() override;

    void CreateFileExtensionsMap();

  private:
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };

  MITKBOUNDINGSHAPE_EXPORT void RegisterBoundingShapeObjectFactory();
}

#endif
