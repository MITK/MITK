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

#include "berryAbstractHandler.h"

#include "berryHandlerEvent.h"

namespace berry
{

AbstractHandler::AbstractHandler() :
  baseEnabled(true)
{

}

void AbstractHandler::AddHandlerListener(IHandlerListener *handlerListener)
{
  handlerListeners.AddListener(handlerListener);
}

void AbstractHandler::Dispose()
{
  // Do nothing.
}

bool AbstractHandler::IsEnabled() const
{
  return baseEnabled;
}

void AbstractHandler::SetEnabled(const Object::Pointer& /*evaluationContext*/)
{
}

bool AbstractHandler::IsHandled() const
{
  return true;
}

void AbstractHandler::RemoveHandlerListener(IHandlerListener *handlerListener)
{
  handlerListeners.RemoveListener(handlerListener);
}

void AbstractHandler::FireHandlerChanged(const SmartPointer<HandlerEvent>& handlerEvent)
{
  if (handlerEvent.IsNull())
  {
    throw ctkInvalidArgumentException("Cannot fire a null handler event");
  }

  handlerListeners.handlerChanged(handlerEvent);
}

void AbstractHandler::SetBaseEnabled(bool state)
{
  if (baseEnabled == state)
  {
    return;
  }
  baseEnabled = state;
  HandlerEvent::Pointer event(new HandlerEvent(IHandler::Pointer(this), true, false));
  FireHandlerChanged(event);
}

bool AbstractHandler::HasListeners() const
{
  return handlerListeners.handlerChanged.HasListeners();
}

}
