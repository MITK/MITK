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


#ifndef CHERRYIMENU_H_
#define CHERRYIMENU_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../cherryUiDll.h"

#include <vector>

namespace cherry {

namespace GuiTk {

struct IMenuListener;

}

struct IMenuItem;

/**
 * Instances of this class are user interface objects that contain
 * menu items.
 * <dl>
 * <dt><b>Styles:</b></dt>
 * <dd>BAR, DROP_DOWN, POP_UP, NO_RADIO_GROUP</dd>
 * </dl>
 * <p>
 * Note: Only one of BAR, DROP_DOWN and POP_UP may be specified.
 * Only one of LEFT_TO_RIGHT or RIGHT_TO_LEFT may be specified.
 * </p>
 */
struct CHERRY_UI IMenu : public Object {

  enum CHERRY_UI Style {
    BAR       = 0x00000001,
    DROP_DOWN = 0x00000002,
    POP_UP    = 0x00000004
  };

  osgiInterfaceMacro(cherry::IMenu)

  virtual void Dispose() = 0;
  virtual bool IsDisposed() = 0;

  virtual SmartPointer<IMenuItem> GetParentItem() = 0;
  virtual SmartPointer<IMenuItem> GetItem(unsigned int index) = 0;
  virtual std::vector<SmartPointer<IMenuItem> > GetItems() = 0;
  virtual unsigned int GetItemCount() = 0;
  virtual Pointer GetParentMenu() = 0;
  virtual Style GetStyle() = 0;

  virtual void AddMenuListener(SmartPointer<GuiTk::IMenuListener> listener) = 0;
  virtual void RemoveMenuListener(SmartPointer<GuiTk::IMenuListener> listener) = 0;
};

}

#endif /* CHERRYIMENU_H_ */
