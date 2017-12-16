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
