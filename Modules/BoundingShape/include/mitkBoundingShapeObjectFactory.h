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

#include <mitkCoreObjectFactoryBase.h>
#include <MitkBoundingShapeExports.h>

namespace mitk
{
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeObjectFactory : public CoreObjectFactoryBase
  {
  public:

    mitkClassMacro(BoundingShapeObjectFactory, CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual Mapper::Pointer CreateMapper(DataNode* node, MapperSlotId slotId) override;
    virtual void SetDefaultProperties(DataNode* node) override;
    virtual const char* GetFileExtensions() override;
    virtual CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    virtual const char* GetSaveFileExtensions() override;
    virtual CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
    virtual const char* GetDescription() const override;

  protected:
    BoundingShapeObjectFactory();
    virtual ~BoundingShapeObjectFactory();

    void CreateFileExtensionsMap();

  private:
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };

  MITKBOUNDINGSHAPE_EXPORT void RegisterBoundingShapeObjectFactory();
}

#endif
