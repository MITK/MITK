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
#include "mitkContourModelIOFactory.h"
#include "mitkContourModelWriterFactory.h"
#include "mitkContourModelWriter.h"
#include "mitkContourModelMapper2D.h"
#include "mitkContourModelGLMapper2D.h"
#include "mitkContourModelMapper3D.h"


mitk::ContourObjectFactory::ContourObjectFactory()
: CoreObjectFactoryBase()
, m_ContourModelIOFactory(mitk::ContourModelIOFactory::New().GetPointer())
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "ContourObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory( m_ContourModelIOFactory );

    mitk::ContourModelWriterFactory::RegisterOneFactory();

    this->m_FileWriters.push_back(mitk::ContourModelWriter::New().GetPointer());

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::ContourObjectFactory::~ContourObjectFactory()
{
  mitk::ContourModelWriterFactory::UnRegisterOneFactory();
  itk::ObjectFactoryBase::UnRegisterFactory(m_ContourModelIOFactory);
}

mitk::Mapper::Pointer mitk::ContourObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("ContourModel");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ContourModelGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    std::string classname("ContourModel");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ContourModelMapper3D::New();
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

  std::string classname("ContourModel");
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::ContourModelGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelMapper3D::SetDefaultProperties(node);
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
