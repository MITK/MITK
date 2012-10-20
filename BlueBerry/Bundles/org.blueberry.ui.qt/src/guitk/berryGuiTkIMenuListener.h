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


#ifndef BERRYGUITKIMENULISTENER_H_
#define BERRYGUITKIMENULISTENER_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <berryMessage.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

namespace GuiTk
{

/**
 * A menu listener that gets informed when a menu is about to show.
 *
 * @see MenuManager#addMenuListener
 */
struct BERRY_UI_QT IMenuListener: public virtual Object
{
  berryObjectMacro(berry::GuiTk::IMenuListener)

  struct Events {

    typedef Message<> MenuEvent;

    MenuEvent menuAboutToShow;
    MenuEvent menuAboutToHide;

    void AddListener(IMenuListener::Pointer listener);
    void RemoveListener(IMenuListener::Pointer listener);

  private:
    typedef MessageDelegate<IMenuListener> Delegate;
  };

  /**
   * Notifies this listener that the menu is about to be shown by
   * the given menu manager.
   *
   * @param manager the menu manager
   */
  virtual void MenuAboutToShow() = 0;

  /**
   * Notifies this listener that the menu is about to be hidden by
   * the given menu manager.
   *
   * @param manager the menu manager
   */
  virtual void MenuAboutToHide() = 0;
};

}

}

#endif /* BERRYGUITKIMENULISTENER_H_ */
