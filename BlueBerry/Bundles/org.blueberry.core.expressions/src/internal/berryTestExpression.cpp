/*=========================================================================

Program:   BlueBerry Platform
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

#include "berryTestExpression.h"

#include "berryExpressions.h"

#include "berryPlatform.h"

#include <berryObjectString.h>

#include <Poco/Hash.h>

namespace berry {

const char TestExpression::PROP_SEP = '.';
const std::string TestExpression::ATT_PROPERTY = "property";
const std::string TestExpression::ATT_ARGS = "args";
const std::string TestExpression::ATT_FORCE_PLUGIN_ACTIVATION = "forcePluginActivation";

TypeExtensionManager TestExpression::fgTypeExtensionManager("propertyTesters");

const std::size_t TestExpression::HASH_INITIAL= Poco::hash("berry::TextExpression");


TestExpression::TestExpression(IConfigurationElement::Pointer element)
{
  std::string property;
  element->GetAttribute(ATT_PROPERTY, property);
  std::size_t pos = property.find_last_of(PROP_SEP);
  if (pos == std::string::npos)
  {
    throw CoreException("No namespace provided");
  }
  fNamespace = property.substr(0, pos);
  fProperty = property.substr(pos + 1);
  Expressions::GetArguments(fArgs, element, ATT_ARGS);
  std::string arg = "";
  bool result = element->GetAttribute(ATT_VALUE, arg);
  fExpectedValue = Expressions::ConvertArgument(arg, result);
  fForcePluginActivation = Expressions::GetOptionalBooleanAttribute(element, ATT_FORCE_PLUGIN_ACTIVATION);
}

TestExpression::TestExpression(Poco::XML::Element* element)
{
  std::string property= element->getAttribute(ATT_PROPERTY);
  std::size_t pos = property.find_last_of(PROP_SEP);
  if (pos == std::string::npos)
  {
    throw CoreException("No namespace provided");
  }
  fNamespace = property.substr(0, pos);
  fProperty = property.substr(pos + 1);
  Expressions::GetArguments(fArgs, element, ATT_ARGS);
  std::string value = element->getAttribute(ATT_VALUE);
  fExpectedValue = Expressions::ConvertArgument(value, value.size() > 0);
  fForcePluginActivation= Expressions::GetOptionalBooleanAttribute(element, ATT_FORCE_PLUGIN_ACTIVATION);
}

TestExpression::TestExpression(const std::string& namespaze, const std::string& property, std::vector<Object::Pointer>& args, Object::Pointer expectedValue)
{
  TestExpression(namespaze, property, args, expectedValue, false);
}

TestExpression::TestExpression(const std::string& namespaze, const std::string& property, std::vector<Object::Pointer>& args, Object::Pointer expectedValue, bool forcePluginActivation)
 : fNamespace(namespaze), fProperty(property), fArgs(args),
   fExpectedValue(expectedValue), fForcePluginActivation(forcePluginActivation)
{

}

EvaluationResult
TestExpression::Evaluate(IEvaluationContext* context)
{
  Object::Pointer element = context->GetDefaultVariable();
  if (typeid(Platform) == typeid(element.GetPointer()))
  {
    std::string str = Platform::GetProperty(fProperty);
    if (str.size() == 0)
    {
      return EvaluationResult::FALSE_EVAL;
    }

    ObjectString::Pointer var = fArgs[0].Cast<ObjectString>();
    if (var)
      return EvaluationResult::ValueOf(*var == str);

    return EvaluationResult::FALSE_EVAL;
  }

  Property::Pointer property= fgTypeExtensionManager.GetProperty(element, fNamespace, fProperty, context->GetAllowPluginActivation() && fForcePluginActivation);
  if (!property->IsInstantiated())
    return EvaluationResult::NOT_LOADED;
  return EvaluationResult::ValueOf(property->Test(element, fArgs, fExpectedValue));
}

void
TestExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  info->MarkDefaultVariableAccessed();
  info->AddAccessedPropertyName(fNamespace + PROP_SEP + fProperty);
}

bool
TestExpression::operator==(Expression& object)
{
  try {
    TestExpression& that = dynamic_cast<TestExpression&>(object);
    return this->fNamespace == that.fNamespace &&
            this->fProperty == that.fProperty &&
            this->fForcePluginActivation == that.fForcePluginActivation &&
            this->Equals(this->fArgs, that.fArgs) &&
            this->fExpectedValue == that.fExpectedValue;
  }
  catch (std::bad_cast)
  {
    return false;
  }

}

std::size_t
TestExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fArgs)
  * HASH_FACTOR + fExpectedValue->HashCode()
  * HASH_FACTOR + Poco::hash(fNamespace)
  * HASH_FACTOR + Poco::hash(fProperty)
  * HASH_FACTOR + (fForcePluginActivation ? 1 : 0);
}

std::string
TestExpression::ToString()
{
  std::string args("");
  for (unsigned int i= 0; i < fArgs.size(); i++)
  {
    Object::Pointer arg= fArgs[i];
    ObjectString::Pointer strarg = arg.Cast<ObjectString>();
    if (strarg)
    {
      args.append(1,'\'');
      args.append(*strarg);
      args.append(1,'\'');
    }
    else
    {
      args.append(arg->ToString());
    }
    if (i < fArgs.size() - 1)
    args.append(", "); //$NON-NLS-1$
  }

  return "<test property=\"" + fProperty +
  (fArgs.size() != 0 ? "\" args=\"" + args + "\"" : "\"") +
  (!fExpectedValue.IsNull() ? "\" value=\"" + fExpectedValue->ToString() + "\"" : "\"") +
  " plug-in activation: " + (fForcePluginActivation ? "eager" : "lazy") +
  "/>"; //$NON-NLS-1$
}

//---- testing ---------------------------------------------------

bool
TestExpression::TestGetForcePluginActivation()
{
  return fForcePluginActivation;
}

TypeExtensionManager&
TestExpression::TestGetTypeExtensionManager()
{
  return fgTypeExtensionManager;
}

}
