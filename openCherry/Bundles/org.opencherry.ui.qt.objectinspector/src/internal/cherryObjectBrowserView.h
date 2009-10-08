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

#ifndef CHERRY_OBJECTINSPECTORVIEW_H_INCLUDED
#define CHERRY_OBJECTINSPECTORVIEW_H_INCLUDED

#include <cherryQtViewPart.h>
#include <string>

#include <ui_cherryQtObjectBrowserView.h>
#include "cherryQtObjectTableModel.h"

#include <QAction>
#include <QMenu>

class QAbstractProxyModel;

namespace cherry {

/*!
 * \ingroup org_opencherry_ui_qt_objectinspector_internal
 *
 * \brief Object Inspector
 *
 * You need to reimplement the methods SetFocus() and CreateQtPartControl(QWidget*)
 * from cherry::QtViewPart
 *
 * \sa cherry::QtViewPart
 */
class ObjectBrowserView : public QObject, public cherry::QtViewPart
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  ObjectBrowserView();

  void Init(IViewSite::Pointer site, IMemento::Pointer memento);

  /*!
   * \brief Gives focus to a specific control in the view
   * This method is called from the framework when the view is activated.
   */
  void SetFocus();

  void SaveState(IMemento::Pointer memento);

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
  void CreateQtPartControl(QWidget* parent);

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

} //namespace cherry

#endif /*CHERRY_OBJECTINSPECTORVIEW_H_INCLUDED*/
