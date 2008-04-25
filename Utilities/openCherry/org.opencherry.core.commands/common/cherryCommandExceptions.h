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

#ifndef CHERRYCOMMANDEXCEPTIONS_H_
#define CHERRYCOMMANDEXCEPTIONS_H_

#include "../cherryCommandsDll.h"

#include <Poco/Exception.h>

namespace cherry {

POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, CommandException, Poco::RuntimeException);

POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, ExecutionException, CommandException);
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, NotDefinedException, CommandException);

}

#endif /*CHERRYCOMMANDEXCEPTIONS_H_*/
