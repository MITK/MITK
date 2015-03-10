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

//Qmitk headers
#include "QmitkViewNavigatorWidget.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
#include <berryPlatformUI.h>
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>
#include <berryIExtensionRegistry.h>

// Qt
#include <QHash>
#include <QMessageBox>
#include <QTreeView>
#include <QStandardItem>
#include <QSortFilterProxyModel>

class KeywordRegistry
{
public:
    KeywordRegistry()
    {
        berry::IExtensionRegistry* extensionPointService = berry::Platform::GetExtensionRegistry();
        auto keywordExts = extensionPointService->GetConfigurationElementsFor("org.blueberry.ui.keywords");
        for (auto keywordExtsIt = keywordExts.begin(); keywordExtsIt != keywordExts.end(); ++keywordExtsIt)
        {
            QString keywordId = (*keywordExtsIt)->GetAttribute("id");
            QString keywordLabels = (*keywordExtsIt)->GetAttribute("label");
            m_Keywords[keywordId].push_back(keywordLabels);
        }
    }

    QStringList GetKeywords(const QString& id)
    {
        return m_Keywords[id];
    }

    QStringList GetKeywords(const QStringList& ids)
    {
        QStringList result;
        for (int i = 0; i < ids.size(); ++i)
        {
            result.append(this->GetKeywords(ids[i]));
        }
        return result;
    }

private:
    QHash<QString, QStringList> m_Keywords;
};


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
            if (viewItem->m_Description.contains(filterRegExp()))
            {
                return true;
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
            if (viewItem->m_Description.contains(filterRegExp()))
            {
                return true;
            }
        }
    }

    return result;
}

