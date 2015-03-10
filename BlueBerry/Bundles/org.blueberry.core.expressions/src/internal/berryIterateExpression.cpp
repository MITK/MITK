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

#include "berryIterateExpression.h"

#include "berryExpressions.h"
#include "berryDefaultVariable.h"

#include <berryObjectList.h>
#include <berryIConfigurationElement.h>

#include <QStringList>

namespace berry
{

const QString IterateExpression::ATT_OPERATOR = "operator";
const QString IterateExpression::ATT_IF_EMPTY = "ifEmpty";
const int IterateExpression::OR = 1;
const int IterateExpression::AND = 2;

const uint IterateExpression::HASH_INITIAL = qHash("berry::IterateExpression");

struct IteratePool : public IEvaluationContext
{

private:
  QList<Object::Pointer>::const_iterator fIterator;
  QList<Object::Pointer>::const_iterator fIterEnd;
  Object::ConstPointer fDefaultVariable;
  IEvaluationContext* fParent;

public:

  IteratePool(IEvaluationContext* parent, QList<Object::Pointer>::const_iterator begin,
              QList<Object::Pointer>::const_iterator end)
  {
    poco_check_ptr(parent);

    fParent= parent;
    fIterator = begin;
    fIterEnd = end;
  }

  IEvaluationContext* GetParent() const {
    return fParent;
  }

  IEvaluationContext* GetRoot() const {
    return fParent->GetRoot();
  }

  Object::ConstPointer GetDefaultVariable() const {
    return fDefaultVariable;
  }

  bool GetAllowPluginActivation() const {
    return fParent->GetAllowPluginActivation();
  }

  void SetAllowPluginActivation(bool value) {
    fParent->SetAllowPluginActivation(value);
  }

  void AddVariable(const QString& name, const Object::ConstPointer& value) {
    fParent->AddVariable(name, value);
  }

  Object::ConstPointer RemoveVariable(const QString& name) {
    return fParent->RemoveVariable(name);
  }

  Object::ConstPointer GetVariable(const QString& name) const {
    return fParent->GetVariable(name);
  }

  Object::ConstPointer ResolveVariable(const QString& name, const QList<Object::Pointer>& args) const {
    return fParent->ResolveVariable(name, args);
  }

  Poco::Any Next() {
    fDefaultVariable = (++fIterator)->GetPointer();
    return fDefaultVariable;
  }

  bool HasNext() {
    return (fIterator != fIterEnd);
  }
};

IterateExpression::IterateExpression(
    const IConfigurationElement::Pointer& configElement)
{
  QString opValue = configElement->GetAttribute(ATT_OPERATOR);
  this->InitializeOperatorValue(opValue);
  this->InitializeEmptyResultValue(configElement->GetAttribute(ATT_IF_EMPTY));
}

IterateExpression::IterateExpression(Poco::XML::Element* element)
{
  std::string opValue = element->getAttribute(ATT_OPERATOR.toStdString());
  this->InitializeOperatorValue(opValue.size() > 0 ? QString::fromStdString(opValue) : QString());
  std::string ifEmpty = element->getAttribute(ATT_IF_EMPTY.toStdString());
  this->InitializeEmptyResultValue(ifEmpty.size() > 0 ? QString::fromStdString(ifEmpty) : QString());
}

IterateExpression::IterateExpression(const QString& opValue)
{
  this->InitializeOperatorValue(opValue);
}

IterateExpression::IterateExpression(const QString& opValue,
                                     const QString& ifEmpty)
{
  this->InitializeOperatorValue(opValue);
  this->InitializeEmptyResultValue(ifEmpty);
}

void IterateExpression::InitializeOperatorValue(const QString& opValue)
{
  if (opValue.isNull())
  {
    fOperator = AND;
  }
  else
  {
    QStringList fValidOperators;
    fValidOperators.push_back("and");
    fValidOperators.push_back("or");
    Expressions::CheckAttribute(ATT_OPERATOR, opValue, fValidOperators);

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

void IterateExpression::InitializeEmptyResultValue(const QString& value)
{
  if (value.isNull())
  {
    fEmptyResult = -1;
  }
  else
  {
    fEmptyResult = value.compare("true", Qt::CaseInsensitive) == 0 ? 1 : 0;
  }
}

EvaluationResult::ConstPointer IterateExpression::Evaluate(IEvaluationContext* context) const
{
  Object::ConstPointer var = context->GetDefaultVariable();
  const ObjectList<Object::Pointer>* col = dynamic_cast<const ObjectList<Object::Pointer>*>(var.GetPointer());
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
      IteratePool iter(context, col->begin(), col->end());
      EvaluationResult::ConstPointer result = fOperator == AND ? EvaluationResult::TRUE_EVAL
          : EvaluationResult::FALSE_EVAL;
      while (iter.HasNext())
      {
        switch (fOperator)
        {
        case OR:
          result = result->Or(this->EvaluateAnd(&iter));
          if (result == EvaluationResult::TRUE_EVAL)
            return result;
          break;
        case AND:
          result = result->And(this->EvaluateAnd(&iter));
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
    IIterable::ConstPointer iterable = Expressions::GetAsIIterable(var,
                                                              Expression::ConstPointer(this));
    if (iterable.IsNull())
      return EvaluationResult::NOT_LOADED;

    int count = 0;
    IteratePool iter(context, iterable->begin(), iterable->end());
    EvaluationResult::ConstPointer result = fOperator == AND ? EvaluationResult::TRUE_EVAL
                                                             : EvaluationResult::FALSE_EVAL;
    while (iter.HasNext())
    {
      count++;
      switch (fOperator)
      {
      case OR:
        result = result->Or(this->EvaluateAnd(&iter));
        if (result == EvaluationResult::TRUE_EVAL)
          return result;
        break;
      case AND:
        result = result->And(this->EvaluateAnd(&iter));
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

void IterateExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  // Although we access every single variable we only mark the default
  // variable as accessed since we don't have single variables for the
  // elements.
  info->MarkDefaultVariableAccessed();
  CompositeExpression::CollectExpressionInfo(info);
}

bool IterateExpression::operator==(const Object* object) const
{
  if (const IterateExpression* that = dynamic_cast<const IterateExpression*>(object))
  {
    return (this->fOperator == that->fOperator) &&
        this->Equals(this->fExpressions, that->fExpressions);
  }
  return false;
}

uint IterateExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
      * HASH_FACTOR + fOperator;
}

} // namespace berry
