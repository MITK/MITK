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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>
#include <mitkGlobalInteraction.h>
#include <mitkDataStorageEditorInput.h>
#include "berryFileEditorInput.h"

// Qmitk
#include "QmitkDicomEditor.h"
#include "mitkPluginActivator.h"
#include <mitkDicomSeriesReader.h>
//#include "mitkProgressBar.h"

// Qt
#include <QCheckBox>
#include <QMessageBox>
#include <QWidget>

#include <QtSql>
#include <QSqlDatabase>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

//CTK
#include <ctkDICOMModel.h>
#include <ctkDICOMAppWidget.h>
#include <ctkDICOMQueryWidget.h>
#include <ctkFileDialog.h>
#include <ctkDICOMQueryRetrieveWidget.h>


const std::string QmitkDicomEditor::EDITOR_ID = "org.mitk.editors.dicomeditor";


QmitkDicomEditor::QmitkDicomEditor()
: m_Thread(new QThread())
, m_DicomDirectoryListener(new QmitkDicomDirectoryListener())
{
}

QmitkDicomEditor::~QmitkDicomEditor()
{
    m_Thread->terminate();
    delete m_StoreSCPLauncher;
    delete m_Thread;
    delete m_DicomDirectoryListener;
}

void QmitkDicomEditor::CreateQtPartControl(QWidget *parent )
{   
    m_Controls.setupUi( parent );
    SetupDefaults();
    connect(m_Controls.externalDataWidget,SIGNAL(SignalAddDicomData(const QString&)),m_Controls.internalDataWidget,SLOT(StartDicomImport(const QString&)));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalAddDicomData(const QStringList&)),m_Controls.internalDataWidget,SLOT(StartDicomImport(const QStringList&))); 
    connect(m_Controls.internalDataWidget,SIGNAL(FinishedImport(const QString&)),this,SLOT(OnDicomImportFinished(const QString&)));
    connect(m_Controls.internalDataWidget,SIGNAL(FinishedImport(const QStringList&)),this,SLOT(OnDicomImportFinished(const QStringList&)));
    connect(m_Controls.CDButton, SIGNAL(clicked()), m_Controls.externalDataWidget, SLOT(OnFolderCDImport()));
    connect(m_Controls.FolderButton, SIGNAL(clicked()), m_Controls.externalDataWidget, SLOT(OnFolderCDImport()));
    connect(m_Controls.QueryRetrieveButton, SIGNAL(clicked()), this, SLOT(OnQueryRetrieve()));
    connect(m_Controls.LocalStorageButton, SIGNAL(clicked()), this, SLOT(OnLocalStorage()));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalChangePage(int)), this, SLOT(OnChangePage(int)));
}

void QmitkDicomEditor::OnSeriesModelDoubleClicked(QModelIndex index){
    QModelIndex studyIndex = index.parent();
    QModelIndex seriesIndex = index;

    //ctkDICOMModel* model = const_cast<ctkDICOMModel*>(qobject_cast<const ctkDICOMModel*>(index.model()));

    //if(model)
    //{
    //    model->fetchMore(seriesIndex);

    //    int imageCount = model->rowCount(seriesIndex);

    //    MITK_INFO<< "Series Index:"<< imageCount << "\n";

    //    QString filePath = m_Controls.m_ctkDICOMAppWidget->databaseDirectory() +
    //        "/dicom/" + model->data(studyIndex ,ctkDICOMModel::UIDRole).toString() + "/";

    //    MITK_INFO << "filepath: "<< filePath.toStdString() << "\n";

    //    QString series_uid = model->data(seriesIndex ,ctkDICOMModel::UIDRole).toString();

    //    MITK_INFO << "series_uid: " << series_uid.toStdString() << "\n";

    //    if(QFile(filePath).exists())
    //    {
    //        filePath.replace(0,1,"");
    //        QString absolutFilePath("C:/home/bauerm/bin/MITK/MITK-build/Applications/ExtApp/ctkDICOM-Database/dicom/");
    //        absolutFilePath.append(model->data(studyIndex ,ctkDICOMModel::UIDRole).toString());
    //        absolutFilePath.append("/");
    //        //add all fienames from series to strin container
    //        mitk::DicomSeriesReader::StringContainer names = mitk::DicomSeriesReader::GetSeries(absolutFilePath.toStdString(),series_uid.toStdString());
    //        mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(names, true, true);

    //        if (node.IsNull())
    //        {
    //            MITK_ERROR << "Could not load series: " << series_uid.toStdString();
    //        }
    //        else
    //        {
    //            node->SetName(series_uid.toStdString());
    //            this->GetDefaultDataStorage()->Add(node);

    //            this->GetActiveStdMultiWidget();

    //            mitk::RenderingManager::GetInstance()->InitializeViews(node->GetData()->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
    //            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    //        }

    //    }
    //}
}

void QmitkDicomEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
    this->SetSite(site);
    this->SetInput(input);
}

void QmitkDicomEditor::SetFocus()
{
}

berry::IPartListener::Events::Types QmitkDicomEditor::GetPartEventTypes() const
{
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkDicomEditor::OnQueryRetrieve()
{
    OnChangePage(2);
}

void QmitkDicomEditor::OnLocalStorage()
{
    OnChangePage(0);
}

void QmitkDicomEditor::OnChangePage(int page)
{
    try{
        m_Controls.stackedWidget->setCurrentIndex(page);
    }catch(std::exception e){
        MITK_ERROR <<"error: "<< e.what();
        return;
    }
}

void QmitkDicomEditor::OnDicomImportFinished(const QString& path)
{
}

void QmitkDicomEditor::OnDicomImportFinished(const QStringList& path)
{
}

void QmitkDicomEditor::StartDicomDirectoryListener(QString& directory)
{   
    m_DicomDirectoryListener->SetDicomListenerDirectory(directory);
    connect(m_DicomDirectoryListener,SIGNAL(SignalAddDicomData(const QStringList&)),m_Controls.internalDataWidget,SLOT(StartDicomImport(const QStringList&)),Qt::DirectConnection);
    connect(m_Controls.internalDataWidget,SIGNAL(FinishedImport(const QStringList&)),m_DicomDirectoryListener,SLOT(OnDicomImportFinished(const QStringList&)),Qt::DirectConnection);
    m_DicomDirectoryListener->moveToThread(m_Thread);
    m_Thread->start();   
}

void QmitkDicomEditor::SetupDefaults()
{
    QString pluginDirectory;
    mitk::PluginActivator::getContext()->getDataFile(pluginDirectory);
    pluginDirectory.append("/");
    QString databaseDirectory;
    //databaseDirectory.append(pluginDirectory);
    databaseDirectory.append(QString("C:/DicomDatabase"));
    QDir tmp(databaseDirectory);
    tmp.mkpath(databaseDirectory);
    m_Controls.internalDataWidget->SetDatabaseDirectory(databaseDirectory);
    QString listenerDirectory("C:/DICOMListenerDirectory");
    StartDicomDirectoryListener(listenerDirectory);
    QmitkStoreSCPLauncherBuilder builder;
    builder.AddPort()->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(listenerDirectory);
    m_StoreSCPLauncher = new QmitkStoreSCPLauncher(&builder);
    m_StoreSCPLauncher->StartStoreSCP();
}