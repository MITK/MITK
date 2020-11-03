/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYTIMESTAMP_H
#define BERRYREGISTRYTIMESTAMP_H

namespace berry {

/**
 * Aggregated registry timestamp. Corresponds to the current contents of the registry.
 * <p>
 * This class may be instantiated.
 * </p><p>
 * This class is not indended to be subclassed.
 */
class RegistryTimestamp
{

private:

  /**
   * Current aggregated timestamp
   */
  long aggregateTimestamp;

  bool modified;

public:

  /**
   * Public constructor.
   */
  RegistryTimestamp();

  /**
   * Returns value of the aggregated timestamp.
   * @return value of the aggregated timestamp
   */
  long GetContentsTimestamp() const;

  /**
   * Set value of the aggregated timestamp.
   * @param timestamp the aggregated timestamp of the current registry contents
   */
  void Set(long timestamp);

  /**
   * Sets aggregated timestamp to the value corresponding to an empty registry.
   */
  void Reset();

  /**
   * Determines if the aggregate timestamp was modified using add() or remove()
   * methods.
   * @return true: the timestamp was modified after the last set/reset
   */
  bool IsModifed() const;

  /**
   * Add individual contribution timestamp to the aggregated timestamp.
   * @param timestamp the time stamp of the contribution being added to the registry
   */
  void Add(long timestamp);

  /**
   * Remove individual contribution timestamp from the aggregated timestamp.
   * @param timestamp the time stamp of the contribution being removed from the registry
   */
  void Remove(long timestamp);
};

}

#endif // BERRYREGISTRYTIMESTAMP_H
