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

#ifndef mitkBoundingShapeObjectFactory_h
#define mitkBoundingShapeObjectFactory_h

#include <MitkBoundingShapeExports.h>
#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(BoundingShapeObjectFactory, CoreObjectFactoryBase) itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      Mapper::Pointer CreateMapper(DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(DataNode *node) override;
    const char *GetFileExtensions() override;
    CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    const char *GetSaveFileExtensions() override;
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
