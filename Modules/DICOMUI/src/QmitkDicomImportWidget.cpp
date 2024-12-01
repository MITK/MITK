/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomImportWidget.h"
#include <ui_QmitkDicomImportWidget.h>

#include <mitkLog.h>

#include <ctkDICOMIndexer.h>
#include <ctkFileDialog.h>

#include <QMessageBox>
#include <QProgressDialog>

namespace
{
  bool OpenDatabase(ctkDICOMDatabase* database)
  {
    QTemporaryFile tempDatabaseFile;

    if (!tempDatabaseFile.open())
      return false;

    tempDatabaseFile.setAutoRemove(false);

    return database->openDatabase(tempDatabaseFile.fileName());
  }
}

QmitkDicomImportWidget::QmitkDicomImportWidget(QWidget *parent)
  : QWidget(parent),
    m_ImportDatabase(new ctkDICOMDatabase(this)),
    m_ImportIndexer(new ctkDICOMIndexer(this)),
    m_ImportDialog(new ctkFileDialog(this)),
    m_ProgressDialog(nullptr),
    m_Ui(new Ui::QmitkDicomImportWidget)
{
  using Self = QmitkDicomImportWidget;

  m_Ui->setupUi(this);
  m_Ui->tableManager->setTableOrientation(Qt::Vertical);

  if (!OpenDatabase(m_ImportDatabase))
  {
    const auto message = "Database error: " + m_ImportDatabase->lastError();
    MITK_ERROR << message.toStdString();
    QMessageBox::critical(nullptr, "DICOM Import", message);
    this->setEnabled(false);
    return;
  }

  m_Ui->tableManager->setDICOMDatabase(m_ImportDatabase);

  m_ImportDialog->setFileMode(QFileDialog::Directory);
  m_ImportDialog->setLabelText(QFileDialog::Accept, "Import");
  m_ImportDialog->setWindowTitle("Import DICOM files from directory");
  m_ImportDialog->setWindowModality(Qt::WindowModal);

  connect(m_Ui->addToLocalStorageButton, &QPushButton::clicked,
    this, &Self::OnAddToLocalStorageButtonClicked);

  connect(m_Ui->viewButton, &QPushButton::clicked,
    this, &Self::OnViewButtonClicked);

  connect(m_Ui->scanDirectoryButton, &QPushButton::clicked,
    m_ImportDialog, &ctkFileDialog::show);

  connect(m_Ui->tableManager, qOverload<const QStringList&>(&ctkDICOMTableManager::seriesSelectionChanged),
    this, &Self::OnSeriesSelectionChanged);

  connect(m_Ui->tableManager, &ctkDICOMTableManager::seriesDoubleClicked,
    this, &Self::OnViewButtonClicked);

  connect(m_ImportDialog, &ctkFileDialog::fileSelected,
    this, &Self::OnImport);

  connect(m_ImportIndexer, &ctkDICOMIndexer::progressStep,
    this, &Self::OnProgressStep);

  connect(m_ImportIndexer, &ctkDICOMIndexer::progressDetail,
    this, &Self::OnProgressDetail);

  connect(m_ImportIndexer, &ctkDICOMIndexer::progress,
    this, &Self::OnProgress);

  // The progress dialog closes if the maximum value is reached, but the following
  // line is necessary as the indexer does not reach the maximum value.
  connect(m_ImportIndexer, &ctkDICOMIndexer::indexingComplete,
    this, &Self::OnIndexingComplete);
}

QmitkDicomImportWidget::~QmitkDicomImportWidget()
{
}

void QmitkDicomImportWidget::OnProgressStep(const QString& step)
{
  if (m_ProgressDialog == nullptr)
    this->SetupProgressDialog();

  m_ProgressStep = step;
  m_ProgressDialog->setLabelText(step);
}

void QmitkDicomImportWidget::OnProgressDetail(const QString& detail)
{
  if (m_ProgressDialog == nullptr)
    this->SetupProgressDialog();

  m_ProgressDialog->setLabelText(m_ProgressStep + '\n' + detail);
}

