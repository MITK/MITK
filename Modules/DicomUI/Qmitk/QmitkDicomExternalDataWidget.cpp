/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkStatusBar.h>

// Qmitk
#include "QmitkDicomExternalDataWidget.h"
#include <mitklogmacros.h>

// Qt
#include <QCheckBox>
#include <QMessageBox>

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
    delete m_ImportDialog;
    delete m_ExternalDatabase;
    delete m_ExternalModel;
    delete m_ExternalIndexer;
    delete m_Controls;
    //delete m_Timer;
}


void QmitkDicomExternalDataWidget::CreateQtPartControl( QWidget *parent )
{

    // build up qt Widget, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkDicomExternalDataWidgetControls;
        m_Controls->setupUi( parent );

        // 
        m_Controls->ExternalDataTreeView->setSortingEnabled(true);
        m_Controls->ExternalDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Controls->ExternalDataTreeView->setModel(m_ExternalModel);

        //Initialize import widget
        m_ImportDialog = new ctkFileDialog();
        QCheckBox* importCheckbox = new QCheckBox("Copy on import", m_ImportDialog);
        m_ImportDialog->setBottomWidget(importCheckbox);
        m_ImportDialog->setFileMode(QFileDialog::Directory);
        m_ImportDialog->setLabelText(QFileDialog::Accept,"Import");
        m_ImportDialog->setWindowTitle("Import DICOM files from directory ...");
        m_ImportDialog->setWindowModality(Qt::ApplicationModal);
        connect(m_ImportDialog, SIGNAL(fileSelected(QString)),this,SLOT(OnFileSelectedAddExternalData(QString)));

        //connect Buttons
        connect(m_Controls->downloadButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));
        connect(m_Controls->viewExternalDataButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));
        connect(m_Controls->cancelButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));

        //connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnImportFinished()));
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

void QmitkDicomExternalDataWidget::OnFolderCDImport()
{
    m_ImportDialog->show();
    m_ImportDialog->raise();  

}

void QmitkDicomExternalDataWidget::OnFileSelectedAddExternalData(QString directory)
{

    if (QDir(directory).exists())
    {
        QCheckBox* copyOnImport = qobject_cast<QCheckBox*>(m_ImportDialog->bottomWidget());


        if (copyOnImport->isChecked())
        {
            //targetDirectory = d->DICOMDatabase->databaseDirectory();
            MBI_DEBUG<<directory.toStdString();
            emit SignalAddDicomData(directory);

        }else{

            if (m_Watcher.isRunning()){
                m_Watcher.waitForFinished();
            }
            m_Future = QtConcurrent::run(this,(void (QmitkDicomExternalDataWidget::*)(QString)) &QmitkDicomExternalDataWidget::AddDicomTemporary,directory);
            m_Watcher.setFuture(m_Future);

            emit SignalChangePage(1);
        }
    }

}

void QmitkDicomExternalDataWidget::OnDownloadButtonClicked()
{
    QStringList* filePaths= new QStringList();
    GetFileNamesFromIndex(*filePaths);
    emit SignalAddDicomData(*filePaths);
}

//TODO
void QmitkDicomExternalDataWidget::OnViewButtonClicked()
{
}

void QmitkDicomExternalDataWidget::OnCancelButtonClicked()
{
    m_Watcher.cancel();
    m_Watcher.waitForFinished();
}

void QmitkDicomExternalDataWidget::GetFileNamesFromIndex(QStringList& filePaths)
{
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

}

void QmitkDicomExternalDataWidget::AddDicomTemporary(QString directory)
{
    m_ExternalIndexer->addDirectory(*m_ExternalDatabase,directory);
    m_ExternalModel->reset();
}