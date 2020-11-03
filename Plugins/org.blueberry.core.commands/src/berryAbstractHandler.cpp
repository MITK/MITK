/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
