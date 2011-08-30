/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKMODULEINFO_H
#define MITKMODULEINFO_H

#include <MitkExports.h>

#include <string>

namespace mitk {

struct ModuleActivator;

/**
 * This class is not intended to be used directly. It is exported to support
 * the MITK module system.
 */
struct MITK_CORE_EXPORT ModuleInfo
{
  ModuleInfo(const std::string& name, const std::string& libName, const std::string& moduleDeps,
             const std::string& packageDeps, const std::string& version, bool qtModule);

  typedef ModuleActivator*(*ModuleActivatorHook)(void);

  std::string name;
  std::string libName;
  std::string moduleDeps;
  std::string packageDeps;
  std::string version;

  std::string location;

  bool qtModule;

  long id;

  ModuleActivatorHook activatorHook;
};

}

#endif // MITKMODULEINFO_H
