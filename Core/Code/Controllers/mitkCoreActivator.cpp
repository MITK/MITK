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

#include "mitkRenderingManager.h"
#include "mitkPlanePositionManager.h"
#include <mitkCoreDataNodeReader.h>
#include <mitkStandardFileLocations.h>
#include <mitkIShaderRepository.h>
#include <mitkPropertyAliases.h>
#include <mitkPropertyDescriptions.h>
#include <mitkPropertyExtensions.h>
#include <mitkPropertyFilters.h>
#include <mitkSurfaceCutterFactoryPerformanceSelector.h>
#include <mitkIOUtil.h>

#include <usGetModuleContext.h>
#include <usModuleInitialization.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModuleSettings.h>
#include <usModuleEvent.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usServiceTracker.h>

void HandleMicroServicesMessages(us::MsgType type, const char* msg)
{
  switch (type)
  {
  case us::DebugMsg:
    MITK_DEBUG << msg;
    break;
  case us::InfoMsg:
    MITK_INFO << msg;
    break;
  case us::WarningMsg:
    MITK_WARN << msg;
    break;
  case us::ErrorMsg:
    MITK_ERROR << msg;
    break;
  }
}

void AddMitkAutoLoadPaths(const std::string& programPath)
{
  us::ModuleSettings::AddAutoLoadPath(programPath);
#ifdef __APPLE__
  // Walk up three directories since that is where the .dylib files are located
  // for build trees.
  std::string additionalPath = programPath;
  bool addPath = true;
  for(int i = 0; i < 3; ++i)
  {
    std::size_t index = additionalPath.find_last_of('/');
    if (index != std::string::npos)
    {
      additionalPath = additionalPath.substr(0, index);
    }
    else
    {
      addPath = false;
      break;
    }
  }
  if (addPath)
  {
    us::ModuleSettings::AddAutoLoadPath(additionalPath);
  }
#endif
}

class ShaderRepositoryTracker : public us::ServiceTracker<mitk::IShaderRepository>
{

public:

  ShaderRepositoryTracker()
    : Superclass(us::GetModuleContext())
  {
  }

  virtual void Close()
  {
    us::GetModuleContext()->RemoveModuleListener(this, &ShaderRepositoryTracker::HandleModuleEvent);
    Superclass::Close();
  }

  virtual void Open()
  {
    us::GetModuleContext()->AddModuleListener(this, &ShaderRepositoryTracker::HandleModuleEvent);
    Superclass::Open();
  }

private:

  typedef us::ServiceTracker<mitk::IShaderRepository> Superclass;

  TrackedType AddingService(const ServiceReferenceType &reference)
  {
    mitk::IShaderRepository* shaderRepo = Superclass::AddingService(reference);
    if (shaderRepo)
    {
      // Add all existing shaders from modules to the new shader repository.
      // If the shader repository is registered in a modules activator, the
      // GetLoadedModules() function call below will also return the module
      // which is currently registering the repository. The HandleModuleEvent
      // method contains code to avoid double registrations due to a fired
      // ModuleEvent::LOADED event after the activators Load() method finished.
      std::vector<us::Module*> modules = us::ModuleRegistry::GetLoadedModules();
      for (std::vector<us::Module*>::const_iterator iter = modules.begin(),
           endIter = modules.end(); iter != endIter; ++iter)
      {
        this->AddModuleShaderToRepository(*iter, shaderRepo);
      }

      m_ShaderRepositories.push_back(shaderRepo);
    }
    return shaderRepo;
  }

  void RemovedService(const ServiceReferenceType& /*reference*/, TrackedType tracked)
  {
    m_ShaderRepositories.erase(std::remove(m_ShaderRepositories.begin(), m_ShaderRepositories.end(), tracked),
                               m_ShaderRepositories.end());
  }

  void HandleModuleEvent(const us::ModuleEvent moduleEvent)
  {
    if (moduleEvent.GetType() == us::ModuleEvent::LOADED)
    {
      std::vector<mitk::IShaderRepository*> shaderRepos;
      for (std::map<mitk::IShaderRepository*, std::map<long, std::vector<int> > >::const_iterator shaderMapIter = m_ModuleIdToShaderIds.begin(),
           shaderMapEndIter = m_ModuleIdToShaderIds.end(); shaderMapIter != shaderMapEndIter; ++shaderMapIter)
      {
        if (shaderMapIter->second.find(moduleEvent.GetModule()->GetModuleId()) == shaderMapIter->second.end())
        {
          shaderRepos.push_back(shaderMapIter->first);
        }
      }
      AddModuleShadersToRepositories(moduleEvent.GetModule(), shaderRepos);
    }
    else if (moduleEvent.GetType() == us::ModuleEvent::UNLOADED)
    {
      RemoveModuleShadersFromRepositories(moduleEvent.GetModule(), m_ShaderRepositories);
    }
  }

  void AddModuleShadersToRepositories(us::Module* module, const std::vector<mitk::IShaderRepository*>& shaderRepos)
  {
    // search and load shader files
    std::vector<us::ModuleResource> shaderResources = module->FindResources("Shaders", "*.xml", true);
    for (std::vector<us::ModuleResource>::iterator i = shaderResources.begin();
         i != shaderResources.end(); ++i)
    {
      if (*i)
      {
        us::ModuleResourceStream rs(*i);
        for (std::vector<mitk::IShaderRepository*>::const_iterator shaderRepoIter = shaderRepos.begin(),
             shaderRepoEndIter = shaderRepos.end(); shaderRepoIter != shaderRepoEndIter; ++shaderRepoIter)
        {
          int id = (*shaderRepoIter)->LoadShader(rs, i->GetBaseName());
          if (id >= 0)
          {
            m_ModuleIdToShaderIds[*shaderRepoIter][module->GetModuleId()].push_back(id);
          }
        }
        rs.seekg(0, std::ios_base::beg);
      }
    }
  }

