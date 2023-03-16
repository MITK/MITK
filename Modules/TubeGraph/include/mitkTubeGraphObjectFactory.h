/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTubeGraphObjectFactory_h
#define mitkTubeGraphObjectFactory_h

#include <MitkTubeGraphExports.h>

#include <mitkCoreObjectFactory.h>

namespace mitk
{
  class MITKTUBEGRAPH_EXPORT TubeGraphObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(TubeGraphObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode *node) override;
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
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
