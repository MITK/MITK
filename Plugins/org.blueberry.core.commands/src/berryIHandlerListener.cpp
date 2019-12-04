/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIHandlerListener.h"

#include "berryIHandler.h"
#include "berryHandlerEvent.h"

namespace berry {

void
IHandlerListener::Events
::AddListener(IHandlerListener* l)
{
  if (l == nullptr) return;

  handlerChanged += Delegate(l, &IHandlerListener::HandlerChanged);
}

void
IHandlerListener::Events
::RemoveListener(IHandlerListener* l)
{
  if (l == nullptr) return;

  handlerChanged -= Delegate(l, &IHandlerListener::HandlerChanged);
}

IHandlerListener::~IHandlerListener()
{
}

}

