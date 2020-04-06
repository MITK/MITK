/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAddNewPropertyDialog.h"
#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include "QmitkPropertyItemSortFilterProxyModel.h"
#include "QmitkPropertyTreeView.h"
#include <berryIBerryPreferences.h>
#include <berryQtStyleManager.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>
#include <QmitkRenderWindow.h>
#include <QPainter>
#include <memory>

const std::string QmitkPropertyTreeView::VIEW_ID = "org.mitk.views.properties";

QmitkPropertyTreeView::QmitkPropertyTreeView()
  : m_PropertyAliases(mitk::CoreServices::GetPropertyAliases(nullptr), nullptr),
    m_PropertyDescriptions(mitk::CoreServices::GetPropertyDescriptions(nullptr), nullptr),
    m_PropertyPersistence(mitk::CoreServices::GetPropertyPersistence(nullptr), nullptr),
    m_ProxyModel(nullptr),
    m_Model(nullptr),
    m_Delegate(nullptr),
    m_Renderer(nullptr)
{
}

QmitkPropertyTreeView::~QmitkPropertyTreeView()
{
}

void QmitkPropertyTreeView::SetFocus()
{
  m_Controls.filterLineEdit->setFocus();
}

void QmitkPropertyTreeView::RenderWindowPartActivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  if (m_Controls.propertyListComboBox->count() == 2)
  {
    QHash<QString, QmitkRenderWindow*> renderWindows = this->GetRenderWindowPart()->GetQmitkRenderWindows();

    Q_FOREACH(QString renderWindow, renderWindows.keys())
    {
      m_Controls.propertyListComboBox->insertItem(m_Controls.propertyListComboBox->count() - 1, QString("Data node: ") + renderWindow);
    }
  }
}

void QmitkPropertyTreeView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  if (m_Controls.propertyListComboBox->count() > 2)
  {
    m_Controls.propertyListComboBox->clear();
    m_Controls.propertyListComboBox->addItem("Data node: common");
    m_Controls.propertyListComboBox->addItem("Base data");
  }
}

void QmitkPropertyTreeView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.propertyListComboBox->addItem("Data node: common");

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  if (renderWindowPart != nullptr)
  {
    QHash<QString, QmitkRenderWindow*> renderWindows = renderWindowPart->GetQmitkRenderWindows();

    for(const auto& renderWindow : renderWindows.keys())
    {
      m_Controls.propertyListComboBox->addItem(QString("Data node: ") + renderWindow);
    }
  }

  m_Controls.propertyListComboBox->addItem("Base data");

  m_Controls.newButton->setEnabled(false);

  this->HideAllIcons();

  m_ProxyModel = new QmitkPropertyItemSortFilterProxyModel(m_Controls.treeView);
  m_Model = new QmitkPropertyItemModel(m_ProxyModel);

  m_ProxyModel->setSourceModel(m_Model);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setDynamicSortFilter(true);

  m_Delegate = new QmitkPropertyItemDelegate(m_Controls.treeView);

  m_Controls.singleSlot->SetDataStorage(GetDataStorage());
  m_Controls.singleSlot->SetSelectionIsOptional(true);
  m_Controls.singleSlot->SetEmptyInfo(QString("Please select a data node"));
  m_Controls.singleSlot->SetPopUpTitel(QString("Select data node"));

  m_SelectionServiceConnector = std::make_unique<QmitkSelectionServiceConnector>();
  SetAsSelectionListener(true);

  m_Controls.filterLineEdit->setClearButtonEnabled(true);

  m_Controls.treeView->setItemDelegateForColumn(1, m_Delegate);
  m_Controls.treeView->setModel(m_ProxyModel);
  m_Controls.treeView->setColumnWidth(0, 160);
  m_Controls.treeView->sortByColumn(0, Qt::AscendingOrder);
  m_Controls.treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.treeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

  const int ICON_SIZE = 32;

  auto icon = berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/tags.svg"));
  m_Controls.tagsLabel->setPixmap(icon.pixmap(ICON_SIZE));

  icon = berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/tag.svg"));
  m_Controls.tagLabel->setPixmap(icon.pixmap(ICON_SIZE));

  icon = berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg"));
  m_Controls.saveLabel->setPixmap(icon.pixmap(ICON_SIZE));

  connect(m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkPropertyTreeView::OnCurrentSelectionChanged);
  connect(m_Controls.filterLineEdit, &QLineEdit::textChanged,
    this, &QmitkPropertyTreeView::OnFilterTextChanged);
  connect(m_Controls.propertyListComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &QmitkPropertyTreeView::OnPropertyListChanged);
  connect(m_Controls.newButton, &QPushButton::clicked,
    this, &QmitkPropertyTreeView::OnAddNewProperty);
  connect(m_Controls.treeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
    this, &QmitkPropertyTreeView::OnCurrentRowChanged);
  connect(m_Model, &QmitkPropertyItemModel::modelReset,
    this, &QmitkPropertyTreeView::OnModelReset);
}

void QmitkPropertyTreeView::SetAsSelectionListener(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
}

QString QmitkPropertyTreeView::GetPropertyNameOrAlias(const QModelIndex& index)
{
  QString propertyName;

  if (index.isValid())
  {
    QModelIndex current = index;

    while (current.isValid())
    {
      QString name = m_ProxyModel->data(m_ProxyModel->index(current.row(), 0, current.parent())).toString();

      propertyName.prepend(propertyName.isEmpty()
        ? name
        : name.append('.'));

      current = current.parent();
    }
  }

  return propertyName;
}

