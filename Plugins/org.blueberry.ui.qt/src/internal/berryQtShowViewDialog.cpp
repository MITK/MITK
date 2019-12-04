/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtShowViewDialog.h"

#include <berryIViewDescriptor.h>
#include <berryIPreferences.h>

#include <berryViewTreeModel.h>

#include "berryWorkbenchPlugin.h"
#include "berryXMLMemento.h"

#include <QSortFilterProxyModel>
#include <QPushButton>

namespace berry {

static const QString TAG_SHOWVIEWDIALOG = "ShowViewDialog";
static const QString TAG_CATEGORY = "category";
static const QString TAG_SELECTION = "selection";
static const QString TAG_GEOMETRY = "geometry";

class ViewFilterProxyModel : public QSortFilterProxyModel
{
public:
  ViewFilterProxyModel(QObject* parent = nullptr)
    : QSortFilterProxyModel(parent)
    , m_FilterOnKeywords(true)
  {
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
  }

  bool filterOnKeywords() const
  {
    return m_FilterOnKeywords;
  }

  void setFilterOnKeywords(bool filterOnKeywords)
  {
    if (m_FilterOnKeywords != filterOnKeywords)
    {
      m_FilterOnKeywords = filterOnKeywords;
      this->filterChanged();
    }
  }

protected:

  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
  {
    QRegExp regExp = filterRegExp();
    if (!regExp.isValid() || regExp.isEmpty()) return true;

    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    QStringList keywords;

    if (m_FilterOnKeywords)
    {
      keywords = sourceModel()->data(sourceIndex, ViewTreeModel::Keywords).toStringList();
    }
    else
    {
      if (sourceModel()->hasChildren(sourceIndex))
      {
        // this is a category item
        int numChildren = sourceModel()->rowCount(sourceIndex);
        for (int i = 0; i < numChildren; ++i)
        {
          keywords.push_back(sourceModel()->data(sourceIndex.child(i, 0)).toString());
        }
      }
      else
      {
        // this is a view item
        keywords.push_back(sourceModel()->data(sourceIndex).toString());
      }
    }

    for(auto& keyword : keywords)
    {
      if (keyword.contains(regExp)) return true;
    }
    return false;
  }

private:

