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

#ifndef CHERRYGUITKSELECTIONEVENT_H_
#define CHERRYGUITKSELECTIONEVENT_H_

#include "cherryGuiTkEvent.h"

namespace cherry
{

namespace GuiTk
{

/**
 * Instances of this class are sent as a result of
 * widgets being selected.
 *
 * @see ISelectionListener
 */

class CHERRY_UI SelectionEvent: public Event
{

public:

  cherryObjectMacro(SelectionEvent);

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

#endif /* CHERRYGUITKSELECTIONEVENT_H_ */
