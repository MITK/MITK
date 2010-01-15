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

#include "cherryInstanceofExpression.h"

#include "cherryExpressions.h"

#include "Poco/Hash.h"

namespace cherry {

const std::size_t InstanceofExpression::HASH_INITIAL= Poco::Hash<std::string>()("cherry::InstanceofExpression");

InstanceofExpression::InstanceofExpression(IConfigurationElement::Pointer element)
{
  bool result = element->GetAttribute(ATT_VALUE, fTypeName);
  Expressions::CheckAttribute(ATT_VALUE, result);
}

InstanceofExpression::InstanceofExpression(Poco::XML::Element* element)
{
  fTypeName = element->getAttribute(ATT_VALUE);
  Expressions::CheckAttribute(ATT_VALUE, fTypeName.size() > 0);
}

InstanceofExpression::InstanceofExpression(const std::string& typeName)
 : fTypeName(typeName)
{

}

EvaluationResult
InstanceofExpression::Evaluate(IEvaluationContext::Pointer context)
{
  Object::Pointer element= context->GetDefaultVariable();
  return EvaluationResult::ValueOf(Expressions::IsInstanceOf(element, fTypeName));
}

void
InstanceofExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  info->MarkDefaultVariableAccessed();
}

bool
InstanceofExpression::operator==(Expression& object)
{
  try
  {
    InstanceofExpression& that = dynamic_cast<InstanceofExpression&>(object);
    return this->fTypeName == that.fTypeName;
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

std::string
InstanceofExpression::ToString()
{
  return "<instanceof value=\"" + fTypeName + "\"/>"; ;
}

std::size_t
InstanceofExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + Poco::Hash<std::string>()(fTypeName);
}

}
