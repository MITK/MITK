/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
