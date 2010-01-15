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

#include "cherryReferenceExpression.h"

#include "cherryExpressions.h"

#include "Poco/Hash.h"

namespace cherry {

const std::string ReferenceExpression::ATT_DEFINITION_ID= "definitionId";
const std::size_t ReferenceExpression::HASH_INITIAL= Poco::Hash<std::string>()("cherry::ReferenceExpression");

DefinitionRegistry ReferenceExpression::fgDefinitionRegistry = DefinitionRegistry();

DefinitionRegistry&
ReferenceExpression::GetDefinitionRegistry()
{
  return fgDefinitionRegistry;
}

ReferenceExpression::ReferenceExpression(const std::string& definitionId)
{
  fDefinitionId= definitionId;
}

ReferenceExpression::ReferenceExpression(IConfigurationElement::Pointer element)
{
  bool result = element->GetAttribute(ATT_DEFINITION_ID, fDefinitionId);
  Expressions::CheckAttribute(ATT_DEFINITION_ID, result);
}

ReferenceExpression::ReferenceExpression(Poco::XML::Element* element)
{
  fDefinitionId = element->getAttribute(ATT_DEFINITION_ID);
  Expressions::CheckAttribute(ATT_DEFINITION_ID, fDefinitionId.size() > 0);
}

EvaluationResult
ReferenceExpression::Evaluate(IEvaluationContext::Pointer context)
{
  Expression::Pointer expr= GetDefinitionRegistry().GetExpression(fDefinitionId);
  return expr->Evaluate(context);
}

void
ReferenceExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  Expression::Pointer expr;
  try
  {
    expr= GetDefinitionRegistry().GetExpression(fDefinitionId);
  }
  catch (CoreException e)
  {
    // We didn't find the expression definition. So no
    // expression info can be collected.
    return;
  }
  expr->CollectExpressionInfo(info);
}

bool
ReferenceExpression::operator==(Expression& object)
{
  try {
    ReferenceExpression& that = dynamic_cast<ReferenceExpression&>(object);
    return this->fDefinitionId == that.fDefinitionId;
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

std::size_t
ReferenceExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + Poco::Hash<std::string>()(fDefinitionId);
}

}
