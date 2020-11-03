/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourObjectFactory.h"

#include "mitkBaseRenderer.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"

#include "mitkContourModel.h"
#include "mitkContourModelGLMapper2D.h"
#include "mitkContourModelMapper2D.h"
#include "mitkContourModelMapper3D.h"
#include "mitkContourModelSet.h"
#include "mitkContourModelSetGLMapper2D.h"
#include "mitkContourModelSetMapper3D.h"
#include "mitkContourModelSetWriter.h"
#include "mitkContourModelWriter.h"

mitk::ContourObjectFactory::ContourObjectFactory() : CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "ContourObjectFactory c'tor" << std::endl;

    alreadyDone = true;
  }
}

mitk::ContourObjectFactory::~ContourObjectFactory()
{
}

mitk::Mapper::Pointer mitk::ContourObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;

  if (id == mitk::BaseRenderer::Standard2D)
  {
    std::string classname("ContourModel");
    if (dynamic_cast<mitk::ContourModel *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if (dynamic_cast<mitk::ContourModelSet *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelSetGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if (id == mitk::BaseRenderer::Standard3D)
  {
    if (dynamic_cast<mitk::ContourModel *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if (dynamic_cast<mitk::ContourModelSet *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelSetMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::ContourObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  if (node->GetData() == nullptr)
    return;

  if (dynamic_cast<mitk::ContourModel *>(node->GetData()) != nullptr)
  {
    mitk::ContourModelGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelMapper3D::SetDefaultProperties(node);
  }
  else if (dynamic_cast<mitk::ContourModelSet *>(node->GetData()) != nullptr)
  {
    mitk::ContourModelSetGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelSetMapper3D::SetDefaultProperties(node);
  }
}

std::string mitk::ContourObjectFactory::GetFileExtensions()
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
}

std::string mitk::ContourObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::ContourObjectFactory::RegisterIOFactories()
{
}

struct RegisterContourObjectFactory
{
  RegisterContourObjectFactory() : m_Factory(mitk::ContourObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterContourObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::ContourObjectFactory::Pointer m_Factory;
};

static RegisterContourObjectFactory registerContourObjectFactory;
