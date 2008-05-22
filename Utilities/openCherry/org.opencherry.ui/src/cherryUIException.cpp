/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryUIException.h"

#include <typeinfo>

namespace cherry {

POCO_IMPLEMENT_EXCEPTION(UIException, Poco::RuntimeException, "UI exception");

POCO_IMPLEMENT_EXCEPTION(WorkbenchException, UIException, "Workbench error");

POCO_IMPLEMENT_EXCEPTION(PartInitException, WorkbenchException, "Part initialization error");

}
