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

