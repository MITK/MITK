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

#include "mitkIOExtObjectFactory.h"

#include "mitkCoreObjectFactory.h"

#include "mitkParRecFileIOFactory.h"
#include "mitkObjFileIOFactory.h"
#include "mitkVtkUnstructuredGridIOFactory.h"
#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkVtkVolumeTimeSeriesIOFactory.h"

#include "mitkUnstructuredGridVtkWriterFactory.h"
#include "mitkUnstructuredGridVtkWriter.h"

#include "mitkMeshMapper2D.h"
#include "mitkMeshVtkMapper3D.h"
#include "mitkMesh.h"
#include "mitkGPUVolumeMapper3D.h"
#include "mitkUnstructuredGridMapper2D.h"
#include "mitkUnstructuredGridVtkMapper3D.h"

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>


mitk::IOExtObjectFactory::IOExtObjectFactory()
  : CoreObjectFactoryBase()
  , m_ParRecFileIOFactory(ParRecFileIOFactory::New().GetPointer())
  , m_ObjFileIOFactory(ObjFileIOFactory::New().GetPointer())
  , m_VtkUnstructuredGridIOFactory(VtkUnstructuredGridIOFactory::New().GetPointer())
  , m_StlVolumeTimeSeriesIOFactory(StlVolumeTimeSeriesIOFactory::New().GetPointer())
  , m_VtkVolumeTimeSeriesIOFactory(VtkVolumeTimeSeriesIOFactory::New().GetPointer())
  , m_UnstructuredGridVtkWriterFactory(UnstructuredGridVtkWriterFactory::New().GetPointer())
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "IOExtObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory( m_ParRecFileIOFactory );
    itk::ObjectFactoryBase::RegisterFactory( m_ObjFileIOFactory );
    itk::ObjectFactoryBase::RegisterFactory( m_VtkUnstructuredGridIOFactory );
    itk::ObjectFactoryBase::RegisterFactory( m_StlVolumeTimeSeriesIOFactory );
    itk::ObjectFactoryBase::RegisterFactory( m_VtkVolumeTimeSeriesIOFactory );

    itk::ObjectFactoryBase::RegisterFactory( m_UnstructuredGridVtkWriterFactory );

    m_FileWriters.push_back(mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::New().GetPointer());
    m_FileWriters.push_back(mitk::UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::New().GetPointer());
    m_FileWriters.push_back(mitk::UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::New().GetPointer());

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::IOExtObjectFactory::~IOExtObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory( m_ParRecFileIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_ObjFileIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_VtkUnstructuredGridIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_StlVolumeTimeSeriesIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_VtkVolumeTimeSeriesIOFactory );

  itk::ObjectFactoryBase::UnRegisterFactory( m_UnstructuredGridVtkWriterFactory );
}

mitk::Mapper::Pointer mitk::IOExtObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<UnstructuredGrid*>(data)!=NULL))
    {
      newMapper = mitk::UnstructuredGridMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    if((dynamic_cast<Image*>(data) != NULL))
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<Mesh*>(data)!=NULL))
    {
      newMapper = mitk::MeshVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if((dynamic_cast<UnstructuredGrid*>(data)!=NULL))
    {
      newMapper = mitk::UnstructuredGridVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::IOExtObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if(node==NULL)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull() && image->IsInitialized())
  {
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  if (dynamic_cast<mitk::UnstructuredGrid*>(node->GetData()))
  {
    mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(node);
  }

}

const char* mitk::IOExtObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::IOExtObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::IOExtObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::IOExtObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtu", "VTK Unstructured Grid"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "VTK Unstructured Grid"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pvtu", "VTK Unstructured Grid"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.obj", "Surfaces"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pvtu", "VTK Parallel XML Unstructured Grid"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtu", "VTK XML Unstructured Grid"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "VTK Legacy Unstructured Grid"));
}

const char* mitk::IOExtObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

struct RegisterIOExtObjectFactory
{
  RegisterIOExtObjectFactory()
    : m_Factory( mitk::IOExtObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterIOExtObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::IOExtObjectFactory::Pointer m_Factory;
};

static RegisterIOExtObjectFactory registerIOExtObjectFactory;
