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

#include <mitkStatusBar.h>

// Qmitk
#include "QmitkDicomExternalDataWidget.h"
#include <mitkLogMacros.h>

// Qt
#include <QCheckBox>
#include <QMessageBox>
#include <QMap>
#include <QVariant>

// CTK
#include <ctkDICOMDataset.h>



const std::string QmitkDicomExternalDataWidget::Widget_ID = "org.mitk.Widgets.QmitkDicomExternalDataWidget";

QmitkDicomExternalDataWidget::QmitkDicomExternalDataWidget(QWidget *parent)
:  m_Controls( 0 )
{
    Initialize();
    CreateQtPartControl(this);
}

QmitkDicomExternalDataWidget::~QmitkDicomExternalDataWidget()
{
    delete m_ExternalDatabase;
    delete m_ExternalModel;
    delete m_ExternalIndexer;
    delete m_Controls;
}


void QmitkDicomExternalDataWidget::CreateQtPartControl( QWidget *parent )
{

    // build up qt Widget, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkDicomExternalDataWidgetControls;
        m_Controls->setupUi( parent );
        m_Controls->cancelButton->setVisible(false);
        m_Controls->viewExternalDataButton->setVisible(true);
        //
        m_Controls->ExternalDataTreeView->setSortingEnabled(true);
        m_Controls->ExternalDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Controls->ExternalDataTreeView->setModel(m_ExternalModel);

        //connect Buttons
        connect(m_Controls->downloadButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));
        connect(m_Controls->viewExternalDataButton, SIGNAL(clicked()),this,SLOT(OnViewButtonClicked()));
        connect(m_Controls->cancelButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));

        connect(m_ExternalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));
        connect(m_ExternalIndexer, SIGNAL(indexingComplete()),this, SIGNAL(SignalFinishedImport()));

        connect(m_ExternalIndexer, SIGNAL(indexingFilePath(QString)),this, SIGNAL(SignalProcessingFile(QString)));
        connect(m_ExternalIndexer, SIGNAL(progress(int)),this, SIGNAL(SignalProgress(int)));
        connect(this, SIGNAL(SignalCancelImport()),m_ExternalIndexer, SLOT(cancel()));
    }
}

void QmitkDicomExternalDataWidget::Initialize()
{
    m_ExternalDatabase = new ctkDICOMDatabase();
    //m_ExternalDatabase->initializeDatabase();
    try{
        m_ExternalDatabase->openDatabase(QString(":memory:"),QString( "EXTERNAL-DB"));
    }catch(std::exception e){
        MITK_ERROR <<"Database error: "<< m_ExternalDatabase->lastError().toStdString();
        m_ExternalDatabase->closeDatabase();
        return;
    }

    m_ExternalModel = new ctkDICOMModel();
    m_ExternalModel->setDatabase(m_ExternalDatabase->database());
    m_ExternalModel->setEndLevel(ctkDICOMModel::SeriesType);

    m_ExternalIndexer = new ctkDICOMIndexer();
}

void QmitkDicomExternalDataWidget::OnFinishedImport()
{
    m_ExternalModel->setDatabase(m_ExternalDatabase->database());
}

void QmitkDicomExternalDataWidget::OnDownloadButtonClicked()
{
    emit SignalStartDicomImport(GetFileNamesFromIndex());
}

void QmitkDicomExternalDataWidget::OnViewButtonClicked()
{
    QModelIndex currentIndex = m_Controls->ExternalDataTreeView->currentIndex();
    if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::SeriesType))
    {
        QString seriesUID = m_ExternalModel->data(currentIndex,ctkDICOMModel::UIDRole).toString();
        QString seriesName = m_ExternalModel->data(currentIndex).toString();

        QModelIndex studyIndex = m_ExternalModel->parent(currentIndex);
        QString studyUID = m_ExternalModel->data(studyIndex,ctkDICOMModel::UIDRole).toString();
        QString studyName = m_ExternalModel->data(studyIndex).toString();

        QModelIndex patientIndex = m_ExternalModel->parent(studyIndex);
        QString patientName = m_ExternalModel->data(patientIndex).toString();

        QStringList filesForSeries;
        filesForSeries = m_ExternalDatabase->filesForSeries(seriesUID);

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

QStringList QmitkDicomExternalDataWidget::GetFileNamesFromIndex()
{
    QStringList filePaths = QStringList();
    QModelIndex currentIndex = m_Controls->ExternalDataTreeView->currentIndex();
    QString currentUID = m_ExternalModel->data(currentIndex,ctkDICOMModel::UIDRole).toString();

    if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::SeriesType))
    {
        filePaths.append(m_ExternalDatabase->filesForSeries(currentUID));
    }
    else if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::StudyType))
    {
        QStringList seriesList;
        seriesList.append( m_ExternalDatabase->seriesForStudy(currentUID) );

        QStringList::Iterator serieIt;
        for(serieIt=seriesList.begin();serieIt!=seriesList.end();++serieIt)
        {
            filePaths.append(m_ExternalDatabase->filesForSeries(*serieIt));
        }
    }
    else if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::PatientType))
    {
        QStringList studiesList,seriesList;
        studiesList.append( m_ExternalDatabase->studiesForPatient(currentUID) );

        QStringList::Iterator studyIt,serieIt;
        for(studyIt=studiesList.begin();studyIt!=studiesList.end();++studyIt)
        {
            seriesList.append( m_ExternalDatabase->seriesForStudy(*studyIt) );

            for(serieIt=seriesList.begin();serieIt!=seriesList.end();++serieIt)
            {
                filePaths.append(m_ExternalDatabase->filesForSeries(*serieIt));
            }
        }
    }
    return filePaths;
}

void QmitkDicomExternalDataWidget::OnStartDicomImport(const QString& directory)
{
    m_LastImportDirectory = directory;
    m_ExternalIndexer->addDirectory(*m_ExternalDatabase,m_LastImportDirectory);
}

void QmitkDicomExternalDataWidget::OnSearchParameterChanged()
{
    m_ExternalModel->setDatabase(m_ExternalDatabase->database(),m_Controls->SearchOption_2->parameters());
}