bool ClassFilterProxyModel::hasToBeDisplayed(const QModelIndex index) const
{
    bool result = false;
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

class ViewNavigatorPerspectiveListener: public berry::IPerspectiveListener
{
public:

    ViewNavigatorPerspectiveListener(QmitkViewNavigatorWidget* p) :
        parentWidget(p)
    {
    }

    Events::Types GetPerspectiveEventTypes() const
    {
        return Events::ACTIVATED | Events::SAVED_AS | Events::DEACTIVATED
                // remove the following line when command framework is finished
                | Events::CLOSED | Events::OPENED | Events::PART_CHANGED;
    }

    void PerspectiveActivated(const berry::IWorkbenchPage::Pointer& /*page*/,
                              const berry::IPerspectiveDescriptor::Pointer& /*perspective*/)
    {
        parentWidget->UpdateTreeList();
    }

    void PerspectiveSavedAs(const berry::IWorkbenchPage::Pointer& /*page*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*oldPerspective*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*newPerspective*/)
    {

    }

    void PerspectiveDeactivated(const berry::IWorkbenchPage::Pointer& /*page*/,
                                const berry::IPerspectiveDescriptor::Pointer& /*perspective*/)
    {
        parentWidget->UpdateTreeList();
    }

    void PerspectiveOpened(const berry::IWorkbenchPage::Pointer& /*page*/,
                           const berry::IPerspectiveDescriptor::Pointer& /*perspective*/)
    {
        parentWidget->UpdateTreeList();
    }

    void PerspectiveClosed(const berry::IWorkbenchPage::Pointer& /*page*/,
                           const berry::IPerspectiveDescriptor::Pointer& /*perspective*/)
    {
        parentWidget->UpdateTreeList();
    }

    using IPerspectiveListener::PerspectiveChanged;

    void PerspectiveChanged(const berry::IWorkbenchPage::Pointer&,
                            const berry::IPerspectiveDescriptor::Pointer&,
                            const berry::IWorkbenchPartReference::Pointer& partRef, const std::string& changeId)
    {
        if (changeId=="viewHide" && partRef->GetId()=="org.mitk.views.viewnavigatorview")
            berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->RemovePerspectiveListener(parentWidget->m_PerspectiveListener.data());
        else
            parentWidget->UpdateTreeList(NULL, partRef.GetPointer(), changeId);
    }

private:
    QmitkViewNavigatorWidget* parentWidget;
};

struct ViewNavigatorWindowListener : public berry::IWindowListener
{
    ViewNavigatorWindowListener(QmitkViewNavigatorWidget* switcher)
        : switcher(switcher)
        , m_Done(false)
    {}

    virtual void WindowOpened(const berry::IWorkbenchWindow::Pointer& window)
    {
        if (m_Done)
            return;
        if ( switcher->FillTreeList() )
        {
            m_Done = true;
            switcher->m_PerspectiveListener.reset(new ViewNavigatorPerspectiveListener(switcher));
            window->AddPerspectiveListener(switcher->m_PerspectiveListener.data());
        }
    }

    virtual void WindowActivated(const berry::IWorkbenchWindow::Pointer& window)
    {
        if (m_Done)
            return;
        if ( switcher->FillTreeList() )
        {
            m_Done = true;
            switcher->m_PerspectiveListener.reset(new ViewNavigatorPerspectiveListener(switcher));
            window->AddPerspectiveListener(switcher->m_PerspectiveListener.data());
        }
    }

private:
    QmitkViewNavigatorWidget* switcher;
    bool m_Done;
};

bool compareViews(const berry::IViewDescriptor::Pointer& a, const berry::IViewDescriptor::Pointer& b)
{
    if (a.IsNull() || b.IsNull())
        return false;
    return a->GetLabel().compare(b->GetLabel()) < 0;
}

bool comparePerspectives(const berry::IPerspectiveDescriptor::Pointer& a, const berry::IPerspectiveDescriptor::Pointer& b)
{
    if (a.IsNull() || b.IsNull())
        return false;
    return a->GetLabel().compare(b->GetLabel()) < 0;
}

bool compareQStandardItems(const QStandardItem* a, const QStandardItem* b)
{
    if (a==NULL || b==NULL)
        return false;
    return a->text().compare(b->text()) < 0;
}

QmitkViewNavigatorWidget::QmitkViewNavigatorWidget( QWidget * parent, Qt::WindowFlags )
    : QWidget(parent)
{
    m_Generated = false;
    this->CreateQtPartControl(this);
}

QmitkViewNavigatorWidget::~QmitkViewNavigatorWidget()
{

}

void QmitkViewNavigatorWidget::setFocus()
{
  m_Controls.lineEdit->setFocus();
}

void QmitkViewNavigatorWidget::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    if (berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow().IsNotNull())
    {
        m_PerspectiveListener.reset(new ViewNavigatorPerspectiveListener(this));
        berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->AddPerspectiveListener(m_PerspectiveListener.data());
    }
    else
    {
        m_WindowListener.reset(new ViewNavigatorWindowListener(this));
        berry::PlatformUI::GetWorkbench()->AddWindowListener(m_WindowListener.data());
    }

    m_Parent = parent;
    m_Controls.setupUi( parent );
    connect( m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
    connect( m_Controls.m_PluginTreeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));
    connect( m_Controls.lineEdit, SIGNAL(textChanged(QString)), SLOT(FilterChanged()));

    m_ContextMenu = new QMenu(m_Controls.m_PluginTreeView);
    m_Controls.m_PluginTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create a new TreeModel for the data
    m_TreeModel = new QStandardItemModel();
    m_FilterProxyModel = new ClassFilterProxyModel(this);
    m_FilterProxyModel->setSourceModel(m_TreeModel);
    //proxyModel->setFilterFixedString("Diff");
    m_Controls.m_PluginTreeView->setModel(m_FilterProxyModel);
}

void QmitkViewNavigatorWidget::UpdateTreeList(QStandardItem* root, berry::IWorkbenchPartReference *partRef, const std::string &changeId)
{
    berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
    if (page.IsNull())
        return;

    if( !m_Generated )
    {
      m_Generated = FillTreeList();
    }

    if (root==NULL)
        root = m_TreeModel->invisibleRootItem();
    for (int i=0; i<root->rowCount(); i++)
    {
        QStandardItem* item = root->child(i);
        QFont font;
        if (dynamic_cast<mitk::QtPerspectiveItem*>(item))
        {
            mitk::QtPerspectiveItem* pItem = dynamic_cast<mitk::QtPerspectiveItem*>(item);
            berry::IPerspectiveDescriptor::Pointer currentPersp = page->GetPerspective();

            if (currentPersp.IsNotNull() && currentPersp->GetId()==pItem->m_Perspective->GetId())
                font.setBold(true);
            pItem->setFont(font);
        }
        mitk::QtViewItem* vItem = dynamic_cast<mitk::QtViewItem*>(item);
        if (vItem)
        {
            QList<berry::IViewPart::Pointer> viewParts(page->GetViews());
            for (int i=0; i<viewParts.size(); i++)
                if(viewParts[i]->GetPartName()==vItem->m_View->GetLabel())
                {
                    font.setBold(true);
                    break;
                }

            if( partRef!=NULL && partRef->GetId()==vItem->m_View->GetId() && changeId=="viewHide")
                font.setBold(false);

            vItem->setFont(font);
        }
        UpdateTreeList(item, partRef, changeId);
    }
}

