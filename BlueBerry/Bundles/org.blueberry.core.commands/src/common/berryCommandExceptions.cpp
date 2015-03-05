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

namespace berry {

CTK_IMPLEMENT_EXCEPTION(CommandException, ctkRuntimeException, "Command exception")

CTK_IMPLEMENT_EXCEPTION(ExecutionException, CommandException, "Command execution exception")
CTK_IMPLEMENT_EXCEPTION(NotHandledException, CommandException, "Command not handled exception")
CTK_IMPLEMENT_EXCEPTION(NotDefinedException, CommandException, "Command not defined exception")
CTK_IMPLEMENT_EXCEPTION(NotEnabledException, CommandException, "Command not enabled exception")
CTK_IMPLEMENT_EXCEPTION(ParameterValueConversionException, CommandException, "Parameter value conversion exception")
CTK_IMPLEMENT_EXCEPTION(ParameterValuesException, CommandException, "Parameter values exception")
CTK_IMPLEMENT_EXCEPTION(SerializationException, CommandException, "Serialization exception")
}
