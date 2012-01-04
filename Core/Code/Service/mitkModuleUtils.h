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


#ifndef MITKMODULEUTILS_H
#define MITKMODULEUTILS_H

#include <MitkExports.h>

#include <string>

namespace mitk {

/**
 * This class is not intended to be used directly. It is exported to support
 * the MITK module system.
 */
struct MITK_CORE_EXPORT ModuleUtils
{
  static std::string GetLibraryPath(const std::string& libName, void* symbol);

  static void* GetSymbol(const std::string& libName, const char* symbol);
};

}

#endif // MITKMODULEUTILS_H
