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
      context->AddModuleListener(this, &VtkShadersActivator::HandleModuleEvent);
    }

    void Unload(us::ModuleContext*)
    {
      m_VtkShaderRepository.reset(NULL);
    }

  private:
    VtkShadersActivator(const VtkShadersActivator&);
    VtkShadersActivator& operator=(const VtkShadersActivator&);

    void HandleModuleEvent(const us::ModuleEvent moduleEvent);
    std::map<long, std::vector<int> > moduleIdToShaderIds;

    std::auto_ptr<VtkShaderRepository> m_VtkShaderRepository;
  };

  void VtkShadersActivator::HandleModuleEvent(const us::ModuleEvent moduleEvent)
  {
    if (moduleEvent.GetType() == us::ModuleEvent::LOADED)
    {
      // search and load shader files
      std::vector<us::ModuleResource> shaderResoruces =
          moduleEvent.GetModule()->FindResources("Shaders", "*.xml", true);
      for (std::vector<us::ModuleResource>::iterator i = shaderResoruces.begin();
           i != shaderResoruces.end(); ++i)
      {
        if (*i)
        {
          us::ModuleResourceStream rs(*i);
          int id = m_VtkShaderRepository->LoadShader(rs, i->GetBaseName());
          if (id >= 0)
          {
            moduleIdToShaderIds[moduleEvent.GetModule()->GetModuleId()].push_back(id);
          }
        }
      }
    }
    else if (moduleEvent.GetType() == us::ModuleEvent::UNLOADED)
    {
      std::map<long, std::vector<int> >::iterator shaderIdsIter =
          moduleIdToShaderIds.find(moduleEvent.GetModule()->GetModuleId());
      if (shaderIdsIter != moduleIdToShaderIds.end())
      {
        for (std::vector<int>::iterator idIter = shaderIdsIter->second.begin();
             idIter != shaderIdsIter->second.end(); ++idIter)
        {
          m_VtkShaderRepository->UnloadShader(*idIter);
        }
        moduleIdToShaderIds.erase(shaderIdsIter);
      }
    }


  }
}

US_EXPORT_MODULE_ACTIVATOR(VtkShaders, mitk::VtkShadersActivator);
