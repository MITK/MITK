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
#include <mitkIRESTManager.h>
#include <usModule.h>
#include <usServiceTracker.h>
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

const std::string QmitkThreadView::VIEW_ID = "org.mitk.views.threadview";

QmitkThreadView::QmitkThreadView()
  : m_Ui(new Ui::QmitkThreadView)
{
}

web::json::value QmitkThreadView::Notify(const web::json::value &data, const web::uri &uri)
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

  connect(m_Ui->stopAllPushButton, &QPushButton::clicked, this, &QmitkThreadView::OnStopAllButtonClicked);
  
  connect(m_Ui->testListenCheckBox, &QCheckBox::clicked, this, &QmitkThreadView::OnTestListenCheckBoxClicked);
  connect(m_Ui->exampleListenCheckBox, &QCheckBox::clicked, this, &QmitkThreadView::OnExampleListenCheckBoxClicked);
  connect(m_Ui->port8090CheckBox, &QCheckBox::clicked, this, &QmitkThreadView::OnPort8090ListenCheckBoxClicked);

  connect(m_Ui->stopAllPushButton, &QPushButton::clicked, this, &QmitkThreadView::OnStopAllButtonClicked);
}

void QmitkThreadView::SetFocus()
{
}


void QmitkThreadView::StartListening(web::uri uri)
{
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->ReceiveRequest(uri, this);
    }
  }
}

void QmitkThreadView::StopListening(web::uri uri) 
{
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->HandleDeleteObserver(this,uri);
    }
  }
}

void QmitkThreadView::OnStopAllButtonClicked()
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
  m_Ui->testListenCheckBox->setChecked(false);
  m_Ui->exampleListenCheckBox->setChecked(false);
  m_Ui->port8090CheckBox->setChecked(false);
}

void QmitkThreadView::OnTestListenCheckBoxClicked()
{
  if (m_Ui->testListenCheckBox->isChecked())
  {
    StartListening(L"http://localhost:8080/test");
  }
  else
  {
    StopListening(L"http://localhost:8080/test");
  }
}

void QmitkThreadView::OnExampleListenCheckBoxClicked() 
{
  if (m_Ui->exampleListenCheckBox->isChecked())
  {
    StartListening(L"http://localhost:8080/example");
  }
  else
  {
    StopListening(L"http://localhost:8080/example");
  }
}

void QmitkThreadView::OnPort8090ListenCheckBoxClicked() 
{
  if (m_Ui->port8090CheckBox->isChecked())
  {
    StartListening(L"http://localhost:8090");
  }
  else
  {
    StopListening(L"http://localhost:8090");
  }
}
