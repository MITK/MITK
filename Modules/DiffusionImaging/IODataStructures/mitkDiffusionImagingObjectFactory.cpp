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

mitk::DiffusionImagingObjectFactory::DiffusionImagingObjectFactory(bool registerSelf) 
:CoreObjectFactoryBase()
{
  m_ExternalFileExtensions.append("Diffusion Weighted Images (*.dwi *.hdwi)");
  m_ExternalFileExtensions.append("Q-Ball Images (*.qbi *.hqbi)");
  m_ExternalFileExtensions.append("Tensor Images (*.dti *.hdti)");
  
  m_SaveFileExtensions.append("Diffusion Weighted Images (*.dwi *.hdwi)");
  m_SaveFileExtensions.append("Q-Ball Images (*.qbi *.hqbi)");
  m_SaveFileExtensions.append("Tensor Images (*.dti *.hdti)");

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
    
    alreadyDone = true;
  }

  if (registerSelf) 
  {
    this->RegisterOverride("mitkCoreObjectFactoryBase",
      "mitkDiffusionImagingObjectFactory",
      "mitk Mapper Creation",
      1,
      itk::CreateObjectFunction<mitk::DiffusionImagingObjectFactory>::New());
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
  return m_ExternalFileExtensions.c_str();
};

const char* mitk::DiffusionImagingObjectFactory::GetSaveFileExtensions() 
{ 
  return m_SaveFileExtensions.c_str();
};

void mitk::DiffusionImagingObjectFactory::RegisterIOFactories() 
{
}

void RegisterDiffusionImagingObjectFactory() 
{
  static bool oneDiffusionImagingObjectFactoryRegistered = false;
  if ( ! oneDiffusionImagingObjectFactoryRegistered ) {
    MITK_INFO << "Registering DiffusionImagingObjectFactory..." << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::DiffusionImagingObjectFactory::New());
    oneDiffusionImagingObjectFactoryRegistered = true;
  }
}
