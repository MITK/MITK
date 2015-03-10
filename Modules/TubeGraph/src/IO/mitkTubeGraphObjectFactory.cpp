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

#include "mitkTubeGraphObjectFactory.h"

#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkProperties.h>

#include "mitkTubeGraph.h"
#include "mitkTubeGraphVtkMapper3D.h"

mitk::TubeGraphObjectFactory::TubeGraphObjectFactory()
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_INFO << "TubeGraphObjectFactory c'tor" << std::endl;
    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::TubeGraphObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = NULL;

  if ( id == mitk::BaseRenderer::Standard3D )
  {
    if ((dynamic_cast<mitk::TubeGraph*>(node->GetData()) != NULL))
    {
      newMapper = mitk::TubeGraphVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }

  return newMapper;
}

void mitk::TubeGraphObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if ((dynamic_cast<mitk::TubeGraph*>(node->GetData()) != NULL))
  {
    node->SetProperty( "Tube Graph.Clip Structures", mitk::BoolProperty::New( false ) );
    mitk::TubeGraphVtkMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::TubeGraphObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::TubeGraphObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::TubeGraphObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::TubeGraphObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::TubeGraphObjectFactory::CreateFileExtensionsMap()
{
}

struct RegisterTubeGraphObjectFactory
{
  RegisterTubeGraphObjectFactory()
    : m_Factory( mitk::TubeGraphObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }
  ~RegisterTubeGraphObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }
  mitk::TubeGraphObjectFactory::Pointer m_Factory;
};

static RegisterTubeGraphObjectFactory registerTubeGraphObjectFactory;