void QmitkPropertyTreeView::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  if (nodes.empty() || nodes.front().IsNull())
  {
    m_SelectedNode = nullptr;

    this->SetPartName("Properties");
    m_Model->SetPropertyList(nullptr);
    m_Delegate->SetPropertyList(nullptr);

    m_Controls.newButton->setEnabled(false);

    return;
  }

  // node is selected, create tree with node properties
  m_SelectedNode = nodes.front();
  mitk::PropertyList* propertyList = m_Model->GetPropertyList();
  if (m_Renderer == nullptr && m_Controls.propertyListComboBox->currentText() == "Base data")
  {
    propertyList = m_SelectedNode->GetData() != nullptr
      ? m_SelectedNode->GetData()->GetPropertyList()
      : nullptr;
  }
  else
  {
    propertyList = m_SelectedNode->GetPropertyList(m_Renderer);
  }

  QString selectionClassName = m_SelectedNode->GetData() != nullptr
    ? m_SelectedNode->GetData()->GetNameOfClass()
    : "";

  m_SelectionClassName = selectionClassName.toStdString();

  m_Model->SetPropertyList(propertyList, selectionClassName);
  m_Delegate->SetPropertyList(propertyList);

  m_Controls.newButton->setEnabled(true);

  if (!m_ProxyModel->filterRegExp().isEmpty())
  {
    m_Controls.treeView->expandAll();
  }
}

void QmitkPropertyTreeView::HideAllIcons()
{
  m_Controls.tagLabel->hide();
  m_Controls.tagsLabel->hide();
  m_Controls.saveLabel->hide();
}

void QmitkPropertyTreeView::OnCurrentRowChanged(const QModelIndex& current, const QModelIndex&)
{
  if (current.isValid())
  {
    QString name = this->GetPropertyNameOrAlias(current);

    if (!name.isEmpty())
    {
      QString alias;
      bool isTrueName = true;

      std::string trueName = m_PropertyAliases->GetPropertyName(name.toStdString());

      if (trueName.empty() && !m_SelectionClassName.empty())
        trueName = m_PropertyAliases->GetPropertyName(name.toStdString(), m_SelectionClassName);

      if (!trueName.empty())
      {
        alias = name;
        name = QString::fromStdString(trueName);
        isTrueName = false;
      }

      QString description = QString::fromStdString(m_PropertyDescriptions->GetDescription(name.toStdString()));
      std::vector<std::string> aliases;

      if (!isTrueName)
      {
        aliases = m_PropertyAliases->GetAliases(name.toStdString(), m_SelectionClassName);

        if (aliases.empty() && !m_SelectionClassName.empty())
          aliases = m_PropertyAliases->GetAliases(name.toStdString());
      }

      bool isPersistent = m_PropertyPersistence->HasInfo(name.toStdString());

      if (!description.isEmpty() || !aliases.empty() || isPersistent)
      {
        QString customizedDescription;

        if (!aliases.empty())
        {
          customizedDescription = "<h3 style=\"margin-bottom:0\">" + name + "</h3>";

          std::size_t numAliases = aliases.size();
          std::size_t lastAlias = numAliases - 1;

          for (std::size_t i = 0; i < numAliases; ++i)
          {
            customizedDescription += i != lastAlias
              ? "<h5 style=\"margin-top:0;margin-bottom:0\">"
              : "<h5 style=\"margin-top:0;margin-bottom:10px\">";

            customizedDescription += QString::fromStdString(aliases[i]) + "</h5>";
          }
        }
        else
        {
          customizedDescription = "<h3 style=\"margin-bottom:10px\">" + name + "</h3>";
        }

        if (!description.isEmpty())
          customizedDescription += "<p>" + description + "</p>";

        m_Controls.tagsLabel->setVisible(!aliases.empty() && aliases.size() > 1);
        m_Controls.tagLabel->setVisible(!aliases.empty() && aliases.size() == 1);
        m_Controls.saveLabel->setVisible(isPersistent);

        m_Controls.descriptionLabel->setText(customizedDescription);
        m_Controls.descriptionLabel->show();

        return;
      }
    }
  }

  m_Controls.descriptionLabel->hide();
  this->HideAllIcons();
}

void QmitkPropertyTreeView::OnPropertyListChanged(int index)
{
  if (index == -1)
    return;

  QString renderer = m_Controls.propertyListComboBox->itemText(index);

  if (renderer.startsWith("Data node: "))
    renderer = QString::fromStdString(renderer.toStdString().substr(11));

  m_Renderer = renderer != "common" && renderer != "Base data"
    ? this->GetRenderWindowPart()->GetQmitkRenderWindow(renderer)->GetRenderer()
    : nullptr;

  QList<mitk::DataNode::Pointer> nodes;

  if (m_SelectedNode.IsNotNull())
    nodes << m_SelectedNode;

  this->OnCurrentSelectionChanged(nodes);
}

void QmitkPropertyTreeView::OnAddNewProperty()
{
  std::unique_ptr<QmitkAddNewPropertyDialog> dialog(m_Controls.propertyListComboBox->currentText() != "Base data"
    ? new QmitkAddNewPropertyDialog(m_SelectedNode, m_Renderer)
    : new QmitkAddNewPropertyDialog(m_SelectedNode->GetData()));

  if (dialog->exec() == QDialog::Accepted)
    this->m_Model->Update();
}

void QmitkPropertyTreeView::OnFilterTextChanged(const QString& filter)
{
  m_ProxyModel->setFilterWildcard(filter);

  if (filter.isEmpty())
    m_Controls.treeView->collapseAll();
  else
    m_Controls.treeView->expandAll();
}

void QmitkPropertyTreeView::OnModelReset()
{
  m_Controls.descriptionLabel->hide();
  this->HideAllIcons();
}
