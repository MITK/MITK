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
#ifndef _QMITKViewBrowserWidget_H_INCLUDED
#define _QMITKViewBrowserWidget_H_INCLUDED

//QT headers
#include <QWidget>
#include <QString>
#include "ui_QmitkViewBrowserWidgetControls.h"

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
#include <ViewTagsRegistry.h>

class ClassFilterProxyModel;

/** @brief
  */
class QmitkViewBrowserWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    QmitkViewBrowserWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkViewBrowserWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    void FillTreeList();

public slots:

    void CustomMenuRequested(QPoint pos);
    void ItemClicked(const QModelIndex &index);
    void AddPerspective();
    void ClonePerspective();
    void ResetPerspective();
    void DeletePerspective();
    void ClosePerspectives();
    void ClosePerspective();
    void FilterChanged();

protected:

    // member variables
    Ui::QmitkViewBrowserWidgetControls          m_Controls;
    QWidget*                                    m_Parent;
    QStandardItemModel*                         m_TreeModel;
    ClassFilterProxyModel*                      m_FilterProxyModel;
    QMenu*                                      m_ContextMenu;
    berry::IPerspectiveDescriptor::Pointer      m_RegisteredPerspective;
    berry::IWindowListener::Pointer             m_WindowListener;
    friend struct ViewBrowserViewListener;
    ViewTagsRegistry m_Registry;

private:

};

#endif // _QMITKViewBrowserWidget_H_INCLUDED

