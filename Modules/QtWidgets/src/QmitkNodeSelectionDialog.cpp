/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeSelectionDialog.h"

#include <QmitkStyleManager.h>

#include <mitkDataStorageInspectorGenerator.h>
#include <QmitkNodeSelectionPreferenceHelper.h>
#include <QmitkDataStorageSelectionHistoryInspector.h>
#include <QmitkDataStorageFavoriteNodesInspector.h>

#include <string>
#include <map>

QmitkNodeSelectionDialog::QmitkNodeSelectionDialog(QWidget* parent, QString title, QString hint)
  : QDialog(parent)
  , m_NodePredicate(nullptr)
  , m_SelectOnlyVisibleNodes(false)
  , m_SelectedNodes(NodeList())
  , m_SelectionMode(QAbstractItemView::SingleSelection)
{
  m_Controls.setupUi(this);

  m_CheckFunction = [](const NodeList &) { return ""; };

  auto providers = mitk::DataStorageInspectorGenerator::GetProviders();
  auto visibleProviders = mitk::GetVisibleDataStorageInspectors();
  auto preferredID = mitk::GetPreferredDataStorageInspector();

  if (visibleProviders.empty())
  {
    MITK_DEBUG << "No presets for visible node selection inspectors available. Use fallback (show all available inspectors)";
    unsigned int order = 0;
    for (const auto &proIter : providers)
    {
      visibleProviders.insert(std::make_pair(order, proIter.first));
      ++order;
    }
  }

  int preferredIndex = 0;
  bool preferredFound = false;
  for (const auto &proIter : visibleProviders)
  {
    auto finding = providers.find(proIter.second);
    if (finding != providers.end())
    {
      if (finding->second->GetInspectorID() != QmitkDataStorageFavoriteNodesInspector::INSPECTOR_ID() && finding->second->GetInspectorID() != QmitkDataStorageSelectionHistoryInspector::INSPECTOR_ID())
      {
        auto provider = finding->second;
        this->AddPanel(provider, preferredID, preferredFound, preferredIndex);
      }
    }
    else
    {
      MITK_DEBUG << "No provider registered for inspector that is defined as visible in the preferences. Illegal inspector ID: " << proIter.second;
    }
  }

  if (mitk::GetShowFavoritesInspector())
  {
    auto favoritesPorvider = mitk::DataStorageInspectorGenerator::GetProvider(QmitkDataStorageFavoriteNodesInspector::INSPECTOR_ID());
    if (favoritesPorvider != nullptr)
    {
      this->AddPanel(favoritesPorvider, preferredID, preferredFound, preferredIndex);
    }
  }

  if (mitk::GetShowHistoryInspector())
  {
    auto historyPorvider = mitk::DataStorageInspectorGenerator::GetProvider(QmitkDataStorageSelectionHistoryInspector::INSPECTOR_ID());
    if (historyPorvider != nullptr)
    {
      this->AddPanel(historyPorvider, preferredID, preferredFound, preferredIndex);
    }
  }

  m_Controls.tabWidget->setCurrentIndex(preferredIndex);
  this->setWindowTitle(title);
  this->setToolTip(hint);

  m_Controls.hint->setText(hint);
  m_Controls.hint->setVisible(!hint.isEmpty());
  if(hint.isEmpty())
  {
    m_Controls.layoutHint->setContentsMargins(0, 0, 0, 0);
  }
  else
  {
    m_Controls.layoutHint->setContentsMargins(6, 6, 6, 6);
  }

  this->SetErrorText("");

  m_Controls.btnAddToFav->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/favorite_add.svg")));

  connect(m_Controls.btnAddToFav, &QPushButton::clicked, this, &QmitkNodeSelectionDialog::OnFavoriteNodesButtonClicked);
  connect(m_Controls.buttonBox, &QDialogButtonBox::accepted, this, &QmitkNodeSelectionDialog::OnOK);
  connect(m_Controls.buttonBox, &QDialogButtonBox::rejected, this, &QmitkNodeSelectionDialog::OnCancel);
}

void QmitkNodeSelectionDialog::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    m_DataStorage = dataStorage;
    auto lockedDataStorage = m_DataStorage.Lock();

    if (lockedDataStorage.IsNotNull())
    {
      for (auto panel : m_Panels)
      {
        panel->SetDataStorage(lockedDataStorage);
      }
    }
  }
}

void QmitkNodeSelectionDialog::SetNodePredicate(const mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    for (auto panel : m_Panels)
    {
      panel->SetNodePredicate(m_NodePredicate);
    }
  }
}

const mitk::NodePredicateBase* QmitkNodeSelectionDialog::GetNodePredicate() const
{
  return m_NodePredicate;
}

QmitkNodeSelectionDialog::NodeList QmitkNodeSelectionDialog::GetSelectedNodes() const
{
  return m_SelectedNodes;
}

