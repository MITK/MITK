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
, m_DirectoryName(new QString())
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
    delete m_DirectoryName;
    delete m_ProgressDialogLabel;
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
        m_Controls->viewExternalDataButton->setVisible(false);
        // 
        m_Controls->ExternalDataTreeView->setSortingEnabled(true);
        m_Controls->ExternalDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_Controls->ExternalDataTreeView->setModel(m_ExternalModel);

        //connect Buttons
        connect(m_Controls->downloadButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));
        connect(m_Controls->viewExternalDataButton, SIGNAL(clicked()),this,SLOT(OnViewButtonClicked()));
        connect(m_Controls->cancelButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));

        //Initialize import widget
        m_ImportDialog = new ctkFileDialog();
        QCheckBox* importCheckbox = new QCheckBox("Copy on import", m_ImportDialog);
        m_ImportDialog->setBottomWidget(importCheckbox);
        m_ImportDialog->setFileMode(QFileDialog::Directory);
        m_ImportDialog->setLabelText(QFileDialog::Accept,"Import");
        m_ImportDialog->setWindowTitle("Import DICOM files from directory ...");
        m_ImportDialog->setWindowModality(Qt::ApplicationModal);
        connect(m_ImportDialog, SIGNAL(fileSelected(QString)),this,SLOT(OnFileSelectedAddExternalData(QString)));

        m_ProgressDialog= new QProgressDialog ("DICOM Import", "Cancel", 0, 100, this,Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        // We don't want the m_ProgressDialog dialog to resize itself, so we bypass the label
        // by creating our own
        m_ProgressDialogLabel = new QLabel(tr("Initialization..."));
        m_ProgressDialog->setLabel(m_ProgressDialogLabel);
        #ifdef Q_WS_MAC
        // BUG: avoid deadlock of dialogs on mac
        m_ProgressDialog->setWindowModality(Qt::NonModal);
        #else
        m_ProgressDialog->setWindowModality(Qt::ApplicationModal);
        #endif

    connect(m_ProgressDialog, SIGNAL(canceled()), m_ExternalIndexer, SLOT(cancel()));
    connect(m_ExternalIndexer, SIGNAL(indexingFilePath(QString)),
            m_ProgressDialogLabel, SLOT(setText(QString)));
    connect(m_ExternalIndexer, SIGNAL(progress(int)),
            m_ProgressDialog, SLOT(setValue(int)));
    connect(m_ExternalIndexer, SIGNAL(progress(int)),
            this, SLOT(OnProgress(int)));
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
        m_DirectoryName = new QString(directory);
        QCheckBox* copyOnImport = qobject_cast<QCheckBox*>(m_ImportDialog->bottomWidget());

        if (copyOnImport->isChecked())
        {
            emit SignalAddDicomData(directory);
        }else{
            AddDicomTemporary(directory);
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

        QStringList eventProperties;
        eventProperties << patientName << studyUID << studyName << seriesUID << seriesName << *m_DirectoryName;
        emit SignalDicomToDataManager(eventProperties);
    }
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
    m_ProgressDialog->setMinimumDuration(0);
    m_ProgressDialog->setValue(0);
    m_ProgressDialog->show();
    m_ExternalIndexer->addDirectory(*m_ExternalDatabase,directory);
    m_ExternalModel->reset();
}

void QmitkDicomExternalDataWidget::OnProgress(int progress)
{
  Q_UNUSED(progress);
  QApplication::processEvents();
}

void QmitkDicomExternalDataWidget::OnSearchParameterChanged()
{
    m_ExternalModel->setDatabase(m_ExternalDatabase->database(),m_Controls->SearchOption_2->parameters());
}
