/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// View navigator plugin
#include <QmitkViewNavigatorWidget.h>
#include <QmitkPerspectiveItem.h>
#include <QmitkViewItem.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
#include <berryPlatformUI.h>
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>
#include <berryIExtensionRegistry.h>
#include <berryIHandlerService.h>
#include <berryIWorkbenchCommandConstants.h>
#include <berryUIElement.h>

// MITK
#include <mitkLogMacros.h>

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
        for (const auto& id : ids)
        {
            result.append(this->GetKeywords(id));
        }
        return result;
    }

private:

    QHash<QString, QStringList> m_Keywords;
};


class ClassFilterProxyModel : public QSortFilterProxyModel
{
public:

  ClassFilterProxyModel(QObject* parent = nullptr)
    : QSortFilterProxyModel(parent)
  {
  }

  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
  {
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    return hasToBeDisplayed(index);
  }

private:

  bool displayElement(const QModelIndex index) const
  {
    QString type = sourceModel()->data(index, Qt::DisplayRole).toString();
    QStandardItem* item = dynamic_cast<QStandardItemModel*>(sourceModel())->itemFromIndex(index);

    if (type.contains(filterRegExp()))
    {
        return true;
    }

    QmitkViewItem* viewItem = dynamic_cast<QmitkViewItem*>(item);
    if (nullptr != viewItem)
    {
        for (const auto& tag : viewItem->m_Tags)
        {
            if (tag.contains(filterRegExp()))
            {
                return true;
            }
        }
        if (viewItem->m_Description.contains(filterRegExp()))
        {
            return true;
        }
    }

    QmitkPerspectiveItem* perspectiveItem = dynamic_cast<QmitkPerspectiveItem*>(item);
    if (nullptr != perspectiveItem)
    {
        for (const auto& tag : perspectiveItem->m_Tags)
        {
            if (tag.contains(filterRegExp()))
            {
                return true;
            }
        }
        if (perspectiveItem->m_Description.contains(filterRegExp()))
        {
            return true;
        }
    }

    return false;
  }

