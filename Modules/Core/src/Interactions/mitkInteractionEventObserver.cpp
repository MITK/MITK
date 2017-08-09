/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

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
