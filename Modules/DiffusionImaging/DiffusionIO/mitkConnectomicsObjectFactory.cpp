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


#include "mitkConnectomicsObjectFactory.h"

#include "mitkConnectomicsNetwork.h"
#include "mitkConnectomicsNetworkMapper3D.h"

mitk::ConnectomicsObjectFactory::ConnectomicsObjectFactory()
  : CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "ConnectomicsObjectFactory c'tor" << std::endl;

    alreadyDone = true;
  }

}

mitk::ConnectomicsObjectFactory::~ConnectomicsObjectFactory()
{
}

mitk::Mapper::Pointer mitk::ConnectomicsObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard3D )
  {
    std::string classname("ConnectomicsNetwork");
    if (node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ConnectomicsNetworkMapper3D::New();
      newMapper->SetDataNode(node);
    }

  }

  return newMapper;
}

void mitk::ConnectomicsObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  std::string classname("ConnectomicsNetwork");
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::ConnectomicsNetworkMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::ConnectomicsObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::ConnectomicsObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::ConnectomicsObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::ConnectomicsObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::ConnectomicsObjectFactory::CreateFileExtensionsMap()
{
}

struct RegisterConnectomicsObjectFactory{
  RegisterConnectomicsObjectFactory()
    : m_Factory( mitk::ConnectomicsObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterConnectomicsObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::ConnectomicsObjectFactory::Pointer m_Factory;
};

static RegisterConnectomicsObjectFactory registerDiffusionCoreObjectFactory;
