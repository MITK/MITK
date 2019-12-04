/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYGUITKSELECTIONEVENT_H_
#define BERRYGUITKSELECTIONEVENT_H_

#include "berryGuiTkEvent.h"

namespace berry
{

namespace GuiTk
{

/**
 * Instances of this class are sent as a result of
 * widgets being selected.
 *
 * @see ISelectionListener
 */

class BERRY_UI_QT SelectionEvent: public Event
{

public:

  berryObjectMacro(SelectionEvent);

  /**
   * Constructs a new instance of this class based on the
   * information in the given untyped event.
   *
   * @param item the GUI dependent widget which has been selected
   */
  SelectionEvent(QWidget* item);

};

}

}

#endif /* BERRYGUITKSELECTIONEVENT_H_ */
