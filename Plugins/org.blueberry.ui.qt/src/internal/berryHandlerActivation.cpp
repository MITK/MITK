/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerActivation.h"

#include "berryISources.h"
#include "berryIHandler.h"
#include "berryExpression.h"

namespace berry {

HandlerActivation::HandlerActivation(const QString& commandId, const SmartPointer<IHandler>& handler,
                                     const SmartPointer<Expression>& expression, int depth,
                                     IHandlerService* handlerService)
  : EvaluationResultCache(expression)
  , commandId(commandId)
  , depth(depth)
  , handler(handler)
  , handlerService(handlerService)
{
  if (commandId.isNull())
  {
    throw ctkInvalidArgumentException(
        "The command identifier for a handler activation cannot be null");
  }

  if (handlerService == nullptr)
  {
    throw ctkInvalidArgumentException("The handler service for an activation cannot be null");
  }
}

void HandlerActivation::ClearActive()
{
  ClearResult();
}

bool HandlerActivation::operator<(const Object* object) const
{
  const HandlerActivation* activation = dynamic_cast<const HandlerActivation*>(object);
  return this->CompareTo(activation) < 0;
}

int berry::HandlerActivation::CompareTo(const HandlerActivation* other) const
{
  const IHandlerActivation* const activation = other;
  int difference = 0;

  // Check the priorities
  int thisPriority = this->GetSourcePriority();
  int thatPriority = activation->GetSourcePriority();

  // rogue bit problem - ISources.ACTIVE_MENU
  int thisLsb = 0;
  int thatLsb = 0;

  if (((thisPriority & ISources::ACTIVE_MENU()) | (thatPriority & ISources::ACTIVE_MENU())) != 0)
  {
    thisLsb = thisPriority & 1;
    thisPriority = (thisPriority >> 1) & 0x7fffffff;
    thatLsb = thatPriority & 1;
    thatPriority = (thatPriority >> 1) & 0x7fffffff;
  }

  difference = thisPriority - thatPriority;
  if (difference != 0) {
    return difference;
  }

  // if all of the higher bits are the same, check the
  // difference of the LSB
  difference = thisLsb - thatLsb;
  if (difference != 0) {
    return difference;
  }

  // Check depth
  const int thisDepth = this->GetDepth();
  const int thatDepth = activation->GetDepth();
  difference = thisDepth - thatDepth;
  return difference;
}

QString HandlerActivation::GetCommandId() const
{
  return commandId;
}

int HandlerActivation::GetDepth() const
{
  return depth;
}

SmartPointer<IHandler> HandlerActivation::GetHandler() const
{
  return handler;
}

IHandlerService* HandlerActivation::GetHandlerService() const
{
  return handlerService;
}

QString HandlerActivation::ToString() const
{
  QString str;
  QDebug buffer(&str);

  const Expression::Pointer exp = GetExpression();
  buffer << "HandlerActivation(commandId=" << commandId << ',';
  buffer << "\thandler=" << (handler.IsNull() ? QString("") : handler->ToString()) << ',';
  buffer << "\texpression=" << (exp.IsNull() ? QString("") : exp->ToString())
         << ",sourcePriority=" << GetSourcePriority() << ')';

  return str;
}

SmartPointer<IEvaluationReference> HandlerActivation::GetReference()
{
  return reference;
}

void HandlerActivation::SetReference(const SmartPointer<IEvaluationReference>& reference)
{
  this->reference = reference;
}

void HandlerActivation::SetListener(IPropertyChangeListener* listener)
{
  this->listener.reset(listener);
}

IPropertyChangeListener* HandlerActivation::GetListener() const
{
  return listener.data();
}

}
