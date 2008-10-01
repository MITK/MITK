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

#include <tweaklets/cherryGuiWidgetsTweaklet.h>

#include <list>
#include <QObject>

namespace cherry {

class QtSelectionListenerWrapper : public QObject
{

  Q_OBJECT

public:

  QtSelectionListenerWrapper(QObject* widget, GuiTk::ISelectionListener::Pointer listener);

  QObject* widget;
  GuiTk::ISelectionListener::Pointer listener;

protected slots:

  void QAbstractButtonClicked(bool checked);

};

class QtControlListenerWrapper : public QObject
{
  Q_OBJECT

public:

  QtControlListenerWrapper(QObject* widget, GuiTk::IControlListener::Pointer listener);

  QObject* widget;
  GuiTk::IControlListener::Pointer listener;

  protected slots:


};

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

  Rectangle ToDisplay(void* coordinateSystem,
          const Rectangle& toConvert);

private:

  typedef std::map<void*, std::list<QtSelectionListenerWrapper*> > SelectionListenerMap;
  SelectionListenerMap selectionListenerMap;

  bool ContainsSelectionListener(void* widget, GuiTk::ISelectionListener::Pointer) const;

  static std::list<Shell::Pointer> shellList;

  friend class QtMainWindowShell;

};

}

#endif /* CHERRYQTWIDGETSTWEAKLET_H_ */
