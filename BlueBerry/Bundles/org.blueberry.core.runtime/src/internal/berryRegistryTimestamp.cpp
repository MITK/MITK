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
