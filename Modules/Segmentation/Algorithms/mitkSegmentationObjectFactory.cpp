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

#include "mitkContour.h"
#include "mitkContourMapper2D.h"
#include "mitkContourSetMapper2D.h"
#include "mitkContourSetVtkMapper3D.h"
#include "mitkContourVtkMapper3D.h"


#include "mitkContourModel.h"
#include "mitkContourModelIOFactory.h"
#include "mitkContourModelWriterFactory.h"
#include "mitkContourModelWriter.h"
#include "mitkContourModelMapper2D.h"
#include "mitkContourModelGLMapper2D.h"
#include "mitkContourModelMapper3D.h"


mitk::SegmentationObjectFactory::SegmentationObjectFactory()
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "SegmentationObjectFactory c'tor" << std::endl;

    RegisterIOFactories();

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::SegmentationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<Contour*>(data)!=NULL))
    {
      newMapper = mitk::ContourMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<ContourSet*>(data)!=NULL))
    {
      newMapper = mitk::ContourSetMapper2D::New();
      newMapper->SetDataNode(node);
    }

    std::string classname("ContourModel");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ContourModelGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if((dynamic_cast<Contour*>(data)!=NULL))
    {
      newMapper = mitk::ContourVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<ContourSet*>(data)!=NULL))
    {
      newMapper = mitk::ContourSetVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }

    std::string classname("ContourModel");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::ContourModelMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::SegmentationObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{

  if(node==NULL)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  std::string classname("ContourModel");
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::ContourModelGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelMapper3D::SetDefaultProperties(node);
  }

//  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
//  if(image.IsNotNull() && image->IsInitialized())
//  {
//    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
//  }
//
//  if (dynamic_cast<mitk::UnstructuredGrid*>(node->GetData()))
//  {
//    mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(node);
//  }

}

const char* mitk::SegmentationObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

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
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.cnt", "Contour Files"));

  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.cnt", "Contour File"));
}

const char* mitk::SegmentationObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::SegmentationObjectFactory::RegisterIOFactories()
{
  //register io classes of mitkContourModel
  mitk::ContourModelIOFactory::RegisterOneFactory();

  mitk::ContourModelWriterFactory::RegisterOneFactory();

  this->m_FileWriters.push_back(mitk::ContourModelWriter::New().GetPointer());

  CreateFileExtensionsMap();
}

void RegisterSegmentationObjectFactory()
{
  static bool oneSegmentationObjectFactoryRegistered = false;
  if ( ! oneSegmentationObjectFactoryRegistered )
  {
    MITK_DEBUG << "Registering SegmentationObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::SegmentationObjectFactory::New());
    oneSegmentationObjectFactoryRegistered = true;
  }
}
