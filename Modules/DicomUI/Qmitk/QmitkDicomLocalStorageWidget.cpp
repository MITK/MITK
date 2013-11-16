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
#include <mitkLogMacros.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
// Qt
#include <QMessageBox>
#include <QModelIndex>
#include <QHash>
#include <QVariant>

const std::string QmitkDicomLocalStorageWidget::Widget_ID = "org.mitk.Widgets.QmitkDicomLocalStorageWidget";

QmitkDicomLocalStorageWidget::QmitkDicomLocalStorageWidget(QWidget *parent)
:  m_Controls( 0 )
,m_LocalIndexer(new ctkDICOMIndexer())
,m_LocalModel(new ctkDICOMModel())
{
    CreateQtPartControl(this);
}

QmitkDicomLocalStorageWidget::~QmitkDicomLocalStorageWidget()
{
    m_LocalDatabase->closeDatabase();
    delete m_LocalDatabase;
    delete m_LocalIndexer;
    delete m_LocalModel;
    delete m_Controls;
}

void QmitkDicomLocalStorageWidget::CreateQtPartControl( QWidget *parent )
{
    if ( !m_Controls )
    {
        m_Controls = new Ui::QmitkDicomLocalStorageWidgetControls;
        m_Controls->setupUi( parent );

        connect(m_Controls->deleteButton,SIGNAL(clicked()),this,SLOT(OnDeleteButtonClicked()));
        connect(m_Controls->viewInternalDataButton, SIGNAL(clicked()), this , SLOT(OnViewButtonClicked()));

        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));
        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SIGNAL(SignalFinishedImport()));

        connect(m_LocalIndexer, SIGNAL(indexingFilePath(QString)),this, SIGNAL(SignalProcessingFile(QString)));
        connect(m_LocalIndexer, SIGNAL(progress(int)),this, SIGNAL(SignalProgress(int)));
        connect(this, SIGNAL(SignalCancelImport()),m_LocalIndexer, SLOT(cancel()));

        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesSelectionChanged(const QStringList&)),
                this, SLOT(OnSeriesSelectionChanged(const QStringList&)));
        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesSelectionChanged(const QStringList&)),
                this, SLOT(OnSeriesSelectionChanged(const QStringList&)));
        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesDoubleClicked(const QModelIndex&)),
                this, SLOT(OnViewButtonClicked()));
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
        m_LocalIndexer->addListOfFiles(*m_LocalDatabase,dicomData,m_LocalDatabase->databaseDirectory());
    }
}

void QmitkDicomLocalStorageWidget::OnFinishedImport()
{
    m_LocalModel->setDatabase(m_LocalDatabase->database());
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
    m_Controls->ctkDICOMBrowser->setCTKDICOMDatabase(m_LocalDatabase);
}

void QmitkDicomLocalStorageWidget::OnSeriesSelectionChanged(const QStringList &s)
{
  m_Controls->viewInternalDataButton->setEnabled((s.size() != 0));
}
