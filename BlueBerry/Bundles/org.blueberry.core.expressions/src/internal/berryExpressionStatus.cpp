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

#include "berryExpressionStatus.h"

#include "berryExpressionPlugin.h"


namespace berry {

const int ExpressionStatus::VARIABLE_IS_NOT_A_COLLECTION = 3;
const int ExpressionStatus::VARIABLE_IS_NOT_A_LIST = 4;
const int ExpressionStatus::VALUE_IS_NOT_AN_INTEGER = 5;
const int ExpressionStatus::MISSING_ATTRIBUTE = 50;
const int ExpressionStatus::WRONG_ATTRIBUTE_VALUE = 51;
const int ExpressionStatus::MISSING_EXPRESSION  = 52;
const int ExpressionStatus::VARAIBLE_POOL_WRONG_NUMBER_OF_ARGUMENTS = 100;
const int ExpressionStatus::VARAIBLE_POOL_ARGUMENT_IS_NOT_A_STRING = 101;
const int ExpressionStatus::TYPE_EXTENDER_PLUGIN_NOT_LOADED = 200;
const int ExpressionStatus::TYPE_EXTENDER_UNKOWN_METHOD = 201;
const int ExpressionStatus::TYPE_EXTENDER_INCORRECT_TYPE = 202;
const int ExpressionStatus::TEST_EXPRESSION_NOT_A_BOOLEAN = 203;
const int ExpressionStatus::NO_NAMESPACE_PROVIDED = 300;
const int ExpressionStatus::VARIABLE_NOT_DEFINED = 301;
const int ExpressionStatus::STRING_NOT_CORRECT_ESCAPED = 302;
const int ExpressionStatus::STRING_NOT_TERMINATED = 303;

ExpressionStatus::ExpressionStatus(int errorCode, const QString& message, const SourceLocation& sl)
  : Status(IStatus::ERROR_TYPE, ExpressionPlugin::GetPluginId(), errorCode, message, sl)
{
}

ExpressionStatus::ExpressionStatus(int errorCode, const QString& message, const ctkException &exc,
                                   const SourceLocation& sl)
  : Status(IStatus::ERROR_TYPE, ExpressionPlugin::GetPluginId(), errorCode, message, exc, sl)
{
}

}
