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

private:

  //std::auto_ptr<mitk::CoreDataNodeReader> m_CoreDataNodeReader;

  us::ModuleContext* m_Context;

  std::vector<itk::ObjectFactoryBase::Pointer> m_ObjectFactories;

};

US_EXPORT_MODULE_ACTIVATOR(MitkLegacyIO, Activator)
