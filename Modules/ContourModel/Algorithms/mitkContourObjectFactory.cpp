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

#include "mitkContourObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkCoreObjectFactory.h"


#include "mitkContourModel.h"
#include "mitkContourModelSet.h"
#include "mitkContourModelIOFactory.h"
#include "mitkContourModelWriterFactory.h"
#include "mitkContourModelWriter.h"
#include "mitkContourModelSetWriter.h"
#include "mitkContourModelMapper2D.h"
#include "mitkContourModelGLMapper2D.h"
#include "mitkContourModelSetGLMapper2D.h"
#include "mitkContourModelMapper3D.h"
#include "mitkContourModelSetMapper3D.h"


mitk::ContourObjectFactory::ContourObjectFactory()
  : CoreObjectFactoryBase()
  , m_ContourModelIOFactory(mitk::ContourModelIOFactory::New().GetPointer())
  , m_ContourModelWriterFactory(mitk::ContourModelWriterFactory::New().GetPointer())
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "ContourObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory( m_ContourModelIOFactory );
    itk::ObjectFactoryBase::RegisterFactory( m_ContourModelWriterFactory );

    this->m_FileWriters.push_back(mitk::ContourModelWriter::New().GetPointer());
    this->m_FileWriters.push_back(mitk::ContourModelSetWriter::New().GetPointer());

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::ContourObjectFactory::~ContourObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_ContourModelIOFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_ContourModelWriterFactory);
}

mitk::Mapper::Pointer mitk::ContourObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("ContourModel");
    if( dynamic_cast<mitk::ContourModel*>(node->GetData())!=NULL )
    {
      newMapper = mitk::ContourModelGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if( dynamic_cast<mitk::ContourModelSet*>(node->GetData())!=NULL )
    {
      newMapper = mitk::ContourModelSetGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if( dynamic_cast<mitk::ContourModel*>(node->GetData())!=NULL )
    {
      newMapper = mitk::ContourModelMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if( dynamic_cast<mitk::ContourModelSet*>(node->GetData())!=NULL )
    {
      newMapper = mitk::ContourModelSetMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::ContourObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{

  if(node==NULL)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  if(node->GetData() ==NULL)
    return;

  if( dynamic_cast<mitk::ContourModel*>(node->GetData())!=NULL )
  {
    mitk::ContourModelGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelMapper3D::SetDefaultProperties(node);
  }
  else if( dynamic_cast<mitk::ContourModelSet*>(node->GetData())!=NULL )
  {
    mitk::ContourModelSetGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelSetMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::ContourObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::ContourObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::ContourObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::ContourObjectFactory::CreateFileExtensionsMap()
{
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.cnt", "Contour Files"));

  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.cnt", "Contour File"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.cnt_set", "ContourModelSet Files"));

  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.cnt_set", "ContourModelSet File"));
}

const char* mitk::ContourObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::ContourObjectFactory::RegisterIOFactories()
{
}


struct RegisterContourObjectFactory{
  RegisterContourObjectFactory()
    : m_Factory( mitk::ContourObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterContourObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::ContourObjectFactory::Pointer m_Factory;
};

static RegisterContourObjectFactory registerContourObjectFactory;
