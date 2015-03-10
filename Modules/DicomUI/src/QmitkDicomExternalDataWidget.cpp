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
  : QWidget(parent)
  , m_Controls (0)
{
    Initialize();
    CreateQtPartControl(this);
}

QmitkDicomExternalDataWidget::~QmitkDicomExternalDataWidget()
{
}


void QmitkDicomExternalDataWidget::CreateQtPartControl( QWidget *parent )
{

    // build up qt Widget, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkDicomExternalDataWidgetControls;
        m_Controls->setupUi( parent );
        m_Controls->viewExternalDataButton->setVisible(true);
        m_Controls->ctkDICOMBrowser->setTableOrientation(Qt::Vertical);
        m_Controls->ctkDICOMBrowser->setDICOMDatabase(m_ExternalDatabase);

        this->SetupImportDialog();
        this->SetupProgressDialog(parent);

        //connect Buttons
        connect(m_Controls->downloadButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));
        connect(m_Controls->viewExternalDataButton, SIGNAL(clicked()),this,SLOT(OnViewButtonClicked()));
        connect(m_Controls->directoryButton, SIGNAL(clicked()), this, SLOT(OnScanDirectory()));

        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesSelectionChanged(const QStringList&)),
                this, SLOT(OnSeriesSelectionChanged(const QStringList&)));
        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesDoubleClicked(const QModelIndex&)),
                this, SLOT(OnViewButtonClicked()));

        connect(m_ProgressDialog, SIGNAL(canceled()), m_ExternalIndexer, SLOT(cancel()));
        connect(m_ExternalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));

        connect(m_ExternalIndexer, SIGNAL(indexingFilePath(const QString&)), m_ProgressDialogLabel, SLOT(setText(const QString&)));
        connect(m_ExternalIndexer, SIGNAL(progress(int)), m_ProgressDialog, SLOT(setValue(int)));
    }
}

void QmitkDicomExternalDataWidget::Initialize()
{
    m_ExternalDatabase = new ctkDICOMDatabase(this);

    try{
        m_ExternalDatabase->openDatabase(QString(":memory:"),QString( "EXTERNAL-DB"));
    }catch(std::exception e){
        MITK_ERROR <<"Database error: "<< m_ExternalDatabase->lastError().toStdString();
        m_ExternalDatabase->closeDatabase();
        return;
    }

    m_ExternalIndexer = new ctkDICOMIndexer(this);
}

void QmitkDicomExternalDataWidget::OnFinishedImport()
{
  m_ProgressDialog->setValue(m_ProgressDialog->maximum());
}

void QmitkDicomExternalDataWidget::OnDownloadButtonClicked()
{
  QStringList filesToDownload = GetFileNamesFromIndex();
  if (filesToDownload.size() == 0)
  {
    QMessageBox info;
    info.setText("You have to select an entry in the dicom browser for import.");
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

void QmitkDicomExternalDataWidget::OnStartDicomImport(const QString& directory)
{
  m_ImportDialog->close();
  m_ProgressDialog->show();

    m_LastImportDirectory = directory;
    m_ExternalIndexer->addDirectory(*m_ExternalDatabase,m_LastImportDirectory);
}

void QmitkDicomExternalDataWidget::OnSeriesSelectionChanged(const QStringList& s)
{
  m_Controls->viewExternalDataButton->setEnabled( (s.size() != 0) );
}

void QmitkDicomExternalDataWidget::SetupImportDialog()
{
        //Initialize import widget
        m_ImportDialog = new ctkFileDialog(this);
        // Since copy on import is not working at the moment
        // this feature is diabled
//        QCheckBox* importCheckbox = new QCheckBox("Copy on import", m_ImportDialog);
//        m_ImportDialog->setBottomWidget(importCheckbox);
        m_ImportDialog->setFileMode(QFileDialog::Directory);
        m_ImportDialog->setLabelText(QFileDialog::Accept,"Import");
        m_ImportDialog->setWindowTitle("Import DICOM files from directory ...");
        m_ImportDialog->setWindowModality(Qt::ApplicationModal);
        connect(m_ImportDialog, SIGNAL(fileSelected(QString)),this,SLOT(OnStartDicomImport(QString)));
}

void QmitkDicomExternalDataWidget::SetupProgressDialog(QWidget* parent)
{
    m_ProgressDialog = new QProgressDialog("DICOM Import", "Cancel", 0, 100, parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    m_ProgressDialogLabel = new QLabel("Initialization...", m_ProgressDialog);
    m_ProgressDialog->setLabel(m_ProgressDialogLabel);
    m_ProgressDialog->setWindowModality(Qt::ApplicationModal);
    m_ProgressDialog->setMinimumDuration(0);
}

void QmitkDicomExternalDataWidget::OnScanDirectory()
{
  m_ImportDialog->show();
}
