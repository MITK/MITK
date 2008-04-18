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

#include "cherryAdaptExpression.h"

#include "org.opencherry.osgi/cherryPlatform.h"
#include "org.opencherry.core.runtime/cherryRuntime.h"
#include "org.opencherry.core.runtime/cherryIAdapterManager.h"
#include "org.opencherry.core.runtime/cherryExpressionVariables.h"

#include "cherryExpressions.h"
#include "cherryDefaultVariable.h"

#include "Poco/Hash.h"
#include "Poco/Exception.h"

namespace cherry {

const std::string AdaptExpression::ATT_TYPE= "type"; //$NON-NLS-1$

/**
 * The seed for the hash code for all adapt expressions.
 */
const intptr_t AdaptExpression::HASH_INITIAL = Poco::Hash<std::string>()("cherry::AdaptExpression");

AdaptExpression::AdaptExpression(IConfigurationElement* configElement)
{
  bool attr = configElement->GetAttribute(ATT_TYPE, fTypeName);
  Expressions::CheckAttribute(ATT_TYPE, attr);
}

AdaptExpression::AdaptExpression(Poco::XML::Node* /*element*/) 
{
  throw Poco::NotImplementedException();
  //fTypeName = element->GetAttribute(ATT_TYPE);
  //Expressions::CheckAttribute(ATT_TYPE, fTypeName.length() > 0 ? fTypeName : null);
}

AdaptExpression::AdaptExpression(const std::string& typeName) 
{
  if (typeName.size() == 0)
    throw Poco::AssertionViolationException("typename must not be empty");
  fTypeName= typeName;
}

//bool
//AdaptExpression::equals(final Object object) 
//{
//  if (!(object instanceof AdaptExpression))
//    return false;
//  
//  final AdaptExpression that= (AdaptExpression)object;
//  return this.fTypeName.equals(that.fTypeName)
//      && equals(this.fExpressions, that.fExpressions);
//}

intptr_t 
AdaptExpression::ComputeHashCode() 
{
  throw Poco::NotImplementedException("ComputeHashCode not implemented");
  //return HASH_INITIAL * HASH_FACTOR + HashCode(fExpressions)
  //  * HASH_FACTOR + fTypeName.hashCode();
}

/* (non-Javadoc)
 * @see Expression#evaluate(IVariablePool)
 */
EvaluationResult 
AdaptExpression::Evaluate(IEvaluationContext* context)
{
  if (fTypeName.size() == 0)
    return EvaluationResult::FALSE;
  ExpressionVariable::Pointer var(context->GetDefaultVariable());
  ExpressionVariable::Pointer adapted;
  IAdapterManager::Pointer manager = Platform::GetServiceRegistry().GetServiceById<IAdapterManager>(Runtime::ADAPTER_SERVICE_ID);
  if (Expressions::IsInstanceOf(var, fTypeName)) {
    adapted= var;
  } else {
    if (!manager->HasAdapter(var, fTypeName))
      return EvaluationResult::FALSE;
  
    adapted = manager->GetAdapter(var, fTypeName);
  }
  // the adapted result is null but hasAdapter returned true check
  // if the adapter is loaded.
  if (adapted.IsNull()) {
    if (manager->QueryAdapter(var, fTypeName) == IAdapterManager::NOT_LOADED) {
      return EvaluationResult::NOT_LOADED;
    } else {
      return EvaluationResult::FALSE;
    }
  }
  return this->EvaluateAnd(new DefaultVariable(context, adapted));
}

void 
AdaptExpression::CollectExpressionInfo(ExpressionInfo* info) 
{
  // Although the default variable is passed to the children of this
  // expression as an instance of the adapted type it is OK to only
  // mark a default variable access.
  info->MarkDefaultVariableAccessed();
  CompositeExpression::CollectExpressionInfo(info);
} 

}  // namespace cherry
