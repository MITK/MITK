/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewNavigatorView.h"

#include "QmitkViewItem.h"
#include "QmitkViewModel.h"
#include "QmitkViewProxyModel.h"

#include <berryUIException.h>

#include <ui_QmitkViewNavigatorView.h>

QmitkViewNavigatorView::QmitkViewNavigatorView()
  : m_Ui(new Ui::QmitkViewNavigatorView),
    m_Model(nullptr),
    m_ProxyModel(nullptr)
{
}

QmitkViewNavigatorView::~QmitkViewNavigatorView()
{
  this->GetPartService()->RemovePartListener(this);
}

void QmitkViewNavigatorView::CreateQtPartControl(QWidget* parent)
{
  m_Ui->setupUi(parent);

  m_Model = new QmitkViewModel(parent);

  m_ProxyModel = new QmitkViewProxyModel(parent);
  m_ProxyModel->setSourceModel(m_Model);

  m_Ui->viewTreeView->setModel(m_ProxyModel);
  m_Ui->viewTreeView->expandAll();

  connect(m_Ui->filterLineEdit, &QLineEdit::textChanged, this, &QmitkViewNavigatorView::OnFilterTextChanged);
  connect(m_Ui->viewTreeView, &QTreeView::doubleClicked, this, &QmitkViewNavigatorView::OnItemDoubleClicked);

  this->GetPartService()->AddPartListener(this);
}

void QmitkViewNavigatorView::SetFocus()
{
  m_Ui->filterLineEdit->setFocus();
}

berry::IWorkbenchPage* QmitkViewNavigatorView::GetActivePage() const
{
  if (auto site = this->GetSite(); site.IsNotNull())
  {
    if (auto workbenchWindow = site->GetWorkbenchWindow(); workbenchWindow.IsNotNull())
      return workbenchWindow->GetActivePage().GetPointer();
  }

  return nullptr;
}

berry::IPartService* QmitkViewNavigatorView::GetPartService() const
{
  if (auto site = this->GetSite(); site.IsNotNull())
  {
    if (auto workbenchWindow = site->GetWorkbenchWindow(); workbenchWindow.IsNotNull())
      return workbenchWindow->GetPartService();
  }

  return nullptr;
}

void QmitkViewNavigatorView::OnFilterTextChanged(const QString& pattern)
{
  m_ProxyModel->setFilterWildcard(pattern);
  m_Ui->viewTreeView->expandAll();
}

void QmitkViewNavigatorView::OnItemDoubleClicked(const QModelIndex& index)
{
  auto viewItem = dynamic_cast<QmitkViewItem*>(m_Model->itemFromIndex(m_ProxyModel->mapToSource(index)));

  if (viewItem != nullptr)
  {
    if (auto activePage = this->GetActivePage(); activePage != nullptr)
    {
      try
      {
        activePage->ShowView(viewItem->data().toString());
      }
      catch (const berry::PartInitException& e)
      {
        MITK_ERROR << e.what();
      }
    }
  }
}

berry::IPartListener::Events::Types QmitkViewNavigatorView::GetPartEventTypes() const
{
  return Events::OPENED | Events::CLOSED;
}

void QmitkViewNavigatorView::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() != "org.mitk.views.viewnavigator")
  {
    auto viewItem = m_Model->GetViewItemFromId(partRef->GetId());

    if (viewItem != nullptr)
      viewItem->SetBoldFont(true);
  }
  else if (auto activePage = this->GetActivePage(); activePage != nullptr)
  {
    for (const auto& view : activePage->GetViews())
    {
      auto viewItem = m_Model->GetViewItemFromId(view->GetSite()->GetId());

      if (viewItem != nullptr)
        viewItem->SetBoldFont(true);
    }
  }
}

void QmitkViewNavigatorView::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  auto viewItem = m_Model->GetViewItemFromId(partRef->GetId());

  if (viewItem != nullptr)
    viewItem->SetBoldFont(false);
}
