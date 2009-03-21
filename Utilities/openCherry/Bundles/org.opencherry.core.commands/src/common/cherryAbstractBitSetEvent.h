/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYABSTRACTBITSETEVENT_H_
#define CHERRYABSTRACTBITSETEVENT_H_

#include "cherryObject.h"
#include "cherryMacros.h"

#include "../cherryCommandsDll.h"

namespace cherry {

/**
 * <p>
 * An event that carries with it two or more boolean values.  This provides a
 * single integer value which can then be used as a bit set.
 * </p>
 *
 * @since 3.1
 */
class CHERRY_COMMANDS AbstractBitSetEvent : public virtual Object {

public:

  cherryObjectMacro(AbstractBitSetEvent)

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

#endif /* CHERRYABSTRACTBITSETEVENT_H_ */
