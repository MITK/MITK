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

#include "cherryIterateExpression.h"

#include "cherryExpressions.h"
#include "cherryDefaultVariable.h"

#include <cherryObjectVector.h>

#include <Poco/String.h>
#include <Poco/Hash.h>

namespace cherry
{

const std::string IterateExpression::ATT_OPERATOR = "operator"; //$NON-NLS-1$
const std::string IterateExpression::ATT_IF_EMPTY = "ifEmpty"; //$NON-NLS-1$
const int IterateExpression::OR = 1;
const int IterateExpression::AND = 2;

const std::size_t IterateExpression::HASH_INITIAL = Poco::hash(
    "cherry::IterateExpression");

IterateExpression::IterateExpression(
    IConfigurationElement::Pointer configElement)
{
  std::string opValue = "";
  configElement->GetAttribute(ATT_OPERATOR, opValue);
  this->InitializeOperatorValue(opValue);
  std::string ifEmpty = "";
  configElement->GetAttribute(ATT_IF_EMPTY, ifEmpty);
  this->InitializeEmptyResultValue(ifEmpty);
}

IterateExpression::IterateExpression(Poco::XML::Element* element)
{
  std::string opValue = element->getAttribute(ATT_OPERATOR);
  this->InitializeOperatorValue(opValue);
  std::string ifEmpty = element->getAttribute(ATT_IF_EMPTY);
  this->InitializeEmptyResultValue(ifEmpty);
}

IterateExpression::IterateExpression(const std::string& opValue)
{
  this->InitializeOperatorValue(opValue);
}

IterateExpression::IterateExpression(const std::string& opValue,
    const std::string& ifEmpty)
{
  this->InitializeOperatorValue(opValue);
  this->InitializeEmptyResultValue(ifEmpty);
}

void IterateExpression::InitializeOperatorValue(const std::string& opValue)
{
  if (opValue == "")
  {
    fOperator = AND;
  }
  else
  {
    std::vector<std::string> fValidOperators;
    fValidOperators.push_back("and");
    fValidOperators.push_back("or");
    Expressions::CheckAttribute(ATT_OPERATOR, true, opValue, fValidOperators);

    if ("and" == opValue)
    {
      fOperator = AND;
    }
    else
    {
      fOperator = OR;
    }
  }
}

void IterateExpression::InitializeEmptyResultValue(const std::string& value)
{
  if (value == "")
  {
    fEmptyResult = -1;
  }
  else
  {
    fEmptyResult = Poco::toLower(value) == "TRUE_EVAL" ? 1 : 0;
  }
}

EvaluationResult IterateExpression::Evaluate(IEvaluationContext* context)
{
  Object::Pointer var = context->GetDefaultVariable();
  ObjectVector<Object::Pointer>::Pointer col = var.Cast<ObjectVector<
      Object::Pointer> > ();
  if (col)
  {
    switch (col->size())
    {
    case 0:
    {
      if (fEmptyResult == -1)
      {
        return fOperator == AND ? EvaluationResult::TRUE_EVAL
            : EvaluationResult::FALSE_EVAL;
      }
      else
      {
        return fEmptyResult == 1 ? EvaluationResult::TRUE_EVAL
            : EvaluationResult::FALSE_EVAL;
      }
    }
    case 1:
    {
      IEvaluationContext::Pointer scope(new DefaultVariable(context,
          col->front()));
      return this->EvaluateAnd(scope.GetPointer());
    }
    default:
      IteratePool iter(context, col->begin(),
          col->end());
      EvaluationResult result = fOperator == AND ? EvaluationResult::TRUE_EVAL
          : EvaluationResult::FALSE_EVAL;
      while (iter.HasNext())
      {
        switch (fOperator)
        {
        case OR:
          result = result.Or(this->EvaluateAnd(&iter));
          if (result == EvaluationResult::TRUE_EVAL)
            return result;
          break;
        case AND:
          result = result.And(this->EvaluateAnd(&iter));
          if (result != EvaluationResult::TRUE_EVAL)
            return result;
          break;
        }
        iter.Next();
      }
      return result;
    }
  }
  else
  {
    IIterable::Pointer iterable = Expressions::GetAsIIterable(var,
        Expression::Pointer(this));
    if (iterable.IsNull())
      return EvaluationResult::NOT_LOADED;

    int count = 0;
    IteratePool iter(context, iterable->begin(), iterable->end());
    EvaluationResult result = fOperator == AND ? EvaluationResult::TRUE_EVAL
        : EvaluationResult::FALSE_EVAL;
    while (iter.HasNext())
    {
      count++;
      switch (fOperator)
      {
      case OR:
        result = result.Or(this->EvaluateAnd(&iter));
        if (result == EvaluationResult::TRUE_EVAL)
          return result;
        break;
      case AND:
        result = result.And(this->EvaluateAnd(&iter));
        if (result != EvaluationResult::TRUE_EVAL)
          return result;
        break;
      }
      iter.Next();
    }
    if (count > 0)
    {
      return result;
    }
    else
    {
      if (fEmptyResult == -1)
      {
        return fOperator == AND ? EvaluationResult::TRUE_EVAL
            : EvaluationResult::FALSE_EVAL;
      }
      else
      {
        return fEmptyResult == 1 ? EvaluationResult::TRUE_EVAL
            : EvaluationResult::FALSE_EVAL;
      }
    }
  }
}

void IterateExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  // Although we access every single variable we only mark the default
  // variable as accessed since we don't have single variables for the
  // elements.
  info->MarkDefaultVariableAccessed();
  CompositeExpression::CollectExpressionInfo(info);
}

bool IterateExpression::operator==(Expression& object)
{
  try
  {
    IterateExpression& that = dynamic_cast<IterateExpression&> (object);
    return (this->fOperator == that.fOperator) && this->Equals(
        this->fExpressions, that.fExpressions);
  } catch (std::bad_cast)
  {
    return false;
  }
}

std::size_t IterateExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
      * HASH_FACTOR + fOperator;
}

} // namespace cherry
