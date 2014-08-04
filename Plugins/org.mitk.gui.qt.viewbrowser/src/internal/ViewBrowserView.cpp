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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

// Qmitk
#include "ViewBrowserView.h"

// Qt
#include <QMessageBox>
#include <QTreeView>
#include <QStandardItem>


const std::string ViewBrowserView::VIEW_ID = "org.mitk.views.viewbrowser";

void ViewBrowserView::SetFocus()
{
    m_Controls.buttonPerformImageProcessing->setFocus();
}

void ViewBrowserView::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi( parent );
    connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoSomething()) );
    connect( m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
    connect( m_Controls.m_PluginTreeView, SIGNAL(clicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));

    m_TreeModel = new QStandardItemModel();
    QStandardItem *item = m_TreeModel->invisibleRootItem();

    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
    bool skip = false;

    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());

    MITK_INFO << "PERSPECTIVES";
    for (unsigned int i=0; i<perspectives.size(); i++)
    {
        //QList<QStandardItem *> preparedRow =prepareRow("first", "second", "third");
        berry::IPerspectiveDescriptor::Pointer p = perspectives.at(i);
        MITK_INFO << p->GetId();
        QList<QStandardItem *> preparedRow;
        preparedRow << new QStandardItem(QString::fromStdString(p->GetLabel()));
        preparedRow << new QStandardItem(QString::fromStdString(p->GetId()));
        item->appendRow(preparedRow);

        for (unsigned int i=0; i<views.size(); i++)
        {
            berry::IViewDescriptor::Pointer w = views.at(i);
            QList<QStandardItem *> secondRow;
            secondRow << new QStandardItem(QString::fromStdString(w->GetLabel()));
            secondRow << new QStandardItem(QString::fromStdString(w->GetId()));
            preparedRow.first()->appendRow(secondRow);
        }
        // if perspectiveExcludeList is set, it contains the id-strings of perspectives, which
        // should not appear as an menu-entry in the perspective menu
        //        if (perspectiveExcludeList.size() > 0)
        //        {
        //            for (unsigned int i=0; i<perspectiveExcludeList.size(); i++)
        //            {
        //                if (perspectiveExcludeList.at(i) == (*perspIt)->GetId())
        //                {
        //                    skip = true;
        //                    break;
        //                }
        //            }
        //            if (skip)
        //            {
        //                skip = false;
        //                continue;
        //            }
        //        }

        //        QAction* perspAction = new berry::QtOpenPerspectiveAction(window,
        //                                                                  *perspIt, perspGroup);
        //        mapPerspIdToAction.insert(std::make_pair((*perspIt)->GetId(), perspAction));
    }

    MITK_INFO << "VIEWS";


    //berry::QTOpenPers


    // adding a row to the invisible root item produces a root element
    //item->appendRow(preparedRow);

    //QList<QStandardItem *> secondRow =prepareRow("111", "222", "333");
    // adding a row to an item starts a subtree
    //preparedRow.first()->appendRow(secondRow);

    m_Controls.m_PluginTreeView->setModel(m_TreeModel);
    m_Controls.m_PluginTreeView->expandAll();
}

void ViewBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
    // iterate all selected objects, adjust warning visibility
    foreach( mitk::DataNode::Pointer node, nodes )
    {
        if( node.IsNotNull() )
        {
            return;
        }
    }
}

void ViewBrowserView::ItemClicked(const QModelIndex &index)
{
    MITK_INFO << index.row() << "-" << index.column();
//    if (index.column()==0)
//    {
        try
        {
            std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry()->GetPerspectives());
            berry::PlatformUI::GetWorkbench()->ShowPerspective( perspectives.at(index.row())->GetId(), berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows().at(0) );
        }
        catch (...)
        {
            QMessageBox::critical(0, "Opening Perspective Failed", QString("The requested perspective could not be opened.\nSee the log for details."));
        }
//    }
}

void ViewBrowserView::CustomMenuRequested(QPoint pos)
{
    //    m_ContextMenu->popup(m_Controls.m_PerspectiveTree->viewport()->mapToGlobal(pos));
}

void ViewBrowserView::DoSomething()
{

}
