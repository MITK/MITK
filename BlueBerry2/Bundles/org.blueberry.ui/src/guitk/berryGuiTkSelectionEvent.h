/*=========================================================================

 Program:   BlueBerry Platform
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

class BERRY_UI SelectionEvent: public Event
{

public:

  osgiObjectMacro(SelectionEvent);

  /**
   * Constructs a new instance of this class based on the
   * information in the given untyped event.
   *
   * @param item the GUI dependent widget which has been selected
   */
  SelectionEvent(void* item);

};

}

}

#endif /* BERRYGUITKSELECTIONEVENT_H_ */
