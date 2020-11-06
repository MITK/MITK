/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIOExtObjectFactory.h"

#include "mitkCoreObjectFactory.h"

#include "mitkParRecFileIOFactory.h"
//#include "mitkObjFileIOFactory.h"
#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkVtkVolumeTimeSeriesIOFactory.h"

#include "mitkUnstructuredGridVtkWriter.h"
#include "mitkUnstructuredGridVtkWriterFactory.h"

#include "mitkVolumeMapperVtkSmart3D.h"
#include "mitkUnstructuredGridMapper2D.h"
#include "mitkUnstructuredGridVtkMapper3D.h"
#include "mitkVtkGLMapperWrapper.h"

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

mitk::IOExtObjectFactory::IOExtObjectFactory()
  : CoreObjectFactoryBase(),
    m_ParRecFileIOFactory(ParRecFileIOFactory::New().GetPointer())
    //, m_ObjFileIOFactory(ObjFileIOFactory::New().GetPointer())
    ,
    m_StlVolumeTimeSeriesIOFactory(StlVolumeTimeSeriesIOFactory::New().GetPointer()),
    m_VtkVolumeTimeSeriesIOFactory(VtkVolumeTimeSeriesIOFactory::New().GetPointer()),
    m_UnstructuredGridVtkWriterFactory(UnstructuredGridVtkWriterFactory::New().GetPointer())
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "IOExtObjectFactory c'tor" << std::endl;

    itk::ObjectFactoryBase::RegisterFactory(m_ParRecFileIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_StlVolumeTimeSeriesIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_VtkVolumeTimeSeriesIOFactory);

    itk::ObjectFactoryBase::RegisterFactory(m_UnstructuredGridVtkWriterFactory);

    m_FileWriters.push_back(mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::New().GetPointer());
    m_FileWriters.push_back(mitk::UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::New().GetPointer());
    m_FileWriters.push_back(mitk::UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::New().GetPointer());

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::IOExtObjectFactory::~IOExtObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_ParRecFileIOFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_StlVolumeTimeSeriesIOFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_VtkVolumeTimeSeriesIOFactory);

  itk::ObjectFactoryBase::UnRegisterFactory(m_UnstructuredGridVtkWriterFactory);
}

mitk::Mapper::Pointer mitk::IOExtObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;
  mitk::BaseData *data = node->GetData();

  if (id == mitk::BaseRenderer::Standard2D)
  {
    if ((dynamic_cast<UnstructuredGrid *>(data) != nullptr))
    {
      newMapper = mitk::VtkGLMapperWrapper::New(mitk::UnstructuredGridMapper2D::New().GetPointer());
      newMapper->SetDataNode(node);
    }
  }
  else if (id == mitk::BaseRenderer::Standard3D)
  {
    if ((dynamic_cast<Image *>(data) != nullptr) && std::string("Image").compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::VolumeMapperVtkSmart3D::New();
      newMapper->SetDataNode(node);
    }
    else if ((dynamic_cast<UnstructuredGrid *>(data) != nullptr))
    {
      newMapper = mitk::UnstructuredGridVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::IOExtObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());
  if (image.IsNotNull() && image->IsInitialized())
  {
    mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(node);
  }

  if (dynamic_cast<mitk::UnstructuredGrid *>(node->GetData()))
  {
    mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(node);
  }
}

std::string mitk::IOExtObjectFactory::GetFileExtensions()
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

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pvtu", "VTK Parallel XML Unstructured Grid"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtu", "VTK XML Unstructured Grid"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "VTK Legacy Unstructured Grid"));
}

std::string mitk::IOExtObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

struct RegisterIOExtObjectFactory
{
  RegisterIOExtObjectFactory() : m_Factory(mitk::IOExtObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterIOExtObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::IOExtObjectFactory::Pointer m_Factory;
};

static RegisterIOExtObjectFactory registerIOExtObjectFactory;
