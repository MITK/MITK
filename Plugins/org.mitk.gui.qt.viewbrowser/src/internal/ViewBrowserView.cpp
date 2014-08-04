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

}

void ViewBrowserView::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi( parent );
    connect( m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
    connect( m_Controls.m_PluginTreeView, SIGNAL(clicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));

    InitTreeView();
}

void ViewBrowserView::InitTreeView()
{
    m_ContextMenu = new QMenu(m_Controls.m_PluginTreeView);
    m_Controls.m_PluginTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_TreeModel = new QStandardItemModel();
    QStandardItem *item = m_TreeModel->invisibleRootItem();

    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
    bool skip = false;

    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());

    for (unsigned int i=0; i<perspectives.size(); i++)
    {
        berry::IPerspectiveDescriptor::Pointer p = perspectives.at(i);
        QList< QStandardItem*> preparedRow;
        mitk::QtPerspectiveItem* pItem = new mitk::QtPerspectiveItem(QString::fromStdString(p->GetLabel()));
        pItem->m_Perspective = p;
        preparedRow << pItem;
        item->appendRow(preparedRow);

        for (unsigned int i=0; i<views.size(); i++)
        {
            berry::IViewDescriptor::Pointer v = views.at(i);
            QList<QStandardItem *> secondRow;

            mitk::QtViewItem* vItem = new mitk::QtViewItem(QString::fromStdString(v->GetLabel()));
            vItem->m_View = v;
            secondRow << vItem;
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

    m_Controls.m_PluginTreeView->setModel(m_TreeModel);
    //    m_Controls.m_PluginTreeView->expandAll();
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
    QStandardItem* item = m_TreeModel->itemFromIndex(index);
    if ( dynamic_cast< mitk::QtPerspectiveItem* >(item) )
    {
        try
        {
            //            berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
            //            page->CloseAllPerspectives(false, false);
            mitk::QtPerspectiveItem* pItem = dynamic_cast< mitk::QtPerspectiveItem* >(item);
            //            page->ClosePerspective( pItem->m_Perspective, true, false );
            berry::PlatformUI::GetWorkbench()->ShowPerspective( pItem->m_Perspective->GetId(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
        }
        catch (...)
        {
            QMessageBox::critical(0, "Opening Perspective Failed", QString("The requested perspective could not be opened.\nSee the log for details."));
        }
    }
    else if ( dynamic_cast< mitk::QtViewItem* >(item) )
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();

        if (page.IsNotNull())
        {
            try
            {
                mitk::QtViewItem* vItem = dynamic_cast< mitk::QtViewItem* >(item);
                page->ShowView(vItem->m_View->GetId());
            }
            catch (berry::PartInitException e)
            {
                BERRY_ERROR << "Error: " << e.displayText() << std::endl;
            }
        }
    }

}

void ViewBrowserView::MapSignal()
{
    if (m_RegisteredPerspective!=NULL)
    {
        MITK_INFO << m_RegisteredPerspective->GetId();
        berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
        perspRegistry->ClonePerspective(m_RegisteredPerspective->GetId(), "TESTPERSPECTIVE", m_RegisteredPerspective);
        InitTreeView();
    }
}

void ViewBrowserView::CustomMenuRequested(QPoint pos)
{

    QStandardItem* item = m_TreeModel->itemFromIndex(m_Controls.m_PluginTreeView->indexAt(pos));

    if (m_ContextMenu!=NULL && item!=NULL && dynamic_cast< mitk::QtPerspectiveItem* >(item) )
    {
        m_ContextMenu->clear();
        m_RegisteredPerspective = dynamic_cast< mitk::QtPerspectiveItem* >(item)->m_Perspective;

        QAction* action = new QAction("Clone Perspective", this);
        m_ContextMenu->addAction(action);
        connect(action, SIGNAL(triggered()), SLOT(MapSignal()));

        m_ContextMenu->popup(m_Controls.m_PluginTreeView->viewport()->mapToGlobal(pos));
    }
}