bool QmitkViewNavigatorWidget::FillTreeList()
{
    // active workbench window available?
    if (berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow().IsNull())
        return false;

    // active page available?
    berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
    if (page.IsNull())
        return false;

    // everything is fine and we can remove the window listener
    if (m_WindowListener != nullptr)
        berry::PlatformUI::GetWorkbench()->RemoveWindowListener(m_WindowListener.data());

    // initialize tree model
    m_TreeModel->clear();
    QStandardItem *treeRootItem = m_TreeModel->invisibleRootItem();

    // get all available perspectives
    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    QList<berry::IPerspectiveDescriptor::Pointer> perspectiveDescriptors(perspRegistry->GetPerspectives());
    qSort(perspectiveDescriptors.begin(), perspectiveDescriptors.end(), comparePerspectives);

    // get all Keywords
    KeywordRegistry keywordRegistry;

    berry::IPerspectiveDescriptor::Pointer currentPersp = page->GetPerspective();
    QStringList perspectiveExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetPerspectiveExcludeList();

    std::vector< QStandardItem* > categoryItems;
    QStandardItem *perspectiveRootItem = new QStandardItem("Workflows");
    perspectiveRootItem->setEditable(false);
    perspectiveRootItem->setFont(QFont("", 12, QFont::Normal));
    treeRootItem->appendRow(perspectiveRootItem);

    for (int i=0; i<perspectiveDescriptors.size(); i++)
    {
        berry::IPerspectiveDescriptor::Pointer p = perspectiveDescriptors.at(i);

        bool skipPerspective = false;
        for(int e=0; e<perspectiveExcludeList.size(); e++)
            if(perspectiveExcludeList.at(e)==p->GetId())
            {
                skipPerspective = true;
                break;
            }
        if (skipPerspective)
            continue;

        //QIcon* pIcon = static_cast<QIcon*>(p->GetImageDescriptor()->CreateImage());
        mitk::QtPerspectiveItem* pItem = new mitk::QtPerspectiveItem(p->GetLabel());
        pItem->m_Perspective = p;
        pItem->m_Description = p->GetDescription();
        QStringList keylist = p->GetKeywordReferences();
        pItem->m_Tags = keywordRegistry.GetKeywords(keylist);
        pItem->setEditable(false);

        QFont font; font.setBold(true);
        if (currentPersp.IsNotNull() && currentPersp->GetId()==p->GetId())
            pItem->setFont(font);

        QStringList catPath = p->GetCategoryPath();
        if (catPath.isEmpty())
        {
            perspectiveRootItem->appendRow(pItem);
        }
        else
        {
            QStandardItem* categoryItem = NULL;

            for (unsigned int c=0; c<categoryItems.size(); c++)
            {
                if (categoryItems.at(c)->text() == catPath.front())
                {
                    categoryItem = categoryItems.at(c);
                    break;
                }
            }

            if (categoryItem==NULL)
            {
                categoryItem  = new QStandardItem(QIcon(),catPath.front());
                categoryItems.push_back(categoryItem);
            }
            categoryItem->setEditable(false);
            categoryItem->appendRow(pItem);
            categoryItem->setFont(QFont("", 12, QFont::Normal));
        }
    }
    std::sort(categoryItems.begin(), categoryItems.end(), compareQStandardItems);
    for (unsigned int i=0; i<categoryItems.size(); i++)
        perspectiveRootItem->appendRow(categoryItems.at(i));

    // get all available views
    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    QList<berry::IViewDescriptor::Pointer> viewDescriptors(viewRegistry->GetViews());
    QList<berry::IViewPart::Pointer> viewParts(page->GetViews());
    qSort(viewDescriptors.begin(), viewDescriptors.end(), compareViews);
    QStandardItem* emptyItem = new QStandardItem();
    emptyItem->setFlags(Qt::ItemIsEnabled);
    treeRootItem->appendRow(emptyItem);
    QStringList viewExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetViewExcludeList();
    QStandardItem* viewRootItem = new QStandardItem(QIcon(),"Views");
    viewRootItem->setFont(QFont("", 12, QFont::Normal));
    viewRootItem->setEditable(false);
    treeRootItem->appendRow(viewRootItem);

    categoryItems.clear();
    QStandardItem* noCategoryItem = new QStandardItem(QIcon(),"Miscellaneous");
    noCategoryItem->setEditable(false);
    noCategoryItem->setFont(QFont("", 12, QFont::Normal));

    for (int i = 0; i < viewDescriptors.size(); ++i)
    {
        berry::IViewDescriptor::Pointer v = viewDescriptors[i];
        bool skipView = false;
        for(int e=0; e<viewExcludeList.size(); e++)
            if(viewExcludeList.at(e)==v->GetId())
            {
                skipView = true;
                break;
            }
        if (skipView)
            continue;

        QStringList catPath = v->GetCategoryPath();

        QIcon icon = v->GetImageDescriptor();
        mitk::QtViewItem* vItem = new mitk::QtViewItem(icon, v->GetLabel());
        vItem->m_View = v;
        vItem->setToolTip(v->GetDescription());
        vItem->m_Description = v->GetDescription();

        QStringList keylist = v->GetKeywordReferences();
        vItem->m_Tags = keywordRegistry.GetKeywords(keylist);
        vItem->setEditable(false);

        for (int i=0; i<viewParts.size(); i++)
            if(viewParts[i]->GetPartName()==v->GetLabel())
            {
                QFont font; font.setBold(true);
                vItem->setFont(font);
                break;
            }

        if (catPath.empty())
            noCategoryItem->appendRow(vItem);
        else
        {
            QStandardItem* categoryItem = NULL;

            for (unsigned int c=0; c<categoryItems.size(); c++)
                if (categoryItems.at(c)->text() == catPath.front())
                {
                    categoryItem = categoryItems.at(c);
                    break;
                }

            if (categoryItem==NULL)
            {
                categoryItem  = new QStandardItem(QIcon(),catPath.front());
                categoryItems.push_back(categoryItem);
            }
            categoryItem->setEditable(false);
            categoryItem->appendRow(vItem);
            categoryItem->setFont(QFont("", 12, QFont::Normal));
        }
    }
    std::sort(categoryItems.begin(), categoryItems.end(), compareQStandardItems);

    for (unsigned int i=0; i<categoryItems.size(); i++)
        viewRootItem->appendRow(categoryItems.at(i));
    if (noCategoryItem->hasChildren())
        viewRootItem->appendRow(noCategoryItem);

    m_Controls.m_PluginTreeView->expandAll();

    return true;
}

