/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYABSTRACTBITSETEVENT_H_
#define BERRYABSTRACTBITSETEVENT_H_

#include "berryObject.h"
#include "berryMacros.h"

#include <org_blueberry_core_commands_Export.h>

namespace berry {

/**
 * <p>
 * An event that carries with it two or more boolean values.  This provides a
 * single integer value which can then be used as a bit set.
 * </p>
 */
class BERRY_COMMANDS AbstractBitSetEvent : public virtual Object {

public:

  berryObjectMacro(AbstractBitSetEvent);

protected:

  AbstractBitSetEvent();

  /**
   * A collection of bits representing whether certain values have changed. A
   * bit is set (i.e., <code>1</code>) if the corresponding property has
   * changed. It can be assumed that this value will be correctly initialized
   * by the superconstructor.
   */
  int changedValues;
};

}

#endif /* BERRYABSTRACTBITSETEVENT_H_ */