  void AddModuleShaderToRepository(us::Module* module, mitk::IShaderRepository* shaderRepo)
  {
    std::vector<mitk::IShaderRepository*> shaderRepos;
    shaderRepos.push_back(shaderRepo);
    this->AddModuleShadersToRepositories(module, shaderRepos);
  }

  void RemoveModuleShadersFromRepositories(us::Module* module,
                                           const std::vector<mitk::IShaderRepository*>& shaderRepos)
  {
    for (std::vector<mitk::IShaderRepository*>::const_iterator shaderRepoIter = shaderRepos.begin(),
         shaderRepoEndIter = shaderRepos.end(); shaderRepoIter != shaderRepoEndIter; ++shaderRepoIter)
    {
      std::map<long, std::vector<int> >& moduleIdToShaderIds = m_ModuleIdToShaderIds[*shaderRepoIter];
      std::map<long, std::vector<int> >::iterator shaderIdsIter =
        moduleIdToShaderIds.find(module->GetModuleId());
      if (shaderIdsIter != moduleIdToShaderIds.end())
      {
        for (std::vector<int>::iterator idIter = shaderIdsIter->second.begin();
             idIter != shaderIdsIter->second.end(); ++idIter)
        {
          (*shaderRepoIter)->UnloadShader(*idIter);
        }
        moduleIdToShaderIds.erase(shaderIdsIter);
      }
    }
  }

private:

  // Maps to each shader repository a map containing module ids and related
  // shader registration ids
  std::map<mitk::IShaderRepository*, std::map<long, std::vector<int> > > m_ModuleIdToShaderIds;
  std::vector<mitk::IShaderRepository*> m_ShaderRepositories;
};

/*
 * This is the module activator for the "Mitk" module. It registers core services
 * like ...
 */
class MitkCoreActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    // Handle messages from CppMicroServices
    us::installMsgHandler(HandleMicroServicesMessages);

    // Add the current application directory to the auto-load paths.
    // This is useful for third-party executables.
    std::string programPath = mitk::IOUtil::GetProgramPath();
    if (programPath.empty())
    {
      MITK_WARN << "Could not get the program path.";
    }
    else
    {
      AddMitkAutoLoadPaths(programPath);
    }

    //m_RenderingManager = mitk::RenderingManager::New();
    //context->RegisterService<mitk::RenderingManager>(renderingManager.GetPointer());
    m_PlanePositionManager.reset(new mitk::PlanePositionManagerService);
    context->RegisterService<mitk::PlanePositionManagerService>(m_PlanePositionManager.get());

    m_CoreDataNodeReader.reset(new mitk::CoreDataNodeReader);
    context->RegisterService<mitk::IDataNodeReader>(m_CoreDataNodeReader.get());

    m_PropertyAliases.reset(new mitk::PropertyAliases);
    context->RegisterService<mitk::IPropertyAliases>(m_PropertyAliases.get());

    m_PropertyDescriptions.reset(new mitk::PropertyDescriptions);
    context->RegisterService<mitk::IPropertyDescriptions>(m_PropertyDescriptions.get());

    m_PropertyExtensions.reset(new mitk::PropertyExtensions);
    context->RegisterService<mitk::IPropertyExtensions>(m_PropertyExtensions.get());

    m_PropertyFilters.reset(new mitk::PropertyFilters);
    context->RegisterService<mitk::IPropertyFilters>(m_PropertyFilters.get());

    m_SurfaceCutterFactory.reset(new mitk::SurfaceCutterFactoryPerformanceSelector);
    context->RegisterService<mitk::ISurfaceCutterFactory>(m_SurfaceCutterFactory.get());

    m_ShaderRepositoryTracker.Open();

    /*
    There IS an option to exchange ALL vtkTexture instances against vtkNeverTranslucentTextureFactory.
    This code is left here as a reminder, just in case we might need to do that some time.

    vtkNeverTranslucentTextureFactory* textureFactory = vtkNeverTranslucentTextureFactory::New();
    vtkObjectFactory::RegisterFactory( textureFactory );
    textureFactory->Delete();
    */
  }

  void Unload(us::ModuleContext* )
  {
    // The mitk::ModuleContext* argument of the Unload() method
    // will always be 0 for the Mitk library. It makes no sense
    // to use it at this stage anyway, since all libraries which
    // know about the module system have already been unloaded.

    m_ShaderRepositoryTracker.Close();
  }

private:

  ShaderRepositoryTracker m_ShaderRepositoryTracker;

  //mitk::RenderingManager::Pointer m_RenderingManager;
  std::auto_ptr<mitk::PlanePositionManagerService> m_PlanePositionManager;
  std::auto_ptr<mitk::CoreDataNodeReader> m_CoreDataNodeReader;
  std::auto_ptr<mitk::PropertyAliases> m_PropertyAliases;
  std::auto_ptr<mitk::PropertyDescriptions> m_PropertyDescriptions;
  std::auto_ptr<mitk::PropertyExtensions> m_PropertyExtensions;
  std::auto_ptr<mitk::PropertyFilters> m_PropertyFilters;
  std::auto_ptr<mitk::SurfaceCutterFactoryPerformanceSelector> m_SurfaceCutterFactory;
};

US_EXPORT_MODULE_ACTIVATOR(MitkCore, MitkCoreActivator)

// Call CppMicroservices initialization code at the end of the file.
// This especially ensures that VTK object factories have already
// been registered (VTK initialization code is injected by implicitly
// include VTK header files at the top of this file).
US_INITIALIZE_MODULE("MitkCore", "MitkCore")
