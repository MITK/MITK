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
#include "mitkDataTreeNode.h"

#include "mitkNrrdDiffusionVolumesIOFactory.h"
#include "mitkNrrdDiffusionVolumesWriterFactory.h"
#include "mitkNrrdDiffusionVolumesWriter.h"
#include "mitkDiffusionVolumes.h"

#include "mitkNrrdQBallImageIOFactory.h"
#include "mitkNrrdQBallImageWriterFactory.h"
#include "mitkNrrdQBallImageWriter.h"
#include "mitkQBallImage.h"

typedef short DiffusionPixelType;
typedef mitk::DiffusionVolumes<DiffusionPixelType> DiffusionVolumesShort;

mitk::DiffusionImagingObjectFactory::DiffusionImagingObjectFactory(bool registerSelf) 
:CoreObjectFactory()
{
  m_ExternalFileExtensions = Superclass::GetFileExtensions();
  m_ExternalFileExtensions.append(";;Diffusion Weighted Images (*.dwi *.hdwi)");
  m_ExternalFileExtensions.append(";;Q-Ball Images (*.qball *.hqball)");
  
  m_SaveFileExtensions = Superclass::GetSaveFileExtensions();
  m_SaveFileExtensions.append(";;Diffusion Weighted Images (*.dwi *.hdwi)");
  m_SaveFileExtensions.append(";;Q-Ball Images (*.qball *.hqball)");

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    LOG_INFO << "DiffusionImagingObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    itk::ObjectFactoryBase::RegisterFactory( NrrdDiffusionVolumesIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( NrrdQBallImageIOFactory::New() );

    mitk::NrrdDiffusionVolumesWriterFactory::RegisterOneFactory();
    mitk::NrrdQBallImageWriterFactory::RegisterOneFactory();
     
    m_FileWriters.push_back( NrrdDiffusionVolumesWriter<DiffusionPixelType>::New().GetPointer() );
    m_FileWriters.push_back( NrrdQBallImageWriter::New().GetPointer() );

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

#define CREATE_CPP( TYPE, NAME ) else if ( className == NAME ) {pointer = new TYPE(); pointer->Register();}
#define CREATE_ITK( TYPE, NAME ) else if ( className == NAME ) pointer = TYPE::New();

itk::Object::Pointer mitk::DiffusionImagingObjectFactory::CreateCoreObject( const std::string& className )
{
  itk::Object::Pointer pointer;

  if ( className == "" )
    return NULL;
    CREATE_ITK( DiffusionVolumesShort, "DiffusionVolumes" )
    CREATE_ITK( QBallImage, "QBallImage" )
//  CREATE_ITK( UnstructuredGridVtkMapper3D, "UnstructuredGridVtkMapper3D" )
  else
    pointer = Superclass::CreateCoreObject( className );

  return pointer;
}


mitk::Mapper::Pointer mitk::DiffusionImagingObjectFactory::CreateMapper(mitk::DataTreeNode* node, MapperSlotId id) 
{
  mitk::Mapper::Pointer newMapper=NULL;
  //mitk::BaseData *data = node->GetData();

  //if ( id == mitk::BaseRenderer::Standard2D )
  //{
  //  if((dynamic_cast<Mesh*>(data)!=NULL))
  //  {
  //    newMapper = mitk::MeshMapper2D::New();
  //    newMapper->SetDataTreeNode(node);
  //  }
  //  else if((dynamic_cast<PointData*>(data)!=NULL))
  //  {
  //    newMapper = mitk::PolyDataGLMapper2D::New();
  //    newMapper->SetDataTreeNode(node);
  //  }
  //}
  //else if ( id == mitk::BaseRenderer::Standard3D )
  //{
  //  if((dynamic_cast<Mesh*>(data)!=NULL))
  //  {
  //    newMapper = mitk::MeshVtkMapper3D::New();
  //    newMapper->SetDataTreeNode(node);
  //  }
  //  else if((dynamic_cast<PointData*>(data)!=NULL))
  //  {
  //    newMapper = mitk::PointDataVtkMapper3D::New();
  //    newMapper->SetDataTreeNode(node);
  //  }
  //}

  if (newMapper.IsNull()) {
    newMapper = Superclass::CreateMapper(node,id);
  }
  return newMapper;
}

void mitk::DiffusionImagingObjectFactory::SetDefaultProperties(mitk::DataTreeNode* node)
{
  Superclass::SetDefaultProperties(node);
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
  bool oneDiffusionImagingObjectFactoryRegistered = false;
  if ( ! oneDiffusionImagingObjectFactoryRegistered ) {
    LOG_INFO << "Registering DiffusionImagingObjectFactory..." << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::DiffusionImagingObjectFactory::New());
    oneDiffusionImagingObjectFactoryRegistered = true;
  }
}
