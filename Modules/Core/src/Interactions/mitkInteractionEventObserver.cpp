/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInteractionEventObserver.h"

mitk::InteractionEventObserver::InteractionEventObserver() : m_IsEnabled(true)
{
}

mitk::InteractionEventObserver::~InteractionEventObserver()
{
}

void mitk::InteractionEventObserver::Disable()
{
  m_IsEnabled = false;
}

void mitk::InteractionEventObserver::Enable()
{
  m_IsEnabled = true;
}

bool mitk::InteractionEventObserver::IsEnabled() const
{
  return m_IsEnabled;
}
