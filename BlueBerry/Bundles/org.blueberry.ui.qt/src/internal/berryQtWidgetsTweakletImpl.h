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


#ifndef BERRYQTWIDGETSTWEAKLETIMPL_H_
#define BERRYQTWIDGETSTWEAKLETIMPL_H_

#include "berryQtWidgetController.h"

#include <berryRectangle.h>
#include <berryShell.h>
#include <berryGuiTkISelectionListener.h>

#include <QMetaType>

#include <list>
#include <vector>

Q_DECLARE_METATYPE(berry::Object::Pointer)

namespace berry {

class QtSelectionListenerWrapper : public QObject
{

  Q_OBJECT

public:

  QtSelectionListenerWrapper(QWidget* widget);

  QWidget* widget;

  void AddListener(GuiTk::ISelectionListener::Pointer listener);
  int RemoveListener(GuiTk::ISelectionListener::Pointer listener);

protected slots:

  void QAbstractButtonClicked(bool checked);

private:

  GuiTk::ISelectionListener::Events selectionEvents;

};

class QtWidgetsTweakletImpl
{
  public:

  void AddSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener);
  void RemoveSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener);

  void AddControlListener(QtWidgetController* widget, GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(QtWidgetController* widget, GuiTk::IControlListener::Pointer listener);

  bool GetEnabled(QWidget* widget);
  void SetEnabled(QWidget* widget, bool enabled);

  void SetBounds(QWidget* widget, const Rectangle& bounds);
  Rectangle GetBounds(QWidget* widget);

  void SetVisible(QWidget* widget, bool visible);
  bool GetVisible(QWidget* widget);
  bool IsVisible(QWidget* widget);

  Rectangle GetClientArea(QWidget* widget);

  void* GetParent(QWidget* widget);
  bool SetParent(QWidget* widget, QWidget* parent);

  void SetData(QWidget* widget, const std::string& id, Object::Pointer data);
  Object::Pointer GetData(QWidget* widget, const std::string& id);

  Rectangle GetScreenSize(int i = -1);
  unsigned int GetScreenNumber();
  int GetPrimaryScreenNumber();
  Rectangle GetAvailableScreenSize(int i = -1);
  int GetClosestScreenNumber(const Rectangle&);

  Point GetCursorLocation();
  QWidget* GetCursorControl();
  QWidget* FindControl(const std::vector<Shell::Pointer>& shells, const Point& location);

  /**
   * Determines if one control is a child of another. Returns true iff the second
   * argument is a child of the first (or the same object).
   *
   * @param potentialParent
   * @param childToTest
   * @return
   */
  bool IsChild(QObject* potentialParent, QObject* childToTest);

  /**
   * Returns the control which currently has keyboard focus,
   * or null if keyboard events are not currently going to
   * any of the controls built by the currently running
   * application.
   *
   * @return the control under the cursor
   */
  QWidget* GetFocusControl();

  bool IsReparentable(QWidget* widget);

  void MoveAbove(QWidget* widgetToMove, QWidget* widget);
  void MoveBelow(QWidget* widgetToMove, QWidget* widget);

  void Dispose(QWidget* widget);

  Shell::Pointer CreateShell(Shell::Pointer parent, int style);
  void DisposeShell(Shell::Pointer shell);

  QWidget* CreateComposite(QWidget* parent);

  std::vector<Shell::Pointer> GetShells();
  Shell::Pointer GetShell(QWidget* widget);
  Shell::Pointer GetActiveShell();

  Rectangle ToControl(QWidget* coordinateSystem,
          const Rectangle& toConvert);

  Point ToControl(QWidget* coordinateSystem,
          const Point& toConvert);

  Rectangle ToDisplay(QWidget* coordinateSystem,
          const Rectangle& toConvert);

  Point ToDisplay(QWidget* coordinateSystem,
          const Point& toConvert);

private:

  typedef std::map<void*, QtSelectionListenerWrapper* > SelectionListenerMap;
  SelectionListenerMap selectionListenerMap;

  static std::list<Shell::Pointer> shellList;

  friend class QtShell;

};

}

#endif /* BERRYQTWIDGETSTWEAKLETIMPL_H_ */
