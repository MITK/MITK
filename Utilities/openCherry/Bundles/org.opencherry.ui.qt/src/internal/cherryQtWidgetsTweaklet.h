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


#ifndef CHERRYQTWIDGETSTWEAKLET_H_
#define CHERRYQTWIDGETSTWEAKLET_H_

#include <cherryGuiWidgetsTweaklet.h>

#include "cherryQtWidgetsTweakletImpl.h"

namespace cherry {


class QtWidgetsTweaklet : public GuiWidgetsTweaklet
{

public:

  void AddSelectionListener(void* widget, GuiTk::ISelectionListener::Pointer listener);
  void RemoveSelectionListener(void* widget, GuiTk::ISelectionListener::Pointer listener);

  void AddControlListener(void* widget, GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(void* widget, GuiTk::IControlListener::Pointer listener);

  bool GetEnabled(void* widget);
  void SetEnabled(void* widget, bool enabled);

  void SetBounds(void* widget, const Rectangle& bounds);
  Rectangle GetBounds(void* widget);

  void SetVisible(void* widget, bool visible);
  bool GetVisible(void* widget);
  bool IsVisible(void* widget);

  Rectangle GetClientArea(void* widget);

  void* GetParent(void* widget);
  bool SetParent(void* widget, void* parent);

  void SetData(void* widget, const std::string& id, Object::Pointer data);
  Object::Pointer GetData(void* widget, const std::string& id);

  //IMenu::Pointer CreateMenu(void*, IMenu::Style = IMenu::POP_UP);
  //IMenu::Pointer CreateMenu(IMenu::Pointer parent);
  //IMenuItem::Pointer CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index = -1);

  Rectangle GetScreenSize(int i = -1);
  unsigned int GetScreenNumber();
  int GetPrimaryScreenNumber();

  Point GetCursorLocation();
  void* GetCursorControl();
  void* FindControl(const std::vector<Shell::Pointer>& shells, const Point& location);

  /**
   * Determines if one control is a child of another. Returns true iff the second
   * argument is a child of the first (or the same object).
   *
   * @param potentialParent
   * @param childToTest
   * @return
   */
  bool IsChild(void* potentialParent, void* childToTest);

  /**
   * Returns the control which currently has keyboard focus,
   * or null if keyboard events are not currently going to
   * any of the controls built by the currently running
   * application.
   *
   * @return the control under the cursor
   */
  void* GetFocusControl();

  bool IsReparentable(void* widget);

  void MoveAbove(void* widgetToMove, void* widget);
  void MoveBelow(void* widgetToMove, void* widget);

  void Dispose(void* widget);

  Shell::Pointer CreateShell(Shell::Pointer parent, int style);
  void DisposeShell(Shell::Pointer shell);

  void* CreateComposite(void* parent);

  std::vector<Shell::Pointer> GetShells();
  Shell::Pointer GetShell(void* widget);
  Shell::Pointer GetActiveShell();

  Rectangle ToControl(void* coordinateSystem,
          const Rectangle& toConvert);

  Point ToControl(void* coordinateSystem,
          const Point& toConvert);

  Rectangle ToDisplay(void* coordinateSystem,
          const Rectangle& toConvert);

  Point ToDisplay(void* coordinateSystem,
          const Point& toConvert);

private:

  QtWidgetsTweakletImpl impl;

};

}

#endif /* CHERRYQTWIDGETSTWEAKLET_H_ */
