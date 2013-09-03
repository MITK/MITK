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

#include <mitkCoreDataNodeReader.h>

#include "mitkSurfaceVtkWriterFactory.h"
#include "mitkPointSetWriterFactory.h"
#include "mitkImageWriterFactory.h"

class US_ABI_LOCAL Activator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    m_CoreDataNodeReader.reset(new mitk::CoreDataNodeReader);
    context->RegisterService<mitk::IDataNodeReader>(m_CoreDataNodeReader.get());

    m_ObjectFactories.push_back(PointSetIOFactory::New());
    m_ObjectFactories.push_back(STLFileIOFactory::New());
    m_ObjectFactories.push_back(VtkSurfaceIOFactory::New());
    m_ObjectFactories.push_back(VtkImageIOFactory::New());
    m_ObjectFactories.push_back(VtiFileIOFactory::New());
    m_ObjectFactories.push_back(ItkImageFileIOFactory::New());

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

  std::auto_ptr<mitk::CoreDataNodeReader> m_CoreDataNodeReader;

  std::vector<itk::ObjectFactoryBase::Pointer> m_ObjectFactories;
};

US_EXPORT_MODULE_ACTIVATOR(LegacyIO, Activator)
