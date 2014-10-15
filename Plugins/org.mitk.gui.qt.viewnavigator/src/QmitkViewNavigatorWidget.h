/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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
#include <QmitkNewPerspectiveDialog.h>
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

    QmitkViewNavigatorWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkViewNavigatorWidget();

    virtual void CreateQtPartControl(QWidget *parent);
    void setFocus();

    bool FillTreeList();
    void UpdateTreeList(QStandardItem* item = NULL, berry::IWorkbenchPartReference* partRef=NULL, const std::string& changeId="");

    berry::IPerspectiveListener::Pointer    m_PerspectiveListener;
    berry::IWindowListener::Pointer         m_WindowListener;

public slots:

    void CustomMenuRequested(QPoint pos);
    void ItemClicked(const QModelIndex &index);
    void AddPerspective();
    void ClonePerspective();
    void ResetPerspective();
    void DeletePerspective();
    void CloseAllPerspectives();
    void ClosePerspective();
    void ExpandAll();
    void CollapseAll();
    void FilterChanged();

protected:

    // member variables
    Ui::QmitkViewNavigatorWidgetControls          m_Controls;
    QWidget*                                    m_Parent;
    QStandardItemModel*                         m_TreeModel;
    ClassFilterProxyModel*                      m_FilterProxyModel;
    QMenu*                                      m_ContextMenu;
    berry::IPerspectiveDescriptor::Pointer      m_RegisteredPerspective;
    bool                                        m_Generated;

private:

};

#endif // _QMITKViewNavigatorWidget_H_INCLUDED

