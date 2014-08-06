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
#include <src/internal/mitkQtPerspectiveItem.h>
#include <src/internal/mitkQtViewItem.h>
#include <src/internal/QmitkNewPerspectiveDialog.h>

// Qt
#include <QMessageBox>
#include <QTreeView>
#include <QStandardItem>


struct ViewBrowserViewListener : public berry::IPerspectiveListener
{
    ViewBrowserViewListener(ViewBrowserView* switcher)
        : switcher(switcher)
    {}

    Events::Types GetPerspectiveEventTypes() const
    {
        return Events::ACTIVATED;
    }

    virtual void PerspectiveChanged(berry::SmartPointer<berry::IWorkbenchPage> /*page*/,
                                    berry::IPerspectiveDescriptor::Pointer /*perspective*/, const std::string& /*changeId*/)
    {
        switcher->FillTreeList();
    }

    void PerspectiveActivated(berry::IWorkbenchPage::Pointer /*page*/,
                              berry::IPerspectiveDescriptor::Pointer perspective)
    {
        switcher->FillTreeList();
    }

private:
    ViewBrowserView* switcher;
};

const std::string ViewBrowserView::VIEW_ID = "org.mitk.views.viewbrowser";

bool compareViews(berry::IViewDescriptor::Pointer a, berry::IViewDescriptor::Pointer b)
{
    if (a.IsNull() || b.IsNull())
        return false;
    return a->GetLabel().compare(b->GetLabel()) < 0;
}

void ViewBrowserView::SetFocus()
{

}

void ViewBrowserView::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Parent = parent;
    m_Controls.setupUi( parent );
    connect( m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
    connect( m_Controls.m_PluginTreeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));
    connect( m_Controls.pushButton, SIGNAL(clicked()), SLOT(ButtonClicked()) );

    m_ContextMenu = new QMenu(m_Controls.m_PluginTreeView);
    m_Controls.m_PluginTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create a new TreeModel for the data
    m_TreeModel = new QStandardItemModel();
    FillTreeList();
    m_Controls.m_PluginTreeView->setModel(m_TreeModel);

    this->perspListener = new ViewBrowserViewListener(this);
    //berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->AddPerspectiveListener(this->perspListener);
}

