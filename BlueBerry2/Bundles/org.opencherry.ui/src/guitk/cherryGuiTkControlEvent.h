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

#include "cherryGuiTkEvent.h"

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

class CHERRY_UI ControlEvent: public Event
{

public:

  osgiObjectMacro(ControlEvent);



  ControlEvent();

  ControlEvent(void* item, int x = 0, int y = 0, int width = 0, int height = 0);

};

}

}

#endif /* CHERRYGUITKCONTROLEVENT_H_ */
