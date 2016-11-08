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
#include <berryIBerryPreferences.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkAddNewPropertyDialog.h"
#include "QmitkOverlayManagerView.h"
#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include <QmitkRenderWindow.h>

// Qt
#include <QMessageBox>
#include <QPainter>
#include <QSortFilterProxyModel>

#include "internal/org_mitk_gui_qt_overlaymanager_Activator.h"

#include "mitkAnnotationPlacer.h"
#include "mitkAnnotationService.h"
#include "mitkGetPropertyService.h"
#include "mitkOverlayManager.h"
#include "mitkRenderingManager.h"
#include <mitkColorBarOverlay.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>
#include <mitkLabelOverlay3D.h>
#include <mitkLogoOverlay.h>
#include <mitkScaleLegendOverlay.h>
#include <mitkTextOverlay2D.h>
#include <mitkTextOverlay3D.h>

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

void QmitkOverlayManagerView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Controls.m_OverlayList->clear();

  mitk::IRenderWindowPart *renderWindowPart = this->GetRenderWindowPart();

  if (renderWindowPart != NULL)
  {
    QHash<QString, QmitkRenderWindow *> renderWindows = renderWindowPart->GetQmitkRenderWindows();

    Q_FOREACH (QString renderWindow, renderWindows.keys())
    {
      if (!m_Renderer)
        m_Renderer = renderWindows[renderWindow]->GetRenderer();
      m_Controls.m_RendererCB->addItem(renderWindow);
    }
  }

  InitializeAddOverlayMenu();

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

  connect(m_Controls.m_RendererCB, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPropertyListChanged(int)));
  connect(m_Controls.newButton, SIGNAL(clicked()), this, SLOT(OnAddNewProperty()));
  connect(m_Controls.m_PropertyTree->selectionModel(),
          SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
          this,
          SLOT(OnCurrentRowChanged(const QModelIndex &, const QModelIndex &)));
  connect(m_Controls.m_OverlayList,
          SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this,
          SLOT(OnOverlaySelectionChanged(QListWidgetItem *, QListWidgetItem *)));
  connect(m_Controls.m_DeleteOverlay, SIGNAL(clicked()), this, SLOT(OnDelete()));
  connect(m_Controls.m_AddOverlay, SIGNAL(clicked()), this, SLOT(OnAddOverlay()));

  itk::MemberCommand<QmitkOverlayManagerView>::Pointer command = itk::MemberCommand<QmitkOverlayManagerView>::New();
  command->SetCallbackFunction(this, &QmitkOverlayManagerView::OnFocusChanged);
  m_RenderWindowFocusObserverTag =
    mitk::RenderingManager::GetInstance()->AddObserver(mitk::FocusChangedEvent(), command);
}

void QmitkOverlayManagerView::OnFocusChanged(itk::Object * /*caller*/, const itk::EventObject &event)
{
  const mitk::FocusChangedEvent *focusEvent = dynamic_cast<const mitk::FocusChangedEvent *>(&event);
  if (focusEvent)
  {
    QHash<QString, QmitkRenderWindow *> renderWindows = this->GetRenderWindowPart()->GetQmitkRenderWindows();
    m_Controls.m_RendererCB->clear();
    Q_FOREACH (QString renderWindow, renderWindows.keys())
    {
      m_Controls.m_RendererCB->addItem(renderWindow);
      if (renderWindows[renderWindow]->GetVtkRenderWindow() ==
          mitk::RenderingManager::GetInstance()->GetFocusedRenderWindow())
      {
        m_Controls.m_RendererCB->setCurrentText(renderWindow);
      }
    }
    this->OnActivateOverlayList();
  }
}

QString QmitkOverlayManagerView::GetPropertyNameOrAlias(const QModelIndex &index)
{
  QString propertyName;

  if (index.isValid())
  {
    QModelIndex current = index;

    while (current.isValid())
    {
      QString name = m_ProxyModel->data(m_ProxyModel->index(current.row(), 0, current.parent())).toString();

      propertyName.prepend(propertyName.isEmpty() ? name : name.append('.'));

      current = current.parent();
    }
  }

  return propertyName;
}

