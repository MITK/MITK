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

// Qmitk
#include "QmitkDicomLocalStorageWidget.h"
#include <mitklogmacros.h>
//#include <mitkProgressBar.h>
// Qt
#include <QMessageBox>



const std::string QmitkDicomLocalStorageWidget::Widget_ID = "org.mitk.Widgets.QmitkDicomLocalStorageWidget";

QmitkDicomLocalStorageWidget::QmitkDicomLocalStorageWidget(QWidget *parent)
:  m_Controls( 0 )
,m_LocalDatabase(new ctkDICOMDatabase(QString("./ctkDICOM-Database/ctkDICOM.sql")))
,m_LocalIndexer(new ctkDICOMIndexer())
,m_LocalModel(new ctkDICOMModel())
,m_LocalDatabaseDirectory(QString(m_LocalDatabase->databaseDirectory()))
{
    CreateQtPartControl(this);
}

QmitkDicomLocalStorageWidget::~QmitkDicomLocalStorageWidget()
{
    delete m_LocalDatabase;
    delete m_LocalIndexer;
    delete m_LocalModel;
    delete m_Controls;
    delete m_Timer;
}


void QmitkDicomLocalStorageWidget::CreateQtPartControl( QWidget *parent )
{
    // build up qt Widget, unless already done
    if ( !m_Controls )
    {
        
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkDicomLocalStorageWidgetControls;
        m_Controls->setupUi( parent );

        m_Controls->groupBox->setVisible(false);

        m_LocalModel->setEndLevel(ctkDICOMModel::SeriesType);
        m_LocalModel->setDatabase(m_LocalDatabase->database());

        m_Controls->InternalDataTreeView->setSortingEnabled(true);
        m_Controls->InternalDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Controls->InternalDataTreeView->setModel(m_LocalModel);

        connect(m_Controls->deleteButton,SIGNAL(clicked()),this,SLOT(OnDeleteButtonClicked()));

        //connect(&m_Watcher, SIGNAL(started()), this, SLOT());
        connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnImportFinished()));

        connect(m_Controls->CancelButton, SIGNAL(clicked()), this , SLOT(OnCancelButtonClicked()));
        connect(m_Controls->viewInternalDataButton, SIGNAL(clicked()), this , SLOT(OnViewButtonClicked()));


    }
}


void QmitkDicomLocalStorageWidget::SetDatabaseDirectory(QString newDatatbaseDirectory)
{
    m_LocalDatabase->closeDatabase();
    m_LocalDatabaseDirectory = newDatatbaseDirectory;
    QString databaseFileName = m_LocalDatabaseDirectory + QString("/ctkDICOM.sql");
    //m_ExternalDatabase->initializeDatabase();
    try{
        m_LocalDatabase->openDatabase(databaseFileName);
    }catch(std::exception e){
        MITK_ERROR <<"Database error: "<< m_LocalDatabase->lastError().toStdString();
        m_LocalDatabase->closeDatabase();
        return;
    }  
    m_LocalDatabase->closeDatabase();
}

void QmitkDicomLocalStorageWidget::OnAddDICOMData(QString& directory)
{
    //mitk::ProgressBar::GetInstance()->AddStepsToDo(1); 
    if(m_LocalDatabase->isOpen())
    {
        m_LocalIndexer->addDirectory(*m_LocalDatabase,directory,m_LocalDatabaseDirectory);
    }
    m_LocalModel->setDatabase(m_LocalDatabase->database());
}

void QmitkDicomLocalStorageWidget::OnAddDICOMData(QStringList& patientFiles)
{
    if(m_LocalDatabase->isOpen())
    {
        float length=static_cast<float>(patientFiles.length());
        float i(0);
        for (QStringList::iterator currentPatientFilePath = patientFiles.begin();
            currentPatientFilePath != patientFiles.end(); ++currentPatientFilePath) 
        {   ++i;
        m_LocalIndexer->addFile(*m_LocalDatabase,*currentPatientFilePath,m_LocalDatabaseDirectory);

        }
    }
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

void QmitkDicomLocalStorageWidget::StartDicomImport(QString& dicomData)
{
    if (m_Watcher.isRunning()){
        m_Watcher.waitForFinished();
    }
    m_Future = QtConcurrent::run(this,(void (QmitkDicomLocalStorageWidget::*)(QString&)) &QmitkDicomLocalStorageWidget::OnAddDICOMData,dicomData);
    m_Watcher.setFuture(m_Future); 
}

void QmitkDicomLocalStorageWidget::StartDicomImport(QStringList& dicomData)
{
    if (m_Watcher.isRunning())
    {
        m_Watcher.waitForFinished();
    }
    m_Future = QtConcurrent::run(this,(void (QmitkDicomLocalStorageWidget::*)(QStringList&)) &QmitkDicomLocalStorageWidget::OnAddDICOMData,dicomData);
    m_Watcher.setFuture(m_Future);
}

void QmitkDicomLocalStorageWidget::OnCancelButtonClicked()
{
    m_Watcher.cancel();
    m_Watcher.waitForFinished();
}
//TODO
void QmitkDicomLocalStorageWidget::OnViewButtonClicked()
{
}
