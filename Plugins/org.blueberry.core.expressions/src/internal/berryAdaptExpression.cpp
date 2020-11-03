/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryAdaptExpression.h"

#include <berryPlatform.h>
#include <berryIAdapterManager.h>
#include <berryIConfigurationElement.h>

#include "berryExpressions.h"
#include "berryDefaultVariable.h"

#include <Poco/DOM/Node.h>

namespace berry {

const QString AdaptExpression::ATT_TYPE= "type";

/**
 * The seed for the hash code for all adapt expressions.
 */
const uint AdaptExpression::HASH_INITIAL = qHash("berry::AdaptExpression");

AdaptExpression::AdaptExpression(IConfigurationElement::Pointer configElement)
{
  fTypeName = configElement->GetAttribute(ATT_TYPE);
  Expressions::CheckAttribute(ATT_TYPE, fTypeName);
}

AdaptExpression::AdaptExpression(Poco::XML::Node* /*element*/)
{
  throw Poco::NotImplementedException();
  //fTypeName = element->GetAttribute(ATT_TYPE);
  //Expressions::CheckAttribute(ATT_TYPE, fTypeName.length() > 0 ? fTypeName : null);
}

AdaptExpression::AdaptExpression(const QString& typeName)
{
  poco_assert(typeName.size() != 0);
  fTypeName= typeName;
}

//bool
//AdaptExpression::equals(final Object object)
//{
//  if (!(object instanceof AdaptExpression))
//    return FALSE_EVAL;
//
//  final AdaptExpression that= (AdaptExpression)object;
//  return this.fTypeName.equals(that.fTypeName)
//      && equals(this.fExpressions, that.fExpressions);
//}

uint
AdaptExpression::ComputeHashCode() const
{
  throw Poco::NotImplementedException("ComputeHashCode not implemented");
  //return HASH_INITIAL * HASH_FACTOR + HashCode(fExpressions)
  //  * HASH_FACTOR + fTypeName.hashCode();
}

/* (non-Javadoc)
 * @see Expression#evaluate(IVariablePool)
 */
EvaluationResult::ConstPointer
AdaptExpression::Evaluate(IEvaluationContext* context) const
{
  if (fTypeName.size() == 0)
    return EvaluationResult::FALSE_EVAL;
  Object::ConstPointer var = context->GetDefaultVariable();
  Object::ConstPointer adapted;
  IAdapterManager* manager = Platform::GetAdapterManager();
  if (Expressions::IsInstanceOf(var.GetPointer(), fTypeName))
  {
    adapted = var;
  }
  else
  {
    if (!manager->HasAdapter(var.GetPointer(), fTypeName))
      return EvaluationResult::FALSE_EVAL;

    adapted = manager->GetAdapter(var.GetPointer(), fTypeName);
  }
  // the adapted result is null but hasAdapter returned TRUE_EVAL check
  // if the adapter is loaded.
  if (adapted.IsNull())
  {
    if (manager->QueryAdapter(var.GetPointer(), fTypeName) == IAdapterManager::NOT_LOADED)
    {
      return EvaluationResult::NOT_LOADED;
    }
    else
    {
      return EvaluationResult::FALSE_EVAL;
    }
  }
  DefaultVariable scope(context, adapted);
  return this->EvaluateAnd(&scope);
}

void
AdaptExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  // Although the default variable is passed to the children of this
  // expression as an instance of the adapted type it is OK to only
  // mark a default variable access.
  info->MarkDefaultVariableAccessed();
  CompositeExpression::CollectExpressionInfo(info);
}

}  // namespace berry
