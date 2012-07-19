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
// Qt
#include <QMessageBox>
#include <QModelIndex>
#include <QMap>
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
        m_Controls->addSortingTagButton_2->setVisible(false);
        m_Controls->deleteSortingTagButton_2->setVisible(false);
        m_Controls->InternalDataTreeView->setSortingEnabled(true);
        m_Controls->InternalDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Controls->InternalDataTreeView->setModel(m_LocalModel);
        connect(m_Controls->deleteButton,SIGNAL(clicked()),this,SLOT(OnDeleteButtonClicked()));
        connect(m_Controls->CancelButton, SIGNAL(clicked()), this , SLOT(OnCancelButtonClicked()));
        connect(m_Controls->viewInternalDataButton, SIGNAL(clicked()), this , SLOT(OnViewButtonClicked()));
        connect(m_Controls->SearchOption, SIGNAL(parameterChanged()), this, SLOT(OnSearchParameterChanged()));
    }
}

void QmitkDicomLocalStorageWidget::StartDicomImport(const QString& dicomData)
{
    if (m_Watcher.isRunning()){
        m_Watcher.waitForFinished();
    }
    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
    m_Future = QtConcurrent::run(this,(void (QmitkDicomLocalStorageWidget::*)(const QString&)) &QmitkDicomLocalStorageWidget::AddDICOMData,dicomData);
    m_Watcher.setFuture(m_Future); 
}

void QmitkDicomLocalStorageWidget::StartDicomImport(const QStringList& dicomData)
{
    if (m_Watcher.isRunning())
    {
        m_Watcher.waitForFinished();
    }
    mitk::ProgressBar::GetInstance()->AddStepsToDo(dicomData.count());
    m_Future = QtConcurrent::run(this,(void (QmitkDicomLocalStorageWidget::*)(const QStringList&)) &QmitkDicomLocalStorageWidget::AddDICOMData,dicomData);
    m_Watcher.setFuture(m_Future);
}

void QmitkDicomLocalStorageWidget::AddDICOMData(const QString& directory)
{
    if(m_LocalDatabase->isOpen())
    {
        mitk::ProgressBar::GetInstance()->Progress();
        m_LocalIndexer->addDirectory(*m_LocalDatabase,directory,m_LocalDatabase->databaseDirectory());
    }
    m_LocalModel->setDatabase(m_LocalDatabase->database());
    mitk::ProgressBar::GetInstance()->Progress();
    emit FinishedImport(directory);
}

void QmitkDicomLocalStorageWidget::AddDICOMData(const QStringList& patientFiles)
{
    if(m_LocalDatabase->isOpen())
    {
        QStringListIterator fileIterator(patientFiles);
        while(fileIterator.hasNext())
        {
            m_LocalIndexer->addFile(*m_LocalDatabase,fileIterator.next(),m_LocalDatabase->databaseDirectory());
            mitk::ProgressBar::GetInstance()->Progress();
        }
    }
    m_LocalModel->setDatabase(m_LocalDatabase->database());
    emit FinishedImport(patientFiles);
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

void QmitkDicomLocalStorageWidget::OnCancelButtonClicked()
{
    m_Watcher.cancel();
    m_Watcher.waitForFinished();
    m_LocalDatabase->closeDatabase();
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

        QString filePath;
        filePath.append(m_LocalDatabase->databaseDirectory());
        filePath.append("/dicom/");
        filePath.append(studyUID);
        filePath.append("/");
        filePath.append(seriesUID);
        filePath.append("/");

        QStringList eventProperties;
        eventProperties << patientName << studyUID <<studyName << seriesUID << seriesName << filePath;
        MITK_INFO << filePath.toStdString();
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
