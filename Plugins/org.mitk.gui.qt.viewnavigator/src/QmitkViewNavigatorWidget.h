/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkViewNavigatorWidget_h
#define QmitkViewNavigatorWidget_h

#include "ui_QmitkViewNavigatorWidgetControls.h"

//QT headers
#include <QWidget>
#include <QStandardItemModel>
#include <QMenu>

#include <berryIPartListener.h>
#include <berryIPerspectiveListener.h>
#include <berryIWorkbenchWindow.h>

class ClassFilterProxyModel;

/**
* @brief
*
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
    friend class ViewNavigatorViewListener;

    berry::IPerspectiveDescriptor::Pointer      m_ActivePerspective;

private:

    void CreateQtPartControl(QWidget* parent);
    bool FillTreeList();
    void UpdateTreeList(berry::IWorkbenchPart* workbenchPart = nullptr);

    void AddPerspectivesToTree();
    void AddViewsToTree();
    template<typename D, typename I>
    void AddItemsToTree(D itemDescriptors, QStandardItem* rootItem,
      QStandardItem* miscellaneousItem = nullptr, const QStringList& itemExcludeList = QStringList());

    Ui::QmitkViewNavigatorWidgetControls        m_Controls;
    QStandardItemModel*                         m_TreeModel;
    ClassFilterProxyModel*                      m_FilterProxyModel;
    QMenu*                                      m_ContextMenu;

    QScopedPointer<berry::IPerspectiveListener>    m_PerspectiveListener;
    QScopedPointer<berry::IPartListener>           m_ViewPartListener;

    berry::IWorkbenchWindow::Pointer m_Window;
};

#endif
