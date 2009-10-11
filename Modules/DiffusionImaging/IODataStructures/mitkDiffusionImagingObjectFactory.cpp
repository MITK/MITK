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
//#include "mitkQBallImage.h"

#include "mitkNrrdTensorImageIOFactory.h"
#include "mitkNrrdTensorImageWriterFactory.h"
#include "mitkNrrdTensorImageWriter.h"

#include "mitkOdfVtkMapper2D.h"
#include "mitkImageMapper2D.h"

typedef short DiffusionPixelType;
typedef mitk::DiffusionVolumes<DiffusionPixelType> DiffusionVolumesShort;

mitk::DiffusionImagingObjectFactory::DiffusionImagingObjectFactory(bool registerSelf) 
:CoreObjectFactoryBase()
{
  m_ExternalFileExtensions.append("Diffusion Weighted Images (*.dwi *.hdwi)");
  m_ExternalFileExtensions.append(";;Q-Ball Images (*.qball *.hqball)");
  m_ExternalFileExtensions.append(";;Tensor Images (*.dti *.hdti)");
  
  m_SaveFileExtensions.append("Diffusion Weighted Images (*.dwi *.hdwi)");
  m_SaveFileExtensions.append(";;Q-Ball Images (*.qball *.hqball)");
  m_SaveFileExtensions.append(";;Q-Ball Images (*.dti *.hdti)");

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    LOG_INFO << "DiffusionImagingObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    itk::ObjectFactoryBase::RegisterFactory( NrrdDiffusionVolumesIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( NrrdQBallImageIOFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( NrrdTensorImageIOFactory::New() );

    mitk::NrrdDiffusionVolumesWriterFactory::RegisterOneFactory();
    mitk::NrrdQBallImageWriterFactory::RegisterOneFactory();
    mitk::NrrdTensorImageWriterFactory::RegisterOneFactory();
    
    //m_FileWriters.push_back( NrrdDiffusionVolumesWriter<DiffusionPixelType>::New().GetPointer() );
    //m_FileWriters.push_back( NrrdQBallImageWriter::New().GetPointer() );
    //m_FileWriters.push_back( NrrdTensorImageWriter::New().GetPointer() );

    //mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
    
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
  /*
  if ( className == "" )
    return NULL;
    CREATE_ITK( DiffusionVolumesShort, "DiffusionVolumes" )
    CREATE_ITK( QBallImage, "QBallImage" )
//  CREATE_ITK( UnstructuredGridVtkMapper3D, "UnstructuredGridVtkMapper3D" )
  else
    pointer = Superclass::CreateCoreObject( className );
*/
  return pointer;
}


mitk::Mapper::Pointer mitk::DiffusionImagingObjectFactory::CreateMapper(mitk::DataTreeNode* node, MapperSlotId id) 
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("QBallImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::New();
      newMapper->SetDataTreeNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    // do nothing
  }

  return newMapper;
}

void mitk::DiffusionImagingObjectFactory::SetDefaultProperties(mitk::DataTreeNode* node)
{
  std::string classname("DiffusionImages");
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    // do nothing
  }

  classname = "QBallImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::SetDefaultProperties(node);
    //mitk::VolumeDataVtkMapper3D::SetDefaultProperties(node);
  }

  classname = "TensorImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    // do nothing
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
  bool oneDiffusionImagingObjectFactoryRegistered = false;
  if ( ! oneDiffusionImagingObjectFactoryRegistered ) {
    LOG_INFO << "Registering DiffusionImagingObjectFactory..." << std::endl;
    itk::ObjectFactoryBase::RegisterFactory(mitk::DiffusionImagingObjectFactory::New());
    oneDiffusionImagingObjectFactoryRegistered = true;
  }
}
