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

// Qmitk
#include "QmitkOverlayManagerView.h"

// Qt
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include "mitkGetPropertyService.h"
#include "QmitkAddNewPropertyDialog.h"
#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include <berryIBerryPreferences.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>
#include <QmitkRenderWindow.h>
#include <QPainter>
#include <memory>

const std::string QmitkOverlayManagerView::VIEW_ID = "org.mitk.views.overlaymanager";

QmitkOverlayManagerView::QmitkOverlayManagerView()
  : m_Parent(nullptr),
    m_PropertyNameChangedTag(0),
    m_PropertyAliases(nullptr),
    m_PropertyDescriptions(nullptr),
    m_PropertyPersistence(nullptr),
    m_ProxyModel(nullptr),
    m_Model(nullptr),
    m_Delegate(nullptr),
    m_Renderer(nullptr)
{
}

QmitkOverlayManagerView::~QmitkOverlayManagerView()
{
}

void QmitkOverlayManagerView::SetFocus()
{
}

void QmitkOverlayManagerView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
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

  m_ProxyModel = new QSortFilterProxyModel(m_Controls.m_PropertyTree);
  m_Model = new QmitkPropertyItemModel(m_ProxyModel);

  m_ProxyModel->setSourceModel(m_Model);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setDynamicSortFilter(true);

  m_Delegate = new QmitkPropertyItemDelegate(m_Controls.m_PropertyTree);

  m_Controls.m_PropertyTree->setItemDelegateForColumn(1, m_Delegate);
  m_Controls.m_PropertyTree->setModel(m_ProxyModel);
  m_Controls.m_PropertyTree->setColumnWidth(0, 160);
  m_Controls.m_PropertyTree->sortByColumn(0, Qt::AscendingOrder);
  m_Controls.m_PropertyTree->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.m_PropertyTree->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.m_PropertyTree->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

  connect(m_Controls.propertyListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPropertyListChanged(int)));
  connect(m_Controls.newButton, SIGNAL(clicked()), this, SLOT(OnAddNewProperty()));
  connect(m_Controls.m_PropertyTree->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnCurrentRowChanged(const QModelIndex&, const QModelIndex&)));
}

QString QmitkOverlayManagerView::GetPropertyNameOrAlias(const QModelIndex& index)
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

void QmitkOverlayManagerView::OnCurrentRowChanged(const QModelIndex& current, const QModelIndex&)
{
  if (m_PropertyDescriptions != nullptr && current.isValid())
  {
    QString name = this->GetPropertyNameOrAlias(current);

    if (!name.isEmpty())
    {
      QString alias;
      bool isTrueName = true;

      if (m_PropertyAliases != nullptr)
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

      if (!isTrueName && m_PropertyAliases != nullptr)
      {
        aliases = m_PropertyAliases->GetAliases(name.toStdString(), m_SelectionClassName);

        if (aliases.empty() && !m_SelectionClassName.empty())
          aliases = m_PropertyAliases->GetAliases(name.toStdString());
      }

      bool isPersistent = false;
      // QString persistenceKey;

      if (m_PropertyPersistence != nullptr)
      {
        isPersistent = m_PropertyPersistence->HasInfo(name.toStdString());

        /*if (isPersistent)
        {
          persistenceKey = QString::fromStdString(m_PropertyPersistence->GetInfo(name.toStdString())->GetKey());

          if (persistenceKey.isEmpty())
            persistenceKey = name;
        }*/
      }

      if (!description.isEmpty() || !aliases.empty() || isPersistent)
      {
        QString customizedDescription;

        if (!description.isEmpty())
          customizedDescription += "<p>" + description + "</p>";

        if (!aliases.empty() || isPersistent)
        {
          customizedDescription += "<div align=\"right\">";

          if (!aliases.empty())
          {
            customizedDescription += aliases.size() > 1
              ? "<img height=\"32\" src=\":/org_mitk_icons/icons/awesome/scalable/tags.svg\"/>"
              : "<img height=\"32\" src=\":/org_mitk_icons/icons/awesome/scalable/tag.svg\"/>";
          }

          if (isPersistent)
            customizedDescription += "<img height=\"32\" src=\":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg\"/>";

          customizedDescription += "</div>";
        }

        return;
      }
    }
  }
}

void QmitkOverlayManagerView::OnPropertyNameChanged(const itk::EventObject&)
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

void QmitkOverlayManagerView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
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
      itk::ReceptorMemberCommand<QmitkOverlayManagerView>::Pointer command = itk::ReceptorMemberCommand<QmitkOverlayManagerView>::New();
      command->SetCallbackFunction(this, &QmitkOverlayManagerView::OnPropertyNameChanged);
      m_PropertyNameChangedTag = nameProperty->AddObserver(itk::ModifiedEvent(), command);
    }

    m_Controls.newButton->setEnabled(true);
  }

  if (!m_ProxyModel->filterRegExp().isEmpty())
    m_Controls.m_PropertyTree->expandAll();
}

void QmitkOverlayManagerView::OnPropertyListChanged(int index)
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

void QmitkOverlayManagerView::OnAddNewProperty()
{
  std::unique_ptr<QmitkAddNewPropertyDialog> dialog(m_Controls.propertyListComboBox->currentText() != "Base data"
      ? new QmitkAddNewPropertyDialog(m_SelectedNode, m_Renderer, m_Parent)
      : new QmitkAddNewPropertyDialog(m_SelectedNode->GetData()));

  if (dialog->exec() == QDialog::Accepted)
    this->m_Model->Update();
}
