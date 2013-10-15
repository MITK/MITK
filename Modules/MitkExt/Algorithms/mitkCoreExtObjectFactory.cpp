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

#include "mitkCoreExtObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkCoreObjectFactory.h"

#include "mitkParRecFileIOFactory.h"
#include "mitkObjFileIOFactory.h"
#include "mitkVtkUnstructuredGridIOFactory.h"
#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkVtkVolumeTimeSeriesIOFactory.h"

#include "mitkUnstructuredGridVtkWriterFactory.h"
#include "mitkUnstructuredGridVtkWriter.h"

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>

#include "mitkCone.h"
#include "mitkCuboid.h"
#include "mitkCylinder.h"
#include "mitkEllipsoid.h"
#include "mitkMeshMapper2D.h"
#include "mitkMeshVtkMapper3D.h"
#include "mitkUnstructuredGridMapper2D.h"
#include "mitkEnhancedPointSetVtkMapper3D.h"
#include "mitkSeedsImage.h"
#include "mitkUnstructuredGrid.h"
#include "mitkUnstructuredGridVtkMapper3D.h"
#include "mitkPolyDataGLMapper2D.h"
#include "mitkGPUVolumeMapper3D.h"

#include "mitkVolumeDataVtkMapper3D.h"


mitk::CoreExtObjectFactory::CoreExtObjectFactory()
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
    MITK_DEBUG << "CoreExtObjectFactory c'tor" << std::endl;

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

mitk::CoreExtObjectFactory::~CoreExtObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory( m_ParRecFileIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_ObjFileIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_VtkUnstructuredGridIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_StlVolumeTimeSeriesIOFactory );
  itk::ObjectFactoryBase::UnRegisterFactory( m_VtkVolumeTimeSeriesIOFactory );

  itk::ObjectFactoryBase::UnRegisterFactory( m_UnstructuredGridVtkWriterFactory );
}

mitk::Mapper::Pointer mitk::CoreExtObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
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

void mitk::CoreExtObjectFactory::SetDefaultProperties(mitk::DataNode* node)
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

const char* mitk::CoreExtObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::CoreExtObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::CoreExtObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::CoreExtObjectFactory::CreateFileExtensionsMap()
{
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.mitk", "MITK scene files")); //a better place to add this file ending might be the scene serialization class
                                                                                                 //at the moment this is not done because there is a plan to restructure the
                                                                                                 //ObjectFactories. When this is done we have to check where we want to add this file ending.
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtu", "VTK Unstructured Grid"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "VTK Unstructured Grid"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pvtu", "VTK Unstructured Grid"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pvtu", "VTK Parallel XML Unstructured Grid"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtu", "VTK XML Unstructured Grid"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "VTK Legacy Unstructured Grid"));
}

const char* mitk::CoreExtObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::CoreExtObjectFactory::RegisterIOFactories()
{
}

void RegisterCoreExtObjectFactory()
{
  static bool oneCoreExtObjectFactoryRegistered = false;
  if ( ! oneCoreExtObjectFactoryRegistered )
  {
    MITK_DEBUG << "Registering CoreExtObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::CoreExtObjectFactory::New());
    oneCoreExtObjectFactoryRegistered = true;
  }
}
