#include "mitkFiberTrackingObjectFactory.h"


mitk::FiberTrackingObjectFactory::FiberTrackingObjectFactory(bool /*registerSelf*/)
  :CoreObjectFactoryBase()
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "FiberTrackingObjectFactory c'tor" << std::endl;
    RegisterIOFactories();

    mitk::FiberBundleXIOFactory::RegisterOneFactory(); //modernized

    mitk::FiberBundleXWriterFactory::RegisterOneFactory();//modernized

    m_FileWriters.push_back( mitk::FiberBundleXWriter::New().GetPointer() );//modernized

    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
    CreateFileExtensionsMap();

    alreadyDone = true;
  }

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

    classname = "FiberBundleXThreadMonitor";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::FiberBundleXThreadMonitorMapper3D::New();
      newMapper->SetDataNode(node);
    }
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

  classname = "FiberBundleXThreadMonitor";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::FiberBundleXThreadMonitorMapper3D::SetDefaultProperties(node);
  }
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
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.fib", "Fiber Bundle"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "Fiber Bundle"));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.fib", "Fiber Bundle"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "Fiber Bundle"));
}

void mitk::FiberTrackingObjectFactory::RegisterIOFactories()
{
}

void RegisterFiberTrackingObjectFactory()
{
  static bool oneFiberTrackingObjectFactoryRegistered = false;
  if ( ! oneFiberTrackingObjectFactoryRegistered ) {
    MITK_DEBUG << "Registering FiberTrackingObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::FiberTrackingObjectFactory::New());

    oneFiberTrackingObjectFactoryRegistered = true;
  }
}