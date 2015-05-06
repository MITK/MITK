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

#include "berryPersistentState.h"

namespace berry {

PersistentState::PersistentState()
  : persisted(false)
{}

void PersistentState::SetShouldPersist(bool persisted)
{
  this->persisted = persisted;
}

bool PersistentState::ShouldPersist()
{
  return persisted;
}

}
