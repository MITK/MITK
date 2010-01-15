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


#ifndef CHERRYABSTRACTNAMEDHANDLEEVENT_H_
#define CHERRYABSTRACTNAMEDHANDLEEVENT_H_

#include "cherryAbstractHandleObjectEvent.h"

namespace cherry {

/**
 * <p>
 * An event fired from a <code>NamedHandleObject</code>. This provides
 * notification of changes to the defined state, the name and the description.
 * </p>
 *
 */
class CHERRY_COMMANDS AbstractNamedHandleEvent : public AbstractHandleObjectEvent {

public:

  cherryObjectMacro(AbstractNamedHandleEvent)

  /**
   * Returns whether or not the description property changed.
   *
   * @return <code>true</code>, iff the description property changed.
   */
  bool IsDescriptionChanged() const;

  /**
   * Returns whether or not the name property changed.
   *
   * @return <code>true</code>, iff the name property changed.
   */
  bool IsNameChanged() const;

protected:

  /**
   * The bit used to represent whether the category has changed its
   * description.
   */
  static const int CHANGED_DESCRIPTION; // = LAST_BIT_USED_ABSTRACT_HANDLE << 1;

  /**
   * The bit used to represent whether the category has changed its name.
   */
  static const int CHANGED_NAME; // = LAST_BIT_USED_ABSTRACT_HANDLE << 2;

  /**
   * The last used bit so that subclasses can add more properties.
   */
  static const int LAST_USED_BIT; // = CHANGED_NAME;

  /**
   * Constructs a new instance of <code>AbstractHandleObjectEvent</code>.
   *
   * @param definedChanged
   *            <code>true</code>, iff the defined property changed.
   * @param descriptionChanged
   *            <code>true</code>, iff the description property changed.
   * @param nameChanged
   *            <code>true</code>, iff the name property changed.
   */
  AbstractNamedHandleEvent(bool definedChanged,
      bool descriptionChanged, bool nameChanged);

};

}

#endif /* CHERRYABSTRACTNAMEDHANDLEEVENT_H_ */