  bool hasToBeDisplayed(const QModelIndex index) const
  {
    bool result = false;
    if (sourceModel()->rowCount(index) > 0)
    {
        for (int i = 0; i < sourceModel()->rowCount(index); i++)
        {
            QModelIndex childIndex = sourceModel()->index(i, 0, index);
            if (!childIndex.isValid())
            {
                break;
            }

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
};

class ViewNavigatorPerspectiveListener: public berry::IPerspectiveListener
{
public:

    ViewNavigatorPerspectiveListener(QmitkViewNavigatorWidget* parent)
      : m_ParentWidget(parent)
    {
    }

    Events::Types GetPerspectiveEventTypes() const override
    {
        return Events::ACTIVATED | Events::SAVED_AS | Events::DEACTIVATED
                // remove the following line when command framework is finished
                | Events::CLOSED | Events::OPENED | Events::PART_CHANGED;
    }

    void PerspectiveActivated(const berry::IWorkbenchPage::Pointer& /*page*/,
                              const berry::IPerspectiveDescriptor::Pointer& perspective) override
    {
        m_ParentWidget->m_ActivePerspective = perspective;
        m_ParentWidget->UpdateTreeList();
    }

    void PerspectiveSavedAs(const berry::IWorkbenchPage::Pointer& /*page*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*oldPerspective*/,
                            const berry::IPerspectiveDescriptor::Pointer& newPerspective) override
    {
        m_ParentWidget->m_ActivePerspective = newPerspective;
        m_ParentWidget->UpdateTreeList();
    }

    void PerspectiveDeactivated(const berry::IWorkbenchPage::Pointer& /*page*/,
                                const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
    {
        m_ParentWidget->m_ActivePerspective = nullptr;
        m_ParentWidget->UpdateTreeList();
    }

    void PerspectiveOpened(const berry::IWorkbenchPage::Pointer& /*page*/,
                           const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
    {
        m_ParentWidget->UpdateTreeList();
    }

    void PerspectiveClosed(const berry::IWorkbenchPage::Pointer& /*page*/,
                           const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
    {
        m_ParentWidget->m_ActivePerspective = nullptr;
        m_ParentWidget->UpdateTreeList();
    }

    using IPerspectiveListener::PerspectiveChanged;

    void PerspectiveChanged(const berry::IWorkbenchPage::Pointer& /*page*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*perspective*/,
                            const berry::IWorkbenchPartReference::Pointer& partRef, const std::string& changeId)
    {
        if (changeId == "viewHide" && partRef->GetId() == "org.mitk.views.viewnavigator")
            berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->RemovePerspectiveListener(m_ParentWidget->m_PerspectiveListener.data());
        else
            m_ParentWidget->UpdateTreeList(nullptr, partRef.GetPointer(), changeId);
    }

private:

    QmitkViewNavigatorWidget* m_ParentWidget;
};

bool compareViews(const berry::IViewDescriptor::Pointer& a, const berry::IViewDescriptor::Pointer& b)
{
    if (a.IsNull() || b.IsNull())
    {
        return false;
    }

    return a->GetLabel().compare(b->GetLabel()) < 0;
}

bool comparePerspectives(const berry::IPerspectiveDescriptor::Pointer& a, const berry::IPerspectiveDescriptor::Pointer& b)
{
    if (a.IsNull() || b.IsNull())
    {
        return false;
    }

    return a->GetLabel().compare(b->GetLabel()) < 0;
}

bool compareQStandardItems(const QStandardItem* a, const QStandardItem* b)
{
    if (nullptr == a || nullptr== b)
    {
        return false;
    }

    return a->text().compare(b->text()) < 0;
}

QmitkViewNavigatorWidget::QmitkViewNavigatorWidget(berry::IWorkbenchWindow::Pointer window,
                                                   QWidget* parent,
                                                   Qt::WindowFlags)
    : QWidget(parent)
    , m_Window(window)
{
    m_Generated = false;
    this->CreateQtPartControl(this);
}

QmitkViewNavigatorWidget::~QmitkViewNavigatorWidget()
{
  m_Window->RemovePerspectiveListener(m_PerspectiveListener.data());
}

void QmitkViewNavigatorWidget::SetFocus()
{
  m_Controls.lineEdit->setFocus();
}

void QmitkViewNavigatorWidget::UpdateTreeList(QStandardItem* root,
                                              berry::IWorkbenchPartReference* partRef,
                                              const std::string& changeId)
{
    berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
    if (page.IsNull())
        return;

    if (!m_Generated)
    {
      m_Generated = FillTreeList();
    }

    if (nullptr == root)
        root = m_TreeModel->invisibleRootItem();

    for (int i = 0; i < root->rowCount(); i++)
    {
        QStandardItem* item = root->child(i);
        QFont font;
        QmitkPerspectiveItem* perspectiveItem = dynamic_cast<QmitkPerspectiveItem*>(item);
        if (nullptr != perspectiveItem)
        {
            berry::IPerspectiveDescriptor::Pointer currentPerspective = page->GetPerspective();

            if (currentPerspective.IsNotNull() && currentPerspective->GetId() == perspectiveItem->m_Perspective->GetId())
                font.setBold(true);
            perspectiveItem->setFont(font);
        }

        QmitkViewItem* viewItem = dynamic_cast<QmitkViewItem*>(item);
        if (nullptr != viewItem)
        {
            QList<berry::IViewPart::Pointer> viewParts(page->GetViews());
            for (int i = 0; i < viewParts.size(); i++)
            {
                if (viewParts[i]->GetPartName() == viewItem->m_View->GetLabel())
                {
                    font.setBold(true);
                    break;
                }
            }

            if (partRef != nullptr && partRef->GetId() == viewItem->m_View->GetId() && changeId == "viewHide")
                font.setBold(false);

            viewItem->setFont(font);
        }

        UpdateTreeList(item, partRef, changeId);
    }
}

bool QmitkViewNavigatorWidget::FillTreeList()
{
    // active workbench window available?
    if (m_Window.IsNull())
        return false;

    // active page available?
    berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
    if (page.IsNull())
        return false;

    // everything is fine and we can remove the window listener
    if (nullptr != m_WindowListener)
        berry::PlatformUI::GetWorkbench()->RemoveWindowListener(m_WindowListener.data());

    // initialize tree model
    m_TreeModel->clear();
    QStandardItem* treeRootItem = m_TreeModel->invisibleRootItem();

    // get all available perspectives
    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
    QList<berry::IPerspectiveDescriptor::Pointer> perspectiveDescriptors(perspRegistry->GetPerspectives());
    qSort(perspectiveDescriptors.begin(), perspectiveDescriptors.end(), comparePerspectives);

    KeywordRegistry keywordRegistry;
    berry::IPerspectiveDescriptor::Pointer currentPersp = page->GetPerspective();
    std::vector< QStandardItem* > categoryItems;
    QStandardItem *perspectiveRootItem = new QStandardItem("Perspectives");
    perspectiveRootItem->setEditable(false);
    perspectiveRootItem->setFont(QFont("", 12, QFont::Normal));
    treeRootItem->appendRow(perspectiveRootItem);

    for (const auto& perspectiveDescriptor : perspectiveDescriptors)
    {
        QmitkPerspectiveItem* perspectiveItem = new QmitkPerspectiveItem(perspectiveDescriptor->GetLabel());
        perspectiveItem->m_Perspective = perspectiveDescriptor;
        perspectiveItem->m_Description = perspectiveDescriptor->GetDescription();
        QStringList keylist = perspectiveDescriptor->GetKeywordReferences();
        perspectiveItem->m_Tags = keywordRegistry.GetKeywords(keylist);
        perspectiveItem->setEditable(false);


        QFont font;
        font.setBold(true);
        if (currentPersp.IsNotNull() && currentPersp->GetId() == perspectiveDescriptor->GetId())
            perspectiveItem->setFont(font);

        QStringList catPath = perspectiveDescriptor->GetCategoryPath();
        if (catPath.isEmpty())
        {
            perspectiveRootItem->appendRow(perspectiveItem);
        }
        else
        {
            QStandardItem* categoryItem = nullptr;
            for (const auto& currentCategoryItem : categoryItems)
            {
                if (currentCategoryItem->text() == catPath.front())
                {
                    categoryItem = currentCategoryItem;
                    break;
                }
            }

            if (nullptr == categoryItem)
            {
                categoryItem = new QStandardItem(QIcon(), catPath.front());
                categoryItems.push_back(categoryItem);
            }
            categoryItem->setEditable(false);
            categoryItem->appendRow(perspectiveItem);
            categoryItem->setFont(QFont("", 12, QFont::Normal));
        }
    }

    std::sort(categoryItems.begin(), categoryItems.end(), compareQStandardItems);
    for (const auto& categoryItem : categoryItems)
    {
        perspectiveRootItem->appendRow(categoryItem);
    }

    // get all available views
    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    QList<berry::IViewDescriptor::Pointer> viewDescriptors(viewRegistry->GetViews());
    QList<berry::IViewPart::Pointer> viewParts(page->GetViews());
    qSort(viewDescriptors.begin(), viewDescriptors.end(), compareViews);
    auto emptyItem = new QStandardItem();
    emptyItem->setFlags(Qt::ItemIsEnabled);
    treeRootItem->appendRow(emptyItem);
    //QStringList viewExcludeList = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetViewExcludeList();

    // There currently is no way to get the list of excluded views at application start
    QStringList viewExcludeList;
    // internal view used for the intro screen, will crash when opened directly, see T22352
    viewExcludeList.append(QString("org.blueberry.ui.internal.introview"));
    viewExcludeList.append(QString("org.mitk.views.controlvisualizationpropertiesview"));
    viewExcludeList.append(QString("org.mitk.views.modules"));
    viewExcludeList.append(QString("org.mitk.views.viewnavigator"));

    QStandardItem* viewRootItem = new QStandardItem(QIcon(), "Views");
    viewRootItem->setFont(QFont("", 12, QFont::Normal));
    viewRootItem->setEditable(false);
    treeRootItem->appendRow(viewRootItem);

    categoryItems.clear();
    QStandardItem* noCategoryItem = new QStandardItem(QIcon(), "Miscellaneous");
    noCategoryItem->setEditable(false);
    noCategoryItem->setFont(QFont("", 12, QFont::Normal));

    for (const auto& viewDescriptor : viewDescriptors)
    {
        bool skipView = false;
        for (const auto& viewExcludeElement : viewExcludeList)
        {
            if(viewExcludeElement == viewDescriptor->GetId())
            {
                skipView = true;
                break;
            }
        }

        if (skipView)
            continue;

        QStringList catPath = viewDescriptor->GetCategoryPath();

        QIcon icon = viewDescriptor->GetImageDescriptor();
        QmitkViewItem* viewItem = new QmitkViewItem(icon, viewDescriptor->GetLabel());
        viewItem->m_View = viewDescriptor;
        viewItem->setToolTip(viewDescriptor->GetDescription());
        viewItem->m_Description = viewDescriptor->GetDescription();

        QStringList keylist = viewDescriptor->GetKeywordReferences();
        viewItem->m_Tags = keywordRegistry.GetKeywords(keylist);
        viewItem->setEditable(false);

        for (const auto& viewPart : viewParts)
        {
            if (viewPart->GetPartName() == viewDescriptor->GetLabel())
            {
                QFont font; font.setBold(true);
                viewItem->setFont(font);
                break;
            }
        }

        if (catPath.empty())
            noCategoryItem->appendRow(viewItem);
        else
        {
            QStandardItem* categoryItem = nullptr;
            for (const auto& currentCategoryItem : categoryItems)
            {
                if (currentCategoryItem->text() == catPath.front())
                {
                    categoryItem = currentCategoryItem;
                    break;
                }
            }

            if (nullptr == categoryItem)
            {
                categoryItem = new QStandardItem(QIcon(),catPath.front());
                categoryItems.push_back(categoryItem);
            }
            categoryItem->setEditable(false);
            categoryItem->appendRow(viewItem);
            categoryItem->setFont(QFont("", 12, QFont::Normal));
        }
    }

    std::sort(categoryItems.begin(), categoryItems.end(), compareQStandardItems);

    for (const auto& categoryItem : categoryItems)
    {
        viewRootItem->appendRow(categoryItem);
    }

    if (noCategoryItem->hasChildren())
        viewRootItem->appendRow(noCategoryItem);

    m_Controls.m_PluginTreeView->expandAll();

    return true;
}

void QmitkViewNavigatorWidget::FilterChanged()
{
    QString filterString = m_Controls.lineEdit->text();
    m_Controls.m_PluginTreeView->expandAll();

    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
    QString strPattern = "^*" + filterString;
    QRegExp regExp(strPattern, caseSensitivity);

    m_FilterProxyModel->setFilterRegExp(regExp);
}

void QmitkViewNavigatorWidget::ItemClicked(const QModelIndex &index)
{
    QStandardItem* item = m_TreeModel->itemFromIndex(m_FilterProxyModel->mapToSource(index));

    QmitkPerspectiveItem* perspectiveItem = dynamic_cast<QmitkPerspectiveItem*>(item);
    if (nullptr != perspectiveItem)
    {
        try
        {
            berry::PlatformUI::GetWorkbench()->ShowPerspective(
              perspectiveItem->m_Perspective->GetId(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow());
        }
        catch (...)
        {
            QMessageBox::critical(nullptr, "Opening Perspective Failed",
                                  QString("The requested perspective could not be opened.\nSee the log for details."));
        }

        return;
    }

    QmitkViewItem* viewItem = dynamic_cast<QmitkViewItem*>(item);
    if (nullptr != viewItem)
    {
        berry::IWorkbenchPage::Pointer page =
          berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();

        if (page.IsNotNull())
        {
            try
            {
                page->ShowView(viewItem->m_View->GetId());
            }
            catch (const berry::PartInitException& e)
            {
                BERRY_ERROR << "Error: " << e.what() << std::endl;
            }
        }
    }
}

void QmitkViewNavigatorWidget::SaveCurrentPerspectiveAs()
{
  berry::IHandlerService* handlerService = m_Window->GetService<berry::IHandlerService>();
  try
  {
    handlerService->ExecuteCommand(berry::IWorkbenchCommandConstants::WINDOW_SAVE_PERSPECTIVE_AS,
                                   berry::UIElement::Pointer());
    FillTreeList();
  }
  catch(const berry::NotHandledException&)
  {
  }
  catch(const berry::CommandException& e)
  {
    MITK_ERROR << e.what();
  }
}

void QmitkViewNavigatorWidget::ResetCurrentPerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm",
                                        "Do you really want to reset the current perspective?",
                                        QMessageBox::Yes | QMessageBox::No).exec())
    {
        berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
    }
}

void QmitkViewNavigatorWidget::ClosePerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm",
                                        "Do you really want to close the current perspective?",
                                        QMessageBox::Yes | QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
        page->ClosePerspective(page->GetPerspective(), true, true);
    }
}

void QmitkViewNavigatorWidget::CloseAllPerspectives()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm",
                                        "Do you really want to close all perspectives?",
                                        QMessageBox::Yes | QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
        page->CloseAllPerspectives(true, true);
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

    if (nullptr == m_ContextMenu)
        return;

    m_ContextMenu->clear();

    QmitkPerspectiveItem* perspectiveItem = dynamic_cast<QmitkPerspectiveItem*>(item);
    if (nullptr != perspectiveItem)
    {
        berry::IPerspectiveDescriptor::Pointer perspectiveDescriptor = perspectiveItem->m_Perspective;
        if (this->m_ActivePerspective.IsNotNull() && this->m_ActivePerspective == perspectiveDescriptor)
        {
          QAction* saveAsAction = new QAction("Save As...", this);
          m_ContextMenu->addAction(saveAsAction);
          connect(saveAsAction, SIGNAL(triggered()), SLOT(SaveCurrentPerspectiveAs()));
          m_ContextMenu->addSeparator();
        }
    }

    QAction* resetAction = new QAction("Reset current perspective", this);
    m_ContextMenu->addAction(resetAction);
    connect(resetAction, SIGNAL(triggered()), SLOT(ResetCurrentPerspective()));

    QAction* closeAction = new QAction("Close perspective", this);
    m_ContextMenu->addAction(closeAction);
    connect(closeAction, SIGNAL(triggered()), SLOT(ClosePerspective()));

    QAction* closeAllAction = new QAction("Close all perspectives", this);
    m_ContextMenu->addAction(closeAllAction);
    connect(closeAllAction, SIGNAL(triggered()), SLOT(CloseAllPerspectives()));

    m_ContextMenu->addSeparator();

    QAction* expandAction = new QAction("Expand tree", this);
    m_ContextMenu->addAction(expandAction);
    connect(expandAction, SIGNAL(triggered()), SLOT(ExpandAll()));

    QAction* collapseAction = new QAction("Collapse tree", this);
    m_ContextMenu->addAction(collapseAction);
    connect(collapseAction, SIGNAL(triggered()), SLOT(CollapseAll()));

    m_ContextMenu->popup(m_Controls.m_PluginTreeView->viewport()->mapToGlobal(pos));
}

void QmitkViewNavigatorWidget::CreateQtPartControl(QWidget* parent)
{
  m_PerspectiveListener.reset(new ViewNavigatorPerspectiveListener(this));
  m_Window->AddPerspectiveListener(m_PerspectiveListener.data());

  m_Parent = parent;
  m_Controls.setupUi(parent);
  connect(m_Controls.m_PluginTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
  connect(m_Controls.m_PluginTreeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemClicked(const QModelIndex&)));
  connect(m_Controls.lineEdit, SIGNAL(textChanged(QString)), SLOT(FilterChanged()));

  m_ContextMenu = new QMenu(m_Controls.m_PluginTreeView);
  m_Controls.m_PluginTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

  // Create a new TreeModel for the data
  m_TreeModel = new QStandardItemModel();
  m_FilterProxyModel = new ClassFilterProxyModel(this);
  m_FilterProxyModel->setSourceModel(m_TreeModel);
  m_Controls.m_PluginTreeView->setModel(m_FilterProxyModel);

  this->UpdateTreeList();
}
