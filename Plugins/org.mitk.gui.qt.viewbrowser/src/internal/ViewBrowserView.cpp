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
#include <QSortFilterProxyModel>

class ClassFilterProxyModel : public QSortFilterProxyModel
{
private :
    bool hasToBeDisplayed(const QModelIndex index) const;
    bool displayElement(const QModelIndex index) const;
public:
    ClassFilterProxyModel(QObject *parent = NULL);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};
ClassFilterProxyModel::ClassFilterProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
}

bool ClassFilterProxyModel::filterAcceptsRow(int sourceRow,
                                             const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    return hasToBeDisplayed(index);
}

bool ClassFilterProxyModel::displayElement(const QModelIndex index) const
{
  bool result = false;
    QString type = sourceModel()->data(index, Qt::DisplayRole).toString();
  QStandardItem * item = dynamic_cast<QStandardItemModel*>(sourceModel())->itemFromIndex(index);

  if (type.contains(filterRegExp()))
    {
    return true;
    }
    {
    mitk::QtViewItem* viewItem = dynamic_cast<mitk::QtViewItem*>(item);
    if (viewItem)
    {
      for (int i = 0; i < viewItem->m_Tags.size(); ++i)
      {
        if (viewItem->m_Tags[i].contains(filterRegExp()))
        {
          return true;
        }
      }
    }
  }
  {
    mitk::QtPerspectiveItem* viewItem = dynamic_cast<mitk::QtPerspectiveItem*>(item);
    if (viewItem)
    {
      for (int i = 0; i < viewItem->m_Tags.size(); ++i)
      {
        if (viewItem->m_Tags[i].contains(filterRegExp()))
        {
          return true;
        }
      }
    }
  }

    return result;
}

bool ClassFilterProxyModel::hasToBeDisplayed(const QModelIndex index) const
{
    bool result = false;
    // How many child this element have
    if ( sourceModel()->rowCount(index) > 0 )
    {
        for( int ii = 0; ii < sourceModel()->rowCount(index); ii++)
        {
            QModelIndex childIndex = sourceModel()->index(ii,0,index);
            if ( ! childIndex.isValid() )
                break;
            result = hasToBeDisplayed(childIndex);
            result |= displayElement(index);
            if (result)
            {
                // there is atless one element to display
                break;
            }
        }
    }
    else
    {
        result = displayElement(index);
    }
    return result;
}

struct ViewBrowserWindowListener : public berry::IWindowListener
{
    ViewBrowserWindowListener(ViewBrowserView* switcher)
        : switcher(switcher),
          m_Running(false)
    {}

    virtual void WindowOpened(berry::IWorkbenchWindow::Pointer /*window*/)
    {
        if (m_Running)
            return;
        m_Running = true;
        switcher->FillTreeList();
        m_Running = false;
    }

    virtual void WindowActivated(berry::IWorkbenchWindow::Pointer /*window*/)
    {
        if (m_Running)
            return;
        m_Running = true;
        switcher->FillTreeList();
        m_Running = false;
    }

private:
    ViewBrowserView* switcher;
    bool m_Running;
};

const std::string ViewBrowserView::VIEW_ID = "org.mitk.views.viewbrowser";

bool compareViews(berry::IViewDescriptor::Pointer a, berry::IViewDescriptor::Pointer b)
{
    if (a.IsNull() || b.IsNull())
        return false;
    return a->GetLabel().compare(b->GetLabel()) < 0;
}

bool compareQStandardItems(QStandardItem* a, QStandardItem* b)
{
    if (a==NULL || b==NULL)
        return false;
    return a->text().compare(b->text()) < 0;
}

void ViewBrowserView::SetFocus()
{
}

void ViewBrowserView::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    m_WindowListener = ViewBrowserWindowListener::Pointer(new ViewBrowserWindowListener(this));
    berry::PlatformUI::GetWorkbench()->AddWindowListener(m_WindowListener);

    m_Parent = parent;
    m_Controls.setupUi( parent );
    connect( m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
    connect( m_Controls.m_PluginTreeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));
    connect( m_Controls.pushButton, SIGNAL(clicked()), SLOT(ButtonClicked()) );
    connect( m_Controls.lineEdit, SIGNAL(textChanged(QString)), SLOT(FilterChanged()));

    m_ContextMenu = new QMenu(m_Controls.m_PluginTreeView);
    m_Controls.m_PluginTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create a new TreeModel for the data
    m_TreeModel = new QStandardItemModel();
    m_FilterProxyModel = new ClassFilterProxyModel(this);
    m_FilterProxyModel->setSourceModel(m_TreeModel);
    //proxyModel->setFilterFixedString("Diff");
    m_Controls.m_PluginTreeView->setModel(m_FilterProxyModel);
    FillTreeList();

    QList<ViewTagsDescriptor::Pointer> additions = m_Registry.GetViewTags();
    foreach (const ViewTagsDescriptor::Pointer& var, additions)
    {
      MITK_INFO << var->GetID().toStdString();
    }
}

