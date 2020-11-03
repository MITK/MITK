/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
