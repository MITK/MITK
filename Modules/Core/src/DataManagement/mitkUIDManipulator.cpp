/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkUIDManipulator.h>
#include <mitkExceptionMacro.h>
#include <mitkIdentifiable.h>

mitk::UIDManipulator::UIDManipulator(Identifiable *identifiable)
  : m_Identifiable(identifiable)
{
  if (nullptr == m_Identifiable)
    mitkThrow() << "Invalid identifiable object.";
}

mitk::UIDManipulator::~UIDManipulator()
{
}

void mitk::UIDManipulator::SetUID(const Identifiable::UIDType &uid)
{
  m_Identifiable->SetUID(uid);
}
