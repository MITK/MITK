/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomLocalStorageWidget.h"
#include <ui_QmitkDicomLocalStorageWidgetControls.h>

#include <QMessageBox>

#include <ctkDICOMIndexer.h>

using Self = QmitkDicomLocalStorageWidget;

QmitkDicomLocalStorageWidget::QmitkDicomLocalStorageWidget(QWidget *parent)
  : QWidget(parent),
    m_LocalIndexer(std::make_unique<ctkDICOMIndexer>(parent)),
    m_Controls(new Ui::QmitkDicomLocalStorageWidgetControls)
{
  this->CreateQtPartControl(this);
}

QmitkDicomLocalStorageWidget::~QmitkDicomLocalStorageWidget()
{
}

void QmitkDicomLocalStorageWidget::CreateQtPartControl(QWidget *parent)
{
  m_Controls->setupUi(parent);

  connect(m_Controls->deleteButton, &QPushButton::clicked,
          this, &Self::OnDeleteButtonClicked);

  connect(m_Controls->viewButton, &QPushButton::clicked,
          this, &Self::OnViewButtonClicked);

  connect(m_Controls->tableManager, qOverload<const QStringList&>(&ctkDICOMTableManager::seriesSelectionChanged),
          this, &Self::OnSeriesSelectionChanged);

  connect(m_Controls->tableManager, &ctkDICOMTableManager::seriesDoubleClicked,
          this, &Self::OnViewButtonClicked);

  connect(m_LocalIndexer.get(), &ctkDICOMIndexer::indexingComplete,
          this, &Self::SignalFinishedImport);

  m_Controls->tableManager->setTableOrientation(Qt::Vertical);
}

void QmitkDicomLocalStorageWidget::OnStartDicomImport(const QString &dicomData)
{
  if (m_LocalDatabase->isOpen())
    m_LocalIndexer->addDirectory(dicomData);
}

void QmitkDicomLocalStorageWidget::OnStartDicomImport(const QStringList &dicomData)
{
  if (m_LocalDatabase->isOpen())
    m_LocalIndexer->addListOfFiles(dicomData);
}

void QmitkDicomLocalStorageWidget::OnDeleteButtonClicked()
{
  if (!this->DeletePatients() && !this->DeleteStudies())
    this->DeleteSeries();

  m_Controls->tableManager->updateTableViews();
}

bool QmitkDicomLocalStorageWidget::DeletePatients()
{
  const auto selectedPatientUIDs = m_Controls->tableManager->currentPatientsSelection();

  if (selectedPatientUIDs.empty())
    return false;

  QStringList studyUIDs;

  for (const auto &patientUID : selectedPatientUIDs)
    studyUIDs << m_LocalDatabase->studiesForPatient(patientUID);

  QStringList seriesUIDs;

  for (const auto &studyUID : studyUIDs)
    seriesUIDs << m_LocalDatabase->seriesForStudy(studyUID);

  auto answer = QMessageBox::question(nullptr,
    "Delete Patients",
    QString("Do you really want to delete %1 %2, containing %3 series in %4 %5?")
      .arg(selectedPatientUIDs.size())
      .arg(selectedPatientUIDs.size() != 1 ? "patients" : "patient")
      .arg(seriesUIDs.size())
      .arg(studyUIDs.size())
      .arg(studyUIDs.size() != 1 ? "studies" : "study"));

  if (answer == QMessageBox::Yes)
  {
    for (const auto &patientUID : selectedPatientUIDs)
      m_LocalDatabase->removePatient(patientUID);
  }

  return true;
}

bool QmitkDicomLocalStorageWidget::DeleteStudies()
{
  const auto selectedStudyUIDs = m_Controls->tableManager->currentStudiesSelection();

  if (selectedStudyUIDs.empty())
    return false;

  QStringList seriesUIDs;

  for (const auto &studyUID : selectedStudyUIDs)
    seriesUIDs << m_LocalDatabase->seriesForStudy(studyUID);

  auto answer = QMessageBox::question(nullptr,
    "Delete Studies",
    QString("Do you really want to delete %1 %2, containing %3 series?")
      .arg(selectedStudyUIDs.size())
      .arg(selectedStudyUIDs.size() != 1 ? "studies" : "study")
      .arg(seriesUIDs.size()));

  if (answer == QMessageBox::Yes)
  {
    for (const auto &studyUID : selectedStudyUIDs)
      m_LocalDatabase->removeStudy(studyUID);
  }

  return true;
}

bool QmitkDicomLocalStorageWidget::DeleteSeries()
{
  const auto selectedSeriesUIDs = m_Controls->tableManager->currentSeriesSelection();

  if (selectedSeriesUIDs.empty())
    return false;

  auto answer = QMessageBox::question(nullptr,
    "Delete Series",
    QString("Do you really want to delete %1 series?").arg(selectedSeriesUIDs.size()));

  if (answer == QMessageBox::Yes)
  {
    for (const auto &seriesUID : selectedSeriesUIDs)
      m_LocalDatabase->removeSeries(seriesUID);
  }

  return true;
}

void QmitkDicomLocalStorageWidget::OnViewButtonClicked()
{
  for (const auto& uid : m_Controls->tableManager->currentSeriesSelection())
  {
    const auto filesForSeries = m_LocalDatabase->filesForSeries(uid);

    QHash<QString, QVariant> eventProperty;
    eventProperty.insert("FilesForSeries", filesForSeries);

    if (!filesForSeries.isEmpty())
    {
      const auto modality = m_LocalDatabase->fileValue(filesForSeries.at(0), "0008,0060");
      eventProperty.insert("Modality", modality);
    }

    emit SignalDicomToDataManager(eventProperty);
  }
}

QSharedPointer<ctkDICOMDatabase> QmitkDicomLocalStorageWidget::SetDatabaseDirectory(const QString& newDatatbaseDirectory)
{
  QDir databaseDirecory(newDatatbaseDirectory);

  if (!databaseDirecory.exists())
    databaseDirecory.mkpath(databaseDirecory.absolutePath());

  const auto newDatabaseFile = databaseDirecory.absolutePath() + QString("/ctkDICOM.sql");
  this->SetDatabase(newDatabaseFile);

  return m_LocalDatabase;
}

void QmitkDicomLocalStorageWidget::SetDatabase(const QString& databaseFile)
{
  m_LocalDatabase = QSharedPointer<ctkDICOMDatabase>::create(databaseFile);

  m_Controls->tableManager->setDICOMDatabase(m_LocalDatabase.get());
  m_LocalIndexer->setDatabase(m_LocalDatabase.get());
}

void QmitkDicomLocalStorageWidget::OnSeriesSelectionChanged(const QStringList &s)
{
  m_Controls->viewButton->setEnabled((s.size() != 0));
}
