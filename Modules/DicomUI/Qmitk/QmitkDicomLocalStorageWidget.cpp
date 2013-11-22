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
#include "QmitkDicomLocalStorageWidget.h"
//#include <mitkLogMacros.h>

// Qt
#include <QLabel>
#include <QProgressDialog>
#include <QVariant>

const std::string QmitkDicomLocalStorageWidget::Widget_ID = "org.mitk.Widgets.QmitkDicomLocalStorageWidget";

QmitkDicomLocalStorageWidget::QmitkDicomLocalStorageWidget(QWidget *parent)
  : QWidget(parent)
  , m_Controls(0)
  , m_LocalIndexer(new ctkDICOMIndexer(parent))
{
    CreateQtPartControl(this);
}

QmitkDicomLocalStorageWidget::~QmitkDicomLocalStorageWidget()
{
    m_LocalDatabase->closeDatabase();
}

void QmitkDicomLocalStorageWidget::CreateQtPartControl( QWidget *parent )
{
    if ( !m_Controls )
    {
        m_Controls = new Ui::QmitkDicomLocalStorageWidgetControls;
        m_Controls->setupUi( parent );

        this->SetupProgressDialog(this);

        connect(m_Controls->deleteButton,SIGNAL(clicked()),this,SLOT(OnDeleteButtonClicked()));
        connect(m_Controls->viewInternalDataButton, SIGNAL(clicked()), this , SLOT(OnViewButtonClicked()));

        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesSelectionChanged(const QStringList&)),
                this, SLOT(OnSeriesSelectionChanged(const QStringList&)));
        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesSelectionChanged(const QStringList&)),
                this, SLOT(OnSeriesSelectionChanged(const QStringList&)));
        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesDoubleClicked(const QModelIndex&)),
                this, SLOT(OnViewButtonClicked()));

        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));
        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SIGNAL(SignalFinishedImport()));
        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));
        connect(m_LocalIndexer, SIGNAL(indexingFilePath(QString)), m_ProgressDialogLabel, SLOT(setText(QString)));
        connect(m_LocalIndexer, SIGNAL(progress(int)), m_ProgressDialog, SLOT(setValue(int)));
        connect(m_ProgressDialog, SIGNAL(canceled()), m_LocalIndexer, SLOT(cancel()));

        m_Controls->ctkDICOMBrowser->setDynamicTableLayout(true);
    }
}

void QmitkDicomLocalStorageWidget::OnStartDicomImport(const QString& dicomData)
{
    if(m_LocalDatabase->isOpen())
    {
        m_LocalIndexer->addDirectory(*m_LocalDatabase,dicomData,m_LocalDatabase->databaseDirectory());
    }
}

void QmitkDicomLocalStorageWidget::OnStartDicomImport(const QStringList& dicomData)
{
    if(m_LocalDatabase->isOpen())
    {
        m_ProgressDialog->show();
        m_LocalIndexer->addListOfFiles(*m_LocalDatabase,dicomData,m_LocalDatabase->databaseDirectory());
    }
}

void QmitkDicomLocalStorageWidget::OnFinishedImport()
{
    m_ProgressDialog->setValue(m_ProgressDialog->maximum());
}

void QmitkDicomLocalStorageWidget::OnDeleteButtonClicked()
{
  QStringList selectedSeriesUIDs = m_Controls->ctkDICOMBrowser->currentSeriesSelection();
  QString uid;
  foreach (uid, selectedSeriesUIDs)
  {
    m_LocalDatabase->removeSeries(uid);
  }
  QStringList selectedStudiesUIDs = m_Controls->ctkDICOMBrowser->currentStudiesSelection();
  foreach(uid, selectedStudiesUIDs)
  {
    m_LocalDatabase->removeStudy(uid);
  }
  QStringList selectedPatientUIDs = m_Controls->ctkDICOMBrowser->currentPatientsSelection();
  foreach(uid, selectedPatientUIDs)
  {
    m_LocalDatabase->removePatient(uid);
  }
}

void QmitkDicomLocalStorageWidget::OnViewButtonClicked()
{
  QStringList uids = m_Controls->ctkDICOMBrowser->currentSeriesSelection();
  QString uid;
  foreach (uid, uids)
  {
    QStringList filesForSeries = m_LocalDatabase->filesForSeries(uid);
    QHash<QString, QVariant> eventProperty;
    eventProperty.insert("FilesForSeries", filesForSeries);
    emit SignalDicomToDataManager(eventProperty);
  }
}

void QmitkDicomLocalStorageWidget::SetDatabaseDirectory(QString newDatatbaseDirectory)
{
    QDir databaseDirecory = QDir(newDatatbaseDirectory);
    if(!databaseDirecory.exists())
    {
        databaseDirecory.mkpath(databaseDirecory.absolutePath());
    }
    QString newDatatbaseFile = databaseDirecory.absolutePath() + QString("/ctkDICOM.sql");
    this->SetDatabase(newDatatbaseFile);
}

void QmitkDicomLocalStorageWidget::SetDatabase(QString databaseFile)
{
    m_LocalDatabase = new ctkDICOMDatabase(databaseFile);
    m_LocalDatabase->setParent(this);
    m_Controls->ctkDICOMBrowser->setCTKDICOMDatabase(m_LocalDatabase);
}

void QmitkDicomLocalStorageWidget::OnSeriesSelectionChanged(const QStringList &s)
{
  m_Controls->viewInternalDataButton->setEnabled((s.size() != 0));
}

void QmitkDicomLocalStorageWidget::SetupProgressDialog(QWidget* parent)
{
    m_ProgressDialog = new QProgressDialog("DICOM Import", "Cancel", 0, 100, parent,Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    m_ProgressDialogLabel = new QLabel("Initialization...", m_ProgressDialog);
    m_ProgressDialog->setLabel(m_ProgressDialogLabel);
    m_ProgressDialog->setWindowModality(Qt::ApplicationModal);
    m_ProgressDialog->setMinimumDuration(0);
}
