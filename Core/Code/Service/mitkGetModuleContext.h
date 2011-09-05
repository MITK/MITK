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


#ifndef MITKGETMODULECONTEXT_H
#define MITKGETMODULECONTEXT_H

namespace mitk {

class ModuleContext;

/**
 * Returns the module context of the calling module.
 *
 * This function allows easy access to the own ModuleContext instance from
 * inside a MITK module.
 *
 * \internal
 *
 * Note that the corresponding function definition is provided for each
 * module by the mitkModuleInit.cpp file. This file is customized via CMake
 * configure_file(...) and automatically compiled into each MITK module.
 * Consequently, the GetModuleContext function is not exported, since each
 * module gets its "own version".
 */
MITK_LOCAL ModuleContext* GetModuleContext();

}

#endif // MITKGETMODULECONTEXT_H
