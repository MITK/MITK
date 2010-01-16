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

#include "berrySystemTestExpression.h"

#include "berryExpressions.h"

#include <berryPlatform.h>

#include <Poco/Hash.h>

namespace berry {

const std::string SystemTestExpression::ATT_PROPERTY= "property";

const std::size_t SystemTestExpression::HASH_INITIAL = Poco::Hash<std::string>()("berry::SystemTestExpression");

SystemTestExpression::SystemTestExpression(IConfigurationElement::Pointer element)
{
  bool result = element->GetAttribute(ATT_PROPERTY, fProperty);
  Expressions::CheckAttribute(ATT_PROPERTY, result);
  result = element->GetAttribute(ATT_VALUE, fExpectedValue);
  Expressions::CheckAttribute(ATT_VALUE, result);
}

SystemTestExpression::SystemTestExpression(Poco::XML::Element* element)
{
  fProperty= element->getAttribute(ATT_PROPERTY);
  Expressions::CheckAttribute(ATT_PROPERTY, fProperty.length()> 0);
  fExpectedValue = element->getAttribute(ATT_VALUE);
  Expressions::CheckAttribute(ATT_VALUE, fExpectedValue.length()> 0);
}

SystemTestExpression::SystemTestExpression(const std::string& property, const std::string& expectedValue)
 : fProperty(property), fExpectedValue(expectedValue)
{

}

EvaluationResult
SystemTestExpression::Evaluate(IEvaluationContext::Pointer /*context*/)
{
  std::string str = Platform::GetProperty(fProperty);
  if (str.size() == 0)
    return EvaluationResult::FALSE_EVAL;

  return EvaluationResult::ValueOf(str == fExpectedValue);
}

void
SystemTestExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  info->MarkSystemPropertyAccessed();
}

bool
SystemTestExpression::operator==(Expression& object)
{
  try {
    SystemTestExpression& that = dynamic_cast<SystemTestExpression&>(object);
    return this->fProperty == that.fProperty
      && this->fExpectedValue == that.fExpectedValue;
  }
  catch (std::bad_cast)
  {
    return false;
  }

}

std::size_t
SystemTestExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + Poco::Hash<std::string>()(fExpectedValue)
  * HASH_FACTOR + Poco::Hash<std::string>()(fProperty);
}

std::string
SystemTestExpression::ToString()
{
  return "<systemTest property=\"" + fProperty + //$NON-NLS-1$
    "\" value=\"" + fExpectedValue + "\""; //$NON-NLS-1$ //$NON-NLS-2$
  }

}
