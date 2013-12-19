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
#include "mitkSimpleMimeType.h"

#include <usServiceProperties.h>
#include <usModuleContext.h>

class US_ABI_LOCAL Activator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    //m_CoreDataNodeReader.reset(new mitk::CoreDataNodeReader);
    //context->RegisterService<mitk::IDataNodeReader>(m_CoreDataNodeReader.get());
    this->m_Context = context;

    // Register some mime-types

    // 3D Images
    std::vector<std::string> mimeTypeExtensions;
    mimeTypeExtensions.push_back("dc3");
    mimeTypeExtensions.push_back("dcm");
    RegisterMimeType("application/dicom", "Images", "Dicom Images", mimeTypeExtensions);
    RegisterMimeType("application/vnd.mitk.pic", "Images", "DKFZ PIC Format", "pic");
    RegisterMimeType("application/vnd.mitk.pic+gz", "Images", "DKFZ Compressed PIC Format", "pic.gz");

    // REMOVE: Test multiple mime types for same extension
    RegisterMimeType("application/vnd.fancy", "Images", "Fancy Compressed PIC Format", "pic.gz");

    // 2D Images
    RegisterMimeType("image/bmp", "2D Images", "Bitmap Image", "bmp");

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

    mitk::SurfaceVtkWriterFactory::RegisterOneFactory();
    mitk::PointSetWriterFactory::RegisterOneFactory();
    mitk::ImageWriterFactory::RegisterOneFactory();
  }

  void Unload(us::ModuleContext* )
  {
    for(std::vector<itk::ObjectFactoryBase::Pointer>::const_iterator iter = m_ObjectFactories.begin(),
        end = m_ObjectFactories.end(); iter != end; ++iter)
    {
      itk::ObjectFactoryBase::UnRegisterFactory(*iter);
    }

    // FIXME: There is no "UnRegisterOneFactory" method
  }

  void RegisterMimeType(const std::string& id, const std::string& category, const std::string& description,
                        const std::string& extension)
  {
    std::vector<std::string> extensions;
    extensions.push_back(extension);
    this->RegisterMimeType(id, category, description, extensions);
  }

  void RegisterMimeType(const std::string& id, const std::string& category, const std::string& description,
                        const std::vector<std::string>& extensions)
  {
    us::ServiceProperties mimeTypeProps;
    mimeTypeProps[mitk::IMimeType::PROP_ID()] = id;
    mimeTypeProps[mitk::IMimeType::PROP_CATEGORY()] = category;
    mimeTypeProps[mitk::IMimeType::PROP_DESCRIPTION()] = description;
    mimeTypeProps[mitk::IMimeType::PROP_EXTENSIONS()] = extensions;
    mimeTypeProps[us::ServiceConstants::SERVICE_RANKING()] = -100;
    m_Context->RegisterService<mitk::IMimeType>(&m_MimeType, mimeTypeProps);
  }

private:

  //std::auto_ptr<mitk::CoreDataNodeReader> m_CoreDataNodeReader;

  us::ModuleContext* m_Context;

  std::vector<itk::ObjectFactoryBase::Pointer> m_ObjectFactories;

  mitk::SimpleMimeType m_MimeType;
};

US_EXPORT_MODULE_ACTIVATOR(LegacyIO, Activator)
