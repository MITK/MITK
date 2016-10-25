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

// Qmitk
#include "QmitkDicomExternalDataWidget.h"
#include <mitkLogMacros.h>

// CTK
#include <ctkFileDialog.h>

// Qt
#include <QCheckBox>
#include <QMessageBox>

const std::string QmitkDicomExternalDataWidget::Widget_ID = "org.mitk.Widgets.QmitkDicomExternalDataWidget";

QmitkDicomExternalDataWidget::QmitkDicomExternalDataWidget(QWidget *parent)
  : QWidget(parent), m_Controls(nullptr), m_ProgressDialog(nullptr)
{
  Initialize();
  CreateQtPartControl(this);
}

QmitkDicomExternalDataWidget::~QmitkDicomExternalDataWidget()
{
}

void QmitkDicomExternalDataWidget::CreateQtPartControl(QWidget *parent)
{
  // build up qt Widget, unless already done
  if (!m_Controls)
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDicomExternalDataWidgetControls;
    m_Controls->setupUi(parent);
    m_Controls->viewExternalDataButton->setVisible(true);
    m_Controls->ctkDICOMBrowser->setTableOrientation(Qt::Vertical);
    m_Controls->ctkDICOMBrowser->setDICOMDatabase(m_ExternalDatabase);

    SetupImportDialog();
    SetupProgressDialog();

    // connect buttons
    connect(m_Controls->downloadButton, SIGNAL(clicked()), this, SLOT(OnDownloadButtonClicked()));
    connect(m_Controls->viewExternalDataButton, SIGNAL(clicked()), this, SLOT(OnViewButtonClicked()));
    connect(m_Controls->directoryButton, SIGNAL(clicked()), m_ImportDialog, SLOT(show()));

    connect(m_Controls->ctkDICOMBrowser,
            SIGNAL(seriesSelectionChanged(const QStringList &)),
            this,
            SLOT(OnSeriesSelectionChanged(const QStringList &)));
    connect(
      m_Controls->ctkDICOMBrowser, SIGNAL(seriesDoubleClicked(const QModelIndex &)), this, SLOT(OnViewButtonClicked()));

    connect(m_ImportDialog, SIGNAL(fileSelected(QString)), this, SLOT(OnStartDicomImport(QString)));

    connect(m_ExternalIndexer,
            SIGNAL(indexingFilePath(const QString &)),
            m_ProgressDialog,
            SLOT(setLabelText(const QString &)));
    connect(m_ExternalIndexer, SIGNAL(progress(int)), m_ProgressDialog, SLOT(setValue(int)));
    // actually the progress dialog closes if the maximum value is reached, BUT
    // the following line is needed since the external indexer wont reach maximum value (100 % progress)
    connect(m_ExternalIndexer, SIGNAL(indexingComplete()), m_ProgressDialog, SLOT(close()));
    connect(m_ProgressDialog, SIGNAL(canceled()), m_ExternalIndexer, SLOT(cancel()));
  }
}

void QmitkDicomExternalDataWidget::Initialize()
{
  m_ExternalDatabase = new ctkDICOMDatabase(this);

  try
  {
    m_ExternalDatabase->openDatabase(QString(":memory:"), QString("EXTERNAL-DB"));
  }
  catch (std::exception e)
  {
    MITK_ERROR << "Database error: " << m_ExternalDatabase->lastError().toStdString();
    m_ExternalDatabase->closeDatabase();
    return;
  }

  m_ExternalIndexer = new ctkDICOMIndexer(this);
}

void QmitkDicomExternalDataWidget::OnDownloadButtonClicked()
{
  QStringList filesToDownload = GetFileNamesFromIndex();
  if (filesToDownload.size() == 0)
  {
    QMessageBox info;
    info.setText("You have to select an entry in the DICOM browser for import.");
    info.exec();
    return;
  }
  emit SignalStartDicomImport(GetFileNamesFromIndex());
}

