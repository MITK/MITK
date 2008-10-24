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


#ifndef CHERRYCONSTANTS_H_
#define CHERRYCONSTANTS_H_

#include "cherryUiDll.h"

namespace cherry
{

struct CHERRY_UI Constants
{

  static const int DEFAULT; // = -1

  /**
   * Indicates that a user-interface component is being dragged,
   * for example dragging the thumb of a scroll bar (value is 1).
   */
  static const int DRAG; // = 1;

  /**
   * Style constant for horizontal alignment or orientation behavior (value is 1&lt;&lt;8).
   * <p><b>Used By:</b><ul>
   * <li><code>Label</code></li>
   * <li><code>ProgressBar</code></li>
   * <li><code>Sash</code></li>
   * <li><code>Scale</code></li>
   * <li><code>ScrollBar</code></li>
   * <li><code>Slider</code></li>
   * <li><code>ToolBar</code></li>
   * <li><code>FillLayout</code> type</li>
   * <li><code>RowLayout</code> type</li>
   * </ul></p>
   */
  static const int HORIZONTAL; // = 1 << 8;


  /**
   * Style constant for vertical alignment or orientation behavior (value is 1&lt;&lt;9).
   * <p><b>Used By:</b><ul>
   * <li><code>Label</code></li>
   * <li><code>ProgressBar</code></li>
   * <li><code>Sash</code></li>
   * <li><code>Scale</code></li>
   * <li><code>ScrollBar</code></li>
   * <li><code>Slider</code></li>
   * <li><code>ToolBar</code></li>
   * <li><code>CoolBar</code></li>
   * <li><code>FillLayout</code> type</li>
   * <li><code>RowLayout</code> type</li>
   * </ul></p>
   */
  static const int VERTICAL; // = 1 << 9;

  /**
   * Style constant for minimize box trim (value is 1&lt;&lt;7).
   * <p><b>Used By:</b><ul>
   * <li><code>Decorations</code> and subclasses</li>
   * </ul></p>
   */
  static const int MIN; // = 1 << 7;

  /**
   * Style constant for maximize box trim (value is 1&lt;&lt;10).
   * <p><b>Used By:</b><ul>
   * <li><code>Decorations</code> and subclasses</li>
   * </ul></p>
   */
  static const int MAX; // = 1 << 10;

  /**
   * Style constant for align up behavior (value is 1&lt;&lt;7,
   * since align UP and align TOP are considered the same).
   * <p><b>Used By:</b><ul>
   * <li><code>Button</code> with <code>ARROW</code> style</li>
   * <li><code>Tracker</code></li>
   * <li><code>Table</code></li>
   * <li><code>Tree</code></li>
   * </ul></p>
   */
  static const int UP; // = 1 << 7;

  /**
   * Style constant for align top behavior (value is 1&lt;&lt;7,
   * since align UP and align TOP are considered the same).
   * <p><b>Used By:</b><ul>
   * <li><code>FormAttachment</code> in a <code>FormLayout</code></li>
   * </ul></p>
   */
  static const int TOP; // = UP;

  /**
   * Style constant for align down behavior (value is 1&lt;&lt;10,
   * since align DOWN and align BOTTOM are considered the same).
   * <p><b>Used By:</b><ul>
   * <li><code>Button</code> with <code>ARROW</code> style</li>
   * <li><code>Tracker</code></li>
   * <li><code>Table</code></li>
   * <li><code>Tree</code></li>
   * </ul></p>
   */
  static const int DOWN; // = 1 << 10;

  /**
   * Style constant for align bottom behavior (value is 1&lt;&lt;10,
   * since align DOWN and align BOTTOM are considered the same).
   * <p><b>Used By:</b><ul>
   * <li><code>FormAttachment</code> in a <code>FormLayout</code></li>
   * </ul></p>
   */
  static const int BOTTOM; // = DOWN;

  /**
   * Style constant for leading alignment (value is 1&lt;&lt;14).
   * <p><b>Used By:</b><ul>
   * <li><code>Button</code></li>
   * <li><code>Label</code></li>
   * <li><code>TableColumn</code></li>
   * <li><code>Tracker</code></li>
   * <li><code>FormAttachment</code> in a <code>FormLayout</code></li>
   * </ul></p>
   *
   * @since 2.1.2
   */
  static const int LEAD; // = 1 << 14;

  /**
   * Style constant for align left behavior (value is 1&lt;&lt;14).
   * This is a synonym for LEAD (value is 1&lt;&lt;14).  Newer
   * applications should use LEAD instead of LEFT to make code more
   * understandable on right-to-left platforms.
   */
  static const int LEFT; // = LEAD;