void QmitkViewNavigatorWidget::FilterChanged()
{
    QString filterString = m_Controls.lineEdit->text();
//    if (filterString.size() > 0 )
    m_Controls.m_PluginTreeView->expandAll();
//    else
//        m_Controls.m_PluginTreeView->collapseAll();
    //    QRegExp::PatternSyntax syntax = QRegExp::RegExp;

    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
    QString strPattern = "^*" + filterString;
    QRegExp regExp(strPattern, caseSensitivity);

    m_FilterProxyModel->setFilterRegExp(regExp);
}

void QmitkViewNavigatorWidget::ItemClicked(const QModelIndex &index)
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
                BERRY_ERROR << "Error: " << e.what() << std::endl;
            }
        }
    }
}

void QmitkViewNavigatorWidget::AddPerspective()
{
    QmitkNewPerspectiveDialog* dialog = new QmitkNewPerspectiveDialog( m_Parent );

    int dialogReturnValue = dialog->exec();
    if ( dialogReturnValue == QDialog::Rejected )
        return;

    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    try
    {
        berry::IPerspectiveDescriptor::Pointer perspDesc;
        perspDesc = perspRegistry->CreatePerspective(dialog->GetPerspectiveName(), perspRegistry->FindPerspectiveWithId(perspRegistry->GetDefaultPerspective()));
        berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->SetPerspective(perspDesc);
    }
    catch(...)
    {
        QMessageBox::warning(m_Parent, "Error", "Duplication of selected perspective failed. Please make sure the specified perspective name is not already in use!");
    }
    FillTreeList();
}

