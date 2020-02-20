/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _BERRY_MULTIRULE_H
#define _BERRY_MULTIRULE_H

#include "berryISchedulingRule.h"

namespace berry
{

/**
 * A MultiRule is a compound scheduling rule that represents a fixed group of child
 * scheduling rules.  A MultiRule conflicts with another rule if any of its children conflict
 * with that rule.  More formally, a compound rule represents a logical intersection
 * of its child rules with respect to the <code>isConflicting</code> equivalence
 * relation.
 * <p>
 * A MultiRule will never contain other MultiRules as children.  If a MultiRule is provided
 * as a child, its children will be added instead.
 * </p>
 *
 */
class MultiRule: public ISchedulingRule
{

  bool IsConflicting(ISchedulingRule::Pointer myRule) const override;

  bool Contains(ISchedulingRule::Pointer rule) const override;

};

}

#endif /* _BERRY_MULTIRULE_H */
