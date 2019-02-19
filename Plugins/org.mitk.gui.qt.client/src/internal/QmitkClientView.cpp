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

#include "QmitkClientView.h"

#include <QThread>

#include "cpprest/json.h"
#include <mitkCppRestSdk.h>
#include <mitkIRESTManager.h>
#include <ui_QmitkClientView.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>
#include <usServiceTracker.h>

const std::string QmitkClientView::VIEW_ID = "org.mitk.views.clientview";

QmitkClientView::QmitkClientView() : m_Ui(new Ui::QmitkClientView)
{
  mitk::ForceLinkage();
}

QmitkClientView::~QmitkClientView()
{
  delete m_Ui;
}

void QmitkClientView::CreateQtPartControl(QWidget *parent)
{
  m_Ui->setupUi(parent);

  connect(m_Ui->getPushButton, &QPushButton::clicked, this, &QmitkClientView::OnGetButtonClicked);
  connect(m_Ui->putPushButton, &QPushButton::clicked, this, &QmitkClientView::OnPutButtonClicked);
  connect(m_Ui->postPushButton, &QPushButton::clicked, this, &QmitkClientView::OnPostButtonClicked);
  connect(this, &QmitkClientView::UpdateProgressBar, this, &QmitkClientView::OnUpdateProgressBar);
  connect(this, SIGNAL(UpdateLabel(QString)), this, SLOT(OnUpdateLabel(QString)));
  m_Ui->progressBar->setValue(0);
}

void QmitkClientView::OnUpdateProgressBar()
{
  m_Ui->progressBar->setValue(m_Ui->progressBar->value() + 5);
}

void QmitkClientView::OnUpdateLabel(QString text)
{
  m_Ui->responseLabel->setText(text);
}

void QmitkClientView::SetFocus() {}

void QmitkClientView::OnGetButtonClicked()
{
  m_Ui->progressBar->setValue(0);
  m_Ui->getPushButton->setDisabled(true);
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      std::vector<pplx::task<void>> tasks;
      for (int i = 0; i < 20; i++)
      {
        pplx::task<void> singleTask = managerService->SendRequest(L"https://jsonplaceholder.typicode.com/photos")
            .then([=](pplx::task<web::json::value> resultTask) {
          try
          {
            resultTask.get();
            emit UpdateProgressBar();
          }
          catch (const mitk::Exception &exception)
          {
            MITK_ERROR << exception.what();
            return;
          }
          });
        tasks.push_back(singleTask);
      }
      auto joinTask = pplx::when_all(begin(tasks), end(tasks));
      joinTask.then([=](pplx::task<void> resultTask) { 
        try
        {
          resultTask.get();
          emit UpdateLabel("All tasks finished");
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        }
        });
      m_Ui->responseLabel->setText("Waiting for change");
    }
  }
  m_Ui->getPushButton->setEnabled(true);
}

void QmitkClientView::OnPutButtonClicked()
{
  m_Ui->putPushButton->setDisabled(true);
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      web::json::value data;
      data[L"userId"] = web::json::value(1);
      data[L"id"] = web::json::value(1);
      data[L"title"] = web::json::value(U("this is a changed title"));
      data[L"body"] = web::json::value(U("and the body is changed as well"));
      managerService->SendRequest(
        L"https://jsonplaceholder.typicode.com/posts/1", mitk::IRESTManager::RequestType::put, data)
        .then([=](pplx::task<web::json::value> resultTask) {
          try
          {
            web::json::value result = resultTask.get();
            utility::string_t stringT = result.to_string();
            std::string stringStd(stringT.begin(), stringT.end());
            QString stringQ = QString::fromStdString(stringStd);
            emit UpdateLabel(stringQ);
          }
          catch (const mitk::Exception &exception)
          {
            MITK_ERROR << exception.what();
            return;
          }
        });
    }
  }
  m_Ui->putPushButton->setEnabled(true);
}

void QmitkClientView::OnPostButtonClicked()
{
  m_Ui->postPushButton->setDisabled(true);
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      web::json::value data;
      data[L"userId"] = web::json::value(1);
      data[L"title"] = web::json::value(U("this is a new title"));
      data[L"body"] = web::json::value(U("this is a new body"));
      managerService
        ->SendRequest(L"https://jsonplaceholder.typicode.com/posts", mitk::IRESTManager::RequestType::post, data)
        .then([=](pplx::task<web::json::value> resultTask) {
        try
        {
          web::json::value result = resultTask.get();
          utility::string_t stringT = result.to_string();
          std::string stringStd(stringT.begin(), stringT.end());
          QString stringQ = QString::fromStdString(stringStd);
          emit UpdateLabel(stringQ);
        }
        catch (const mitk::Exception &exception)
        {
          MITK_ERROR << exception.what();
          return;
        } 
        });
    }
  }
  m_Ui->postPushButton->setEnabled(true);
}