  /**
   * Style constant for trailing alignment (value is 1&lt;&lt;17).
   * <p><b>Used By:</b><ul>
   * <li><code>Button</code></li>
   * <li><code>Label</code></li>
   * <li><code>TableColumn</code></li>
   * <li><code>Tracker</code></li>
   * <li><code>FormAttachment</code> in a <code>FormLayout</code></li>
   * </ul></p>
   *
   * @since 2.1.2
   */
  static const int TRAIL; // = 1 << 17;

  /**
   * Style constant for align right behavior (value is 1&lt;&lt;17).
   * This is a synonym for TRAIL (value is 1&lt;&lt;17).  Newer
   * applications should use TRAIL instead of RIGHT to make code more
   * understandable on right-to-left platforms.
   */
  static const int RIGHT; // = TRAIL;

  /**
   * Style constant for vertical alignment or orientation behavior (value is 4).
   */
  static const int FILL; // = 4;

  /**
   * Style constant for automatic line wrap behavior (value is 1&lt;&lt;6).
   */
  static const int WRAP; // = 1 << 6;

  /**
   * Style constant for bordered behavior (value is 1&lt;&lt;11).
   * <br>Note that this is a <em>HINT</em>.
   * <p><b>Used By:</b><ul>
   * <li><code>Control</code> and subclasses</li>
   * </ul></p>
   */
  static const int BORDER; // = 1 << 11;

  /**
   * Style constant for close box trim (value is 1&lt;&lt;6,
   * since we do not distinguish between CLOSE style and MENU style).
   * <p><b>Used By:</b><ul>
   * <li><code>Decorations</code> and subclasses</li>
   * </ul></p>
   */
  static const int CLOSE; // = 1 << 6;

  /**
   * Style constant for resize box trim (value is 1&lt;&lt;4).
   * <p><b>Used By:</b><ul>
   * <li><code>Decorations</code> and subclasses</li>
   * <li><code>Tracker</code></li>
   * </ul></p>
   */
  static const int RESIZE; // = 1 << 4;

  /**
   * Style constant for title area trim (value is 1&lt;&lt;5).
   * <p><b>Used By:</b><ul>
   * <li><code>Decorations</code> and subclasses</li>
   * </ul></p>
   */
  static const int TITLE; // = 1 << 5;

  /**
   * Trim style convenience constant for the most common top level shell appearance
   * (value is CLOSE|TITLE|MIN|MAX|RESIZE).
   * <p><b>Used By:</b><ul>
   * <li><code>Shell</code></li>
   * </ul></p>
   */
  static const int SHELL_TRIM; // = CLOSE | TITLE | MIN | MAX | RESIZE;

  /**
   * Trim style convenience constant for the most common dialog shell appearance
   * (value is CLOSE|TITLE|BORDER).
   * <p><b>Used By:</b><ul>
   * <li><code>Shell</code></li>
   * </ul></p>
   */
  static const int DIALOG_TRIM; // = TITLE | CLOSE | BORDER;

  /**
   * Style constant for modeless behavior (value is 0).
   * <br>Note that this is a <em>HINT</em>.
   * <p><b>Used By:</b><ul>
   * <li><code>Dialog</code></li>
   * <li><code>Shell</code></li>
   * </ul></p>
   */
  static const int MODELESS; // = 0;

  /**
   * Style constant for primary modal behavior (value is 1&lt;&lt;15).
   * <br>Note that this is a <em>HINT</em>.
   * <p><b>Used By:</b><ul>
   * <li><code>Dialog</code></li>
   * <li><code>Shell</code></li>
   * </ul></p>
   */
  static const int PRIMARY_MODAL; // = 1 << 15;

  /**
   * Style constant for application modal behavior (value is 1&lt;&lt;16).
   * <br>Note that this is a <em>HINT</em>.
   * <p><b>Used By:</b><ul>
   * <li><code>Dialog</code></li>
   * <li><code>Shell</code></li>
   * </ul></p>
   */
  static const int APPLICATION_MODAL; // = 1 << 16;

  /**
   * Style constant for system modal behavior (value is 1&lt;&lt;17).
   * <br>Note that this is a <em>HINT</em>.
   * <p><b>Used By:</b><ul>
   * <li><code>Dialog</code></li>
   * <li><code>Shell</code></li>
   * </ul></p>
   */
  static const int SYSTEM_MODAL; // = 1 << 17;
};

}

#endif /* CHERRYCONSTANTS_H_ */
