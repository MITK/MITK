/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryTimestamp.h"

namespace berry {

RegistryTimestamp::RegistryTimestamp()
{
  Reset();
}

long RegistryTimestamp::GetContentsTimestamp() const
{
  return aggregateTimestamp;
}

void RegistryTimestamp::Set(long timestamp)
{
  aggregateTimestamp = timestamp;
  modified = false;
}

void RegistryTimestamp::Reset()
{
  aggregateTimestamp = 0;
  modified = false;
}

bool RegistryTimestamp::IsModifed() const
{
  return modified;
}

void RegistryTimestamp::Add(long timestamp)
{
  aggregateTimestamp ^= timestamp;
  modified = true;
}

void RegistryTimestamp::Remove(long timestamp)
{
  aggregateTimestamp ^= timestamp;
  modified = true;
}

}
