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
        m_Controls->groupBox->setVisible(false);
        m_Controls->CancelButton->setVisible(false);
        m_Controls->addSortingTagButton_2->setVisible(false);
        m_Controls->deleteSortingTagButton_2->setVisible(false);
        m_Controls->InternalDataTreeView->setSortingEnabled(true);
        m_Controls->InternalDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Controls->InternalDataTreeView->setModel(m_LocalModel);
        connect(m_Controls->deleteButton,SIGNAL(clicked()),this,SLOT(OnDeleteButtonClicked()));
        connect(m_Controls->CancelButton, SIGNAL(clicked()), this , SLOT(OnCancelButtonClicked()));
        connect(m_Controls->viewInternalDataButton, SIGNAL(clicked()), this , SLOT(OnViewButtonClicked()));
        connect(m_Controls->SearchOption, SIGNAL(parameterChanged()), this, SLOT(OnSearchParameterChanged()));

        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));
        connect(m_LocalIndexer, SIGNAL(indexingComplete()),this, SIGNAL(SignalFinishedImport()));

        connect(m_LocalIndexer, SIGNAL(indexingFilePath(QString)),this, SIGNAL(SignalProcessingFile(QString)));
        connect(m_LocalIndexer, SIGNAL(progress(int)),this, SIGNAL(SignalProgress(int)));
        connect(this, SIGNAL(SignalCancelImport()),m_LocalIndexer, SLOT(cancel()));
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
    QModelIndex currentIndex = m_Controls->InternalDataTreeView->currentIndex();
    QString currentUID = m_LocalModel->data(currentIndex,ctkDICOMModel::UIDRole).toString();
    if(m_LocalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::SeriesType))
    {
        m_LocalDatabase->removeSeries(currentUID);
    }
    else if(m_LocalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::StudyType))
    {
        m_LocalDatabase->removeStudy(currentUID);
    }
    else if(m_LocalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::PatientType))
    {
        m_LocalDatabase->removePatient(currentUID);
    }
    m_LocalModel->reset();
}

void QmitkDicomLocalStorageWidget::OnViewButtonClicked()
{
    QModelIndex currentIndex = m_Controls->InternalDataTreeView->currentIndex();
    if(m_LocalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::SeriesType))
    {
        QString seriesUID = m_LocalModel->data(currentIndex,ctkDICOMModel::UIDRole).toString();
        QString seriesName = m_LocalModel->data(currentIndex).toString();

        QModelIndex studyIndex = m_LocalModel->parent(currentIndex);
        QString studyUID = m_LocalModel->data(studyIndex,ctkDICOMModel::UIDRole).toString();
        QString studyName = m_LocalModel->data(studyIndex).toString();

        QModelIndex patientIndex = m_LocalModel->parent(studyIndex);
        QString patientName = m_LocalModel->data(patientIndex).toString();

        QStringList filesForSeries;
        filesForSeries = m_LocalDatabase->filesForSeries(seriesUID);

        QHash<QString,QVariant> eventProperties;
        eventProperties.insert("PatientName",patientName);
        eventProperties.insert("StudyUID",studyUID);
        eventProperties.insert("StudyName",studyName);
        eventProperties.insert("SeriesUID",seriesUID);
        eventProperties.insert("SeriesName",seriesName);
        eventProperties.insert("FilesForSeries",filesForSeries);
        emit SignalDicomToDataManager(eventProperties);
    }
}

void QmitkDicomLocalStorageWidget::OnSearchParameterChanged()
{
    m_LocalModel->setDatabase(m_LocalDatabase->database(),m_Controls->SearchOption->parameters());
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
    m_LocalModel->setEndLevel(ctkDICOMModel::SeriesType);
    m_LocalModel->setDatabase(m_LocalDatabase->database());
}
