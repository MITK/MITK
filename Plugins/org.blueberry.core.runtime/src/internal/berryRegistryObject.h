/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYOBJECT_H
#define BERRYREGISTRYOBJECT_H

#include "berryKeyedElement.h"

namespace berry {

class ExtensionRegistry;

/**
 * An object which has the general characteristics of all the nestable elements
 * in a plug-in manifest.
 */
class RegistryObject : public KeyedElement
{

public:

  berryObjectMacro(berry::RegistryObject);

  RegistryObject();

  //Implementation of the KeyedElement interface
  QString GetKey() const override;

  int GetObjectId() const;

  //This can not return null. It returns the singleton empty array or an array
  QList<int> GetRawChildren() const;

  bool NoExtraData() const;

protected:

  friend class RegistryObjectManager;
  friend class ExtensionRegistry;
  friend class ExtensionsParser;

  QList<int> children;

  // it is assumed that int has 32 bits (bits #0 to #31);
  // bits #0 - #29 are the offset (limited to about 1Gb)
  // bit #30 - persistance flag
  // bit #31 - registry object has no extra data offset
  // the bit#31 is a sign bit; bit#30 is the highest mantissa bit
  static const int EMPTY_MASK; // = 0x80000000; // only taking bit #31
  static const int PERSIST_MASK; // = 0x40000000; // only taking bit #30
  static const int OFFSET_MASK; // = 0x3FFFFFFF; // all bits but #30, #31

  //The registry that owns this object
  ExtensionRegistry* registry;

  RegistryObject(ExtensionRegistry* registry, bool persist);

  void SetRawChildren(const QList<int>& values);

  void SetObjectId(int value);

  bool ShouldPersist() const;

  // Convert no extra data to -1 on output
  int GetExtraDataOffset() const;

  // Accept -1 as "no extra data" on input
  void SetExtraDataOffset(int offset);

  QLocale GetLocale() const;

private:

  //Object identifier
  uint objectId; // = RegistryObjectManager::UNKNOWN;
  // The key
  QString objectKey;

  // The field combines offset, persistence flag, and no offset flag
  int extraDataOffset; // = EMPTY_MASK;

  void SetPersist(bool persist);

  bool IsEqual(const KeyedElement& other) const override;
};

}

#endif // BERRYREGISTRYOBJECT_H
