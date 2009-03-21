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
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, NotHandledException, CommandException);
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, NotDefinedException, CommandException);
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, NotEnabledException, CommandException);
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, ParameterValueConversionException, CommandException);

/**
 * Signals that a problem has occurred while trying to create an instance of
 * <code>IParameterValues</code>. In applications based on the registry
 * provided by core, this usually indicates a problem creating an
 * <code>IExecutableExtension</code>. For other applications, this exception
 * could be used to signify any general problem during initialization.
 *
 */
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, ParameterValuesException, CommandException);

/**
 * Signals that an exception occured while serializing a
 * {@link ParameterizedCommand} to a string or deserializing a string to a
 * {@link ParameterizedCommand}.
 *
 * This class is not intended to be extended by clients.
 *
 */
POCO_DECLARE_EXCEPTION(CHERRY_COMMANDS, SerializationException, CommandException);


}

#endif /*CHERRYCOMMANDEXCEPTIONS_H_*/
