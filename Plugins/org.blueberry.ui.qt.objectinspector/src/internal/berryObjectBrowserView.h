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

#ifndef BERRY_OBJECTINSPECTORVIEW_H_INCLUDED
#define BERRY_OBJECTINSPECTORVIEW_H_INCLUDED

#include <berryQtViewPart.h>
#include <string>

#include <ui_berryQtObjectBrowserView.h>
#include "berryQtObjectTableModel.h"

#include <QAction>
#include <QMenu>

class QAbstractProxyModel;

namespace berry {

/*!
 * \ingroup org_blueberry_ui_qt_objectinspector_internal
 *
 * \brief Object Inspector
 *
 * You need to reimplement the methods SetFocus() and CreateQtPartControl(QWidget*)
 * from berry::QtViewPart
 *
 * \sa berry::QtViewPart
 */
class ObjectBrowserView : public berry::QtViewPart
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  ObjectBrowserView();

  void Init(IViewSite::Pointer site, IMemento::Pointer memento) override;

  /*!
   * \brief Gives focus to a specific control in the view
   * This method is called from the framework when the view is activated.
   */
  void SetFocus() override;

  void SaveState(IMemento::Pointer memento) override;

protected slots:

  void ResetAction(bool checked);
  void SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void ContextMenuRequested(const QPoint&);
  void ToggleBreakpoint(bool checked);

protected:

  /*!
   * \brief Builds the user interface of the view
   * This method is called from the framework. The parent widget has no layout, so
   * you should set one adapted to your needs.
   */
  void CreateQtPartControl(QWidget* parent) override;

  void RestoreGuiState(IMemento::Pointer memento);

private:

  Ui::QtObjectBrowserView m_Controls;
  QtObjectTableModel* m_ObjectModel;
  QAbstractProxyModel* m_ProxyModel;

  QAction m_ActionToggleBreakpoint;
  QAction m_ActionEnableBreakpoint;
  QAction m_ActionDisableBreakpoint;

  QMenu m_ContextMenu;

  IMemento::Pointer m_StateMemento;

  bool m_Useful;
};

} //namespace berry

#endif /*BERRY_OBJECTINSPECTORVIEW_H_INCLUDED*/
