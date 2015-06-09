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

#include "mitkGetPropertyService.h"
#include "QmitkAddNewPropertyDialog.h"
#include "QmitkPropertiesPreferencePage.h"
#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include "QmitkPropertyItemSortFilterProxyModel.h"
#include "QmitkPropertyTreeView.h"
#include <berryIBerryPreferences.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <QmitkRenderWindow.h>
#include <QPainter>
#include <memory>

const std::string QmitkPropertyTreeView::VIEW_ID = "org.mitk.views.properties";

QmitkPropertyTreeView::QmitkPropertyTreeView()
  : m_Parent(NULL),
    m_PropertyNameChangedTag(0),
    m_PropertyAliases(NULL),
    m_PropertyDescriptions(NULL),
    m_ShowAliasesInDescription(true),
    m_DeveloperMode(false),
    m_ProxyModel(NULL),
    m_Model(NULL),
    m_Delegate(NULL),
    m_Renderer(NULL)
{
}

QmitkPropertyTreeView::~QmitkPropertyTreeView()
{
}

void QmitkPropertyTreeView::CreateQtPartControl(QWidget* parent)
{
  m_Parent = parent;

  m_Controls.setupUi(parent);

  m_Controls.propertyListComboBox->addItem("Data node: common");

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

  if (renderWindowPart != NULL)
  {
    QHash<QString, QmitkRenderWindow*> renderWindows = renderWindowPart->GetQmitkRenderWindows();

    Q_FOREACH(QString renderWindow, renderWindows.keys())
    {
      m_Controls.propertyListComboBox->addItem(QString("Data node: ") + renderWindow);
    }
  }

  m_Controls.propertyListComboBox->addItem("Base data");

  m_Controls.newButton->setEnabled(false);

  m_Controls.description->hide();
  m_Controls.propertyListLabel->hide();
  m_Controls.propertyListComboBox->hide();
  m_Controls.newButton->hide();

  m_ProxyModel = new QmitkPropertyItemSortFilterProxyModel(m_Controls.treeView);
  m_Model = new QmitkPropertyItemModel(m_ProxyModel);

  m_ProxyModel->setSourceModel(m_Model);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setDynamicSortFilter(true);

  m_Delegate = new QmitkPropertyItemDelegate(m_Controls.treeView);

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  m_Controls.filterLineEdit->setClearButtonEnabled(true);
#endif

  m_Controls.treeView->setItemDelegateForColumn(1, m_Delegate);
  m_Controls.treeView->setModel(m_ProxyModel);
  m_Controls.treeView->setColumnWidth(0, 160);
  m_Controls.treeView->sortByColumn(0, Qt::AscendingOrder);
  m_Controls.treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.treeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

  connect(m_Controls.filterLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTextChanged(const QString&)));
  connect(m_Controls.propertyListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPropertyListChanged(int)));
  connect(m_Controls.newButton, SIGNAL(clicked()), this, SLOT(OnAddNewProperty()));
  connect(m_Controls.treeView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnCurrentRowChanged(const QModelIndex&, const QModelIndex&)));
  connect(m_Model, SIGNAL(modelReset()), this, SLOT(OnModelReset()));
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

void QmitkPropertyTreeView::OnCurrentRowChanged(const QModelIndex& current, const QModelIndex&)
{
  if (m_PropertyDescriptions != NULL && current.isValid())
  {
    QString name = this->GetPropertyNameOrAlias(current);

    if (!name.isEmpty())
    {
      QString alias;
      bool isTrueName = true;

      if (m_PropertyAliases != NULL)
      {
        std::string trueName = m_PropertyAliases->GetPropertyName(name.toStdString());

        if (trueName.empty() && !m_SelectionClassName.empty())
          trueName = m_PropertyAliases->GetPropertyName(name.toStdString(), m_SelectionClassName);

        if (!trueName.empty())
        {
          alias = name;
          name = QString::fromStdString(trueName);
          isTrueName = false;
        }
      }

      QString description = QString::fromStdString(m_PropertyDescriptions->GetDescription(name.toStdString()));
      std::vector<std::string> aliases;

      if (!isTrueName && m_PropertyAliases != NULL)
      {
        aliases = m_PropertyAliases->GetAliases(name.toStdString(), m_SelectionClassName);

        if (aliases.empty() && !m_SelectionClassName.empty())
          aliases = m_PropertyAliases->GetAliases(name.toStdString());
      }

      if (!description.isEmpty() || !aliases.empty())
      {
        QString customizedDescription;

        if (m_ShowAliasesInDescription && !aliases.empty())
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
          customizedDescription += description;

        m_Controls.description->setText(customizedDescription);
        m_Controls.description->show();

        return;
      }
    }
  }

  m_Controls.description->hide();
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
  m_Controls.description->hide();
}

