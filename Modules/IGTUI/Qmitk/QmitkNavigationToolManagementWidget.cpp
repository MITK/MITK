/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: QmitkTrackingDeviceWidget.cpp $
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkNavigationToolManagementWidget.h"

//mitk headers
#include "mitkTrackingTypes.h"
#include <mitkSTLFileReader.h>
#include <mitkSurface.h>
#include <mitkNavigationToolReader.h>
#include <mitkNavigationToolWriter.h>
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkNavigationToolStorageSerializer.h>

//qt headers
#include <qfiledialog.h>
#include <qmessagebox.h>

const std::string QmitkNavigationToolManagementWidget::VIEW_ID = "org.mitk.views.navigationtoolmanagementwidget";

QmitkNavigationToolManagementWidget::QmitkNavigationToolManagementWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_NavigationToolStorage = mitk::NavigationToolStorage::New();
  m_Controls->m_SurfaceChooser->SetAutoSelectNewItems(true);
}


QmitkNavigationToolManagementWidget::~QmitkNavigationToolManagementWidget()
{
}

void QmitkNavigationToolManagementWidget::CreateQtPartControl(QWidget *parent)
  {
    if (!m_Controls)
    {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationToolManagementWidgetControls;
    m_Controls->setupUi(parent);
    }
  }

void QmitkNavigationToolManagementWidget::CreateConnections()
  {
    if ( m_Controls )
    {
      //main widget page:
      connect( (QObject*)(m_Controls->m_AddTool), SIGNAL(clicked()), this, SLOT(OnAddTool()) );
      connect( (QObject*)(m_Controls->m_DeleteTool), SIGNAL(clicked()), this, SLOT(OnDeleteTool()) );
      connect( (QObject*)(m_Controls->m_EditTool), SIGNAL(clicked()), this, SLOT(OnEditTool()) );
      connect( (QObject*)(m_Controls->m_LoadSingleTool), SIGNAL(clicked()), this, SLOT(OnLoadSingleTool()) );
      connect( (QObject*)(m_Controls->m_SaveSingleTool), SIGNAL(clicked()), this, SLOT(OnSaveSingleTool()) );
      connect( (QObject*)(m_Controls->m_LoadStorage), SIGNAL(clicked()), this, SLOT(OnLoadStorage()) );
      connect( (QObject*)(m_Controls->m_SaveStorage), SIGNAL(clicked()), this, SLOT(OnSaveStorage()) );
          
      //widget page "add tool":
      connect( (QObject*)(m_Controls->m_AddToolCancel), SIGNAL(clicked()), this, SLOT(OnAddToolCancel()) );
      connect( (QObject*)(m_Controls->m_AddToolSave), SIGNAL(clicked()), this, SLOT(OnAddToolSave()) );
      connect( (QObject*)(m_Controls->m_LoadSurface), SIGNAL(clicked()), this, SLOT(OnLoadSurface()) );
      connect( (QObject*)(m_Controls->m_LoadCalibrationFile), SIGNAL(clicked()), this, SLOT(OnLoadCalibrationFile()) );
    }
  }

void QmitkNavigationToolManagementWidget::Initialize(mitk::DataStorage* dataStorage)
  {
  m_DataStorage = dataStorage;
  }

//##################################################################################
//############################## slots: main widget ################################
//##################################################################################

void QmitkNavigationToolManagementWidget::OnAddTool()
  {
    //initialize UI components
    m_Controls->m_SurfaceChooser->SetDataStorage(m_DataStorage);
    m_Controls->AddToolLabel->setText("<b>Add Tool:</b>");
    m_Controls->m_MainWidgets->setCurrentIndex(1);

    //reset input fields
    m_Controls->m_ToolNameEdit->setText("");
    m_Controls->m_IdentifierEdit->setText("NavigationTool#"+QString::number(m_NavigationToolStorage->GetToolCount()));
    m_Controls->m_SerialNumberEdit->setText("");
    m_Controls->m_CalibrationFileName->setText("");   

    m_edit = false;
  }

void QmitkNavigationToolManagementWidget::OnDeleteTool()
  {
    //if no item is selected, show error message:
    if (m_Controls->m_ToolList->currentItem() == NULL) {MessageBox("Error: Please select tool first!");return;}

    m_DataStorage->Remove(m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row())->GetDataTreeNode());
    m_NavigationToolStorage->DeleteTool(m_Controls->m_ToolList->currentIndex().row());
    UpdateToolTable();

  }

