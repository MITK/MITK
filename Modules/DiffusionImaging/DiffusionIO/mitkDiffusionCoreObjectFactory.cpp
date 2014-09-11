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

#include "mitkDiffusionCoreObjectFactory.h"

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
#include "mitkGPUVolumeMapper3D.h"
#include "mitkVolumeDataVtkMapper3D.h"


typedef short DiffusionPixelType;

typedef mitk::DiffusionImage<DiffusionPixelType> DiffusionImageShort;
typedef std::multimap<std::string, std::string> MultimapType;

mitk::DiffusionCoreObjectFactory::DiffusionCoreObjectFactory()
  : CoreObjectFactoryBase()
  , m_NrrdDiffusionImageIOFactory(mitk::NrrdDiffusionImageIOFactory::New().GetPointer())
  , m_NrrdQBallImageIOFactory(mitk::NrrdQBallImageIOFactory::New().GetPointer())
  , m_NrrdTensorImageIOFactory(mitk::NrrdTensorImageIOFactory::New().GetPointer())
  , m_NrrdDiffusionImageWriterFactory(mitk::NrrdDiffusionImageWriterFactory::New().GetPointer())
  , m_NrrdQBallImageWriterFactory(mitk::NrrdQBallImageWriterFactory::New().GetPointer())
  , m_NrrdTensorImageWriterFactory(mitk::NrrdTensorImageWriterFactory::New().GetPointer())
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "DiffusionCoreObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory(m_NrrdDiffusionImageIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_NrrdQBallImageIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_NrrdTensorImageIOFactory);

    itk::ObjectFactoryBase::RegisterFactory(m_NrrdDiffusionImageWriterFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_NrrdQBallImageWriterFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_NrrdTensorImageWriterFactory);

    m_FileWriters.push_back( NrrdDiffusionImageWriter<DiffusionPixelType>::New().GetPointer() );
    m_FileWriters.push_back( NrrdQBallImageWriter::New().GetPointer() );
    m_FileWriters.push_back( NrrdTensorImageWriter::New().GetPointer() );


    CreateFileExtensionsMap();

    alreadyDone = true;
  }

}

mitk::DiffusionCoreObjectFactory::~DiffusionCoreObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdDiffusionImageIOFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdQBallImageIOFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdTensorImageIOFactory);

  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdDiffusionImageWriterFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdQBallImageWriterFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_NrrdTensorImageWriterFactory);

}

mitk::Mapper::Pointer mitk::DiffusionCoreObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("QBallImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, ((CompositeMapper*)newMapper.GetPointer())->GetImageMapper());
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, ((CompositeMapper*)newMapper.GetPointer())->GetImageMapper());
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
    std::string classname("QBallImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    classname = "DiffusionImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }

  return newMapper;
}

void mitk::DiffusionCoreObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  std::string classname = "QBallImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "TensorImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "DiffusionImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::DiffusionImageMapper<short>::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::DiffusionCoreObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionCoreObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::DiffusionCoreObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionCoreObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::DiffusionCoreObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.nii", "Diffusion Weighted Images for FSL"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.fsl", "Diffusion Weighted Images for FSL"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.fslgz", "Diffusion Weighted Images for FSL"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.dti", "Tensor Images"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdti", "Tensor Images"));
  //  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pf", "Planar Figure File"));


  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdwi", "Diffusion Weighted Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.nii", "Diffusion Weighted Images for FSL"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.fsl", "Diffusion Weighted Images for FSL"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.fslgz", "Diffusion Weighted Images for FSL"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.qbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hqbi", "Q-Ball Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.dti", "Tensor Images"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.hdti", "Tensor Images"));
  // m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pf", "Planar Figure File"));

}

struct RegisterDiffusionCoreObjectFactory{
  RegisterDiffusionCoreObjectFactory()
    : m_Factory( mitk::DiffusionCoreObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterDiffusionCoreObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::DiffusionCoreObjectFactory::Pointer m_Factory;
};

static RegisterDiffusionCoreObjectFactory registerDiffusionCoreObjectFactory;

