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
        m_Controls->viewExternalDataButton->setVisible(true);

        //connect Buttons
        connect(m_Controls->downloadButton, SIGNAL(clicked()),this,SLOT(OnDownloadButtonClicked()));
        connect(m_Controls->viewExternalDataButton, SIGNAL(clicked()),this,SLOT(OnViewButtonClicked()));

        connect(m_ExternalIndexer, SIGNAL(indexingComplete()),this, SLOT(OnFinishedImport()));
        connect(m_ExternalIndexer, SIGNAL(indexingComplete()),this, SIGNAL(SignalFinishedImport()));

        connect(m_ExternalIndexer, SIGNAL(indexingFilePath(QString)),this, SIGNAL(SignalProcessingFile(QString)));
        connect(m_ExternalIndexer, SIGNAL(progress(int)),this, SIGNAL(SignalProgress(int)));
        connect(this, SIGNAL(SignalCancelImport()),m_ExternalIndexer, SLOT(cancel()));
        connect(m_Controls->ctkDICOMBrowser, SIGNAL(seriesSelectionChanged(const QStringList&)),
                this, SLOT(OnSeriesSelectionChanged(const QStringList&)));
    }
}

void QmitkDicomExternalDataWidget::Initialize()
{
    m_ExternalDatabase = new ctkDICOMDatabase();

    try{
        m_ExternalDatabase->openDatabase(QString(":memory:"),QString( "EXTERNAL-DB"));
    }catch(std::exception e){
        MITK_ERROR <<"Database error: "<< m_ExternalDatabase->lastError().toStdString();
        m_ExternalDatabase->closeDatabase();
        return;
    }

    m_ExternalIndexer = new ctkDICOMIndexer();
}

void QmitkDicomExternalDataWidget::OnFinishedImport()
{
  m_Controls->ctkDICOMBrowser->setCTKDICOMDatabase(m_ExternalDatabase);
}

void QmitkDicomExternalDataWidget::OnDownloadButtonClicked()
{
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
    QStringList filePaths = QStringList();
    QStringList uids = m_Controls->ctkDICOMBrowser->currentSeriesSelection();
    QString uid;
    foreach (uid, uids)
    {
      filePaths.append(m_ExternalDatabase->filesForSeries(uid));

    }
    if (!filePaths.empty())
      return filePaths;

//    uids = m_Controls->ctkDICOMBrowser->currentStudiesSelection();
//    foreach (uid, uids)
//      {
//        filePaths.append(m_ExternalDatabase->seriesForStudy());
//      }

//    QModelIndex currentIndex = m_Controls->ExternalDataTreeView->currentIndex();
//    QString currentUID = m_ExternalModel->data(currentIndex,ctkDICOMModel::UIDRole).toString();

//    if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::SeriesType))
//    {
//        filePaths.append(m_ExternalDatabase->filesForSeries(currentUID));
//    }
//    else if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::StudyType))
//    {
//        QStringList seriesList;
//        seriesList.append( m_ExternalDatabase->seriesForStudy(currentUID) );

//        QStringList::Iterator serieIt;
//        for(serieIt=seriesList.begin();serieIt!=seriesList.end();++serieIt)
//        {
//            filePaths.append(m_ExternalDatabase->filesForSeries(*serieIt));
//        }
//    }
//    else if(m_ExternalModel->data(currentIndex,ctkDICOMModel::TypeRole)==static_cast<int>(ctkDICOMModel::PatientType))
//    {
//        QStringList studiesList,seriesList;
//        studiesList.append( m_ExternalDatabase->studiesForPatient(currentUID) );

//        QStringList::Iterator studyIt,serieIt;
//        for(studyIt=studiesList.begin();studyIt!=studiesList.end();++studyIt)
//        {
//            seriesList.append( m_ExternalDatabase->seriesForStudy(*studyIt) );

//            for(serieIt=seriesList.begin();serieIt!=seriesList.end();++serieIt)
//            {
//                filePaths.append(m_ExternalDatabase->filesForSeries(*serieIt));
//            }
//        }
//    }
    return filePaths;
}

void QmitkDicomExternalDataWidget::OnStartDicomImport(const QString& directory)
{
    m_LastImportDirectory = directory;
    m_ExternalIndexer->addDirectory(*m_ExternalDatabase,m_LastImportDirectory);
}

void QmitkDicomExternalDataWidget::OnSeriesSelectionChanged(const QStringList& s)
{
  m_Controls->viewExternalDataButton->setEnabled((s.size() != 0));
}
