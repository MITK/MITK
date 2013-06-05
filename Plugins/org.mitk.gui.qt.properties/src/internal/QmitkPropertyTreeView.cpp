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
#include <mitkPropertyAliases.h>
#include <mitkPropertyDescriptions.h>
#include <QPainter>

const std::string QmitkPropertyTreeView::VIEW_ID = "org.mitk.views.properties";

QmitkPropertyTreeView::QmitkPropertyTreeView()
  : m_PropertyNameChangedTag(0),
    m_PropertyAliases(NULL),
    m_PropertyDescriptions(NULL),
    m_ShowGenuineNames(false),
    m_ProxyModel(NULL),
    m_Model(NULL)
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

  m_Controls.treeView->setItemDelegateForColumn(1, new QmitkPropertyItemDelegate(m_Controls.treeView));
  m_Controls.treeView->setModel(m_ProxyModel);
  m_Controls.treeView->setColumnWidth(0, 150);
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

        if (!trueName.empty())
        {
          alias = name;
          name = QString::fromStdString(trueName);
          isTrueName = false;
        }
      }

      QString description = QString::fromStdString(m_PropertyDescriptions->GetDescription(name.toStdString()));

      if (!description.isEmpty())
      {
        if (!isTrueName && alias.isEmpty() && m_PropertyAliases != NULL)
          alias = QString::fromStdString(m_PropertyAliases->GetAlias(name.toStdString()));

        QString customizedDescription;

        if (!alias.isEmpty())
        {
          if (m_ShowGenuineNames)
          {
            customizedDescription = "<h3 style=\"margin-bottom:0\">" + alias + "</h3>";
            customizedDescription += "<h5 style=\"margin-top:0;margin-bottom:10px\">" + name + "</h5>";
          }
          else
          {
            customizedDescription = "<h3 style=\"margin-bottom:10px\">" + alias + "</h3>";
          }
        }
        else
        {
          customizedDescription = "<h3 style=\"margin-bottom:10px\">" + name + "</h3>";
        }

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
  bool showGenuineNames = preferences->GetBool(QmitkPropertiesPreferencePage::SHOW_GENUINE_NAMES, true);

  bool updateAliases = showAliases != (m_PropertyAliases != NULL);
  bool updateDescriptions = showDescriptions != (m_PropertyDescriptions != NULL);
  bool updateGenuineNames = showGenuineNames != m_ShowGenuineNames;

  if (updateAliases)
    m_PropertyAliases = showAliases
      ? mitk::GetPropertyService<mitk::PropertyAliases>()
      : NULL;

  if (updateDescriptions)
    m_PropertyDescriptions = showDescriptions
      ? mitk::GetPropertyService<mitk::PropertyDescriptions>()
      : NULL;

  if (updateGenuineNames)
    m_ShowGenuineNames = showGenuineNames;

  if (updateDescriptions || updateGenuineNames)
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
  }
  else
  {
    QString selectionClassName = nodes.front()->GetData() != NULL
      ? nodes.front()->GetData()->GetNameOfClass()
      : "";

    m_Model->SetPropertyList(nodes.front()->GetPropertyList(), selectionClassName);
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
