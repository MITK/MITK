/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTWIDGETSTWEAKLETIMPL_H_
#define BERRYQTWIDGETSTWEAKLETIMPL_H_

#include <internal/berryQtWidgetController.h>

#include <berryShell.h>
#include <berryGuiTkISelectionListener.h>

#include <QMetaType>

#include <list>
#include <vector>

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

  void SetBounds(QWidget* widget, const QRect& bounds);
  QRect GetBounds(QWidget* widget);

  void SetVisible(QWidget* widget, bool visible);
  bool GetVisible(QWidget* widget);
  bool IsVisible(QWidget* widget);

  QRect GetClientArea(QWidget* widget);

  QWidget* GetParent(QWidget* widget);
  bool SetParent(QWidget* widget, QWidget* parent);

  void SetData(QWidget* widget, const QString& id, Object::Pointer data);
  Object::Pointer GetData(QWidget* widget, const QString& id);

  QRect GetScreenSize(int i = -1);
  unsigned int GetScreenNumber();
  int GetPrimaryScreenNumber();
  QRect GetAvailableScreenSize(int i = -1);
  int GetClosestScreenNumber(const QRect&);

  QPoint GetCursorLocation();
  QWidget* GetCursorControl();
  QWidget* FindControl(const QList<Shell::Pointer>& shells, const QPoint& location);

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

  QList<Shell::Pointer> GetShells();
  Shell::Pointer GetShell(QWidget* widget);
  Shell::Pointer GetActiveShell();

  QRect ToControl(QWidget* coordinateSystem,
          const QRect& toConvert);

  QPoint ToControl(QWidget* coordinateSystem,
          const QPoint& toConvert);

  QRect ToDisplay(QWidget* coordinateSystem,
          const QRect& toConvert);

  QPoint ToDisplay(QWidget* coordinateSystem,
          const QPoint& toConvert);

private:

  typedef QHash<QWidget*, QtSelectionListenerWrapper* > SelectionListenerMap;
  SelectionListenerMap selectionListenerMap;

  QList<Shell::Pointer> shellList;

  friend class QtShell;

};

}

#endif /* BERRYQTWIDGETSTWEAKLETIMPL_H_ */