void QmitkOverlayManagerView::OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &)
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
            customizedDescription += aliases.size() > 1 ?
                                       "<img height=\"32\" src=\":/org_mitk_icons/icons/awesome/scalable/tags.svg\"/>" :
                                       "<img height=\"32\" src=\":/org_mitk_icons/icons/awesome/scalable/tag.svg\"/>";
          }

          if (isPersistent)
            customizedDescription +=
              "<img height=\"32\" src=\":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg\"/>";

          customizedDescription += "</div>";
        }

        return;
      }
    }
  }
}

void QmitkOverlayManagerView::OnPropertyNameChanged(const itk::EventObject &)
{
  mitk::PropertyList *propertyList = m_Model->GetPropertyList();

  if (propertyList != NULL)
  {
    mitk::BaseProperty *nameProperty = propertyList->GetProperty("name");

    if (nameProperty != NULL)
    {
      QString partName = "Properties (";
      partName.append(QString::fromStdString(nameProperty->GetValueAsString())).append(')');
      this->SetPartName(partName);
    }
  }
}

void QmitkOverlayManagerView::OnSelectionChanged(berry::IWorkbenchPart::Pointer,
                                                 const QList<mitk::DataNode::Pointer> &nodes)
{
}

void QmitkOverlayManagerView::InitializeAddOverlayMenu()
{
  m_AddOverlayMenu = new QMenu(m_Controls.m_AddOverlay);

  m_AddOverlayMenu->addAction("TextOverlay2D");
  m_AddOverlayMenu->addAction("TextOverlay3D");
  m_AddOverlayMenu->addAction("LabelOverlay");
  m_AddOverlayMenu->addAction("ColorBarOverlay");
  m_AddOverlayMenu->addAction("ScaleLegendOverlay");
  m_AddOverlayMenu->addAction("LogoOverlay");
}

void QmitkOverlayManagerView::Activated()
{
  //  this->OnActivateOverlayList();
}

void QmitkOverlayManagerView::Deactivated()
{
}

void QmitkOverlayManagerView::Visible()
{
  this->OnActivateOverlayList();
}

void QmitkOverlayManagerView::Hidden()
{
}