void QmitkViewNavigatorWidget::ClonePerspective()
{
    if (m_RegisteredPerspective.IsNotNull())
    {
        QmitkNewPerspectiveDialog* dialog = new QmitkNewPerspectiveDialog( m_Parent );
        QString defaultName = m_RegisteredPerspective->GetLabel();
        defaultName.append(" Copy");
        dialog->SetPerspectiveName(defaultName);

        int dialogReturnValue = dialog->exec();
        if ( dialogReturnValue == QDialog::Rejected )
            return;

        berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
        try
        {
            berry::IPerspectiveDescriptor::Pointer perspDesc = perspRegistry->ClonePerspective(dialog->GetPerspectiveName(), dialog->GetPerspectiveName(), m_RegisteredPerspective);
            berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->SetPerspective(perspDesc);
        }
        catch(...)
        {
            QMessageBox::warning(m_Parent, "Error", "Duplication of selected perspective failed. Please make sure the specified perspective name is not already in use!");
        }
        FillTreeList();
    }
}

void QmitkViewNavigatorWidget::ResetPerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", "Do you really want to reset the current perspective?", QMessageBox::Yes|QMessageBox::No).exec())
        berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
}

void QmitkViewNavigatorWidget::DeletePerspective()
{
    if (m_RegisteredPerspective.IsNotNull())
    {
        QString question = "Do you really want to remove the perspective '";
        question.append(m_RegisteredPerspective->GetLabel());
        question.append("'?");
        if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", question, QMessageBox::Yes|QMessageBox::No).exec())
        {
          if( m_RegisteredPerspective == berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->GetPerspective() )
          {
            berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->CloseCurrentPerspective(true, true);
          }
            berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
            perspRegistry->DeletePerspective(m_RegisteredPerspective);
            berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->RemovePerspective(m_RegisteredPerspective);
            FillTreeList();
            if (! berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->GetPerspective())
            {
                berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->Close();
            }
        }
    }
}

void QmitkViewNavigatorWidget::ClosePerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", "Do you really want to close the current perspective?", QMessageBox::Yes|QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
        page->CloseCurrentPerspective(true, true);

        //        if ( page->GetPerspective().IsNull() )
        //        {
        //            berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
        //            berry::PlatformUI::GetWorkbench()->ShowPerspective( perspRegistry->GetDefaultPerspective(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
        //        }
    }
}

void QmitkViewNavigatorWidget::CloseAllPerspectives()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm", "Do you really want to close all perspectives?", QMessageBox::Yes|QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
        page->CloseAllPerspectives(true, true);

        //        berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
        //        berry::PlatformUI::GetWorkbench()->ShowPerspective( perspRegistry->GetDefaultPerspective(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow() );
    }
}

void QmitkViewNavigatorWidget::ExpandAll()
{
    m_Controls.m_PluginTreeView->expandAll();
}

void QmitkViewNavigatorWidget::CollapseAll()
{
    m_Controls.m_PluginTreeView->collapseAll();
}

void QmitkViewNavigatorWidget::CustomMenuRequested(QPoint pos)
{
    QModelIndex index = m_Controls.m_PluginTreeView->indexAt(pos);
    QStandardItem* item = m_TreeModel->itemFromIndex(m_FilterProxyModel->mapToSource(index));

    if (m_ContextMenu==NULL)
        return;

    m_ContextMenu->clear();
    m_RegisteredPerspective = NULL;

    QAction* expandAction = new QAction("Expand tree", this);
    m_ContextMenu->addAction(expandAction);
    connect(expandAction, SIGNAL(triggered()), SLOT(ExpandAll()));

    QAction* collapseAction = new QAction("Collapse tree", this);
    m_ContextMenu->addAction(collapseAction);
    connect(collapseAction, SIGNAL(triggered()), SLOT(CollapseAll()));

    m_ContextMenu->addSeparator();

    if ( item!=NULL && dynamic_cast< mitk::QtPerspectiveItem* >(item) )
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

        m_ContextMenu->addSeparator();
    }

    QAction* resetAction = new QAction("Reset current perspective", this);
    m_ContextMenu->addAction(resetAction);
    connect(resetAction, SIGNAL(triggered()), SLOT(ResetPerspective()));

    QAction* closeAction = new QAction("Close current perspective", this);
    m_ContextMenu->addAction(closeAction);
    connect(closeAction, SIGNAL(triggered()), SLOT(ClosePerspective()));

    m_ContextMenu->addSeparator();

    QAction* closeAllAction = new QAction("Close all perspectives", this);
    m_ContextMenu->addAction(closeAllAction);
    connect(closeAllAction, SIGNAL(triggered()), SLOT(CloseAllPerspectives()));

    m_ContextMenu->popup(m_Controls.m_PluginTreeView->viewport()->mapToGlobal(pos));
}
