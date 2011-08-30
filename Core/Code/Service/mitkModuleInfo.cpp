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


#include "mitkModuleInfo.h"

namespace mitk {

ModuleInfo::ModuleInfo(const std::string& name, const std::string& libName, const std::string& moduleDeps,
                       const std::string& packageDeps, const std::string& version, bool qtModule)
  : name(name), libName(libName), moduleDeps(moduleDeps), packageDeps(packageDeps),
    version(version), qtModule(qtModule), id(0),
    activatorHook(0)
{}

}
