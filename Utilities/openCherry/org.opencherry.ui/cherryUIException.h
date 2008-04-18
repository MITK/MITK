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

#include <org.opencherry.osgi/cherryDll.h>
#include "Poco/Exception.h"

namespace cherry {

POCO_DECLARE_EXCEPTION(CHERRY_API, UIException, Poco::RuntimeException);

POCO_DECLARE_EXCEPTION(CHERRY_API, WorkbenchException, UIException);

POCO_DECLARE_EXCEPTION(CHERRY_API, PartInitException, WorkbenchException);

}

#endif /*CHERRYUIEXCEPTIONS_H_*/