void QmitkPropertyTreeView::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
  bool showAliases = preferences->GetBool(QmitkPropertiesPreferencePage::SHOW_ALIASES, true);
  bool showDescriptions = preferences->GetBool(QmitkPropertiesPreferencePage::SHOW_DESCRIPTIONS, true);
  bool showAliasesInDescription = preferences->GetBool(QmitkPropertiesPreferencePage::SHOW_ALIASES_IN_DESCRIPTION, true);
  bool developerMode = preferences->GetBool(QmitkPropertiesPreferencePage::DEVELOPER_MODE, false);

  bool updateAliases = showAliases != (m_PropertyAliases != NULL);
  bool updateDescriptions = showDescriptions != (m_PropertyDescriptions != NULL);
  bool updateAliasesInDescription = showAliasesInDescription != m_ShowAliasesInDescription;
  bool updateDeveloperMode = developerMode != m_DeveloperMode;

  if (updateAliases)
    m_PropertyAliases = showAliases
      ? mitk::GetPropertyService<mitk::IPropertyAliases>()
      : NULL;

  if (updateDescriptions)
    m_PropertyDescriptions = showDescriptions
      ? mitk::GetPropertyService<mitk::IPropertyDescriptions>()
      : NULL;

  if (updateAliasesInDescription)
    m_ShowAliasesInDescription = showAliasesInDescription;

  if (updateDescriptions || updateAliasesInDescription)
  {
    QModelIndexList selection = m_Controls.treeView->selectionModel()->selectedRows();

    if (!selection.isEmpty())
      this->OnCurrentRowChanged(selection[0], selection[0]);
  }

  if (updateDeveloperMode)
  {
    m_DeveloperMode = developerMode;

    if (!developerMode)
      m_Controls.propertyListComboBox->setCurrentIndex(0);

    m_Controls.propertyListLabel->setVisible(developerMode);
    m_Controls.propertyListComboBox->setVisible(developerMode);
    m_Controls.newButton->setVisible(developerMode);
  }

  m_Model->OnPreferencesChanged(preferences);
}

void QmitkPropertyTreeView::OnPropertyNameChanged(const itk::EventObject&)
{
  mitk::PropertyList* propertyList = m_Model->GetPropertyList();

  if (propertyList != NULL)
  {
    mitk::BaseProperty* nameProperty = propertyList->GetProperty("name");

    if (nameProperty != NULL)
    {
      QString partName = "Properties (";
      partName.append(QString::fromStdString(nameProperty->GetValueAsString())).append(')');
      this->SetPartName(partName);
    }
  }
}

void QmitkPropertyTreeView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  mitk::PropertyList* propertyList = m_Model->GetPropertyList();

  if (propertyList != NULL)
  {
    mitk::BaseProperty* nameProperty = propertyList->GetProperty("name");

    if (nameProperty != NULL)
      nameProperty->RemoveObserver(m_PropertyNameChangedTag);

    m_PropertyNameChangedTag = 0;
  }

  if (nodes.empty() || nodes.front().IsNull())
  {
    m_SelectedNode = NULL;

    this->SetPartName("Properties");
    m_Model->SetPropertyList(NULL);
    m_Delegate->SetPropertyList(NULL);

    m_Controls.newButton->setEnabled(false);
  }
  else
  {
    m_SelectedNode = nodes.front();

    QString selectionClassName = m_SelectedNode->GetData() != NULL
      ? m_SelectedNode->GetData()->GetNameOfClass()
      : "";

    m_SelectionClassName = selectionClassName.toStdString();

    mitk::PropertyList::Pointer propertyList;

    if (m_Renderer == NULL && m_Controls.propertyListComboBox->currentText() == "Base data")
    {
      propertyList = m_SelectedNode->GetData() != NULL
        ? m_SelectedNode->GetData()->GetPropertyList()
        : NULL;
    }
    else
    {
      propertyList = m_SelectedNode->GetPropertyList(m_Renderer);
    }

    m_Model->SetPropertyList(propertyList, selectionClassName);
    m_Delegate->SetPropertyList(propertyList);

    OnPropertyNameChanged(itk::ModifiedEvent());

    mitk::BaseProperty* nameProperty = m_SelectedNode->GetProperty("name");

    if (nameProperty != NULL)
    {
      itk::ReceptorMemberCommand<QmitkPropertyTreeView>::Pointer command = itk::ReceptorMemberCommand<QmitkPropertyTreeView>::New();
      command->SetCallbackFunction(this, &QmitkPropertyTreeView::OnPropertyNameChanged);
      m_PropertyNameChangedTag = nameProperty->AddObserver(itk::ModifiedEvent(), command);
    }

    m_Controls.newButton->setEnabled(true);
  }

  if (!m_ProxyModel->filterRegExp().isEmpty())
    m_Controls.treeView->expandAll();
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

void QmitkPropertyTreeView::OnPropertyListChanged(int index)
{
  if (index == -1)
    return;

  QString renderer = m_Controls.propertyListComboBox->itemText(index);

  if (renderer.startsWith("Data node: "))
    renderer = QString::fromStdString(renderer.toStdString().substr(11));

  m_Renderer = renderer != "common" && renderer != "Base data"
    ? this->GetRenderWindowPart()->GetQmitkRenderWindow(renderer)->GetRenderer()
    : NULL;

  QList<mitk::DataNode::Pointer> nodes;

  if (m_SelectedNode.IsNotNull())
    nodes << m_SelectedNode;

  berry::IWorkbenchPart::Pointer workbenchPart;

  this->OnSelectionChanged(workbenchPart, nodes);
}

void QmitkPropertyTreeView::OnAddNewProperty()
{
  std::unique_ptr<QmitkAddNewPropertyDialog> dialog(m_Controls.propertyListComboBox->currentText() != "Base data"
      ? new QmitkAddNewPropertyDialog(m_SelectedNode, m_Renderer, m_Parent)
      : new QmitkAddNewPropertyDialog(m_SelectedNode->GetData()));

  if (dialog->exec() == QDialog::Accepted)
    this->m_Model->Update();
}
