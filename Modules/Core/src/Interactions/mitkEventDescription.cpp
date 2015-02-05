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


#include "mitkEventDescription.h"

mitk::EventDescription::EventDescription(int type, int button, int buttonState, int key, std::string name, int id)
: Event(NULL, type, button, buttonState, key), m_Name(name), m_Id(id)
{}

std::string mitk::EventDescription::GetName() const
{
  return m_Name;
}

int mitk::EventDescription::GetId() const
{
  return m_Id;
}