  bool m_FilterOnKeywords;
};

QtShowViewDialog::QtShowViewDialog(const IWorkbenchWindow* window, IViewRegistry* registry,
                                   QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , m_Window(window)
  , m_ViewReg(registry)
  , m_FilterModel(nullptr)
{
  m_UserInterface.setupUi(this);
  m_UserInterface.m_TreeView->header()->setVisible(false);
  m_UserInterface.m_TreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_FilterModel = new ViewFilterProxyModel(this);
  auto   sourceModel = new ViewTreeModel(window, m_FilterModel);
  m_FilterModel->setSourceModel(sourceModel);
  m_UserInterface.m_TreeView->setModel(m_FilterModel);

  connect(m_UserInterface.m_Filter, SIGNAL(textChanged(QString)), this, SLOT(setFilter(QString)));
  connect(m_UserInterface.m_TreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(setDescription(QModelIndex)));
  connect(m_UserInterface.m_TreeView, SIGNAL(collapsed(QModelIndex)), this, SLOT(categoryCollapsed(QModelIndex)));
  connect(m_UserInterface.m_TreeView, SIGNAL(expanded(QModelIndex)), this, SLOT(categoryExpanded(QModelIndex)));
  connect(m_UserInterface.m_TreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
  connect(m_UserInterface.m_TreeView, SIGNAL(activated(QModelIndex)), this, SLOT(accept()));
  connect(m_UserInterface.m_TreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
  connect(m_UserInterface.m_KeywordFilter, SIGNAL(clicked(bool)), this, SLOT(enableKeywordFilter(bool)));

  this->RestoreState();
  m_UserInterface.m_Filter->selectAll();
  this->UpdateButtons();
}

void QtShowViewDialog::setDescription(const QModelIndex& index)
{
  QString description = m_UserInterface.m_TreeView->model()->data(index, Qt::WhatsThisRole).toString();
  m_UserInterface.m_Description->setText(description);
}

void QtShowViewDialog::enableKeywordFilter(bool enable)
{
  m_FilterModel->setFilterOnKeywords(enable);
  this->RestoreExpandedState();
}

void QtShowViewDialog::setFilter(const QString& filter)
{
  m_FilterModel->setFilterWildcard(filter);
  this->RestoreExpandedState();
}

void QtShowViewDialog::categoryCollapsed(const QModelIndex& index)
{
  m_ExpandedCategories.removeAll(m_FilterModel->mapToSource(index));
}

void QtShowViewDialog::categoryExpanded(const QModelIndex& index)
{
  m_ExpandedCategories.push_back(m_FilterModel->mapToSource(index));
}

void QtShowViewDialog::selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
  UpdateButtons();
}

void QtShowViewDialog::RestoreExpandedState()
{
  int rowCount = m_FilterModel->rowCount();
  for (int i = 0; i < rowCount; ++i)
  {
    QModelIndex index = m_FilterModel->index(i, 0);
    if (m_ExpandedCategories.contains(m_FilterModel->mapToSource(index)))
    {
      m_UserInterface.m_TreeView->expand(index);
    }
  }
}

void QtShowViewDialog::UpdateButtons()
{
  QPushButton* okBtn = m_UserInterface.m_ButtonBox->button(QDialogButtonBox::Ok);
  if (okBtn)
  {
    okBtn->setEnabled(!m_UserInterface.m_TreeView->selectionModel()->selection().isEmpty());
  }
}

void QtShowViewDialog::RestoreState()
{
  IPreferences::Pointer prefs = WorkbenchPlugin::GetDefault()->GetPreferences();
  QString str = prefs->Get(TAG_SHOWVIEWDIALOG, QString::null);
  if (str.isEmpty()) return;

  std::stringstream ss(str.toStdString());

  XMLMemento::Pointer memento = XMLMemento::CreateReadRoot(ss);

  bool keywordFilter = false;
  if (memento->GetBoolean("keywordFilter", keywordFilter))
  {
    m_UserInterface.m_KeywordFilter->setChecked(keywordFilter);
    m_FilterModel->setFilterOnKeywords(keywordFilter);
  }

  QString filter;
  if (memento->GetString("filter", filter))
  {
    m_UserInterface.m_Filter->setText(filter);
  }

  IMemento::Pointer geomChild = memento->GetChild(TAG_GEOMETRY);
  if (geomChild.IsNotNull())
  {
    QString geom = geomChild->GetTextData();
    if (!geom.isEmpty())
    {
      QByteArray ba = QByteArray::fromBase64(geom.toLatin1());
      this->restoreGeometry(ba);
    }
  }

  QHash<QString, QModelIndex> rootIndices;
  int rowCount = m_FilterModel->sourceModel()->rowCount();
  for (int i = 0; i < rowCount; ++i)
  {
    QModelIndex sourceIndex = m_FilterModel->sourceModel()->index(i, 0);
    QString id = sourceIndex.data(ViewTreeModel::Id).toString();
    if (!id.isEmpty())
    {
      rootIndices[id] = sourceIndex;
    }
  }
  for (IMemento::Pointer categoryChild : memento->GetChildren(TAG_CATEGORY))
  {
    QString id = categoryChild->GetID();
    if (!id.isEmpty())
    {
      if (rootIndices.contains(id))
      {
        m_ExpandedCategories.push_back(rootIndices[id]);
      }
    }
  }
  this->RestoreExpandedState();

  QItemSelection itemSelection;
  for (IMemento::Pointer selectionChild : memento->GetChildren(TAG_SELECTION))
  {
    QString id = selectionChild->GetID();
    if (!id.isEmpty())
    {
      QModelIndexList indexList = m_FilterModel->match(m_FilterModel->index(0, 0), ViewTreeModel::Id, QVariant::fromValue(id),
                                                       1, Qt::MatchExactly | Qt::MatchRecursive);
      if (!indexList.isEmpty())
      {
        QItemSelection subSelection(indexList.front(), indexList.front());
        itemSelection.merge(subSelection, QItemSelectionModel::SelectCurrent);
      }
    }
  }
  m_UserInterface.m_TreeView->selectionModel()->select(itemSelection, QItemSelectionModel::ClearAndSelect);
}

void QtShowViewDialog::SaveState()
{
  XMLMemento::Pointer memento = XMLMemento::CreateWriteRoot(TAG_SHOWVIEWDIALOG);
  memento->PutString("filter", m_UserInterface.m_Filter->text());
  memento->PutBoolean("keywordFilter", m_UserInterface.m_KeywordFilter->isChecked());

  // dialog geometry
  QByteArray geom = this->saveGeometry();
  IMemento::Pointer geomChild = memento->CreateChild(TAG_GEOMETRY);
  geomChild->PutTextData(geom.toBase64().constData());

  // expanded categories
  for (QPersistentModelIndex index : m_ExpandedCategories)
  {
    if (index.isValid())
    {
      QString id = index.data(ViewTreeModel::Id).toString();
      if (!id.isEmpty())
      {
        memento->CreateChild(TAG_CATEGORY, id);
      }
    }
  }

  // we only record a single selected item. restoring a multi-selection might be
  // confusing for the user
  QModelIndexList selectedIndices = m_UserInterface.m_TreeView->selectionModel()->selectedIndexes();
  if (!selectedIndices.isEmpty())
  {
    QString id = selectedIndices.back().data(ViewTreeModel::Id).toString();
    if (!id.isEmpty())
    {
      memento->CreateChild(TAG_SELECTION, id);
    }
  }

  std::stringstream ss;
  memento->Save(ss);

  IPreferences::Pointer prefs = WorkbenchPlugin::GetDefault()->GetPreferences();
  prefs->Put(TAG_SHOWVIEWDIALOG, QString::fromStdString(ss.str()));
  prefs->Sync();
}

void QtShowViewDialog::done(int r)
{
  this->SaveState();
  QDialog::done(r);
}

QList<QString>
QtShowViewDialog::GetSelection() const
{
  QList<QString> selected;

  QModelIndexList indices = m_UserInterface.m_TreeView->selectionModel()->selectedIndexes();
  for(QModelIndex index : indices)
  {
    QString id = m_FilterModel->data(index, ViewTreeModel::Id).toString();
    selected.push_back(id);
  }

  return selected;
}

}