void QmitkNavigationToolManagementWidget::OnEditTool()
  {
    //if no item is selected, show error message:
    if (m_Controls->m_ToolList->currentItem() == NULL) {MessageBox("Error: Please select tool first!");return;}

    //initialize UI components
    m_Controls->m_SurfaceChooser->SetDataStorage(m_DataStorage);
    m_Controls->AddToolLabel->setText("<b>Edit Tool:</b>");
    m_Controls->m_MainWidgets->setCurrentIndex(1);

    //fill forms
    mitk::NavigationTool::Pointer selectedTool = m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row());
    m_Controls->m_ToolNameEdit->setText(QString(selectedTool->GetDataTreeNode()->GetName().c_str()));
    m_Controls->m_IdentifierEdit->setText(QString(selectedTool->GetIdentifier().c_str()));
    m_Controls->m_SerialNumberEdit->setText(QString(selectedTool->GetSerialNumber().c_str()));
    switch(selectedTool->GetTrackingDeviceType())
      {
      case mitk::NDIAurora:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(0);break;
      case mitk::NDIPolaris:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(1);break;
      case mitk::ClaronMicron:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(2);break;
      default:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(0);
      }
    m_Controls->m_CalibrationFileName->setText(QString(selectedTool->GetCalibrationFile().c_str()));
    switch(selectedTool->GetType())
      {
      case mitk::NavigationTool::Instrument:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(0); break;
      case mitk::NavigationTool::Fiducial:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(1); break;
      case mitk::NavigationTool::Skinmarker:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(2); break;
      case mitk::NavigationTool::Unknown:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(3); break;
      }

    m_Controls->m_SurfaceChooser->SetSelectedNode(selectedTool->GetDataTreeNode());
    m_edit = true;
  }

void QmitkNavigationToolManagementWidget::OnLoadSingleTool()
  {
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New(m_DataStorage);
    mitk::NavigationTool::Pointer readTool = myReader->DoRead(QFileDialog::getOpenFileName(NULL,tr("Open Navigation Tool"), "/", "*.*").toAscii().data());
    if (readTool.IsNull()) MessageBox("Error: " + myReader->GetErrorMessage());
    else 
      { 
      if (!m_NavigationToolStorage->AddTool(readTool))
        {
        MessageBox("Error: Can't add tool!");
        m_DataStorage->Remove(readTool->GetDataTreeNode());
        }
      UpdateToolTable();
      }
  }

void QmitkNavigationToolManagementWidget::OnSaveSingleTool()
  {
    //if no item is selected, show error message:
    if (m_Controls->m_ToolList->currentItem() == NULL) {MessageBox("Error: Please select tool first!");return;}

    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    if (!myWriter->DoWrite(QFileDialog::getSaveFileName(NULL,tr("Save Navigation Tool"), "/", "*.*").toAscii().data(),m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row()))) 
      MessageBox("Error: "+ myWriter->GetErrorMessage());
  }

void QmitkNavigationToolManagementWidget::OnLoadStorage()
  {
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(m_DataStorage);
    std::string filename = QFileDialog::getOpenFileName(NULL,tr("Open Navigation Tool"), "/", "*.*").toAscii().data();
    mitk::NavigationToolStorage::Pointer tempStorage = myDeserializer->Deserialize(filename);
    if (tempStorage.IsNull()) MessageBox("Error" + myDeserializer->GetErrorMessage());
    else 
      {
      m_NavigationToolStorage = tempStorage;
      m_Controls->m_StorageName->setText(filename.c_str());
      }
    UpdateToolTable();
  }

void QmitkNavigationToolManagementWidget::OnSaveStorage()
  {
    mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();
    std::string filename = QFileDialog::getSaveFileName(NULL,tr("Save Navigation Tool"), "/", "*.*").toAscii().data();
    if (!mySerializer->Serialize(filename,m_NavigationToolStorage)) MessageBox("Error: " + mySerializer->GetErrorMessage());
    else m_Controls->m_StorageName->setText(QString(filename.c_str()));
  }


//##################################################################################
//############################## slots: add tool widget ############################
//##################################################################################

