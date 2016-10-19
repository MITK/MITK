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

#include <memory>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIBerryPreferences.h>

// Qmitk
#include "QmitkOverlayManagerView.h"
#include "QmitkAddNewPropertyDialog.h"
#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include <QmitkRenderWindow.h>

// Qt
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QPainter>

#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include "mitkGetPropertyService.h"
#include <mitkIPropertyPersistence.h>
#include "mitkOverlayManager.h"
#include "mitkRenderingManager.h"
#include <mitkTextOverlay2D.h>
#include <mitkTextOverlay3D.h>
#include <mitkLabelOverlay3D.h>
#include <mitkScaleLegendOverlay.h>
#include <mitkColorBarOverlay.h>
#include <mitkLogoOverlay.h>

const std::string QmitkOverlayManagerView::VIEW_ID = "org.mitk.views.overlaymanager";

QmitkOverlayManagerView::QmitkOverlayManagerView()
  : m_Parent(nullptr),
    m_PropertyNameChangedTag(0),
    m_OverlayManagerObserverTag(0),
    m_PropertyAliases(nullptr),
    m_PropertyDescriptions(nullptr),
    m_PropertyPersistence(nullptr),
    m_ProxyModel(nullptr),
    m_Model(nullptr),
    m_Delegate(nullptr),
    m_Renderer(nullptr),
    m_AddOverlayMenu(nullptr)
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
  m_Controls.propertyListComboBox->addItem("common");
  m_Controls.m_OverlayList->clear();

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

  if (renderWindowPart != NULL)
  {
    QHash<QString, QmitkRenderWindow*> renderWindows = renderWindowPart->GetQmitkRenderWindows();

    Q_FOREACH(QString renderWindow, renderWindows.keys())
    {
      m_Controls.propertyListComboBox->addItem(renderWindow);
    }
  }

  InitializeAddAnimationMenu();

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
  connect(m_Controls.m_OverlayList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(OnOverlaySelectionChanged(QListWidgetItem*,QListWidgetItem*))  );
  connect(m_Controls.m_DeleteOverlay, SIGNAL(clicked()), this, SLOT(OnDelete()));
  connect(m_Controls.m_AddOverlay, SIGNAL(clicked()), this, SLOT(OnAddOverlay()));
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
        isPersistent = m_PropertyPersistence->HasInfos(name.toStdString());

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

}

void QmitkOverlayManagerView::InitializeAddAnimationMenu()
{
  m_AddOverlayMenu = new QMenu(m_Controls.m_AddOverlay);

  m_AddOverlayMenu->addAction("TextOverlay2D");
  m_AddOverlayMenu->addAction("TextOverlay3D");
  m_AddOverlayMenu->addAction("LabelOverlay");
  m_AddOverlayMenu->addAction("ColorBarOverlay");
  m_AddOverlayMenu->addAction("ScaleLegendOverlay");
  m_AddOverlayMenu->addAction("LogoOverlay");
}

void QmitkOverlayManagerView::OnOverlayAdded(itk::Object *,const itk::EventObject &event)
{
  const mitk::OverlayAddEvent* addEvent = dynamic_cast<const mitk::OverlayAddEvent*>(&event);
  if(addEvent)
  {
    OnActivateOverlayList();
  }
}

void QmitkOverlayManagerView::Activated()
{

}

void QmitkOverlayManagerView::Deactivated()
{
}

void QmitkOverlayManagerView::Visible()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  if(om && m_OverlayManagerObserverTag == 0)
  {
    itk::MemberCommand<QmitkOverlayManagerView>::Pointer command
        = itk::MemberCommand<QmitkOverlayManagerView>::New();
    command->SetCallbackFunction(this, &QmitkOverlayManagerView::OnOverlayAdded );
    m_OverlayManagerObserverTag = om->AddObserver(mitk::OverlayAddEvent(), command);
  }
  this->OnActivateOverlayList();
}

void QmitkOverlayManagerView::Hidden()
{
}

void QmitkOverlayManagerView::OnPropertyListChanged(int index)
{
  if (index == -1)
    return;

  QString renderer = m_Controls.propertyListComboBox->itemText(index);

  QmitkRenderWindow* renwin = this->GetRenderWindowPart()->GetQmitkRenderWindow(renderer);
  m_Renderer= renwin ? renwin->GetRenderer() : nullptr;

  this->OnOverlaySelectionChanged(m_Controls.m_OverlayList->currentItem(), nullptr);
}

void QmitkOverlayManagerView::OnAddNewProperty()
{
  std::unique_ptr<QmitkAddNewPropertyDialog> dialog(
        new QmitkAddNewPropertyDialog(m_SelectedOverlay, m_Renderer, m_Parent));

  if (dialog->exec() == QDialog::Accepted)
    this->m_Model->Update();
}

void QmitkOverlayManagerView::OnActivateOverlayList()
{
  typedef mitk::OverlayManager::OverlaySet OverlaySet;
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  m_Controls.m_OverlayList->clear();
  if(om)
  {
    OverlaySet oset = om->GetAllOverlays();
    for (auto overlay : oset)
    {
      QListWidgetItem* item = new QListWidgetItem();
      item->setData(Qt::UserRole,QVariant::fromValue<void*>(overlay.GetPointer()));
      QString text(overlay->GetName().c_str());
      if(text.length()>0)
      {
        text.append(" : ");
      }
      text.append(overlay->GetNameOfClass());
      item->setText(text);
      m_Controls.m_OverlayList->addItem(item);
    }
  }
}

