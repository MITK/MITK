/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRestApiView.h"

#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>

#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>

const std::string QmitkRestApiView::VIEW_ID = "org.mitk.views.restapi";

QmitkRestApiView::QmitkRestApiView()
  : m_StatusTimer(nullptr)
{
}

QmitkRestApiView::~QmitkRestApiView()
{
  if (m_StatusTimer != nullptr)
  {
    m_StatusTimer->stop();
    delete m_StatusTimer;
  }
}

void QmitkRestApiView::SetFocus()
{
  m_Controls.m_StartStopButton->setFocus();
}

void QmitkRestApiView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  // Setup node inspectors
  this->SetupNodeInspectors();

  // Connect signals
  connect(m_Controls.m_StartStopButton, &QPushButton::clicked, this, &QmitkRestApiView::OnStartStopClicked);
  connect(m_Controls.m_RefreshButton, &QPushButton::clicked, this, &QmitkRestApiView::OnRefreshStatus);
  connect(m_Controls.m_CopyUrlButton, &QPushButton::clicked, this, &QmitkRestApiView::OnCopyUrlClicked);
  connect(m_Controls.m_ClearLogButton, &QPushButton::clicked, this, &QmitkRestApiView::OnClearLogClicked);

  // Setup request log table
  m_Controls.m_RequestLogTable->horizontalHeader()->setStretchLastSection(true);
  m_Controls.m_RequestLogTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_Controls.m_RequestLogTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_Controls.m_RequestLogTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

  // Setup status timer to periodically update status
  m_StatusTimer = new QTimer(this);
  connect(m_StatusTimer, &QTimer::timeout, this, &QmitkRestApiView::OnRefreshStatus);
  m_StatusTimer->start(2000); // Update every 2 seconds

  // Initial status update
  this->UpdateServerStatus();
}

void QmitkRestApiView::SetupNodeInspectors()
{
  auto dataStorage = this->GetDataStorage();

  // Setup inspector for queried nodes (have restapi.uid property)
  m_Controls.m_QueriedNodesInspector->SetDataStorage(dataStorage);

  // Create predicate for nodes with restapi.uid property
  auto queriedPredicate = mitk::NodePredicateProperty::New("restapi.uid");
  m_Controls.m_QueriedNodesInspector->SetNodePredicate(queriedPredicate);

  // Setup inspector for modified nodes (have restapi.modified property)
  m_Controls.m_ModifiedNodesInspector->SetDataStorage(dataStorage);

  // Create predicate for nodes with restapi.modified property set to true
  auto modifiedPredicate = mitk::NodePredicateProperty::New("restapi.modified");
  m_Controls.m_ModifiedNodesInspector->SetNodePredicate(modifiedPredicate);
}

mitk::IRestServerService* QmitkRestApiView::GetRestServerService() const
{
  auto restModule = us::ModuleRegistry::GetModule("MitkRESTAPI");

  if (restModule == nullptr)
  {
    return nullptr;
  }

  auto* context = restModule->GetModuleContext();
  if (context == nullptr)
  {
    return nullptr;
  }

  auto refs = context->GetServiceReferences<mitk::IRestServerService>();
  if (refs.empty())
  {
    return nullptr;
  }

  return context->GetService(refs.front());
}

void QmitkRestApiView::OnStartStopClicked()
{
  auto* service = this->GetRestServerService();
  if (service == nullptr)
  {
    m_Controls.m_StatusLabel->setText("REST API service not available");
    return;
  }

  if (service->IsRunning())
  {
    service->Stop();
  }
  else
  {
    if (!service->Start())
    {
      auto error = service->GetLastError();
      m_Controls.m_StatusLabel->setText(QString("Failed to start: %1")
        .arg(error ? QString::fromStdString(*error) : "Unknown error"));
      return;
    }
  }

  this->UpdateServerStatus();
}

void QmitkRestApiView::OnRefreshStatus()
{
  this->UpdateServerStatus();
}

