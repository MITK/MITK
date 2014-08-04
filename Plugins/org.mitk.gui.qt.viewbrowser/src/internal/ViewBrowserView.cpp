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
#include <berryQtOpenPerspectiveAction.h>
#include <berryIWorkbenchPage.h>

// Qmitk
#include "ViewBrowserView.h"

// Qt
#include <QActionGroup>
#include <QStandardItem>
#include <QMenu>
#include <QMessageBox>

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
    connect( m_Controls.m_PerspectiveTree, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
    connect( m_Controls.m_PerspectiveTree, SIGNAL(clicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));
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
    MITK_INFO << "ROW: " << index.row();
    try
    {

        std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry()->GetPerspectives());

        berry::PlatformUI::GetWorkbench()->ShowPerspective( perspectives.at(index.row())->GetId(), berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows().at(0) );
    }
    catch (...)
    {
        QMessageBox::critical(0, "Opening Perspective Failed", QString("The perspective BLABLA could not be opened.\nSee the log for details."));
    }
}

void ViewBrowserView::CustomMenuRequested(QPoint pos)
{
    //    m_ContextMenu->popup(m_Controls.m_PerspectiveTree->viewport()->mapToGlobal(pos));
}

void ViewBrowserView::DoSomething()
{
    m_Controls.m_PerspectiveTree->setContextMenuPolicy(Qt::CustomContextMenu);

    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry()->GetPerspectives());

    MITK_INFO << "PERSPECTIVES";
    for (unsigned int i=0; i<perspectives.size(); i++)
    {
        berry::IPerspectiveDescriptor::Pointer p = perspectives.at(i);
        std::string id = p->GetId();
        MITK_INFO << id;
        QStandardItem *item = new QStandardItem( QString("%0").arg(id.c_str()) );

        //        QAction* perspAction = new berry::QtOpenPerspectiveAction(berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows().at(0), p, perspGroup);
        //        m_Controls.m_PerspectiveTree->addAction(perspAction);
        m_Model.appendRow(item);

        //        QMenu* menu = new QMenu();
        //        menu->addAction(perspAction);
        //        m_PerspectiveContextMenus.push_back(menu);
    }

    MITK_INFO << "VIEWS";
    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());
    for (unsigned int i=0; i<views.size(); i++)
    {
        berry::IViewDescriptor::Pointer w = views.at(i);
        std::string id = w->GetId();
        MITK_INFO << id;
        QStandardItem *item = new QStandardItem( QString("%0").arg(id.c_str()) );
        //        model->appendRow(item);
    }

    m_Controls.m_PerspectiveTree->setModel(&m_Model);
}
