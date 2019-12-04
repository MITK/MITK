/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerEvent.h"
#include "berryIHandler.h"

namespace berry
{

const int HandlerEvent::CHANGED_ENABLED = 1;
const int HandlerEvent::CHANGED_HANDLED = 1 << 1;

HandlerEvent::HandlerEvent(const IHandler::Pointer handler, bool enabledChanged,
    bool handledChanged) :
  handler(handler)
{
  if (!handler)
  {
    throw Poco::NullPointerException("Handler cannot be null");
  }

  if (enabledChanged)
  {
    changedValues |= CHANGED_ENABLED;
  }
  if (handledChanged)
  {
    changedValues |= CHANGED_HANDLED;
  }
}

SmartPointer<IHandler> HandlerEvent::GetHandler() const
{
  return handler;
}

bool HandlerEvent::IsEnabledChanged() const
{
  return ((changedValues & CHANGED_ENABLED) != 0);
}

bool HandlerEvent::IsHandledChanged() const
{
  return ((changedValues & CHANGED_HANDLED) != 0);
}

}

