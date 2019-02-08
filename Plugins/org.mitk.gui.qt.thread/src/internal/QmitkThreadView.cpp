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

#include "QmitkThreadView.h"

#include <QThread>

#include <ui_QmitkThreadView.h>
#include <mitkCppRestSdk.h>
#include <mitkIRESTManager.h>
#include <usModule.h>
#include <usServiceTracker.h>
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

const std::string QmitkThreadView::VIEW_ID = "org.mitk.views.threadview";

QmitkThreadView::QmitkThreadView()
  : m_Ui(new Ui::QmitkThreadView)
{
  mitk::ForceLinkage();
}

web::json::value QmitkThreadView::Notify(web::json::value data)
{
  MITK_INFO << "Observer: Data in observer";
  return data.at(U("key 1"));
}

QmitkThreadView::~QmitkThreadView()
{
  delete m_Ui;
}

void QmitkThreadView::CreateQtPartControl(QWidget* parent)
{
  m_Ui->setupUi(parent);
  m_Ui->stopPushButton->setDisabled(true);

  connect(m_Ui->startPushButton, &QPushButton::clicked, this, &QmitkThreadView::OnStartButtonClicked);
  connect(m_Ui->stopPushButton, &QPushButton::clicked, this, &QmitkThreadView::OnStopButtonClicked);
}

void QmitkThreadView::SetFocus()
{
  m_Ui->startPushButton->setFocus();
}

void QmitkThreadView::OnStartButtonClicked()
{
   us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->ReceiveRequest(L"http://localhost:8080/test", this);
      managerService->ReceiveRequest(L"http://localhost:8090", this);
      managerService->ReceiveRequest(L"http://localhost:8080/example", this);
    }
  }
  m_Ui->stopPushButton->setEnabled(true);
}

void QmitkThreadView::OnStopButtonClicked()
{
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->HandleDeleteObserver(this);
    }
  }
  m_Ui->stopPushButton->setDisabled(true);
  m_Ui->startPushButton->setEnabled(true);
}
