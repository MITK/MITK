/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIEXTENSIONPOINTFILTER_H
#define BERRYIEXTENSIONPOINTFILTER_H

#include <org_blueberry_core_runtime_Export.h>

#include <memory>

namespace berry {

struct IExtensionPoint;

/**
 * A filter compares the given object to some pattern and returns
 * <code>true</code> if the two match and <code>false</code> otherwise.
 * <p>
 * This interface may be implemented by clients, however factory methods are
 * available on IExtensionTracker.
 * </p>
 */
struct org_blueberry_core_runtime_EXPORT IExtensionPointFilter
{
  struct org_blueberry_core_runtime_EXPORT Concept {
    virtual bool Matches(const IExtensionPoint* target) const = 0;

    virtual ~Concept();
  };

  IExtensionPointFilter(const Concept* concept);

  bool IsNull() const;

  const Concept* GetConcept() const;

  /**
   * Return <code>true</code> if the given object matches the criteria
   * for this filter.
   *
   * @param target the object to match
   * @return <code>true</code> if the target matches this filter
   *  and <code>false</code> otherwise
   */
  bool Matches(const IExtensionPoint* target) const;

private:

  std::shared_ptr<const Concept> m_Self;
};

}

#endif // BERRYIEXTENSIONPOINTFILTER_H
