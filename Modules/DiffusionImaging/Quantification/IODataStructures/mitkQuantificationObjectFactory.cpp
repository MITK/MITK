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

#include "mitkQuantificationObjectFactory.h"

#include "mitkTbssImageMapper.h"

#include "mitkNrrdTbssImageIOFactory.h"
#include "mitkNrrdTbssImageWriterFactory.h"
#include "mitkNrrdTbssImageWriter.h"

#include "mitkNrrdTbssRoiImageIOFactory.h"
#include "mitkNrrdTbssRoiImageWriterFactory.h"
#include "mitkNrrdTbssRoiImageWriter.h"

typedef char TbssRoiPixelType;
typedef float TbssPixelType;
typedef int TbssGradientPixelType;

mitk::QuantificationObjectFactory::QuantificationObjectFactory(bool /*registerSelf*/)
:CoreObjectFactoryBase()
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "QuantificationObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    mitk::NrrdTbssImageIOFactory::RegisterOneFactory();
    mitk::NrrdTbssRoiImageIOFactory::RegisterOneFactory();

    mitk::NrrdTbssImageWriterFactory::RegisterOneFactory();
    mitk::NrrdTbssRoiImageWriterFactory::RegisterOneFactory();

    m_FileWriters.push_back( NrrdTbssImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTbssRoiImageWriter::New().GetPointer() );

    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
    CreateFileExtensionsMap();

    alreadyDone = true;
  }

}

mitk::Mapper::Pointer mitk::QuantificationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    classname = "TbssRoiImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ImageVtkMapper2D::New();
      newMapper->SetDataNode(node);
    }

    classname = "TbssImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::TbssImageMapper::New();
      newMapper->SetDataNode(node);
    }

  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    classname = "TbssRoiImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::VolumeDataVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }

    classname = "TbssImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::TbssImageMapper::New();
      newMapper->SetDataNode(node);
    }

  }

  return newMapper;
}

void mitk::QuantificationObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  classname = "TbssRoiImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::ImageVtkMapper2D::SetDefaultProperties(node);
    mitk::VolumeDataVtkMapper3D::SetDefaultProperties(node);
  }

  classname = "TbssImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::TbssImageMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::QuantificationObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::QuantificationObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::QuantificationObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::QuantificationObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::QuantificationObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.tbss", "TBSS data"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.roi", "TBSS ROI data"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.tbss", "TBSS data"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.roi", "TBSS ROI data"));
}

void mitk::QuantificationObjectFactory::RegisterIOFactories()
{
}

void RegisterQuantificationObjectFactory()
{
  static bool oneQuantificationObjectFactoryRegistered = false;
  if ( ! oneQuantificationObjectFactoryRegistered ) {
    MITK_DEBUG << "Registering QuantificationObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::QuantificationObjectFactory::New());

    oneQuantificationObjectFactoryRegistered = true;
  }
}
