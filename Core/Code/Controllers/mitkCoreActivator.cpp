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
#include "mitkCoreDataNodeReader.h"
#include "mitkShaderRepository.h"
#include "mitkStandardFileLocations.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModuleSettings.h>
#include <usModuleEvent.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

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

#if defined(_WIN32) || defined(_WIN64)
std::string GetProgramPath()
{
  char path[512];
  std::size_t index = std::string(path, GetModuleFileName(NULL, path, 512)).find_last_of('\\');
  return std::string(path, index);
}
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
std::string GetProgramPath()
{
  char path[512];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0)
  {
    std::size_t index = std::string(path).find_last_of('/');
    std::string strPath = std::string(path, index);
    const char* execPath = strPath.c_str();
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(execPath,false);
    return strPath;
  }
  return std::string();
}
#else
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
std::string GetProgramPath()
{
  std::stringstream ss;
  ss << "/proc/" << getpid() << "/exe";
  char proc[512] = {0};
  ssize_t ch = readlink(ss.str().c_str(), proc, 512);
  if (ch == -1) return std::string();
  std::size_t index = std::string(proc).find_last_of('/');
  return std::string(proc, index);
}
#endif

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
    std::string programPath = GetProgramPath();
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

    m_ShaderRepository.reset(new mitk::ShaderRepository);
    context->RegisterService<mitk::IShaderRepository>(m_ShaderRepository.get());

    context->AddModuleListener(this, &MitkCoreActivator::HandleModuleEvent);

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
  }

private:

  void HandleModuleEvent(const us::ModuleEvent moduleEvent);

  std::map<long, std::vector<int> > moduleIdToShaderIds;

  //mitk::RenderingManager::Pointer m_RenderingManager;
  std::auto_ptr<mitk::PlanePositionManagerService> m_PlanePositionManager;
  std::auto_ptr<mitk::CoreDataNodeReader> m_CoreDataNodeReader;
  std::auto_ptr<mitk::ShaderRepository> m_ShaderRepository;
};

void MitkCoreActivator::HandleModuleEvent(const us::ModuleEvent moduleEvent)
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

US_EXPORT_MODULE_ACTIVATOR(Mitk, MitkCoreActivator)