void QmitkOverlayManagerView::OnPropertyListChanged(int index)
{
  if (index == -1)
    return;

  QString renderer = m_Controls.m_RendererCB->itemText(index);

  QmitkRenderWindow *renwin = this->GetRenderWindowPart()->GetQmitkRenderWindow(renderer);
  m_Renderer = renwin ? renwin->GetRenderer() : nullptr;

  this->OnOverlaySelectionChanged(m_Controls.m_OverlayList->currentItem(), nullptr);
  this->OnActivateOverlayList();
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
  if (!m_Renderer)
    return;
  std::vector<mitk::AbstractAnnotationRenderer *> arList =
    mitk::AnnotationService::GetAnnotationRenderer(m_Renderer->GetName());
  m_Controls.m_OverlayList->clear();
  for (auto ar : arList)
  {
    for (auto overlay : ar->GetServices())
    {
      QListWidgetItem *item = new QListWidgetItem();
      item->setData(Qt::UserRole, QVariant(overlay->GetMicroserviceID().c_str()));
      QString text(overlay->GetName().c_str());
      if (text.length() > 0)
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
  mitk::PropertyList *propertyList = m_Model->GetPropertyList();

  if (propertyList != NULL)
  {
    mitk::BaseProperty *nameProperty = propertyList->GetProperty("name");

    if (nameProperty != NULL)
      nameProperty->RemoveObserver(m_PropertyNameChangedTag);

    m_PropertyNameChangedTag = 0;
  }

  mitk::Overlay *overlay = nullptr;
  QString oID;
  if (current)
  {
    oID = current->data(Qt::UserRole).toString();
    OverlayMapType::iterator it = m_OverlayMap.find(oID.toStdString());
    if (it != m_OverlayMap.end())
      overlay = it->second;
    else
    {
      std::string ldapFilter = "(" + mitk::Overlay::US_PROPKEY_ID + "=" + oID.toStdString() + ")";
      std::vector<us::ServiceReference<mitk::Overlay>> overlays =
        us::GetModuleContext()->GetServiceReferences<mitk::Overlay>(ldapFilter);
      if (!overlays.empty())
      {
        overlay = us::GetModuleContext()->GetService<mitk::Overlay>(overlays.front());
      }
    }
  }

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

    mitk::PropertyList::Pointer propertyList = overlay->GetPropertyList();

    m_Model->SetPropertyList(propertyList, selectionClassName);
    m_Delegate->SetPropertyList(propertyList);

    OnPropertyNameChanged(itk::ModifiedEvent());

    mitk::BaseProperty *nameProperty = m_SelectedOverlay->GetProperty("name");

    if (nameProperty != NULL)
    {
      itk::ReceptorMemberCommand<QmitkOverlayManagerView>::Pointer command =
        itk::ReceptorMemberCommand<QmitkOverlayManagerView>::New();
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
  if (items.count() < 1)
    return;
  mitk::Overlay *overlay = nullptr;
  QListWidgetItem *item = items[0];
  if (item)
    overlay = reinterpret_cast<mitk::Overlay *>(item->data(Qt::UserRole).value<void *>());
  mitk::OverlayManager::GetInstance()->RemoveOverlay(overlay);
}

void QmitkOverlayManagerView::OnAddOverlay()
{
  QAction *action = m_AddOverlayMenu->exec(QCursor::pos());

  mitk::Overlay::Pointer overlay;

  if (action != NULL)
  {
    const QString widgetKey = action->text();

    if (widgetKey == "TextOverlay2D")
      overlay = CreateTextOverlay2D();

    else if (widgetKey == "TextOverlay3D")
      overlay = CreateTextOverlay3D();

    else if (widgetKey == "LabelOverlay")
      overlay = CreateLabelOverlay();

    else if (widgetKey == "ColorBarOverlay")
      overlay = CreateColorBarOverlay();

    else if (widgetKey == "ScaleLegendOverlay")
      overlay = CreateScaleLegendOverlay();

    else if (widgetKey == "LogoOverlay")
      overlay = CreateLogoOverlay();

    mitk::BaseRenderer *renderer =
      this->GetRenderWindowPart()->GetQmitkRenderWindow(m_Controls.m_RendererCB->currentText())->GetRenderer();
    mitk::AnnotationPlacer::AddOverlay(overlay, renderer);
    m_OverlayMap[overlay->GetMicroserviceID()] = overlay;
  }
  OnActivateOverlayList();
}

mitk::Overlay::Pointer QmitkOverlayManagerView::CreateTextOverlay2D()
{
  mitk::TextOverlay2D::Pointer to = mitk::TextOverlay2D::New();
  return to.GetPointer();
}

mitk::Overlay::Pointer QmitkOverlayManagerView::CreateTextOverlay3D()
{
  mitk::TextOverlay3D::Pointer to = mitk::TextOverlay3D::New();
  return to.GetPointer();
}

mitk::Overlay::Pointer QmitkOverlayManagerView::CreateLabelOverlay()
{
  mitk::LabelOverlay3D::Pointer to = mitk::LabelOverlay3D::New();
  return to.GetPointer();
}

mitk::Overlay::Pointer QmitkOverlayManagerView::CreateColorBarOverlay()
{
  mitk::ColorBarOverlay::Pointer to = mitk::ColorBarOverlay::New();
  return to.GetPointer();
}

mitk::Overlay::Pointer QmitkOverlayManagerView::CreateScaleLegendOverlay()
{
  mitk::ScaleLegendOverlay::Pointer to = mitk::ScaleLegendOverlay::New();
  return to.GetPointer();
}

mitk::Overlay::Pointer QmitkOverlayManagerView::CreateLogoOverlay()
{
  mitk::LogoOverlay::Pointer to = mitk::LogoOverlay::New();
  return to.GetPointer();
}

void QmitkOverlayManagerView::RenderWindowPartActivated(mitk::IRenderWindowPart * /*renderWindowPart*/)
{
  if (m_Controls.m_RendererCB->count() == 0)
  {
    QHash<QString, QmitkRenderWindow *> renderWindows = this->GetRenderWindowPart()->GetQmitkRenderWindows();

    Q_FOREACH (QString renderWindow, renderWindows.keys())
    {
      m_Controls.m_RendererCB->addItem(renderWindow);
    }
  }
  OnActivateOverlayList();
}

void QmitkOverlayManagerView::RenderWindowPartDeactivated(mitk::IRenderWindowPart *)
{
  if (m_Controls.m_RendererCB->count() > 0)
  {
    m_Controls.m_RendererCB->clear();
  }
  m_Controls.m_OverlayList->clear();
}