void QmitkDicomExternalDataWidget::OnViewButtonClicked()
{
  QStringList uids = m_Controls->ctkDICOMBrowser->currentSeriesSelection();
  QString uid;
  foreach (uid, uids)
  {
    QStringList filesForSeries = m_ExternalDatabase->filesForSeries(uid);
    QHash<QString, QVariant> eventProperty;
    eventProperty.insert("FilesForSeries", filesForSeries);
    if (!filesForSeries.isEmpty())
    {
      QString modality = m_ExternalDatabase->fileValue(filesForSeries.at(0), "0008,0060");
      eventProperty.insert("Modality", modality);
    }
    emit SignalDicomToDataManager(eventProperty);
  }
}

QStringList QmitkDicomExternalDataWidget::GetFileNamesFromIndex()
{
  QStringList filePaths;

  QString uid;
  QStringList seriesUIDs = m_Controls->ctkDICOMBrowser->currentSeriesSelection();
  foreach (uid, seriesUIDs)
  {
    filePaths.append(m_ExternalDatabase->filesForSeries(uid));
  }
  if (!filePaths.empty())
    return filePaths;

  QStringList studyUIDs = m_Controls->ctkDICOMBrowser->currentStudiesSelection();

  foreach (uid, studyUIDs)
  {
    seriesUIDs = m_ExternalDatabase->seriesForStudy(uid);
    foreach (uid, seriesUIDs)
    {
      filePaths.append(m_ExternalDatabase->filesForSeries(uid));
    }
  }
  if (!filePaths.empty())
    return filePaths;

  QStringList patientsUIDs = m_Controls->ctkDICOMBrowser->currentPatientsSelection();

  foreach (uid, patientsUIDs)
  {
    studyUIDs = m_ExternalDatabase->studiesForPatient(uid);

    foreach (uid, studyUIDs)
    {
      seriesUIDs = m_ExternalDatabase->seriesForStudy(uid);
      foreach (uid, seriesUIDs)
      {
        filePaths.append(m_ExternalDatabase->filesForSeries(uid));
      }
    }
  }
  return filePaths;
}

void QmitkDicomExternalDataWidget::OnStartDicomImport(const QString &directory)
{
  m_ImportDialog->close();
  // no need to show / start the progress dialog, as the dialog
  // appears by receiving the progress signal from the external indexer

  m_LastImportDirectory = directory;
  m_ExternalIndexer->addDirectory(*m_ExternalDatabase, m_LastImportDirectory);
}

void QmitkDicomExternalDataWidget::OnSeriesSelectionChanged(const QStringList &s)
{
  m_Controls->viewExternalDataButton->setEnabled((s.size() != 0));
}

void QmitkDicomExternalDataWidget::SetupImportDialog()
{
  // Initialize import widget
  m_ImportDialog = new ctkFileDialog(this);
  // Since copy on import is not working at the moment
  // this feature is disabled
  // QCheckBox* importCheckbox = new QCheckBox("Copy on import", m_ImportDialog);
  // m_ImportDialog->setBottomWidget(importCheckbox);
  m_ImportDialog->setFileMode(QFileDialog::Directory);
  m_ImportDialog->setLabelText(QFileDialog::Accept, "Import");
  m_ImportDialog->setWindowTitle("Import DICOM files from directory");
  m_ImportDialog->setWindowModality(Qt::ApplicationModal);
}

void QmitkDicomExternalDataWidget::SetupProgressDialog()
{
  m_ProgressDialog = new QProgressDialog("Initialization ...", "Cancel", 0, 100, this);
  m_ProgressDialog->setWindowTitle("DICOM Import");
  m_ProgressDialog->setWindowModality(Qt::ApplicationModal);
  m_ProgressDialog->setMinimumDuration(0);
  // FIX T20008: immediately set the progress dialog value to maximum --> will close the dialog
  m_ProgressDialog->setValue(100);
}