void QmitkDicomImportWidget::OnProgress(int value)
{
  if (m_ProgressDialog == nullptr)
    this->SetupProgressDialog();

  m_ProgressDialog->setValue(value);
}

void QmitkDicomImportWidget::OnIndexingComplete(int, int, int, int)
{
  if (m_ProgressDialog != nullptr)
  {
    m_ProgressDialog->close();
    m_ProgressDialog = nullptr;
  }
}

void QmitkDicomImportWidget::OnAddToLocalStorageButtonClicked()
{
  const auto files = this->GetFileNamesFromSelection();

  if (!files.empty())
    emit Import(files);
}

void QmitkDicomImportWidget::OnViewButtonClicked()
{
  std::vector<std::pair<std::string, std::optional<std::string>>> series;

  for (const auto& seriesUID : m_Ui->tableManager->currentSeriesSelection())
  {
    const auto filesForSeries = m_ImportDatabase->filesForSeries(seriesUID);

    if (filesForSeries.empty())
      continue;

    std::pair<std::string, std::optional<std::string>> seriesElem;
    seriesElem.first = filesForSeries[0].toStdString();

    const auto modality = m_ImportDatabase->fileValue(filesForSeries[0], "0008,0060").toUpper();

    if (!modality.isEmpty())
      seriesElem.second = modality.toStdString();

    series.push_back(seriesElem);
  }

  if (!series.empty())
    emit ViewSeries(series);
}

QStringList QmitkDicomImportWidget::GetFileNamesFromSelection()
{
  auto seriesUIDs = m_Ui->tableManager->currentSeriesSelection();
  QStringList filePaths;

  for (const auto& seriesUID : seriesUIDs)
    filePaths.append(m_ImportDatabase->filesForSeries(seriesUID));

  if (!filePaths.empty())
    return filePaths;

  auto studyUIDs = m_Ui->tableManager->currentStudiesSelection();

  for (const auto& studyUID : studyUIDs)
  {
    seriesUIDs = m_ImportDatabase->seriesForStudy(studyUID);

    for (const auto& seriesUID : seriesUIDs)
      filePaths.append(m_ImportDatabase->filesForSeries(seriesUID));
  }

  if (!filePaths.empty())
    return filePaths;

  QStringList patientsUIDs = m_Ui->tableManager->currentPatientsSelection();

  for (const auto& patientUID : patientsUIDs)
  {
    studyUIDs = m_ImportDatabase->studiesForPatient(patientUID);

    for (const auto& studyUID : studyUIDs)
    {
      seriesUIDs = m_ImportDatabase->seriesForStudy(studyUID);

      for (const auto& seriesUID : seriesUIDs)
        filePaths.append(m_ImportDatabase->filesForSeries(seriesUID));
    }
  }

  return filePaths;
}

void QmitkDicomImportWidget::OnImport(const QString &directory)
{
  m_ImportDialog->close();
  m_ImportDialog->setDirectory(directory);

  m_ImportIndexer->addDirectory(m_ImportDatabase, directory);
}

void QmitkDicomImportWidget::OnSeriesSelectionChanged(const QStringList &selection)
{
  m_Ui->viewButton->setEnabled(!selection.empty());
}

void QmitkDicomImportWidget::SetupProgressDialog()
{
  if (m_ProgressDialog != nullptr)
    return;

  m_ProgressDialog = new QProgressDialog("Initialization...", "Cancel", 0, 100, this);
  m_ProgressDialog->setAttribute(Qt::WA_DeleteOnClose);
  m_ProgressDialog->setWindowTitle("DICOM Import");
  m_ProgressDialog->setWindowModality(Qt::WindowModal);
  m_ProgressDialog->setMinimumDuration(0);

  connect(m_ProgressDialog, &QProgressDialog::canceled,
          m_ImportIndexer, &ctkDICOMIndexer::cancel);
}
