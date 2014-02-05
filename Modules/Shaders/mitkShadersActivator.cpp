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

#include <mitkShaderRepository.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModuleEvent.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <memory>

namespace mitk
{
  class ShadersActivator : public us::ModuleActivator
  {
  public:
    ShadersActivator()
    {
    }

    ~ShadersActivator()
    {
    }

    void Load(us::ModuleContext* context)
    {
      m_ShaderRepository.reset(new ShaderRepository);
      context->RegisterService<IShaderRepository>(m_ShaderRepository.get());
      context->AddModuleListener(this, &ShadersActivator::HandleModuleEvent);
    }

    void Unload(us::ModuleContext*)
    {
      m_ShaderRepository.reset(NULL);
    }

  private:
    ShadersActivator(const ShadersActivator&);
    ShadersActivator& operator=(const ShadersActivator&);

    void HandleModuleEvent(const us::ModuleEvent moduleEvent);
    std::map<long, std::vector<int> > moduleIdToShaderIds;

    std::auto_ptr<ShaderRepository> m_ShaderRepository;
  };

  void ShadersActivator::HandleModuleEvent(const us::ModuleEvent moduleEvent)
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
          int id = m_ShaderRepository->LoadShader(rs, i->GetBaseName());
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
          m_ShaderRepository->UnloadShader(*idIter);
        }
        moduleIdToShaderIds.erase(shaderIdsIter);
      }
    }


  }
}

US_EXPORT_MODULE_ACTIVATOR(Shaders, mitk::ShadersActivator);
