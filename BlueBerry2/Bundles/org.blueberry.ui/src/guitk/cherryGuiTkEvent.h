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


#ifndef CHERRYGUITKEVENT_H_
#define CHERRYGUITKEVENT_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../cherryUiDll.h"

namespace cherry
{

namespace GuiTk
{

using namespace osgi::framework;

/**
 * Instances of this class are sent as a result of
 * GUI events.
 * <p>
 * Note: The fields that are filled in depend on the widget.
 * </p>
 *
 */

class CHERRY_UI Event: public Object
{

public:

  osgiObjectMacro(Event);

  Event();

  /**
   * the widget that issued the event
   */
  void* item;

  /**
   * the event specific detail field, as defined by the detail constants
   * in class <code>Constants</code>
   */
  int detail;

  /**
   * depending on the event type, the x offset of the bounding
   * rectangle of the region that requires painting or the
   * widget-relative, x coordinate of the pointer at the
   * time the mouse button was pressed or released
   */
  int x;

  /**
   * depending on the event type, the y offset of the bounding
   * rectangle of the  region that requires painting or the
   * widget-relative, y coordinate of the pointer at the
   * time the mouse button was pressed or released
   */
  int y;

  /**
   * the width of the bounding rectangle of the
   * region that requires painting
   */
  int width;

  /**
   * the height of the bounding rectangle of the
   * region that requires painting
   */
  int height;

  /**
   * the button that was pressed or released; 1 for the
   * first button, 2 for the second button, and 3 for the
   * third button, etc.
   */
  int button;

  /**
   * depending on the event, the character represented by the key
   * that was typed.  This is the final character that results
   * after all modifiers have been applied.  For example, when the
   * user types Ctrl+A, the character value is 0x01 (ASCII SOH).
   * It is important that applications do not attempt to modify the
   * character value based on a stateMask (such as SWT.CTRL) or the
   * resulting character will not be correct.
   */
  char character;

  /**
   * depending on the event, the key code of the key that was typed,
   * as defined by the key code constants in class <code>SWT</code>.
   * When the character field of the event is ambiguous, this field
   * contains the unaffected value of the original character.  For
   * example, typing Ctrl+M or Enter both result in the character '\r'
   * but the keyCode field will also contain '\r' when Enter was typed
   * and 'm' when Ctrl+M was typed.
   */
  int keyCode;

  /**
   * depending on the event, the state of the keyboard modifier
   * keys and mouse masks at the time the event was generated.
   */
  int stateMask;

  /**
   * depending on the event, the new text that will be inserted.
   * Setting this field will change the text that is about to
   * be inserted or deleted.
   */
  std::string text;

  /**
   * depending on the event, a flag indicating whether the operation
   * should be allowed.  Setting this field to false will cancel the
   * operation.
   */
  bool doit;

  std::string ToString();

};

}

}

#endif /* CHERRYGUITKEVENT_H_ */
