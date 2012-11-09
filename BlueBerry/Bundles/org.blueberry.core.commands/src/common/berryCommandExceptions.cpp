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

#include "berryCommandExceptions.h"

#include <typeinfo>

namespace berry {

POCO_IMPLEMENT_EXCEPTION(CommandException, Poco::RuntimeException, "Command exception")

POCO_IMPLEMENT_EXCEPTION(ExecutionException, CommandException, "Command execution exception")
POCO_IMPLEMENT_EXCEPTION(NotHandledException, CommandException, "Command not handled exception")
POCO_IMPLEMENT_EXCEPTION(NotDefinedException, CommandException, "Command not defined exception")
POCO_IMPLEMENT_EXCEPTION(NotEnabledException, CommandException, "Command not enabled exception")
POCO_IMPLEMENT_EXCEPTION(ParameterValueConversionException, CommandException, "Parameter value conversion exception");
POCO_IMPLEMENT_EXCEPTION(ParameterValuesException, CommandException, "Parameter values exception")
POCO_IMPLEMENT_EXCEPTION(SerializationException, CommandException, "Serialization exception")
}
