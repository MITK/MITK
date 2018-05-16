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

#include "mitkMAPRegistrationWrapperObjectFactory.h"

#include <mitkProperties.h>
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>

#include "mitkRegistrationWrapperMapper2D.h"
#include "mitkRegistrationWrapperMapper3D.h"
#include "mitkMAPRegistrationWrapper.h"

typedef std::multimap<std::string, std::string> MultimapType;

mitk::MAPRegistrationWrapperObjectFactory::MAPRegistrationWrapperObjectFactory()
: CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    alreadyDone = true;
  }

}

mitk::MAPRegistrationWrapperObjectFactory::~MAPRegistrationWrapperObjectFactory()
{
}

mitk::Mapper::Pointer
mitk::MAPRegistrationWrapperObjectFactory::
CreateMapper(mitk::DataNode* node, MapperSlotId slotId)
{
    mitk::Mapper::Pointer newMapper=nullptr;

    if ( slotId == mitk::BaseRenderer::Standard2D )
    {
        std::string classname("MAPRegistrationWrapper");
        if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
        {
          newMapper = mitk::MITKRegistrationWrapperMapper2D::New();
          newMapper->SetDataNode(node);
        }
    }
    else if ( slotId == mitk::BaseRenderer::Standard3D )
    {
      std::string classname("MAPRegistrationWrapper");
      if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
      {
        newMapper = mitk::MITKRegistrationWrapperMapper3D::New();
        newMapper->SetDataNode(node);
      }
    }

    return newMapper;
};

void mitk::MAPRegistrationWrapperObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if(node==nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  if(node->GetData() ==nullptr)
    return;

  if( dynamic_cast<mitk::MAPRegistrationWrapper*>(node->GetData())!=nullptr )
  {
    mitk::MITKRegistrationWrapperMapperBase::SetDefaultProperties(node);
  }
}

const char* mitk::MAPRegistrationWrapperObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::MAPRegistrationWrapperObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::MAPRegistrationWrapperObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::MAPRegistrationWrapperObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

struct RegisterMAPRegistrationWrapperObjectFactoryHelper{
  RegisterMAPRegistrationWrapperObjectFactoryHelper()
    : m_Factory( mitk::MAPRegistrationWrapperObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterMAPRegistrationWrapperObjectFactoryHelper()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::MAPRegistrationWrapperObjectFactory::Pointer m_Factory;
};

static RegisterMAPRegistrationWrapperObjectFactoryHelper registerMITKRegistrationWrapperIOFactoryHelper;
