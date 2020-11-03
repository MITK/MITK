/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef _QMITKViewNavigatorWidget_H_INCLUDED
#define _QMITKViewNavigatorWidget_H_INCLUDED

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
#include <mitkQtPerspectiveItem.h>
#include <mitkQtViewItem.h>

class ClassFilterProxyModel;

/** @brief
  */
class QmitkViewNavigatorWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    QmitkViewNavigatorWidget (berry::IWorkbenchWindow::Pointer window,
                              QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkViewNavigatorWidget() override;

    virtual void CreateQtPartControl(QWidget *parent);
    void setFocus();

    bool FillTreeList();
    void UpdateTreeList(QStandardItem* item = nullptr, berry::IWorkbenchPartReference* partRef=nullptr, const std::string& changeId="");

    QScopedPointer<berry::IPerspectiveListener>    m_PerspectiveListener;
    QScopedPointer<berry::IWindowListener>         m_WindowListener;

public slots:

    void CustomMenuRequested(QPoint pos);
    void ItemClicked(const QModelIndex &index);
    void SaveCurrentPerspectiveAs();
    void ResetCurrentPerspective();
    void CloseAllPerspectives();
    void ClosePerspective();
    void ExpandAll();
    void CollapseAll();
    void FilterChanged();

protected:

    friend class ViewNavigatorPerspectiveListener;

    // member variables
    Ui::QmitkViewNavigatorWidgetControls        m_Controls;
    QWidget*                                    m_Parent;
    QStandardItemModel*                         m_TreeModel;
    ClassFilterProxyModel*                      m_FilterProxyModel;
    QMenu*                                      m_ContextMenu;
    berry::IPerspectiveDescriptor::Pointer      m_ActivePerspective;
    bool                                        m_Generated;

private:

    berry::IWorkbenchWindow::Pointer m_Window;
};

#endif // _QMITKViewNavigatorWidget_H_INCLUDED

