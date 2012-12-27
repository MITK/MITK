/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYCOMMANDEXCEPTIONS_H_
#define BERRYCOMMANDEXCEPTIONS_H_

#include <org_blueberry_core_commands_Export.h>

#include <Poco/Exception.h>

namespace berry {

POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, CommandException, Poco::RuntimeException);

POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, ExecutionException, CommandException);
POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, NotHandledException, CommandException);
POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, NotDefinedException, CommandException);
POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, NotEnabledException, CommandException);
POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, ParameterValueConversionException, CommandException);

/**
 * Signals that a problem has occurred while trying to create an instance of
 * <code>IParameterValues</code>. In applications based on the registry
 * provided by core, this usually indicates a problem creating an
 * <code>IExecutableExtension</code>. For other applications, this exception
 * could be used to signify any general problem during initialization.
 *
 */
POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, ParameterValuesException, CommandException);

/**
 * Signals that an exception occured while serializing a
 * {@link ParameterizedCommand} to a string or deserializing a string to a
 * {@link ParameterizedCommand}.
 *
 * This class is not intended to be extended by clients.
 *
 */
POCO_DECLARE_EXCEPTION(BERRY_COMMANDS, SerializationException, CommandException);


}

#endif /*BERRYCOMMANDEXCEPTIONS_H_*/