void QmitkNavigationToolManagementWidget::OnAddToolSave()
  {
    mitk::NavigationTool::Pointer workTool;
   
    if (m_edit) //here we edit a existing tool
      {
      workTool = m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row());
      
      //edit existing DataTreeNode...
      workTool->GetDataTreeNode()->SetName(m_Controls->m_ToolNameEdit->text().toLatin1());
      workTool->GetDataTreeNode()->SetData(m_Controls->m_SurfaceChooser->GetSelectedNode()->GetData());
      }
    else //here we create a new tool
      {
      workTool = mitk::NavigationTool::New();
    
      //create DataTreeNode...
      mitk::DataTreeNode::Pointer newNode = mitk::DataTreeNode::New();
      newNode->SetName(m_Controls->m_ToolNameEdit->text().toLatin1());
      newNode->SetData(m_Controls->m_SurfaceChooser->GetSelectedNode()->GetData());
      m_DataStorage->Add(newNode);
      workTool->SetDataTreeNode(newNode);
      }

    //fill NavigationTool object
    workTool->SetCalibrationFile(m_Controls->m_CalibrationFileName->text().toAscii().data());
    workTool->SetIdentifier(m_Controls->m_IdentifierEdit->text().toAscii().data());
    workTool->SetSerialNumber(m_Controls->m_SerialNumberEdit->text().toAscii().data());
    //Tracking Device
    if (m_Controls->m_TrackingDeviceTypeChooser->currentText()=="NDI Aurora") workTool->SetTrackingDeviceType(mitk::NDIAurora);
    else if (m_Controls->m_TrackingDeviceTypeChooser->currentText()=="NDI Polaris") workTool->SetTrackingDeviceType(mitk::NDIPolaris);
    else if (m_Controls->m_TrackingDeviceTypeChooser->currentText()=="Claron Technology Micron Tracker") workTool->SetTrackingDeviceType(mitk::ClaronMicron);
    else workTool->SetTrackingDeviceType(mitk::TrackingSystemNotSpecified);
    //ToolType
    if (m_Controls->m_ToolTypeChooser->currentText()=="Instrument") workTool->SetType(mitk::NavigationTool::Instrument);
    else if (m_Controls->m_ToolTypeChooser->currentText()=="Fiducial") workTool->SetType(mitk::NavigationTool::Fiducial);
    else if (m_Controls->m_ToolTypeChooser->currentText()=="Skinmarker") workTool->SetType(mitk::NavigationTool::Skinmarker);
    else workTool->SetType(mitk::NavigationTool::Unknown);

    if (!m_edit) m_NavigationToolStorage->AddTool(workTool);

    UpdateToolTable();

    m_Controls->m_MainWidgets->setCurrentIndex(0);
  }

void QmitkNavigationToolManagementWidget::OnAddToolCancel()
  {
    m_Controls->m_MainWidgets->setCurrentIndex(0);
  }

void QmitkNavigationToolManagementWidget::OnLoadSurface()
  {
    std::string filename = QFileDialog::getOpenFileName(NULL,tr("Open Surface"), "/", "*.stl").toLatin1().data();
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    try
      {
      stlReader->SetFileName( filename.c_str() );
      stlReader->Update();
      }
    catch (...)
      {
      }

    if ( stlReader->GetOutput() == NULL );
    else
      {
      mitk::DataTreeNode::Pointer newNode = mitk::DataTreeNode::New();
      newNode->SetName(filename);
      newNode->SetData(stlReader->GetOutput());
      m_DataStorage->Add(newNode);
      }
  }

void QmitkNavigationToolManagementWidget::OnLoadCalibrationFile()
  {
    m_Controls->m_CalibrationFileName->setText(QFileDialog::getOpenFileName(NULL,tr("Open Calibration File"), "/", "*.*"));
  }



//##################################################################################
//############################## private help methods ##############################
//##################################################################################
void QmitkNavigationToolManagementWidget::UpdateToolTable()
  {
  m_Controls->m_ToolList->clear();
  for(int i=0; i<m_NavigationToolStorage->GetToolCount(); i++)
    {
      QString currentTool = "Tool" + QString::number(i) + ": " + QString(m_NavigationToolStorage->GetTool(i)->GetDataTreeNode()->GetName().c_str())+ " ";
      switch (m_NavigationToolStorage->GetTool(i)->GetTrackingDeviceType())
        {
        case mitk::ClaronMicron:
              currentTool += "(MicronTracker/"; break;
        case mitk::NDIAurora:
              currentTool += "(NDI Aurora/"; break;
        case mitk::NDIPolaris:
              currentTool += "(NDI Polaris/"; break;
        default:
              currentTool += "(unknown tracking system/"; break;
        }
      switch (m_NavigationToolStorage->GetTool(i)->GetType())
        {
        case mitk::NavigationTool::Instrument:
              currentTool += "Instrument)"; break;
        case mitk::NavigationTool::Fiducial:
              currentTool += "Fiducial)"; break;
        case mitk::NavigationTool::Skinmarker:
              currentTool += "Skinmarker)"; break;
        default:
              currentTool += "Unknown)";
        }
      m_Controls->m_ToolList->addItem(currentTool);
    }

  }

void QmitkNavigationToolManagementWidget::MessageBox(std::string s)
  {
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
  }