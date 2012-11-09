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
#include "mitkStandardFileLocations.h"

#include <mitkModuleActivator.h>
#include <mitkModuleSettings.h>

void HandleMicroServicesMessages(mitk::MsgType type, const char* msg)
{
  switch (type)
  {
  case mitk::DebugMsg:
    MITK_DEBUG << msg;
    break;
  case mitk::InfoMsg:
    MITK_INFO << msg;
    break;
  case mitk::WarningMsg:
    MITK_WARN << msg;
    break;
  case mitk::ErrorMsg:
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
  char proc[512];
  ssize_t ch = readlink(ss.str().c_str(), proc, 512);
  if (ch == -1) return std::string();
  std::size_t index = std::string(proc).find_last_of('/');
  return std::string(proc, index);
}
#endif

void AddMitkAutoLoadPaths(const std::string& programPath)
{
  mitk::ModuleSettings::AddAutoLoadPath(programPath);
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
    mitk::ModuleSettings::AddAutoLoadPath(additionalPath);
  }
#endif
}

/*
 * This is the module activator for the "Mitk" module. It registers core services
 * like ...
 */
class MitkCoreActivator : public mitk::ModuleActivator
{
public:

  void Load(mitk::ModuleContext* context)
  {
    // Handle messages from CppMicroServices
    mitk::installMsgHandler(HandleMicroServicesMessages);

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
    m_PlanePositionManager = mitk::PlanePositionManagerService::New();
    context->RegisterService<mitk::PlanePositionManagerService>(m_PlanePositionManager);

    m_CoreDataNodeReader = mitk::CoreDataNodeReader::New();
    context->RegisterService<mitk::IDataNodeReader>(m_CoreDataNodeReader);
  }

  void Unload(mitk::ModuleContext* )
  {
    // The mitk::ModuleContext* argument of the Unload() method
    // will always be 0 for the Mitk library. It makes no sense
    // to use it at this stage anyway, since all libraries which
    // know about the module system have already been unloaded.
  }

private:

  //mitk::RenderingManager::Pointer m_RenderingManager;
  mitk::PlanePositionManagerService::Pointer m_PlanePositionManager;
  mitk::CoreDataNodeReader::Pointer m_CoreDataNodeReader;
};

US_EXPORT_MODULE_ACTIVATOR(Mitk, MitkCoreActivator)

