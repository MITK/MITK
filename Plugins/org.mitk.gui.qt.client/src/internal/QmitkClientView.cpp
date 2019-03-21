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
#include <mitkIRESTManager.h>
#include <ui_QmitkClientView.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>
#include <usServiceTracker.h>

const std::string QmitkClientView::VIEW_ID = "org.mitk.views.clientview";

QmitkClientView::QmitkClientView() : m_Ui(new Ui::QmitkClientView)
{
}

QmitkClientView::~QmitkClientView()
{
  delete m_Ui;
}

void QmitkClientView::CreateQtPartControl(QWidget *parent)
{
  m_Ui->setupUi(parent);

  connect(m_Ui->getMultiplePushButton, &QPushButton::clicked, this, &QmitkClientView::OnGetMultipleButtonClicked);
  connect(m_Ui->getSinglePushButton, &QPushButton::clicked, this, &QmitkClientView::OnGetSingleButtonClicked);
  connect(m_Ui->getSavePushButton, &QPushButton::clicked, this, &QmitkClientView::OnGetSaveButtonClicked);
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

void QmitkClientView::OnGetMultipleButtonClicked()
{
  m_Ui->progressBar->setValue(0);
  m_Ui->getMultiplePushButton->setDisabled(true);
  //Get microservice
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      //Create multiple tasks e.g. as shown below
      std::vector<pplx::task<void>> tasks;
      for (int i = 0; i < 20; i++)
      {
        pplx::task<void> singleTask = managerService->SendRequest(L"http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE/rs/studies/1.2.840.113654.2.70.1.97144850941324808603541273584489321943/series/1.2.840.113654.2.70.1.15771179684190906938515254678965278540/instances")
            .then([=](pplx::task<web::json::value> resultTask) {
          //Do something when a single task is done
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
      //Create a joinTask which includes all tasks you've created
      auto joinTask = pplx::when_all(begin(tasks), end(tasks));
      //Run asynchonously
      joinTask.then([=](pplx::task<void> resultTask) { 
        //Do something when all tasks are finished
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
  m_Ui->getMultiplePushButton->setEnabled(true);
}

void QmitkClientView::OnGetSingleButtonClicked() 
{
  m_Ui->putPushButton->setDisabled(true);
  //Get the micorservice
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      //Call the send request method which starts the actual request 
      managerService
        ->SendRequest(L"http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE/rs/studies/1.2.840.113654.2.70.1.97144850941324808603541273584489321943/series/1.2.840.113654.2.70.1.15771179684190906938515254678965278540/instances")
        .then([=](pplx::task<web::json::value> resultTask)/*It is important to use task-based continuation*/ {
          try
          {
            //Get the result of the request
            //This will throw an exception if the ascendent task threw an exception (e.g. invalid URI) 
            web::json::value result = resultTask.get();
            //Do something with the result (e.g. convert it to a QSrting to update an UI element)
            utility::string_t stringT = result.to_string();
            std::string stringStd(stringT.begin(), stringT.end());
            QString stringQ = QString::fromStdString(stringStd);
            //Note: if you want to update your UI, do this by using signals and slots. 
            //The UI can't be updated from a Thread different to the Qt main thread
            emit UpdateLabel(stringQ);
          }
          catch (const mitk::Exception &exception)
          {
            //Exceptions from ascendent tasks are catched here
            MITK_ERROR << exception.what();
            return;
          }
        });
    }
  }
  m_Ui->putPushButton->setEnabled(true);
}

void QmitkClientView::OnGetSaveButtonClicked() 
{
  m_Ui->putPushButton->setDisabled(true);
  us::ModuleContext *context = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  auto managerRef = context->GetServiceReference<mitk::IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService
        ->SendRequest(L"http://193.174.48.78:8090/dcm4chee-arc/aets/DCM4CHEE/rs/studies/1.2.840.113654.2.70.1.97144850941324808603541273584489321943/series/1.2.840.113654.2.70.1.15771179684190906938515254678965278540/instances", mitk::IRESTManager::RequestType::Get, NULL,L"FileStream.txt")
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
      m_Ui->responseLabel->setText("Waiting for change");
    }
  }
  m_Ui->putPushButton->setEnabled(true);
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
        L"https://jsonplaceholder.typicode.com/posts/1", mitk::IRESTManager::RequestType::Put, &data)
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
        ->SendRequest(L"https://jsonplaceholder.typicode.com/posts", mitk::IRESTManager::RequestType::Post, &data)
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
