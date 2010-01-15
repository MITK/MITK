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

#ifndef CHERRYUIEXCEPTIONS_H_
#define CHERRYUIEXCEPTIONS_H_

#include "cherryUiDll.h"

#include <osgi/framework/Exceptions.h>

namespace cherry {

using namespace osgi::framework;

/**
 * \ingroup org_opencherry_ui
 * @{
 */

//POCO_DECLARE_EXCEPTION(CHERRY_UI, UIException, PlatformException);

POCO_DECLARE_EXCEPTION(CHERRY_UI, WorkbenchException, CoreException);

POCO_DECLARE_EXCEPTION(CHERRY_UI, PartInitException, WorkbenchException);

/*@}*/
}

#endif /*CHERRYUIEXCEPTIONS_H_*/
