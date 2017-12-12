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

#include "mitkGPUVolumeMapper3D.h"

typedef char TbssRoiPixelType;
typedef float TbssPixelType;
typedef int TbssGradientPixelType;

mitk::QuantificationObjectFactory::QuantificationObjectFactory()
  : CoreObjectFactoryBase()
  , m_NrrdTbssImageIOFactory(mitk::NrrdTbssImageIOFactory::New().GetPointer())
  , m_NrrdTbssRoiImageIOFactory(mitk::NrrdTbssRoiImageIOFactory::New().GetPointer())
  , m_NrrdTbssImageWriterFactory(mitk::NrrdTbssImageWriterFactory::New().GetPointer())
  , m_NrrdTbssRoiImageWriterFactory(mitk::NrrdTbssRoiImageWriterFactory::New().GetPointer())
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "QuantificationObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory(m_NrrdTbssImageIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_NrrdTbssRoiImageIOFactory);

    itk::ObjectFactoryBase::RegisterFactory(m_NrrdTbssImageWriterFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_NrrdTbssRoiImageWriterFactory);

    m_FileWriters.push_back( NrrdTbssImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTbssRoiImageWriter::New().GetPointer() );

    CreateFileExtensionsMap();

    alreadyDone = true;
  }

}

mitk::QuantificationObjectFactory::~QuantificationObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdTbssImageIOFactory);
  itk::ObjectFactoryBase::RegisterFactory(m_NrrdTbssRoiImageIOFactory);

  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdTbssImageWriterFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdTbssRoiImageWriterFactory);
}

mitk::Mapper::Pointer mitk::QuantificationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=nullptr;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname = "TbssRoiImage";
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
    std::string classname = "TbssImage";
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
  std::string classname = "TbssRoiImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::ImageVtkMapper2D::SetDefaultProperties(node);
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

struct RegisterQuantificationObjectFactory{
  RegisterQuantificationObjectFactory()
    : m_Factory( mitk::QuantificationObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterQuantificationObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::QuantificationObjectFactory::Pointer m_Factory;
};

static RegisterQuantificationObjectFactory registerQuantificationObjectFactory;