void ViewBrowserView::ButtonClicked()
{
    FillTreeList();
}

void ViewBrowserView::FillTreeList()
{
    // active workbench window available?
    if (berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow().IsNull())
        return;
    // active page available?
    berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
    if (page.IsNull())
        return;
    // everything is fine and we can remove the window listener
    berry::PlatformUI::GetWorkbench()->RemoveWindowListener(m_WindowListener);

    // initialize tree model
    m_TreeModel->clear();
    QStandardItem *treeRootItem = m_TreeModel->invisibleRootItem();

    // get all available perspectives
    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());

    QModelIndex currentIndex;
    berry::IPerspectiveDescriptor::Pointer currentPersp = page->GetPerspective();
    std::vector<std::string> perspectiveExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetPerspectiveExcludeList();

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

        QIcon* pIcon = static_cast<QIcon*>(p->GetImageDescriptor()->CreateImage());
        mitk::QtPerspectiveItem* pItem = new mitk::QtPerspectiveItem(*pIcon, QString::fromStdString(p->GetLabel()));
        pItem->m_Perspective = p;
        ViewTagsDescriptor::Pointer tags = m_Registry.Find(p->GetId());
        pItem->m_Tags = tags->GetTags();
        perspectiveRootItem->appendRow(pItem);

        if (currentPersp->GetId()==p->GetId())
            currentIndex = pItem->index();
    }

    // get all available views
    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());
    std::sort(views.begin(), views.end(), compareViews);

    std::vector<std::string> viewExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetViewExcludeList();
    QStandardItem* viewRootItem = new QStandardItem(QIcon(),"View categories");
    treeRootItem->appendRow(viewRootItem);

    std::vector< QStandardItem* > categoryItems;
    QStandardItem* noCategoryItem = new QStandardItem(QIcon(),"Miscellaneous");

    for (unsigned int i = 0; i < views.size(); ++i)
    {
        berry::IViewDescriptor::Pointer v = views[i];
        ViewTagsDescriptor::Pointer tags = m_Registry.Find(views[i]->GetId());
        bool skipView = false;
        for(unsigned int e=0; e<viewExcludeList.size(); e++)
            if(viewExcludeList.at(e)==v->GetId())
            {
                break;
            }
        if (skipView)
            continue;

        std::vector<std::string> catPath = v->GetCategoryPath();

        QIcon* icon = static_cast<QIcon*>(v->GetImageDescriptor()->CreateImage());
        mitk::QtViewItem* vItem = new mitk::QtViewItem(*icon, QString::fromStdString(v->GetLabel()));
        vItem->m_View = v;
        vItem->m_Tags = tags->GetTags();


        if (catPath.empty())
            noCategoryItem->appendRow(vItem);
        else
        {
            QStandardItem* categoryItem = NULL;

            for (unsigned int c=0; c<categoryItems.size(); c++)
                if (categoryItems.at(c)->text().toStdString() == catPath.front())
                {
                    categoryItem = categoryItems.at(c);
                    break;
                }

            if (categoryItem==NULL)
            {
                categoryItem  = new QStandardItem(QIcon(),catPath.front().c_str());
                categoryItems.push_back(categoryItem);
            }

            categoryItem->appendRow(vItem);
        }
    }
    std::sort(categoryItems.begin(), categoryItems.end(), compareQStandardItems);

    for (unsigned int i=0; i<categoryItems.size(); i++)
        viewRootItem->appendRow(categoryItems.at(i));
    if (noCategoryItem->hasChildren())
        viewRootItem->appendRow(noCategoryItem);

    QModelIndex correctedIndex = m_FilterProxyModel->mapFromSource(currentIndex);
    m_Controls.m_PluginTreeView->setCurrentIndex(correctedIndex);
}

void ViewBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
}

void ViewBrowserView::FilterChanged()
{
    QString filterString = m_Controls.lineEdit->text();
    if (filterString.size() > 0 )
        m_Controls.m_PluginTreeView->expandAll();
    else
        m_Controls.m_PluginTreeView->collapseAll();
    QRegExp::PatternSyntax syntax = QRegExp::RegExp;

    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
    QString strPattern = "^*" + filterString;
    QRegExp regExp(strPattern, caseSensitivity);

    m_FilterProxyModel->setFilterRegExp(regExp);
}

void ViewBrowserView::ItemClicked(const QModelIndex &index)
{
    QStandardItem* item = m_TreeModel->itemFromIndex(m_FilterProxyModel->mapToSource(index));

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
            berry::IPerspectiveDescriptor::Pointer perspDesc = perspRegistry->ClonePerspective(dialog->GetPerspectiveName().toStdString(), dialog->GetPerspectiveName().toStdString(), m_RegisteredPerspective);
            //berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->CreatePerspective(perspDesc.Cast<berry::IPerspectiveDescriptor>());
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
    QModelIndex index = m_Controls.m_PluginTreeView->indexAt(pos);
    QStandardItem* item = m_TreeModel->itemFromIndex(m_FilterProxyModel->mapToSource(index));

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