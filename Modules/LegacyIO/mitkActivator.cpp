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

#include <usModuleActivator.h>

//#include <mitkCoreDataNodeReader.h>

#include "mitkSurfaceVtkWriterFactory.h"
#include "mitkPointSetIOFactory.h"
#include "mitkPointSetWriterFactory.h"
#include "mitkSTLFileIOFactory.h"
#include "mitkVtkSurfaceIOFactory.h"
#include "mitkVtkImageIOFactory.h"
#include "mitkVtiFileIOFactory.h"
#include "mitkItkImageFileIOFactory.h"
#include "mitkImageWriterFactory.h"
#include "mitkCoreObjectFactoryBase.h"
#include "mitkCoreObjectFactory.h"
#include "mitkSurfaceVtkWriter.h"

#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace mitk {

class LegacyIOObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(LegacyIOObjectFactory,CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* /*node*/, MapperSlotId /*slotId*/)
    {
      return Mapper::Pointer();
    }

    virtual void SetDefaultProperties(mitk::DataNode* /*node*/)
    {
    }

    virtual const char* GetFileExtensions()
    {
      return "";
    }

    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap()
    {
      return m_FileExtensionsMap;
    }

    virtual const char* GetSaveFileExtensions()
    {
      std::string fileExtension;
      this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
      return fileExtension.c_str();
    }

    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap()
    {
      return m_SaveFileExtensionsMap;
    }

    ~LegacyIOObjectFactory()
    {
      for(std::vector<itk::ObjectFactoryBase::Pointer>::const_iterator iter = m_ObjectFactories.begin(),
          end = m_ObjectFactories.end(); iter != end; ++iter)
      {
        itk::ObjectFactoryBase::UnRegisterFactory(*iter);
      }
    }

protected:

    LegacyIOObjectFactory()
      : CoreObjectFactoryBase()
    {
      m_ObjectFactories.push_back(mitk::PointSetIOFactory::New().GetPointer());
      m_ObjectFactories.push_back(mitk::STLFileIOFactory::New().GetPointer());
      m_ObjectFactories.push_back(mitk::VtkSurfaceIOFactory::New().GetPointer());
      m_ObjectFactories.push_back(mitk::VtkImageIOFactory::New().GetPointer());
      m_ObjectFactories.push_back(mitk::VtiFileIOFactory::New().GetPointer());
      m_ObjectFactories.push_back(mitk::ItkImageFileIOFactory::New().GetPointer());

      for(std::vector<itk::ObjectFactoryBase::Pointer>::const_iterator iter = m_ObjectFactories.begin(),
          end = m_ObjectFactories.end(); iter != end; ++iter)
      {
        itk::ObjectFactoryBase::RegisterFactory(*iter);
      }

      m_FileWriters.push_back(mitk::SurfaceVtkWriter<vtkSTLWriter>::New().GetPointer());
      m_FileWriters.push_back(mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New().GetPointer());
      m_FileWriters.push_back(mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::New().GetPointer());
      //m_FileWriters.push_back(mitk::PointSetWriterFactory::New().GetPointer());
      //m_FileWriters.push_back(mitk::ImageWriterFactory::New().GetPointer());

      this->CreateFileExtensionsMap();
    }

    void CreateFileExtensionsMap()
    {
      m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.stl", "STL Surface File"));
      m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.obj", "Surface File"));
      m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtk", "VTK Surface File"));
      m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.vtp", "VTK Polydata File"));
    }

    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

private:

    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;

    std::vector<itk::ObjectFactoryBase::Pointer> m_ObjectFactories;
};

}

class US_ABI_LOCAL Activator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    m_Factory = mitk::LegacyIOObjectFactory::New();
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  void Unload(us::ModuleContext* )
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory);
  }

private:

  //std::auto_ptr<mitk::CoreDataNodeReader> m_CoreDataNodeReader;

  us::ModuleContext* m_Context;

  mitk::LegacyIOObjectFactory::Pointer m_Factory;
};

US_EXPORT_MODULE_ACTIVATOR(MitkLegacyIO, Activator)
