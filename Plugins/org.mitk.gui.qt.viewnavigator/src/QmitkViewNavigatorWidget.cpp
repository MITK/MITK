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
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
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
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardItem>
#include <QSortFilterProxyModel>

namespace
{
  QFont getLargeFont()
  {
    QFont font = qApp->font();
    font.setPointSizeF(font.pointSizeF() * 1.25f);
    return font;
  }
}

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
                              const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
    {
        m_ParentWidget->UpdateTreeList();
    }

    void PerspectiveSavedAs(const berry::IWorkbenchPage::Pointer& /*page*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*oldPerspective*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*newPerspective*/) override
    {
        m_ParentWidget->UpdateTreeList();
    }

    void PerspectiveDeactivated(const berry::IWorkbenchPage::Pointer& /*page*/,
                                const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
    {
        m_ParentWidget->m_ActivePerspective = nullptr;
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
    }

    using IPerspectiveListener::PerspectiveChanged;

    void PerspectiveChanged(const berry::IWorkbenchPage::Pointer& /*page*/,
                            const berry::IPerspectiveDescriptor::Pointer& /*perspective*/,
                            const berry::IWorkbenchPartReference::Pointer& partRef, const std::string& changeId)
    {
        if (changeId == "viewHide" && partRef->GetId() == "org.mitk.views.viewnavigator")
            berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->RemovePerspectiveListener(m_ParentWidget->m_PerspectiveListener.data());
        else
            m_ParentWidget->UpdateTreeList();
    }

private:

    QmitkViewNavigatorWidget* m_ParentWidget;
};

class ViewNavigatorViewListener: public berry::IPartListener
{
public:

  ViewNavigatorViewListener(QmitkViewNavigatorWidget* parent)
      : m_ParentWidget(parent)
    {
    }

    Events::Types GetPartEventTypes() const override
    {
        return Events::OPENED | Events::CLOSED;
    }

    void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override
    {
      if (partRef->GetId() != "org.mitk.views.viewnavigator")
      {
        m_ParentWidget->UpdateTreeList((partRef->GetPart(false)).GetPointer());
      }
      else
      {
        m_ParentWidget->FillTreeList();
        m_ParentWidget->UpdateTreeList();
      }
    }

    void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override
    {
      if (partRef->GetId() != "org.mitk.views.viewnavigator")
      {
        m_ParentWidget->UpdateTreeList();
      }
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
    this->CreateQtPartControl(this);
}

QmitkViewNavigatorWidget::~QmitkViewNavigatorWidget()
{
  m_Window->RemovePerspectiveListener(m_PerspectiveListener.data());
  m_Window->GetPartService()->RemovePartListener(m_ViewPartListener.data());
}

void QmitkViewNavigatorWidget::SetFocus()
{
  m_Controls.lineEdit->setFocus();
}

void QmitkViewNavigatorWidget::UpdateTreeList(berry::IWorkbenchPart* workbenchPart)
{
  berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
  if (page.IsNull())
  {
    return;
  }

  m_ActivePerspective = page->GetPerspective();
  QList<berry::IViewPart::Pointer> viewParts = page->GetViews();

  // iterate over all tree items
  for (const auto& item : m_TreeModel->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive))
  {
    QFont font = qApp->font();
    // check if the item is a view item and if it is equal to any opened view
    QmitkViewItem* viewItem = dynamic_cast<QmitkViewItem *>(item);
    if (nullptr != viewItem)
    {
      if (nullptr != workbenchPart && workbenchPart->GetPartName() == viewItem->m_ItemDescriptor->GetLabel())
      {
        font.setBold(true);
      }
      else
      {
        for (const auto& viewPart : viewParts)
        {
          if (viewPart->GetPartName() == viewItem->m_ItemDescriptor->GetLabel())
          {
            font.setBold(true);
            break;
          }
        }
      }

      viewItem->setFont(font);
    }
    else
    {
      // check if the item is a perspective item and if it is equal to the current perspective
      QmitkPerspectiveItem* perspectiveItem = dynamic_cast<QmitkPerspectiveItem*>(item);
      if (nullptr != perspectiveItem)
      {
        if (m_ActivePerspective.IsNotNull() && m_ActivePerspective->GetId() == perspectiveItem->m_ItemDescriptor->GetId())
        {
          font.setBold(true);
        }

        perspectiveItem->setFont(font);
      }
    }
  }
}

bool QmitkViewNavigatorWidget::FillTreeList()
{
    // initialize tree model
    m_TreeModel->clear();

    // add all available views
    this->AddViewsToTree();

    // add all available perspectives
    this->AddPerspectivesToTree();

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
              perspectiveItem->m_ItemDescriptor->GetId(), berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow());
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
                page->ShowView(viewItem->m_ItemDescriptor->GetId());
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
  berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
  berry::IPerspectiveDescriptor::Pointer currentPerspective = page->GetPerspective();

  bool ok = false;
  QString perspectiveLabel = QInputDialog::getText(this, "Save perspective as ...",
                                "New perspective name:", QLineEdit::Normal,
                                "", &ok);

  if (!ok)
  {
    return;
  }

  if (perspectiveLabel.isEmpty())
  {
    QMessageBox::information(this, "Save perspective as ...", "Please select a valid perspective name.");
    return;
  }

  berry::IPerspectiveRegistry* perspectiveRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
  berry::IPerspectiveDescriptor::Pointer newPerspective = perspectiveRegistry->CreatePerspective(perspectiveLabel, currentPerspective);

  if (nullptr == newPerspective)
  {
    QMessageBox::information(this, "Save perspective as ...", "The selected perspective name is already in use.");
    return;
  }

  page->SavePerspectiveAs(newPerspective);

  this->FillTreeList();
  this->UpdateTreeList();
}

