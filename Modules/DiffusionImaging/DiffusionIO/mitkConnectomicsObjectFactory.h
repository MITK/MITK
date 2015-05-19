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


#ifndef MITKCONNECTOMICSOBJECTFACTORY_H
#define MITKCONNECTOMICSOBJECTFACTORY_H

#include "mitkCoreObjectFactory.h"

namespace mitk {

class ConnectomicsObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(ConnectomicsObjectFactory,CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    ~ConnectomicsObjectFactory();

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

    virtual void SetDefaultProperties(mitk::DataNode* node) override;

    virtual const char* GetFileExtensions() override;

    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    virtual const char* GetSaveFileExtensions() override;

    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

  protected:
    ConnectomicsObjectFactory();
  private:
    void CreateFileExtensionsMap();
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

    itk::ObjectFactoryBase::Pointer m_ConnectomicsNetworkIOFactory;
    itk::ObjectFactoryBase::Pointer m_ConnectomicsNetworkWriterFactory;
};

}

#endif
