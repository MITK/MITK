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

#include "mitkFiberTrackingObjectFactory.h"


mitk::FiberTrackingObjectFactory::FiberTrackingObjectFactory()
  : CoreObjectFactoryBase()
{
}

mitk::FiberTrackingObjectFactory::~FiberTrackingObjectFactory()
{
}

mitk::Mapper::Pointer mitk::FiberTrackingObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=NULL;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("FiberBundleX");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::FiberBundleXMapper2D::New();
      newMapper->SetDataNode(node);
    }

  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    std::string classname("FiberBundleX");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::FiberBundleXMapper3D::New();
      newMapper->SetDataNode(node);
    }

//    classname = "FiberBundleXThreadMonitor";
//    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
//    {
//      newMapper = mitk::FiberBundleXThreadMonitorMapper3D::New();
//      newMapper->SetDataNode(node);
//    }
  }

  return newMapper;
}

void mitk::FiberTrackingObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  std::string classname("FiberBundleX");
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::FiberBundleXMapper3D::SetDefaultProperties(node);
    mitk::FiberBundleXMapper2D::SetDefaultProperties(node);
  }

//  classname = "FiberBundleXThreadMonitor";
//  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
//  {
//    mitk::FiberBundleXThreadMonitorMapper3D::SetDefaultProperties(node);
//  }
}

const char* mitk::FiberTrackingObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::FiberTrackingObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::FiberTrackingObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::FiberTrackingObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::FiberTrackingObjectFactory::CreateFileExtensionsMap()
{

}

void mitk::FiberTrackingObjectFactory::RegisterIOFactories()
{
}

struct RegisterFiberTrackingObjectFactory{
  RegisterFiberTrackingObjectFactory()
    : m_Factory( mitk::FiberTrackingObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterFiberTrackingObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::FiberTrackingObjectFactory::Pointer m_Factory;
};

static RegisterFiberTrackingObjectFactory registerFiberTrackingObjectFactory;
