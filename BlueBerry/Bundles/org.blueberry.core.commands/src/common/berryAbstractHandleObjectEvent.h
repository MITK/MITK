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


#ifndef BERRYABSTRACTHANDLEOBJECTEVENT_H_
#define BERRYABSTRACTHANDLEOBJECTEVENT_H_

#include "berryAbstractBitSetEvent.h"

namespace berry {

/**
 * <p>
 * An event fired from a <code>NamedHandleObject</code>. This provides
 * notification of changes to the defined state, the name and the description.
 * </p>
 *
 * @since 3.2
 */
class BERRY_COMMANDS AbstractHandleObjectEvent : public AbstractBitSetEvent
{

public:
  berryObjectMacro(AbstractHandleObjectEvent);

  /**
   * Returns whether or not the defined property changed.
   *
   * @return <code>true</code>, iff the defined property changed.
   */
  bool IsDefinedChanged() const;

protected:

  /**
   * The bit used to represent whether the category has changed its defined
   * state.
   */
  static const int CHANGED_DEFINED; // = 1;

  /**
   * The last used bit so that subclasses can add more properties.
   */
  static const int LAST_BIT_USED_ABSTRACT_HANDLE; // = CHANGED_DEFINED;

  /**
   * Constructs a new instance of <code>AbstractHandleObjectEvent</code>.
   *
   * @param definedChanged
   *            <code>true</code>, iff the defined property changed.
   */
  AbstractHandleObjectEvent(bool definedChanged);

};

}

#endif /* BERRYABSTRACTHANDLEOBJECTEVENT_H_ */
