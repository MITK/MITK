/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomLocalStorageWidget.h"
#include <ui_QmitkDicomLocalStorageWidget.h>

#include <QMessageBox>

#include <ctkDICOMIndexer.h>

QmitkDicomLocalStorageWidget::QmitkDicomLocalStorageWidget(QWidget *parent)
  : QWidget(parent),
    m_LocalIndexer(std::make_unique<ctkDICOMIndexer>(parent)),
    m_Ui(new Ui::QmitkDicomLocalStorageWidget)
{
  using Self = QmitkDicomLocalStorageWidget;

  m_Ui->setupUi(this);

  connect(m_Ui->deleteButton, &QPushButton::clicked,
    this, &Self::OnDeleteButtonClicked);

  connect(m_Ui->viewButton, &QPushButton::clicked,
    this, &Self::OnViewButtonClicked);

  connect(m_Ui->tableManager, &ctkDICOMTableManager::seriesDoubleClicked,
    this, &Self::OnViewButtonClicked);

  connect(m_Ui->tableManager, qOverload<const QStringList&>(&ctkDICOMTableManager::seriesSelectionChanged),
    this, &Self::OnSeriesSelectionChanged);

  connect(m_LocalIndexer.get(), &ctkDICOMIndexer::indexingComplete,
    this, &Self::IndexingComplete);

  m_Ui->tableManager->setTableOrientation(Qt::Vertical);
}

QmitkDicomLocalStorageWidget::~QmitkDicomLocalStorageWidget()
{
}

void QmitkDicomLocalStorageWidget::OnImport(const QStringList& files)
{
  if (m_LocalDatabase->isOpen())
    m_LocalIndexer->addListOfFiles(files);
}

void QmitkDicomLocalStorageWidget::OnDeleteButtonClicked()
{
  if (!this->DeletePatients() && !this->DeleteStudies())
    this->DeleteSeries();

  m_Ui->tableManager->updateTableViews();
}

bool QmitkDicomLocalStorageWidget::DeletePatients()
{
  const auto selectedPatientUIDs = m_Ui->tableManager->currentPatientsSelection();

  if (selectedPatientUIDs.empty())
    return false;

  QStringList studyUIDs;

  for (const auto &patientUID : selectedPatientUIDs)
    studyUIDs << m_LocalDatabase->studiesForPatient(patientUID);

  QStringList seriesUIDs;

  for (const auto &studyUID : studyUIDs)
    seriesUIDs << m_LocalDatabase->seriesForStudy(studyUID);

  auto answer = QMessageBox::question(nullptr,
    "Delete patients",
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
  const auto selectedStudyUIDs = m_Ui->tableManager->currentStudiesSelection();

  if (selectedStudyUIDs.empty())
    return false;

  QStringList seriesUIDs;

  for (const auto &studyUID : selectedStudyUIDs)
    seriesUIDs << m_LocalDatabase->seriesForStudy(studyUID);

  auto answer = QMessageBox::question(nullptr,
    "Delete studies",
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
  const auto selectedSeriesUIDs = m_Ui->tableManager->currentSeriesSelection();

  if (selectedSeriesUIDs.empty())
    return false;

  auto answer = QMessageBox::question(nullptr,
    "Delete series",
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
  std::vector<std::pair<std::string, std::optional<std::string>>> series;

  for (const auto& seriesUID : m_Ui->tableManager->currentSeriesSelection())
  {
    const auto filesForSeries = m_LocalDatabase->filesForSeries(seriesUID);

    if (filesForSeries.empty())
      continue;

    std::pair<std::string, std::optional<std::string>> seriesElem;
    seriesElem.first = filesForSeries[0].toStdString();

    const auto modality = m_LocalDatabase->fileValue(filesForSeries[0], "0008,0060").toUpper();

    if (!modality.isEmpty())
      seriesElem.second = modality.toStdString();

    series.push_back(seriesElem);
  }

  if (!series.empty())
    emit ViewSeries(series);
}

void QmitkDicomLocalStorageWidget::OnSeriesSelectionChanged(const QStringList& selection)
{
  m_Ui->viewButton->setEnabled(!selection.empty());
}

QSharedPointer<ctkDICOMDatabase> QmitkDicomLocalStorageWidget::SetDatabaseDirectory(const QString& newDatatbaseDirectory)
{
  QDir databaseDirecory(newDatatbaseDirectory);

  if (!databaseDirecory.exists())
    databaseDirecory.mkpath(databaseDirecory.absolutePath());

  const auto databaseFile = databaseDirecory.absolutePath() + QString("/ctkDICOM.sql");
  m_LocalDatabase = QSharedPointer<ctkDICOMDatabase>::create(databaseFile);
  m_Ui->tableManager->setDICOMDatabase(m_LocalDatabase.get());
  m_LocalIndexer->setDatabase(m_LocalDatabase.get());

  return m_LocalDatabase;
}
