/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomBrowser.h"
#include <ui_QmitkDicomBrowser.h>

#include "QmitkDicomEventHandler.h"
#include "QmitkDicomDataEventPublisher.h"

#include "mitkPluginActivator.h"

using Self = QmitkDicomBrowser;

const std::string QmitkDicomBrowser::EDITOR_ID = "org.mitk.editors.dicombrowser";

QmitkDicomBrowser::QmitkDicomBrowser()
  : m_Ui(new Ui::QmitkDicomBrowser),
    m_Handler(std::make_unique<QmitkDicomEventHandler>()),
    m_Publisher(std::make_unique<QmitkDicomDataEventPublisher>())
{
}

QmitkDicomBrowser::~QmitkDicomBrowser()
{
}

void QmitkDicomBrowser::CreateQtPartControl(QWidget *parent)
{
  m_Ui->setupUi(parent);

  auto databaseDir = mitk::PluginActivator::GetContext()->getDataFile("").absolutePath().append("/database");
  auto database = m_Ui->internalDataWidget->SetDatabaseDirectory(databaseDir);
  m_Ui->queryRetrieveWidget->setRetrieveDatabase(database);

  m_Ui->queryRetrieveWidget->useProgressDialog(true);

  connect(m_Ui->externalDataWidget, &QmitkDicomExternalDataWidget::SignalStartDicomImport,
          m_Ui->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport));

  connect(m_Ui->externalDataWidget, &QmitkDicomExternalDataWidget::SignalDicomToDataManager,
          this, &Self::OnViewButtonAddToDataManager);

  connect(m_Ui->internalDataWidget, &QmitkDicomLocalStorageWidget::SignalFinishedImport,
          this, &Self::OnDicomImportFinished);

  connect(m_Ui->internalDataWidget, &QmitkDicomLocalStorageWidget::SignalDicomToDataManager,
          this, &Self::OnViewButtonAddToDataManager);
}

void QmitkDicomBrowser::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  this->SetSite(site);
  this->SetInput(input);
}

void QmitkDicomBrowser::SetFocus()
{
}

berry::IPartListener::Events::Types QmitkDicomBrowser::GetPartEventTypes() const
{
  return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkDicomBrowser::OnDicomImportFinished()
{
  m_Ui->tabWidget->setCurrentIndex(0);
}

void QmitkDicomBrowser::OnViewButtonAddToDataManager(const QHash<QString, QVariant>& eventProperties)
{
  ctkDictionary properties;
  properties["FilesForSeries"] = eventProperties["FilesForSeries"];

  if (eventProperties.contains("Modality"))
    properties["Modality"] = eventProperties["Modality"];

  m_Publisher->PublishSignals(mitk::PluginActivator::GetContext());
  m_Publisher->AddSeriesToDataManagerEvent(properties);
}
