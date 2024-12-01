/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDicomImportWidget.h>
#include <ui_QmitkDicomImportWidget.h>

#include <mitkLog.h>

#include <ctkDICOMIndexer.h>
#include <ctkFileDialog.h>

#include <QMessageBox>
#include <QProgressDialog>
#include <QTemporaryFile>

using Self = QmitkDicomImportWidget;

QmitkDicomImportWidget::QmitkDicomImportWidget(QWidget *parent)
  : QWidget(parent),
    m_DatabaseFile(nullptr),
    m_Database(nullptr),
    m_Indexer(nullptr),
    m_ImportDialog(new ctkFileDialog(this)),
    m_ProgressDialog(nullptr),
    m_Ui(new Ui::QmitkDicomImportWidget)
{
  m_Ui->setupUi(this);
  m_Ui->tableManager->setTableOrientation(Qt::Vertical);

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
}

QmitkDicomImportWidget::~QmitkDicomImportWidget()
{
  this->CloseDatabase();
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
    const auto filesForSeries = m_Database->filesForSeries(seriesUID);

    if (filesForSeries.empty())
      continue;

    std::pair<std::string, std::optional<std::string>> seriesElem;
    seriesElem.first = filesForSeries[0].toStdString();

    const auto modality = m_Database->fileValue(filesForSeries[0], "0008,0060").toUpper();

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
    filePaths.append(m_Database->filesForSeries(seriesUID));

  if (!filePaths.empty())
    return filePaths;

  auto studyUIDs = m_Ui->tableManager->currentStudiesSelection();

  for (const auto& studyUID : studyUIDs)
  {
    seriesUIDs = m_Database->seriesForStudy(studyUID);

    for (const auto& seriesUID : seriesUIDs)
      filePaths.append(m_Database->filesForSeries(seriesUID));
  }

  if (!filePaths.empty())
    return filePaths;

  QStringList patientsUIDs = m_Ui->tableManager->currentPatientsSelection();

  for (const auto& patientUID : patientsUIDs)
  {
    studyUIDs = m_Database->studiesForPatient(patientUID);

    for (const auto& studyUID : studyUIDs)
    {
      seriesUIDs = m_Database->seriesForStudy(studyUID);

      for (const auto& seriesUID : seriesUIDs)
        filePaths.append(m_Database->filesForSeries(seriesUID));
    }
  }

  return filePaths;
}

bool QmitkDicomImportWidget::OpenDatabase()
{
  this->CloseDatabase();

  m_DatabaseFile = new QTemporaryFile(QDir::tempPath() + "/mitk_dicom_import_XXXXXX.sqlite", this);

  if (!m_DatabaseFile->open())
    return false;

  m_Database = new ctkDICOMDatabase(this);

  if (!m_Database->openDatabase(m_DatabaseFile->fileName()))
  {
    const auto message = "Database error: " + m_Database->lastError();
    MITK_ERROR << message.toStdString();
    QMessageBox::critical(nullptr, "DICOM Import", message);
    return false;
  }

  m_Ui->tableManager->setDICOMDatabase(m_Database);

  m_Indexer = new ctkDICOMIndexer(this);
  m_Indexer->setDatabase(m_Database);

  connect(m_Indexer, &ctkDICOMIndexer::progressStep,
    this, &Self::OnProgressStep);

  connect(m_Indexer, &ctkDICOMIndexer::progressDetail,
    this, &Self::OnProgressDetail);

  connect(m_Indexer, &ctkDICOMIndexer::progress,
    this, &Self::OnProgress);

  connect(m_Indexer, &ctkDICOMIndexer::indexingComplete,
    this, &Self::OnIndexingComplete);

  return true;
}

void QmitkDicomImportWidget::CloseDatabase()
{
  if (m_Indexer != nullptr)
  {
    m_Indexer->setDatabase(nullptr);
    m_Indexer->deleteLater();
    m_Indexer = nullptr;
  }

  m_Ui->tableManager->setDICOMDatabase(nullptr);

  if (m_Database != nullptr)
  {
    if (m_Database->isOpen())
      m_Database->closeDatabase();

    m_Database->deleteLater();
    m_Database = nullptr;
  }

  if (m_DatabaseFile != nullptr && m_DatabaseFile->isOpen())
  {
    QFile tagCacheFile(QFileInfo(m_DatabaseFile->fileName()).absolutePath() + "/ctkDICOMTagCache.sql");

    m_DatabaseFile->close();
    m_DatabaseFile->deleteLater();
    m_DatabaseFile = nullptr;

    if (tagCacheFile.exists())
      tagCacheFile.remove();
  }
}

void QmitkDicomImportWidget::OnImport(const QString &directory)
{
  m_ImportDialog->close();
  m_ImportDialog->setDirectory(directory);

  if (this->OpenDatabase())
    m_Indexer->addDirectory(m_Database, directory);
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
          m_Indexer, &ctkDICOMIndexer::cancel);
}