void QmitkNodeSelectionDialog::SetSelectionCheckFunction(const SelectionCheckFunctionType &checkFunction)
{
  m_CheckFunction = checkFunction;
  auto checkResponse = m_CheckFunction(m_SelectedNodes);

  SetErrorText(checkResponse);

  m_Controls.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checkResponse.empty());
}

void QmitkNodeSelectionDialog::SetErrorText(const std::string& checkResponse)
{
  m_Controls.error->setText(QString::fromStdString(checkResponse));
  m_Controls.error->setVisible(!checkResponse.empty());
  if (checkResponse.empty())
  {
    m_Controls.layoutError->setContentsMargins(0, 0, 0, 0);
  }
  else
  {
    m_Controls.layoutError->setContentsMargins(6, 6, 6, 6);
  }
}

bool QmitkNodeSelectionDialog::GetSelectOnlyVisibleNodes() const
{
  return m_SelectOnlyVisibleNodes;
}

void QmitkNodeSelectionDialog::SetSelectionMode(SelectionMode mode)
{
  m_SelectionMode = mode;
  for (auto panel : m_Panels)
  {
    panel->SetSelectionMode(mode);
  }
}

QmitkNodeSelectionDialog::SelectionMode QmitkNodeSelectionDialog::GetSelectionMode() const
{
  return m_SelectionMode;
}

void QmitkNodeSelectionDialog::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  if (m_SelectOnlyVisibleNodes != selectOnlyVisibleNodes)
  {
    m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;

    for (auto panel : m_Panels)
    {
      panel->SetSelectOnlyVisibleNodes(m_SelectOnlyVisibleNodes);
    }
  }
}

void QmitkNodeSelectionDialog::SetCurrentSelection(NodeList selectedNodes)
{
  m_SelectedNodes = selectedNodes;
  auto checkResponse = m_CheckFunction(m_SelectedNodes);

  SetErrorText(checkResponse);

  m_Controls.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checkResponse.empty());

  for (auto panel : m_Panels)
  {
    panel->SetCurrentSelection(selectedNodes);
  }
}

void QmitkNodeSelectionDialog::OnSelectionChanged(NodeList selectedNodes)
{
  SetCurrentSelection(selectedNodes);
  emit CurrentSelectionChanged(selectedNodes);
}

void QmitkNodeSelectionDialog::OnFavoriteNodesButtonClicked()
{
  for (auto node : qAsConst(m_SelectedNodes))
  {
    node->SetBoolProperty("org.mitk.selection.favorite", true);
  }
}

void QmitkNodeSelectionDialog::OnOK()
{
  for (const auto &node : qAsConst(m_SelectedNodes))
  {
    QmitkDataStorageSelectionHistoryInspector::AddNodeToHistory(node);
  }

  this->accept();
}

void QmitkNodeSelectionDialog::OnCancel()
{
  this->reject();
}

void QmitkNodeSelectionDialog::AddPanel(const mitk::IDataStorageInspectorProvider * provider, const mitk::IDataStorageInspectorProvider::InspectorIDType& preferredID, bool &preferredFound, int &preferredIndex)
{
  auto inspector = provider->CreateInspector();
  QString name = QString::fromStdString(provider->GetInspectorDisplayName());
  QString desc = QString::fromStdString(provider->GetInspectorDescription());

  inspector->setParent(this);
  inspector->SetSelectionMode(m_SelectionMode);

  auto tabPanel = new QWidget();
  tabPanel->setObjectName(QString("tab_") + name);
  tabPanel->setToolTip(desc);

  auto verticalLayout = new QVBoxLayout(tabPanel);
  verticalLayout->setSpacing(0);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  verticalLayout->addWidget(inspector);

  auto panelPos = m_Controls.tabWidget->insertTab(m_Controls.tabWidget->count(), tabPanel, name);

  auto icon = provider->GetInspectorIcon();
  if (!icon.isNull())
  {
    m_Controls.tabWidget->setTabIcon(panelPos, icon);
  }

  m_Panels.push_back(inspector);
  connect(inspector, &QmitkAbstractDataStorageInspector::CurrentSelectionChanged, this, &QmitkNodeSelectionDialog::OnSelectionChanged);
  connect(inspector->GetView(), &QAbstractItemView::doubleClicked, this, &QmitkNodeSelectionDialog::OnDoubleClicked);

  preferredFound = preferredFound || provider->GetInspectorID() == preferredID;
  if (!preferredFound)
  {
    ++preferredIndex;
  }
}

void QmitkNodeSelectionDialog::OnDoubleClicked(const QModelIndex& /*index*/)
{
  const auto isOK = m_Controls.buttonBox->button(QDialogButtonBox::Ok)->isEnabled();
  if (!m_SelectedNodes.empty() && isOK)
  {
    this->OnOK();
  }
}
