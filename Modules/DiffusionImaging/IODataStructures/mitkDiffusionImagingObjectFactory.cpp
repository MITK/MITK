/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 15:59:04 +0200 (Do, 18 Jun 2009) $
Version:   $Revision: 16916 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkDiffusionImagingObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"

#include "mitkNrrdDiffusionImageIOFactory.h"
#include "mitkNrrdDiffusionImageWriterFactory.h"
#include "mitkNrrdDiffusionImageWriter.h"
#include "mitkDiffusionImage.h"

#include "mitkNrrdQBallImageIOFactory.h"
#include "mitkNrrdQBallImageWriterFactory.h"
#include "mitkNrrdQBallImageWriter.h"

#include "mitkNrrdTensorImageIOFactory.h"
#include "mitkNrrdTensorImageWriterFactory.h"
#include "mitkNrrdTensorImageWriter.h"

#include "mitkCompositeMapper.h"
#include "mitkDiffusionImageMapper.h"

typedef short DiffusionPixelType;
typedef mitk::DiffusionImage<DiffusionPixelType> DiffusionImageShort;
typedef std::multimap<std::string, std::string> MultimapType;

mitk::DiffusionImagingObjectFactory::DiffusionImagingObjectFactory(bool /*registerSelf*/)
:CoreObjectFactoryBase()
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_INFO << "DiffusionImagingObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    itk::ObjectFactoryBase::RegisterFactory( NrrdDiffusionImageIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( NrrdQBallImageIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( NrrdTensorImageIOFactory::New() );

    mitk::NrrdDiffusionImageWriterFactory::RegisterOneFactory();
    mitk::NrrdQBallImageWriterFactory::RegisterOneFactory();
    mitk::NrrdTensorImageWriterFactory::RegisterOneFactory();

    m_FileWriters.push_back( NrrdDiffusionImageWriter<DiffusionPixelType>::New().GetPointer() );
    m_FileWriters.push_back( NrrdQBallImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTensorImageWriter::New().GetPointer() );

    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
    
    CreateFileExtensionsMap();

    alreadyDone = true;
  }

}

mitk::Mapper::Pointer mitk::DiffusionImagingObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id) 
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("QBallImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
    }
    classname = "DiffusionImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::DiffusionImageMapper<short>::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    // do nothing
  }

  return newMapper;
}

void mitk::DiffusionImagingObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  std::string classname = "QBallImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
  }

  classname = "TensorImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
  }

  classname = "DiffusionImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::DiffusionImageMapper<short>::SetDefaultProperties(node);
  }
}

const char* mitk::DiffusionImagingObjectFactory::GetFileExtensions() 
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionImagingObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::DiffusionImagingObjectFactory::GetSaveFileExtensions() 
{ 
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionImagingObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::DiffusionImagingObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dti", "Tensor Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdti", "Tensor Images"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dti", "Tensor Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdti", "Tensor Images"));
}

void mitk::DiffusionImagingObjectFactory::RegisterIOFactories() 
{
}

void RegisterDiffusionImagingObjectFactory() 
{
  static bool oneDiffusionImagingObjectFactoryRegistered = false;
  if ( ! oneDiffusionImagingObjectFactoryRegistered ) {
    MITK_INFO << "Registering DiffusionImagingObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::DiffusionImagingObjectFactory::New());

    oneDiffusionImagingObjectFactoryRegistered = true;
  }
}