void QmitkOverlayManagerView::OnOverlaySelectionChanged(QListWidgetItem *current, QListWidgetItem *)
{
  mitk::PropertyList* propertyList = m_Model->GetPropertyList();

  if (propertyList != NULL)
  {
    mitk::BaseProperty* nameProperty = propertyList->GetProperty("name");

    if (nameProperty != NULL)
      nameProperty->RemoveObserver(m_PropertyNameChangedTag);

    m_PropertyNameChangedTag = 0;
  }

  mitk::Overlay* overlay = nullptr;
  if(current)
    overlay = reinterpret_cast<mitk::Overlay*>(current->data(Qt::UserRole).value<void*>());
  if (!overlay)
  {
    m_SelectedOverlay = NULL;

    this->SetPartName("Overlay Properties");
    m_Model->SetPropertyList(NULL);
    m_Delegate->SetPropertyList(NULL);

    m_Controls.newButton->setEnabled(false);
  }
  else
  {
    m_SelectedOverlay = overlay;

    QString selectionClassName = m_SelectedOverlay->GetNameOfClass();

    m_SelectionClassName = selectionClassName.toStdString();

    mitk::PropertyList::Pointer propertyList = overlay->GetPropertyList(m_Renderer);

    m_Model->SetPropertyList(propertyList, selectionClassName);
    m_Delegate->SetPropertyList(propertyList);

    OnPropertyNameChanged(itk::ModifiedEvent());

    mitk::BaseProperty* nameProperty = m_SelectedOverlay->GetProperty("name");

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

void QmitkOverlayManagerView::OnDelete()
{
  auto items = m_Controls.m_OverlayList->selectedItems();
  if(items.count() < 1) return;
  mitk::Overlay* overlay = nullptr;
  QListWidgetItem* item = items[0];
  if(item)
    overlay = reinterpret_cast<mitk::Overlay*>(item->data(Qt::UserRole).value<void*>());
  mitk::OverlayManager::GetInstance()->RemoveOverlay(overlay);
}

void QmitkOverlayManagerView::OnAddOverlay()
{
  QAction* action = m_AddOverlayMenu->exec(QCursor::pos());

  if (action != NULL)
  {
    const QString widgetKey = action->text();

    if (widgetKey == "TextOverlay2D")
      OnAddTextOverlay2D();

    else if (widgetKey == "TextOverlay3D")
      OnAddTextOverlay3D();

    else if (widgetKey == "LabelOverlay")
      OnAddLabelOverlay();

    else if (widgetKey == "ColorBarOverlay")
      OnAddColorBarOverlay();

    else if (widgetKey == "ScaleLegendOverlay")
      OnAddScaleLegendOverlay();

    else if (widgetKey == "LogoOverlay")
      OnAddLogoOverlay();
  }
}

void QmitkOverlayManagerView::OnAddTextOverlay2D()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  mitk::TextOverlay2D::Pointer to = mitk::TextOverlay2D::New();
  om->AddOverlay(to.GetPointer());
}

void QmitkOverlayManagerView::OnAddTextOverlay3D()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  mitk::TextOverlay3D::Pointer to = mitk::TextOverlay3D::New();
  om->AddOverlay(to.GetPointer());
}

void QmitkOverlayManagerView::OnAddLabelOverlay()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  mitk::LabelOverlay3D::Pointer to = mitk::LabelOverlay3D::New();
  om->AddOverlay(to.GetPointer());
}

void QmitkOverlayManagerView::OnAddColorBarOverlay()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  mitk::ColorBarOverlay::Pointer to = mitk::ColorBarOverlay::New();
  om->AddOverlay(to.GetPointer());
}

void QmitkOverlayManagerView::OnAddScaleLegendOverlay()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  mitk::ScaleLegendOverlay::Pointer to = mitk::ScaleLegendOverlay::New();
  om->AddOverlay(to.GetPointer());
}

void QmitkOverlayManagerView::OnAddLogoOverlay()
{
  mitk::OverlayManager* om = mitk::OverlayManager::GetInstance();
  mitk::LogoOverlay::Pointer to = mitk::LogoOverlay::New();
  om->AddOverlay(to.GetPointer());
}

void QmitkOverlayManagerView::RenderWindowPartActivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  if (m_Controls.propertyListComboBox->count() == 1)
  {
    QHash<QString, QmitkRenderWindow*> renderWindows = this->GetRenderWindowPart()->GetQmitkRenderWindows();

    Q_FOREACH(QString renderWindow, renderWindows.keys())
    {
      m_Controls.propertyListComboBox->insertItem(m_Controls.propertyListComboBox->count() - 1, renderWindow);
    }
  }
  OnActivateOverlayList();
}

void QmitkOverlayManagerView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  if (m_Controls.propertyListComboBox->count() > 1)
  {
    m_Controls.propertyListComboBox->clear();
    m_Controls.propertyListComboBox->addItem("common");
  }
  m_Controls.m_OverlayList->clear();
}
