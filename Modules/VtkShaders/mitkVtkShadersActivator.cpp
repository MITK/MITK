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

#include <mitkVtkShaderRepository.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModuleEvent.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <memory>

namespace mitk
{
  class VtkShadersActivator : public us::ModuleActivator
  {
  public:
    VtkShadersActivator()
    {
    }

    ~VtkShadersActivator()
    {
    }

    void Load(us::ModuleContext* context)
    {
      m_VtkShaderRepository.reset(new VtkShaderRepository);
      context->RegisterService<IShaderRepository>(m_VtkShaderRepository.get());
    }

    void Unload(us::ModuleContext*)
    {
      m_VtkShaderRepository.reset(NULL);
    }

  private:
    VtkShadersActivator(const VtkShadersActivator&);
    VtkShadersActivator& operator=(const VtkShadersActivator&);

    std::auto_ptr<VtkShaderRepository> m_VtkShaderRepository;
  };

}

US_EXPORT_MODULE_ACTIVATOR(mitk::VtkShadersActivator)
