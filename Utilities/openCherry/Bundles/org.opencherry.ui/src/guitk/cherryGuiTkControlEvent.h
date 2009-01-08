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

#ifndef CHERRYGUITKCONTROLEVENT_H_
#define CHERRYGUITKCONTROLEVENT_H_

#include <cherryMacros.h>
#include <cherryObject.h>

namespace cherry
{

namespace GuiTk
{

/**
 * Instances of this class are sent as a result of
 * controls being moved or resized.
 *
 * @see ControlListener
 * @see <a href="http://www.opencherry.org/swt/">Sample code and further information</a>
 */

class ControlEvent: public Object
{ //extends TypedEvent {

public:

  cherryClassMacro(ControlEvent);

  /**
   * The item that was moved, resized, or activated.
   */
  void* item;

  /**
   * The new x location
   */
  int x;

  /**
   * The new y location
   */
  int y;

  /**
   * The new width
   */
  int width;

  /**
   * The new height
   */
  int height;

  ControlEvent() : item(0), x(0), y(0), width(0), height(0)
  {
  }

  ControlEvent(void* item, int x = 0, int y = 0, int width = 0, int height = 0) :
    item(item), x(x), y(y), width(width), height(height)
  {
  }

};

}

}

#endif /* CHERRYGUITKCONTROLEVENT_H_ */
