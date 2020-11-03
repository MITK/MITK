/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef COREEXTOBJECTFACTORY_H_INCLUDED
#define COREEXTOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"

namespace mitk
{
  class IOExtObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(IOExtObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode *node) override;
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

  private:
    IOExtObjectFactory();
    ~IOExtObjectFactory() override;

    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

    itk::ObjectFactoryBase::Pointer m_ParRecFileIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtkUnstructuredGridIOFactory;
    itk::ObjectFactoryBase::Pointer m_StlVolumeTimeSeriesIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtkVolumeTimeSeriesIOFactory;

    itk::ObjectFactoryBase::Pointer m_UnstructuredGridVtkWriterFactory;
  };
}

#endif
