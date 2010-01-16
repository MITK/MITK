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

#include "berryCountExpression.h"

#include "berryExpressions.h"

#include <berryObjectVector.h>

#include <Poco/Hash.h>
#include <Poco/NumberParser.h>

namespace berry {

const int CountExpression::ANY_NUMBER= 5;
const int CountExpression::EXACT= 4;
const int CountExpression::ONE_OR_MORE= 3;
const int CountExpression::NONE_OR_ONE= 2;
const int CountExpression::NONE= 1;
const int CountExpression::UNKNOWN= 0;

const std::size_t CountExpression::HASH_INITIAL = Poco::Hash<std::string>()("berry::CountExpression");

  void
  CountExpression::InitializeSize(std::string size)
  {

    if (size == "*") //$NON-NLS-1$
    fMode= ANY_NUMBER;
    else if (size == "?") //$NON-NLS-1$
    fMode= NONE_OR_ONE;
    else if (size == "!") //$NON-NLS-1$
    fMode= NONE;
    else if (size == "+") //$NON-NLS-1$
    fMode= ONE_OR_MORE;
    else
    {
      try
      {
        fSize= Poco::NumberParser::parse(size);
        fMode= EXACT;
      }
      catch (Poco::SyntaxException e)
      {
        fMode= UNKNOWN;
      }
    }
  }

  CountExpression::CountExpression(IConfigurationElement::Pointer configElement)
  {
    std::string size;
    if (!configElement->GetAttribute(ATT_VALUE, size))
    size = "*";

    this->InitializeSize(size);
  }

  CountExpression::CountExpression(Poco::XML::Element* element)
  {
    std::string size = element->getAttribute(ATT_VALUE);
    this->InitializeSize(size);
  }

  CountExpression::CountExpression(const std::string& size)
  {
    this->InitializeSize(size);
  }

  EvaluationResult
  CountExpression::Evaluate(IEvaluationContext* context)
  {
    Object::Pointer var(context->GetDefaultVariable());
    ObjectVector<Object::Pointer>::size_type size;

    if(ObjectVector<Object::Pointer>::Pointer coll = var.Cast<ObjectVector<Object::Pointer> >())
    {
      size = coll->size();
    }
    else
    {
      ICountable::Pointer countable = Expressions::GetAsICountable(var, Expression::Pointer(this));
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
      return EvaluationResult::ValueOf(fSize == size);
      case ANY_NUMBER:
      return EvaluationResult::TRUE_EVAL;
    }

    return EvaluationResult::FALSE_EVAL;
  }

  void
  CountExpression::CollectExpressionInfo(ExpressionInfo* info)
  {
    info->MarkDefaultVariableAccessed();
  }

  bool
  CountExpression::operator==(Expression& object)
  {
    try
    {
      CountExpression& that = dynamic_cast<CountExpression&>(object);
      return (this->fMode == that.fMode) && (this->fSize == that.fSize);
    }
    catch (std::bad_cast e)
    {
      return false;
    }

  }

  std::size_t
  CountExpression::ComputeHashCode()
  {
    return HASH_INITIAL * HASH_FACTOR + fMode * HASH_FACTOR + fSize;
  }

}  // namespace berry
