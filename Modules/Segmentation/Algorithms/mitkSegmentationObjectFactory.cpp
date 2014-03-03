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

#include "mitkSegmentationObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkCoreObjectFactory.h"

#include "mitkLabelSetImage.h"
#include "mitkLabelSetImageVtkMapper2D.h"
#include "mitkLabelSetImageWriter.h"
#include "mitkLabelSetImageWriterFactory.h"
#include "mitkLabelSetImageIOFactory.h"

mitk::SegmentationObjectFactory::SegmentationObjectFactory()
  : CoreObjectFactoryBase()
  , m_LabelSetImageIOFactory(mitk::LabelSetImageIOFactory::New().GetPointer())
  , m_LabelSetImageWriterFactory(mitk::LabelSetImageWriterFactory::New().GetPointer())
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    itk::ObjectFactoryBase::RegisterFactory( m_LabelSetImageIOFactory );
    itk::ObjectFactoryBase::RegisterFactory( m_LabelSetImageWriterFactory );

    this->m_FileWriters.push_back(mitk::LabelSetImageWriter::New().GetPointer());

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::SegmentationObjectFactory::~SegmentationObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_LabelSetImageIOFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_LabelSetImageWriterFactory);
}

mitk::Mapper::Pointer mitk::SegmentationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<LabelSetImage*>(data)!=NULL))
      {
        newMapper = mitk::LabelSetImageVtkMapper2D::New();
        newMapper->SetDataNode(node);
      }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if( dynamic_cast<LabelSetImage*>(node->GetData())!=NULL )
    {
//      newMapper = mitk::LabelSetImageMapper3D::New();
//      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::SegmentationObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if(node==NULL)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  if(node->GetData() ==NULL)
    return;

  if( dynamic_cast<LabelSetImage*>(node->GetData())!=NULL )
  {
    mitk::LabelSetImageVtkMapper2D::SetDefaultProperties(node);
//    mitk::LabelSetImageVtkMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::SegmentationObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SegmentationObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SegmentationObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::SegmentationObjectFactory::CreateFileExtensionsMap()
{
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.lset", "Segmentation files"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.lset", "Segmentation files"));
}

const char* mitk::SegmentationObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::SegmentationObjectFactory::RegisterIOFactories()
{
}

struct RegisterSegmentationObjectFactory{
  RegisterSegmentationObjectFactory()
    : m_Factory( mitk::SegmentationObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterSegmentationObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::SegmentationObjectFactory::Pointer m_Factory;
};

static RegisterSegmentationObjectFactory registerContourObjectFactory;
