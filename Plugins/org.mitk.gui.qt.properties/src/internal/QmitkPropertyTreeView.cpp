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
#include "QmitkPropertiesPreferencePage.h"
#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include "QmitkPropertyItemSortFilterProxyModel.h"
#include "QmitkPropertyTreeView.h"
#include <berryIBerryPreferences.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <QPainter>

const std::string QmitkPropertyTreeView::VIEW_ID = "org.mitk.views.properties";

QmitkPropertyTreeView::QmitkPropertyTreeView()
  : m_PropertyNameChangedTag(0),
    m_PropertyAliases(NULL),
    m_PropertyDescriptions(NULL),
    m_ShowAliasesInDescription(true),
    m_ProxyModel(NULL),
    m_Model(NULL),
    m_Delegate(NULL)
{
}

QmitkPropertyTreeView::~QmitkPropertyTreeView()
{
}

void QmitkPropertyTreeView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.filter->SetDefaultText("Filter");

  m_Controls.description->hide();

  m_ProxyModel = new QmitkPropertyItemSortFilterProxyModel(m_Controls.treeView);
  m_Model = new QmitkPropertyItemModel(m_ProxyModel);

  m_ProxyModel->setSourceModel(m_Model);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setDynamicSortFilter(true);

  m_Delegate = new QmitkPropertyItemDelegate(m_Controls.treeView);

  m_Controls.treeView->setItemDelegateForColumn(1, m_Delegate);
  m_Controls.treeView->setModel(m_ProxyModel);
  m_Controls.treeView->setColumnWidth(0, 180);
  m_Controls.treeView->sortByColumn(0, Qt::AscendingOrder);
  m_Controls.treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.treeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

  connect(m_Controls.filter, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTextChanged(const QString&)));
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

  bool updateAliases = showAliases != (m_PropertyAliases != NULL);
  bool updateDescriptions = showDescriptions != (m_PropertyDescriptions != NULL);
  bool updateAliasesInDescription = showAliasesInDescription != m_ShowAliasesInDescription;

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
      std::ostringstream partName;
      partName << "Properties (" << nameProperty->GetValueAsString() << ')';
      this->SetPartName(partName.str());
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
    this->SetPartName("Properties");
    m_Model->SetPropertyList(NULL);
    m_Delegate->SetPropertyList(NULL);
  }
  else
  {
    QString selectionClassName = nodes.front()->GetData() != NULL
      ? nodes.front()->GetData()->GetNameOfClass()
      : "";

    m_SelectionClassName = selectionClassName.toStdString();
    m_Model->SetPropertyList(nodes.front()->GetPropertyList(), selectionClassName);
    m_Delegate->SetPropertyList(nodes.front()->GetPropertyList());
    OnPropertyNameChanged(itk::ModifiedEvent());

    mitk::BaseProperty* nameProperty = nodes.front()->GetProperty("name");

    if (nameProperty != NULL)
    {
      itk::ReceptorMemberCommand<QmitkPropertyTreeView>::Pointer command = itk::ReceptorMemberCommand<QmitkPropertyTreeView>::New();
      command->SetCallbackFunction(this, &QmitkPropertyTreeView::OnPropertyNameChanged);
      m_PropertyNameChangedTag = nameProperty->AddObserver(itk::ModifiedEvent(), command);
    }
  }

  if (!m_ProxyModel->filterRegExp().isEmpty())
    m_Controls.treeView->expandAll();
}

void QmitkPropertyTreeView::SetFocus()
{
  m_Controls.filter->setFocus();
}
