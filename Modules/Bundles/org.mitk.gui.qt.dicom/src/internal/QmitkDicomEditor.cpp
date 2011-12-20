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

// Qmitk
#include "QmitkDicomEditor.h"
#include "QmitkStdMultiWidget.h"
#include <mitkDicomSeriesReader.h>

// Qt
#include <QMessageBox>
#include <QWidget>
#include <QtSql>
#include <QSqlDatabase>

//CTK
#include <ctkDICOMModel.h>
#include <ctkDICOMAppWidget.h>
#include <ctkDICOMQueryWidget.h>
#include <ctkFileDialog.h>
#include <ctkDICOMQueryRetrieveWidget.h>


const std::string QmitkDicomEditor::EDITOR_ID = "org.mitk.editors.mitkdicomeditor";


QmitkDicomEditor::QmitkDicomEditor()
: QmitkFunctionality()
{
}

QmitkDicomEditor::~QmitkDicomEditor()
{
}

void QmitkDicomEditor::CreateQtPartControl(QWidget *parent )
{   
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi( parent );

    m_Controls.queryRetrieveDockWidget->setVisible(false);
    m_Controls.externalDataDockWidget->setVisible(false);
    //connect( m_Controls.m_ctkDICOMAppWidget, SIGNAL(seriesDoubleClicked( QModelIndex )), this, SLOT(onSeriesModelSelected( QModelIndex )) );
}

void QmitkDicomEditor::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    // iterate all selected objects, adjust warning visibility
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
        it != nodes.end();
        ++it )
    {
        mitk::DataNode::Pointer node = *it;
        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            //m_Controls.labelWarning->setVisible( false );
            //m_Controls.buttonPerformImageProcessing->setEnabled( true );
            return;
        }
    }

    //m_Controls.labelWarning->setVisible( true );
    //m_Controls.buttonPerformImageProcessing->setEnabled( false );
}

void QmitkDicomEditor::onSeriesModelSelected(QModelIndex index){
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


void QmitkDicomEditor::openImportDialog(){

}
void QmitkDicomEditor::openQueryDialog(){

}