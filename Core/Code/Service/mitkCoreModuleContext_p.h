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


#ifndef MITKCOREMODULECONTEXT_H
#define MITKCOREMODULECONTEXT_H

#include "mitkServiceListeners_p.h"
#include "mitkServiceRegistry_p.h"

namespace mitk {

class ModuleRegistry;
class Module;

/**
 * This class is not part of the public API.
 */
class CoreModuleContext
{
public:

  /**
   * All listeners in this framework.
   */
  ServiceListeners listeners;

  /**
   * All registered services in this framework.
   */
  ServiceRegistry services;

  /**
   * Contruct a core context
   *
   */
  CoreModuleContext();

  ~CoreModuleContext();

};

}

#endif // MITKCOREMODULECONTEXT_H
