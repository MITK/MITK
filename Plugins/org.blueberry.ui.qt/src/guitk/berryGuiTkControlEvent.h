/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

class BERRY_UI_QT ControlEvent: public Event
{

public:

  berryObjectMacro(ControlEvent);



  ControlEvent();

  ControlEvent(QWidget* item, int x = 0, int y = 0, int width = 0, int height = 0);

};

}

}

#endif /* BERRYGUITKCONTROLEVENT_H_ */