void QmitkViewNavigatorWidget::ResetCurrentPerspective()
{
    if (QMessageBox::Yes == QMessageBox(QMessageBox::Question, "Please confirm",
                                        "Do you really want to reset the current perspective?",
                                        QMessageBox::Yes | QMessageBox::No).exec())
    {
        berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
        page->ResetPerspective();
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
        berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
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
        berry::IPerspectiveDescriptor::Pointer perspectiveDescriptor = perspectiveItem->m_ItemDescriptor;
        if (this->m_ActivePerspective.IsNotNull() && this->m_ActivePerspective == perspectiveDescriptor)
        {
          QAction* saveAsAction = new QAction("Save perspective as ...", this);
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
  // active workbench window available?
  if (m_Window.IsNull())
  {
    return;
  }

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

  m_PerspectiveListener.reset(new ViewNavigatorPerspectiveListener(this));
  m_Window->AddPerspectiveListener(m_PerspectiveListener.data());

  m_ViewPartListener.reset(new ViewNavigatorViewListener(this));
  m_Window->GetPartService()->AddPartListener(m_ViewPartListener.data());
}

void QmitkViewNavigatorWidget::AddPerspectivesToTree()
{
  berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
  QList<berry::IPerspectiveDescriptor::Pointer> perspectiveDescriptors(perspRegistry->GetPerspectives());
  qSort(perspectiveDescriptors.begin(), perspectiveDescriptors.end(), comparePerspectives);

  QStandardItem* perspectiveRootItem = new QStandardItem("Perspectives");
  perspectiveRootItem->setFont(getLargeFont());
  perspectiveRootItem->setEditable(false);
  QStandardItem* treeRootItem = m_TreeModel->invisibleRootItem();
  treeRootItem->appendRow(perspectiveRootItem);

  this->AddItemsToTree<QList<berry::IPerspectiveDescriptor::Pointer>, QmitkPerspectiveItem>(
    perspectiveDescriptors, perspectiveRootItem);
}

void QmitkViewNavigatorWidget::AddViewsToTree()
{
  berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
  QList<berry::IViewDescriptor::Pointer> viewDescriptors(viewRegistry->GetViews());
  qSort(viewDescriptors.begin(), viewDescriptors.end(), compareViews);

  auto largeFont = getLargeFont();

  QStandardItem* viewRootItem = new QStandardItem("Views");
  viewRootItem->setFont(largeFont);
  viewRootItem->setEditable(false);
  QStandardItem* treeRootItem = m_TreeModel->invisibleRootItem();
  treeRootItem->appendRow(viewRootItem);

  QStandardItem* miscellaneousCategoryItem = new QStandardItem("Miscellaneous");
  miscellaneousCategoryItem->setFont(largeFont);
  miscellaneousCategoryItem->setEditable(false);

  QStringList viewExcludeList;
  // internal view used for the intro screen, will crash when opened directly, see T22352
  viewExcludeList.append(QString("org.blueberry.ui.internal.introview"));
  viewExcludeList.append(QString("org.mitk.views.controlvisualizationpropertiesview"));
  viewExcludeList.append(QString("org.mitk.views.modules"));
  viewExcludeList.append(QString("org.mitk.views.viewnavigator"));

  this->AddItemsToTree<QList<berry::IViewDescriptor::Pointer>, QmitkViewItem>(
    viewDescriptors, viewRootItem, miscellaneousCategoryItem, viewExcludeList);
}

template<typename D, typename I>
void QmitkViewNavigatorWidget::AddItemsToTree(D itemDescriptors, QStandardItem* rootItem,
  QStandardItem* miscellaneousItem, const QStringList& itemExcludeList)
{
  KeywordRegistry keywordRegistry;
  std::vector<QStandardItem*> categoryItems;

  for (const auto& itemDescriptor : itemDescriptors)
  {
    bool excludeView = itemExcludeList.contains(itemDescriptor->GetId());
    if (excludeView)
    {
      continue;
    }

    QIcon icon = itemDescriptor->GetImageDescriptor();
    I* item = new I(icon, itemDescriptor->GetLabel());
    item->m_ItemDescriptor = itemDescriptor;
    item->m_Description = itemDescriptor->GetDescription();
    item->setToolTip(itemDescriptor->GetDescription());

    QStringList keylist = itemDescriptor->GetKeywordReferences();
    item->m_Tags = keywordRegistry.GetKeywords(keylist);
    item->setEditable(false);

    QStringList categoryPath = itemDescriptor->GetCategoryPath();
    if (categoryPath.empty())
    {
      // If a root item for general / non-categorized item views is given, use it.
      // Otherwise put the non-categorized item views into the top root item.
      if (nullptr != miscellaneousItem)
      {
        miscellaneousItem->appendRow(item);
      }
      else
      {
        rootItem->appendRow(item);
      }
    }
    else
    {
      QStandardItem* categoryItem = nullptr;
      for (const auto& currentCategoryItem : categoryItems)
      {
        if (currentCategoryItem->text() == categoryPath.front())
        {
          categoryItem = currentCategoryItem;
          break;
        }
      }

      if (nullptr == categoryItem)
      {
        categoryItem = new QStandardItem(QIcon(), categoryPath.front());
        categoryItems.push_back(categoryItem);
      }

      auto font = getLargeFont();

      categoryItem->setFont(font);
      categoryItem->setEditable(false);
      categoryItem->appendRow(item);
    }
  }

  std::sort(categoryItems.begin(), categoryItems.end(), compareQStandardItems);
  for (const auto& categoryItem : categoryItems)
  {
    rootItem->appendRow(categoryItem);
  }

  if (nullptr != miscellaneousItem && miscellaneousItem->hasChildren())
  {
    rootItem->appendRow(miscellaneousItem);
  }
}
