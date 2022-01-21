/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMIKVIEWNAVIGATORWIDGET_H
#define QMIKVIEWNAVIGATORWIDGET_H

//QT headers
#include <QWidget>
#include <QString>

#include <ctkSearchBox.h>
#include "ui_QmitkViewNavigatorWidgetControls.h"

#include <berryISelectionListener.h>
#include <berryIPerspectiveListener.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveDescriptor.h>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>

class ClassFilterProxyModel;

/** @brief
  */
class QmitkViewNavigatorWidget : public QWidget
{
    Q_OBJECT

public:

    QmitkViewNavigatorWidget(berry::IWorkbenchWindow::Pointer window,
                             QWidget* parent = nullptr,
                             Qt::WindowFlags f = nullptr);
    ~QmitkViewNavigatorWidget() override;

    void SetFocus();

    bool FillTreeList();
    void UpdateTreeList(QStandardItem* item = nullptr,
                        berry::IWorkbenchPartReference* partRef = nullptr,
                        const std::string& changeId = "");

    QScopedPointer<berry::IPerspectiveListener>    m_PerspectiveListener;
    QScopedPointer<berry::IWindowListener>         m_WindowListener;

public Q_SLOTS:

    void FilterChanged();
    void ItemClicked(const QModelIndex& index);
    void SaveCurrentPerspectiveAs();
    void ResetCurrentPerspective();
    void ClosePerspective();
    void CloseAllPerspectives();
    void ExpandAll();
    void CollapseAll();
    void CustomMenuRequested(QPoint pos);

protected:

    friend class ViewNavigatorPerspectiveListener;

    Ui::QmitkViewNavigatorWidgetControls        m_Controls;
    QWidget*                                    m_Parent;
    QStandardItemModel*                         m_TreeModel;
    ClassFilterProxyModel*                      m_FilterProxyModel;
    QMenu*                                      m_ContextMenu;
    berry::IPerspectiveDescriptor::Pointer      m_ActivePerspective;
    bool                                        m_Generated;

private:

    void CreateQtPartControl(QWidget* parent);
    berry::IWorkbenchWindow::Pointer m_Window;
};

#endif // QMIKVIEWNAVIGATORWIDGET_H
