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

#include "mitkMultilabelObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkCoreObjectFactory.h"
#include <mitkCoreServices.h>
#include <mitkIPropertyFilters.h>
#include <mitkPropertyFilter.h>
#include <mitkLabelSetImageVtkMapper2D.h>


mitk::MultilabelObjectFactory::MultilabelObjectFactory()
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "MultilabelObjectFactory c'tor" << std::endl;

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::MultilabelObjectFactory::~MultilabelObjectFactory()
{

}

mitk::Mapper::Pointer mitk::MultilabelObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<mitk::LabelSetImage*>(data)!=NULL))
    {
      newMapper = mitk::LabelSetImageVtkMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::MultilabelObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if(node == nullptr)
    return;

  if(node->GetData() == nullptr)
    return;

  if(dynamic_cast<LabelSetImage*>(node->GetData()) != nullptr)
  {
    mitk::LabelSetImageVtkMapper2D::SetDefaultProperties(node);

    auto propertyFilters = CoreServices::GetPropertyFilters();

    if (propertyFilters != nullptr)
    {
      PropertyFilter labelSetImageFilter;
      labelSetImageFilter.AddEntry("binaryimage.hoveringannotationcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("binaryimage.hoveringcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("binaryimage.selectedannotationcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("binaryimage.selectedcolor", PropertyFilter::Blacklist);
      labelSetImageFilter.AddEntry("outline binary shadow color", PropertyFilter::Blacklist);

      propertyFilters->AddFilter(labelSetImageFilter, "LabelSetImage");
    }
  }
}

const char* mitk::MultilabelObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::MultilabelObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::MultilabelObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::MultilabelObjectFactory::CreateFileExtensionsMap()
{

}

const char* mitk::MultilabelObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

struct RegisterMultilabelObjectFactory{
  RegisterMultilabelObjectFactory()
    : m_Factory( mitk::MultilabelObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterMultilabelObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::MultilabelObjectFactory::Pointer m_Factory;
};

static RegisterMultilabelObjectFactory registerMultilabelObjectFactory;