void QmitkRestApiView::UpdateServerStatus()
{
  auto* service = this->GetRestServerService();

  if (service == nullptr)
  {
    m_Controls.m_StatusIndicator->setStyleSheet("background-color: gray; border-radius: 8px;");
    m_Controls.m_StatusLabel->setText("REST API service not available");
    m_Controls.m_StartStopButton->setEnabled(false);
    m_Controls.m_ServerUrlLabel->setText("-");
    m_Controls.m_HostPortLabel->setText("-");
    m_Controls.m_UptimeLabel->setText("-");
    m_Controls.m_ClientIPsLabel->setText("-");
    m_Controls.m_LastEndpointLabel->setText("-");
    m_Controls.m_LastResponseCodeLabel->setText("-");
    return;
  }

  m_Controls.m_StartStopButton->setEnabled(true);

  if (service->IsRunning())
  {
    // Green indicator for running
    m_Controls.m_StatusIndicator->setStyleSheet("background-color: #4CAF50; border-radius: 8px;");
    m_Controls.m_StatusLabel->setText("Running");
    m_Controls.m_StartStopButton->setText("Stop Server");

    auto url = service->GetServerUrl();
    if (url)
    {
      m_Controls.m_ServerUrlLabel->setText(QString::fromStdString(*url));
    }
    else
    {
      m_Controls.m_ServerUrlLabel->setText("-");
    }
  }
  else
  {
    // Red indicator for stopped
    m_Controls.m_StatusIndicator->setStyleSheet("background-color: #F44336; border-radius: 8px;");
    m_Controls.m_StatusLabel->setText("Stopped");
    m_Controls.m_StartStopButton->setText("Start Server");
    m_Controls.m_ServerUrlLabel->setText("-");

    // Check for error
    auto error = service->GetLastError();
    if (error)
    {
      m_Controls.m_StatusLabel->setText(QString("Stopped (Error: %1)").arg(QString::fromStdString(*error)));
    }
  }

  // Update configuration display
  auto config = service->GetRunningConfig();
  if (!config)
  {
    config = service->GetPendingConfig();
  }

  m_Controls.m_HostPortLabel->setText(QString("%1:%2")
    .arg(QString::fromStdString(config->host))
    .arg(config->port));

  // Update uptime display
  auto uptime = service->GetUptimeSeconds();
  if (uptime.has_value())
  {
    int64_t seconds = uptime.value();
    int64_t hours = seconds / 3600;
    int64_t minutes = (seconds % 3600) / 60;
    int64_t secs = seconds % 60;

    if (hours > 0)
    {
      m_Controls.m_UptimeLabel->setText(QString("%1h %2m %3s")
        .arg(hours).arg(minutes).arg(secs));
    }
    else if (minutes > 0)
    {
      m_Controls.m_UptimeLabel->setText(QString("%1m %2s")
        .arg(minutes).arg(secs));
    }
    else
    {
      m_Controls.m_UptimeLabel->setText(QString("%1s").arg(secs));
    }
  }
  else
  {
    m_Controls.m_UptimeLabel->setText("-");
  }

  // Update client IPs list
  auto clientIPs = service->GetClientIPs();
  if (clientIPs.empty())
  {
    m_Controls.m_ClientIPsLabel->setText("No clients connected yet");
  }
  else
  {
    QStringList ipList;
    for (const auto& ip : clientIPs)
    {
      ipList.append(QString::fromStdString(ip));
    }
    m_Controls.m_ClientIPsLabel->setText(ipList.join(", "));
  }

  // Update last request info
  auto lastRequest = service->GetLastRequestInfo();
  if (lastRequest)
  {
    m_Controls.m_LastEndpointLabel->setText(QString("%1 %2")
      .arg(QString::fromStdString(lastRequest->method))
      .arg(QString::fromStdString(lastRequest->endpoint)));
    m_Controls.m_LastResponseCodeLabel->setText(QString::number(lastRequest->responseCode));
  }
  else
  {
    m_Controls.m_LastEndpointLabel->setText("-");
    m_Controls.m_LastResponseCodeLabel->setText("-");
  }

  // Update request log table
  this->UpdateRequestLogTable();
}

void QmitkRestApiView::OnCopyUrlClicked()
{
  auto urlText = m_Controls.m_ServerUrlLabel->text();
  if (!urlText.isEmpty() && urlText != "-")
  {
    QApplication::clipboard()->setText(urlText);
  }
}

void QmitkRestApiView::OnClearLogClicked()
{
  auto* service = this->GetRestServerService();
  if (service != nullptr)
  {
    service->ClearRequestLog();
    this->UpdateRequestLogTable();
  }
}

void QmitkRestApiView::UpdateRequestLogTable()
{
  auto* service = this->GetRestServerService();
  if (service == nullptr)
  {
    m_Controls.m_RequestLogTable->setRowCount(0);
    return;
  }

  auto requestLog = service->GetRequestLog();

  // Only update if the row count changed to avoid flickering
  int currentRowCount = m_Controls.m_RequestLogTable->rowCount();
  int newRowCount = static_cast<int>(requestLog.size());

  if (currentRowCount != newRowCount)
  {
    m_Controls.m_RequestLogTable->setRowCount(newRowCount);
  }

  // Populate table (oldest first, most recent at bottom)
  for (int i = 0; i < newRowCount; ++i)
  {
    const auto& info = requestLog[i];

    auto* ipItem = new QTableWidgetItem(QString::fromStdString(info.clientIP));
    auto* responseItem = new QTableWidgetItem(QString::number(info.responseCode));
    auto* methodItem = new QTableWidgetItem(QString::fromStdString(info.method));
    auto* endpointItem = new QTableWidgetItem(QString::fromStdString(info.endpoint));

    // Color code response codes
    if (info.responseCode >= 200 && info.responseCode < 300)
    {
      responseItem->setData(Qt::BackgroundRole, QColor("#4CAF50")); // Green for success
    }
    else if (info.responseCode >= 400 && info.responseCode < 500)
    {
      responseItem->setData(Qt::BackgroundRole, QColor("#FF9800")); // Orange for client errors
    }
    else if (info.responseCode >= 500)
    {
      responseItem->setData(Qt::BackgroundRole, QColor("#F44336")); // Red for server errors
    }

    m_Controls.m_RequestLogTable->setItem(i, 0, ipItem);
    m_Controls.m_RequestLogTable->setItem(i, 1, responseItem);
    m_Controls.m_RequestLogTable->setItem(i, 2, methodItem);
    m_Controls.m_RequestLogTable->setItem(i, 3, endpointItem);
  }

  // Scroll to bottom to show most recent entries
  if (newRowCount > 0)
  {
    m_Controls.m_RequestLogTable->scrollToBottom();
  }
}
