/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCountExpression.h"

#include "berryExpressions.h"

#include <berryObjectList.h>
#include <berryIConfigurationElement.h>

namespace berry {

const int CountExpression::ANY_NUMBER= 5;
const int CountExpression::EXACT= 4;
const int CountExpression::ONE_OR_MORE= 3;
const int CountExpression::NONE_OR_ONE= 2;
const int CountExpression::NONE= 1;
const int CountExpression::UNKNOWN= 0;

const uint CountExpression::HASH_INITIAL = qHash("berry::CountExpression");

  void
  CountExpression::InitializeSize(QString size)
  {
    if (size.isNull())
      size = "*";
    if (size == "*")
      fMode = ANY_NUMBER;
    else if (size == "?")
      fMode = NONE_OR_ONE;
    else if (size == "!")
      fMode = NONE;
    else if (size == "+")
      fMode = ONE_OR_MORE;
    else
    {
      bool ok = false;
      fSize = size.toInt(&ok);
      if (ok)
      {
        fMode = EXACT;
      }
      else
      {
        fMode = UNKNOWN;
      }
    }
  }

  CountExpression::CountExpression(const IConfigurationElement::Pointer& configElement)
  {
    QString size = configElement->GetAttribute(ATT_VALUE);
    this->InitializeSize(size);
  }

  CountExpression::CountExpression(Poco::XML::Element* element)
  {
    std::string size = element->getAttribute(ATT_VALUE.toStdString());
    this->InitializeSize(size.size() > 0 ? QString::fromStdString(size) : QString());
  }

  CountExpression::CountExpression(const QString& size)
  {
    this->InitializeSize(size);
  }

  EvaluationResult::ConstPointer
  CountExpression::Evaluate(IEvaluationContext* context) const
  {
    Object::ConstPointer var(context->GetDefaultVariable());
    ObjectList<Object::Pointer>::size_type size;

    if(ObjectList<Object::Pointer>::ConstPointer coll = var.Cast<const ObjectList<Object::Pointer> >())
    {
      size = coll->size();
    }
    else
    {
      ICountable::ConstPointer countable = Expressions::GetAsICountable(var, Expression::ConstPointer(this));
      if (!countable)
        return EvaluationResult::NOT_LOADED;
      size = countable->Count();
    }

    switch (fMode)
    {
      case UNKNOWN:
      return EvaluationResult::FALSE_EVAL;
      case NONE:
      return EvaluationResult::ValueOf(size == 0);
      case NONE_OR_ONE:
      return EvaluationResult::ValueOf(size == 0 || size == 1);
      case ONE_OR_MORE:
      return EvaluationResult::ValueOf(size >= 1);
      case EXACT:
      return EvaluationResult::ValueOf( (ObjectList<Object::Pointer>::size_type) fSize == size);
      case ANY_NUMBER:
      return EvaluationResult::TRUE_EVAL;
    }

    return EvaluationResult::FALSE_EVAL;
  }

  void
  CountExpression::CollectExpressionInfo(ExpressionInfo* info) const
  {
    info->MarkDefaultVariableAccessed();
  }

  bool
  CountExpression::operator==(const Object* object) const
  {
    if (const CountExpression* that = dynamic_cast<const CountExpression*>(object))
    {
      return (this->fMode == that->fMode) && (this->fSize == that->fSize);
    }
    return false;
  }

  uint
  CountExpression::ComputeHashCode() const
  {
    return HASH_INITIAL * HASH_FACTOR + fMode * HASH_FACTOR + fSize;
  }

}  // namespace berry
