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

#ifndef BERRYGUITKCONTROLEVENT_H_
#define BERRYGUITKCONTROLEVENT_H_

#include "berryGuiTkEvent.h"

namespace berry
{

namespace GuiTk
{

/**
 * Instances of this class are sent as a result of
 * controls being moved or resized.
 *
 * @see ControlListener
 * @see <a href="http://www.blueberry.org/swt/">Sample code and further information</a>
 */

class BERRY_UI ControlEvent: public Event
{

public:

  berryObjectMacro(ControlEvent);



  ControlEvent();

  ControlEvent(void* item, int x = 0, int y = 0, int width = 0, int height = 0);

};

}

}

#endif /* BERRYGUITKCONTROLEVENT_H_ */