void ViewBrowserView::ButtonClicked()
{
    berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
    berry::IPerspectiveDescriptor::Pointer currentPersp = page->GetPerspective();

    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
    for (unsigned int i=0; i<perspectives.size(); i++)
        berry::PlatformUI::GetWorkbench()->ShowPerspective( perspectives.at(i)->GetId(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );

    FillTreeList();
    berry::PlatformUI::GetWorkbench()->ShowPerspective( currentPersp->GetId(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
}

void ViewBrowserView::FillTreeList()
{
    m_TreeModel->clear();
    QStandardItem *treeRootItem = m_TreeModel->invisibleRootItem();

    // Get all available views and create a map of all views
    std::map<std::string, berry::IViewDescriptor::Pointer> viewMap;
    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());
    for (unsigned int i=0; i<views.size(); i++)
    {
        viewMap[views[i]->GetId()] = views[i];
    }
    std::sort(views.begin(), views.end(), compareViews);

    // Get all available perspectives
    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());

    // workbench window available?
    if (berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow().IsNull())
        return;

    // Fill the TreeModel
    berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
    QModelIndex currentIndex;
    berry::IPerspectiveDescriptor::Pointer currentPersp = page->GetPerspective();
    std::vector<std::string> perspectiveExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetPerspectiveExcludeList();
    std::vector<std::string> viewExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetViewExcludeList();

    QStandardItem *perspectiveRootItem = new QStandardItem("Perspectives");
    treeRootItem->appendRow(perspectiveRootItem);
    for (unsigned int i=0; i<perspectives.size(); i++)
    {
        berry::IPerspectiveDescriptor::Pointer p = perspectives.at(i);

        bool skipPerspective = false;
        for(unsigned int e=0; e<perspectiveExcludeList.size(); e++)
            if(perspectiveExcludeList.at(e)==p->GetId())
            {
                skipPerspective = true;
                break;
            }
        if (skipPerspective)
            continue;

        QList< QStandardItem*> preparedRow;

        QIcon* pIcon = static_cast<QIcon*>(p->GetImageDescriptor()->CreateImage());
        mitk::QtPerspectiveItem* pItem = new mitk::QtPerspectiveItem(*pIcon, QString::fromStdString(p->GetLabel()));
        pItem->m_Perspective = p;
        preparedRow << pItem;
        perspectiveRootItem->appendRow(preparedRow);

        if (currentPersp->GetId()==p->GetId())
            currentIndex = pItem->index();

        for (unsigned int i = 0; i < views.size(); ++i)
        {
            bool skipView = false;
            for(unsigned int e=0; e<viewExcludeList.size(); e++)
                if(viewExcludeList.at(e)==views.at(i)->GetId())
                {
                    skipView = true;
                    break;
                }
            if (skipView)
                continue;

            if ( page->HasView(p->GetId(), views.at(i)->GetId()) )
            {
              QList<QStandardItem *> secondRow;
              QIcon* vIcon = static_cast<QIcon*>(views[i]->GetImageDescriptor()->CreateImage());
              mitk::QtViewItem* vItem = new mitk::QtViewItem(*vIcon, QString::fromStdString(views[i]->GetLabel()));
              vItem->m_View = views[i];
              secondRow << vItem;
              preparedRow.first()->appendRow(secondRow);
            }
        }

//        // If the current perspective is the active perspective add current views
//        if (currentPersp.compare(p->GetId())==0)
//        {
//            if (curWin.IsNull())
//                continue;

//            berry::IWorkbenchPage::Pointer activePage = curWin->GetActivePage();
//            std::vector< std::string > currentViews = activePage->GetShowViewShortcuts();
//            // Create a vector with all current elements
//            std::vector<berry::IViewDescriptor::Pointer> activeViews;
//            for (int j = 0; j < currentViews.size(); ++j)
//            {
//                berry::IViewDescriptor::Pointer view = viewMap[currentViews[j]];
//                if (view.IsNull())
//                    continue;
//                activeViews.push_back(view);
//            }
//            std::sort(activeViews.begin(), activeViews.end(), compareViews);


//            for (int j = 0; j < activeViews.size(); ++j)
//            {
//                berry::IViewDescriptor::Pointer view =activeViews[j];
//                QList<QStandardItem *> secondRow;
//                QIcon* icon = static_cast<QIcon*>(view->GetImageDescriptor()->CreateImage());
//                mitk::QtViewItem* vItem = new mitk::QtViewItem(*icon,QString::fromStdString(view->GetLabel()));
//                vItem->m_View = view;
//                secondRow << vItem;
//                preparedRow.first()->appendRow(secondRow);
//            }
//        }
    }

    // Add a list with all available views
    QList< QStandardItem*> preparedRow;
    QStandardItem* pItem = new QStandardItem(QIcon(),"Views");
    preparedRow << pItem;
    treeRootItem->appendRow(preparedRow);
    for (unsigned int i = 0; i < views.size(); ++i)
    {
        QList<QStandardItem *> secondRow;
        QIcon* icon = static_cast<QIcon*>(views[i]->GetImageDescriptor()->CreateImage());
        mitk::QtViewItem* vItem = new mitk::QtViewItem(*icon, QString::fromStdString(views[i]->GetLabel()));
        vItem->m_View = views[i];
        secondRow << vItem;
        preparedRow.first()->appendRow(secondRow);
    }

    m_Controls.m_PluginTreeView->setCurrentIndex(currentIndex);
}

void ViewBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{

}

void ViewBrowserView::ItemClicked(const QModelIndex &index)
{
    QStandardItem* item = m_TreeModel->itemFromIndex(index);
    if ( dynamic_cast< mitk::QtPerspectiveItem* >(item) )
    {
        try
        {
            mitk::QtPerspectiveItem* pItem = dynamic_cast< mitk::QtPerspectiveItem* >(item);
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

void ViewBrowserView::AddPerspective()
{
    QmitkNewPerspectiveDialog* dialog = new QmitkNewPerspectiveDialog( m_Parent );

    int dialogReturnValue = dialog->exec();
    if ( dialogReturnValue == QDialog::Rejected )
        return;

    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    try
    {
        perspRegistry->CreatePerspective(dialog->GetPerspectiveName().toStdString(), perspRegistry->FindPerspectiveWithId(perspRegistry->GetDefaultPerspective()));
    }
    catch(...)
    {
        QMessageBox::warning(m_Parent, "Error", "Duplication of selected perspective failed. Please make sure the specified perspective name is not already in use!");
    }
    FillTreeList();
}

void ViewBrowserView::ClonePerspective()
{
    if (m_RegisteredPerspective.IsNotNull())
    {
        QmitkNewPerspectiveDialog* dialog = new QmitkNewPerspectiveDialog( m_Parent );
        QString defaultName(m_RegisteredPerspective->GetLabel().c_str());
        defaultName.append(" Copy");
        dialog->SetPerspectiveName(defaultName);

        int dialogReturnValue = dialog->exec();
        if ( dialogReturnValue == QDialog::Rejected )
            return;

        berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
        try
        {
            perspRegistry->ClonePerspective(dialog->GetPerspectiveName().toStdString(), dialog->GetPerspectiveName().toStdString(), m_RegisteredPerspective);
        }
        catch(...)
        {
            QMessageBox::warning(m_Parent, "Error", "Duplication of selected perspective failed. Please make sure the specified perspective name is not already in use!");
        }
        FillTreeList();
    }
}

void ViewBrowserView::ResetPerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", "Do you really want to reset the curent perspective?", QMessageBox::Yes|QMessageBox::No).exec())
        berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
}

void ViewBrowserView::DeletePerspective()
{
    if (m_RegisteredPerspective.IsNotNull())
    {
        QString question = "Do you really want to remove the perspective '";
        question.append(m_RegisteredPerspective->GetLabel().c_str());
        question.append("'?");
        if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", question, QMessageBox::Yes|QMessageBox::No).exec())
        {
            berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
            perspRegistry->DeletePerspective(m_RegisteredPerspective);
            FillTreeList();
        }
    }
}

void ViewBrowserView::ClosePerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", "Do you really want to close the curent perspective?", QMessageBox::Yes|QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
        page->CloseCurrentPerspective(true, true);

        if ( page->GetPerspective().IsNull() )
        {
            berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
            berry::PlatformUI::GetWorkbench()->ShowPerspective( perspRegistry->GetDefaultPerspective(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
        }
    }
}

void ViewBrowserView::ClosePerspectives()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", "Do you really want to close all perspectives?", QMessageBox::Yes|QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
        page->CloseAllPerspectives(true, true);

        berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
        berry::PlatformUI::GetWorkbench()->ShowPerspective( perspRegistry->GetDefaultPerspective(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
    }
}

void ViewBrowserView::CustomMenuRequested(QPoint pos)
{
    QStandardItem* item = m_TreeModel->itemFromIndex(m_Controls.m_PluginTreeView->indexAt(pos));

    if (m_ContextMenu==NULL || item==NULL)
        return;

    m_ContextMenu->clear();
    m_RegisteredPerspective = NULL;

    bool showMenu = false;
    if (item->text()=="Perspectives")
    {
        QAction* addAction = new QAction("Create new perspective", this);
        m_ContextMenu->addAction(addAction);
        connect(addAction, SIGNAL(triggered()), SLOT(AddPerspective()));

        m_ContextMenu->addSeparator();

        QAction* resetAction = new QAction("Reset current perspective", this);
        m_ContextMenu->addAction(resetAction);
        connect(resetAction, SIGNAL(triggered()), SLOT(ResetPerspective()));

        QAction* closeAction = new QAction("Close current perspective", this);
        m_ContextMenu->addAction(closeAction);
        connect(closeAction, SIGNAL(triggered()), SLOT(ClosePerspective()));

        m_ContextMenu->addSeparator();

        QAction* closeAllAction = new QAction("Close all perspectives", this);
        m_ContextMenu->addAction(closeAllAction);
        connect(closeAllAction, SIGNAL(triggered()), SLOT(ClosePerspectives()));

        showMenu = true;
    }
    if (dynamic_cast< mitk::QtPerspectiveItem* >(item) )
    {
        m_RegisteredPerspective = dynamic_cast< mitk::QtPerspectiveItem* >(item)->m_Perspective;

        //m_ContextMenu->addSeparator();

        QAction* cloneAction = new QAction("Duplicate perspective", this);
        m_ContextMenu->addAction(cloneAction);
        connect(cloneAction, SIGNAL(triggered()), SLOT(ClonePerspective()));

        if (!m_RegisteredPerspective->IsPredefined())
        {
            QAction* deleteAction = new QAction("Remove perspective", this);
            m_ContextMenu->addAction(deleteAction);
            connect(deleteAction, SIGNAL(triggered()), SLOT(DeletePerspective()));
        }

        showMenu = true;
    }

    if (showMenu)
        m_ContextMenu->popup(m_Controls.m_PluginTreeView->viewport()->mapToGlobal(pos));
}
