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

#include <cherryMacros.h>

#include "../cherryUiDll.h"

namespace cherry
{

namespace GuiTk
{

/**
 * Instances of this class are sent as a result of
 * widgets being selected.
 * <p>
 * Note: The fields that are filled in depend on the widget.
 * </p>
 *
 * @see SelectionListener
 * @see <a href="http://www.opencherry.org/swt/">Sample code and further information</a>
 */

class CHERRY_UI SelectionEvent: public Object
{

public:

  cherryClassMacro(SelectionEvent);

  /**
   * The item that was selected.
   */
  void* item;

  /**
   * Extra detail information about the selection, depending on the widget.
   *
   * <p><b>Sash</b><ul>
   * <li>{@link org.opencherry.swt.SWT#DRAG}</li>
   * </ul></p><p><b>ScrollBar and Slider</b><ul>
   * <li>{@link org.opencherry.swt.SWT#DRAG}</li>
   * <li>{@link org.opencherry.swt.SWT#HOME}</li>
   * <li>{@link org.opencherry.swt.SWT#END}</li>
   * <li>{@link org.opencherry.swt.SWT#ARROW_DOWN}</li>
   * <li>{@link org.opencherry.swt.SWT#ARROW_UP}</li>
   * <li>{@link org.opencherry.swt.SWT#PAGE_DOWN}</li>
   * <li>{@link org.opencherry.swt.SWT#PAGE_UP}</li>
   * </ul></p><p><b>Table and Tree</b><ul>
   * <li>{@link org.opencherry.swt.SWT#CHECK}</li>
   * </ul></p><p><b>Text</b><ul>
   * <li>{@link org.opencherry.swt.SWT#CANCEL}</li>
   * </ul></p><p><b>CoolItem and ToolItem</b><ul>
   * <li>{@link org.opencherry.swt.SWT#ARROW}</li>
   * </ul></p>
   */
  int detail;

  /**
   * The x location of the selected area.
   */
  int x;

  /**
   * The y location of selected area.
   */
  int y;

  /**
   * The width of selected area.
   */
  int width;

  /**
   * The height of selected area.
   */
  int height;

  /**
   * The state of the keyboard modifier keys at the time
   * the event was generated.
   */
  int stateMask;

  /**
   * The text of the hyperlink that was selected.
   * This will be either the text of the hyperlink or the value of its HREF,
   * if one was specified.
   *
   * @see org.opencherry.swt.widgets.Link#setText(String)
   * @since 3.1
   */
  std::string text;

  /**
   * A flag indicating whether the operation should be allowed.
   * Setting this field to <code>false</code> will cancel the
   * operation, depending on the widget.
   */
  bool doit;

  /**
   * Constructs a new instance of this class based on the
   * information in the given untyped event.
   *
   * @param e the untyped event containing the information
   */
  SelectionEvent(void* item);

  /**
   * Returns a string containing a concise, human-readable
   * description of the receiver.
   *
   * @return a string representation of the event
   */
  std::string ToString();

};

}

}

#endif /* CHERRYGUITKSELECTIONEVENT_H_ */
