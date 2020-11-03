/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYOBJECTREFERENCEMAP_H
#define BERRYREGISTRYOBJECTREFERENCEMAP_H

#include <berrySmartPointer.h>

#include <QHash>

namespace berry {

class RegistryObject;

class RegistryObjectReferenceMap
{
public:

  enum ReferenceType {
    /**
     *  Constant indicating that hard references should be used.
     */
    HARD = 0,

    /**
     *  Constant indiciating that soft references should be used.
     */
    SOFT = 1
  };

  /**
   *  Constructs a new <Code>ReferenceMap</Code> with the
   *  specified reference type, load factor and initial
   *  capacity.
   *
   *  @param referenceType  the type of reference to use for values;
   *   must be {@link #HARD} or {@link #SOFT}
   *  @param capacity  the initial capacity for the map
   */
  RegistryObjectReferenceMap(ReferenceType referenceType, int capacity);

  ~RegistryObjectReferenceMap();

  /**
   *  Returns the value associated with the given key, if any.
   *
   *  @return the value associated with the given key, or <Code>null</Code>
   *   if the key maps to no value
   */
  SmartPointer<RegistryObject> Get(int key) const;

  /**
   *  Associates the given key with the given value.<P>
   *  The value may be null.
   *
   *  @param key  the key of the mapping
   *  @param value  the value of the mapping
   *  @throws ctkInvalidArgumentException if either the key or value
   *   is null
   */
  void Put(int key, const SmartPointer<RegistryObject>& value);

  /**
   *  Removes the key and its associated value from this map.
   *
   *  @param key  the key to remove
   *  @return the value associated with that key, or null if
   *   the key was not in the map
   */
  SmartPointer<RegistryObject> Remove(int key);

private:

  /**
   * The common interface for all elements in the map.  Both
   * smart and weak pointer map values conform to this interface.
   */
  struct IEntry;

  /**
   * IEntry implementation that acts as a hard reference.
   * The value of a hard reference entry is never garbage
   * collected until it is explicitly removed from the map.
   */
  class SmartRef;

  /**
   * IEntry implementation that acts as a weak reference.
   */
  class WeakRef;

  typedef QHash<int, IEntry*> ReferenceMapType;
  mutable ReferenceMapType references;
  ReferenceType valueType;

  /**
   * Constructs a new table entry for the given data
   *
   * @param key The entry key
   * @param value The entry value
   * @param next The next value in the entry's collision chain
   * @return The new table entry
   */
  IEntry* NewEntry(const SmartPointer<RegistryObject>& value) const;

  /**
   *  Purges stale mappings from this map.<P>
   *
   *  Ordinarily, stale mappings are only removed during
   *  a write operation; typically a write operation will
   *  occur often enough that you'll never need to manually
   *  invoke this method.<P>
   *
   *  Note that this method is not synchronized!  Special
   *  care must be taken if, for instance, you want stale
   *  mappings to be removed on a periodic basis by some
   *  background thread.
   */
  void Purge() const;

  /**
   * @param key The key to remove
   * @param cleanup true if doing map maintenance; false if it is a real request to remove
   * @return The removed map value
   */
  SmartPointer<RegistryObject> DoRemove(int key, bool cleanup);
};

}

#endif // BERRYREGISTRYOBJECTREFERENCEMAP_H
