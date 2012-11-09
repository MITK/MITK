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

namespace berry
{

AbstractHandler::AbstractHandler() :
  baseEnabled(true)
{

}

bool AbstractHandler::IsEnabled()
{
  return baseEnabled;
}

bool AbstractHandler::IsHandled()
{
  return true;
}

void AbstractHandler::SetBaseEnabled(bool state)
{
  if (baseEnabled == state)
  {
    return;
  }
  baseEnabled = state;
  //fireHandlerChanged(new HandlerEvent(this, true, false));
}

}
