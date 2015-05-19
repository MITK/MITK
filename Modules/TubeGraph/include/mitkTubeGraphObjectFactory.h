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

#ifndef TubeGraphObjectFactory_H_INCLUDED
#define TubeGraphObjectFactory_H_INCLUDED

#include <MitkTubeGraphExports.h>

#include <mitkCoreObjectFactory.h>

namespace mitk {

  class  MITKTUBEGRAPH_EXPORT TubeGraphObjectFactory : public CoreObjectFactoryBase
  {

  public:
    mitkClassMacro(TubeGraphObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
    virtual void SetDefaultProperties(mitk::DataNode* node) override;
    virtual const char* GetFileExtensions() override;
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    virtual const char* GetSaveFileExtensions() override;
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
    void RegisterIOFactories();

  protected:
    TubeGraphObjectFactory();

  private:
    void CreateFileExtensionsMap();

    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };

}

#endif
