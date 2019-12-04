/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryObject.h"

#include "berryExtensionRegistry.h"
#include "berryRegistryObjectManager.h"

namespace berry {

// it is assumed that int has 32 bits (bits #0 to #31);
// bits #0 - #29 are the offset (limited to about 1Gb)
// bit #30 - persistance flag
// bit #31 - registry object has no extra data offset
// the bit#31 is a sign bit; bit#30 is the highest mantissa bit
const int RegistryObject::EMPTY_MASK = 0x80000000; // only taking bit #31
const int RegistryObject::PERSIST_MASK = 0x40000000; // only taking bit #30
const int RegistryObject::OFFSET_MASK = 0x3FFFFFFF; // all bits but #30, #31

RegistryObject::RegistryObject()
  : registry(nullptr), objectId(RegistryObjectManager::UNKNOWN), extraDataOffset(EMPTY_MASK)
{
  objectKey = QString::number(objectId);
}

QString RegistryObject::GetKey() const
{
  return objectKey;
}

bool RegistryObject::IsEqual(const KeyedElement& other) const
{
  return objectId == static_cast<const RegistryObject&>(other).objectId;
}

RegistryObject::RegistryObject(ExtensionRegistry* registry, bool persist)
  : registry(registry), objectId(RegistryObjectManager::UNKNOWN), extraDataOffset(EMPTY_MASK)
{
  objectKey = QString::number(objectId);
  SetPersist(persist);
}

void RegistryObject::SetRawChildren(const QList<int>& values)
{
  children = values;
}

QList<int> RegistryObject::GetRawChildren() const
{
  return children;
}

void RegistryObject::SetObjectId(int value)
{
  objectId = value;
  objectKey = QString::number(value);
}

int RegistryObject::GetObjectId() const
{
  return objectId;
}

bool RegistryObject::ShouldPersist() const
{
  return (extraDataOffset & PERSIST_MASK) == PERSIST_MASK;
}

bool RegistryObject::NoExtraData() const
{
  return (extraDataOffset & EMPTY_MASK) == EMPTY_MASK;
}

int RegistryObject::GetExtraDataOffset() const
{
  if (NoExtraData())
    return -1;
  return extraDataOffset & OFFSET_MASK;
}

void RegistryObject::SetExtraDataOffset(int offset)
{
  if (offset == -1)
  {
    extraDataOffset &= ~OFFSET_MASK; // clear all offset bits
    extraDataOffset |= EMPTY_MASK;
    return;
  }

  if ((offset & OFFSET_MASK) != offset)
    throw ctkInvalidArgumentException("Registry object: extra data offset is out of range");

  extraDataOffset &= ~(OFFSET_MASK | EMPTY_MASK); // clear all offset bits; mark as non-empty
  extraDataOffset |= (offset & OFFSET_MASK); // set all offset bits
}

QLocale RegistryObject::GetLocale() const
{
  return registry->GetLocale();
}

void RegistryObject::SetPersist(bool persist)
{
  if (persist)
    extraDataOffset |= PERSIST_MASK;
  else
    extraDataOffset &= ~PERSIST_MASK;
}

}
