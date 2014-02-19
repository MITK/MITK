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

#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkTubeGraph.h"
#include "mitkProperties.h"
#include "mitkTubeGraphIOFactory.h"
#include "mitkTubeGraphVtkMapper3D.h"
#include "mitkTubeGraphWriter.h"
#include "mitkTubeGraphWriterFactory.h"

mitk::TubeGraphObjectFactory::TubeGraphObjectFactory(bool /*registerSelf*/)
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_INFO << "TubeGraphObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    m_FileWriters.push_back( mitk::TubeGraphWriter::New().GetPointer() );

    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
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
  std::string classname = "Graph";
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

void mitk::TubeGraphObjectFactory::RegisterIOFactories()
{
  mitk::TubeGraphIOFactory::RegisterOneFactory();

  mitk::TubeGraphWriterFactory::RegisterOneFactory();
}

void mitk::TubeGraphObjectFactory::CreateFileExtensionsMap()
{ //TODO: Think about file extension
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.tsf", "Tube Graph Structure File"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.tsf", "Tube Graph Structure File"));
}

void RegisterTubeGraphObjectFactory()
{
  static bool oneTubeGraphObjectFactoryRegistered = false;
  if ( ! oneTubeGraphObjectFactoryRegistered )
  {
    MITK_INFO << "Registering TubeGraphObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::TubeGraphObjectFactory::New());

    oneTubeGraphObjectFactoryRegistered = true;
  }
}
