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
#include <qinputdialog.h>
#include <qmessagebox.h>

//poco headers
#include <Poco/Path.h>

const std::string QmitkNavigationToolManagementWidget::VIEW_ID = "org.mitk.views.navigationtoolmanagementwidget";

QmitkNavigationToolManagementWidget::QmitkNavigationToolManagementWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
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

    //Disable StorageControls in the beginning, because there is no storage to edit
    DisableStorageControls();
  }

void QmitkNavigationToolManagementWidget::OnLoadTool()
{
  if(m_NavigationToolStorage->isLocked())
  {
    MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
   return;
  }
  mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
  std::string filename = QFileDialog::getOpenFileName(NULL,tr("Add Navigation Tool"), "/", "*.IGTTool").toLatin1().data();
  if (filename == "") return;
  mitk::NavigationTool::Pointer readTool = myReader->DoRead(filename);
  if (readTool.IsNull()) MessageBox("Error: " + myReader->GetErrorMessage());
  else
    {
    if (!m_NavigationToolStorage->AddTool(readTool))
      {
      MessageBox("Error: Can't add tool!");
      m_DataStorage->Remove(readTool->GetDataNode());
      }
    UpdateToolTable();
    }
}

void QmitkNavigationToolManagementWidget::OnSaveTool()
{
    //if no item is selected, show error message:
    if (m_Controls->m_ToolList->currentItem() == NULL) {MessageBox("Error: Please select tool first!");return;}

    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = QFileDialog::getSaveFileName(NULL,tr("Save Navigation Tool"), "/", "*.IGTTool").toLatin1().data();
    if (filename == "") return;
    if (!myWriter->DoWrite(filename,m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row())))
      MessageBox("Error: "+ myWriter->GetErrorMessage());

}

void QmitkNavigationToolManagementWidget::CreateConnections()
  {
    if ( m_Controls )
    {
      //main widget page:
      connect( (QObject*)(m_Controls->m_AddTool), SIGNAL(clicked()), this, SLOT(OnAddTool()) );
      connect( (QObject*)(m_Controls->m_DeleteTool), SIGNAL(clicked()), this, SLOT(OnDeleteTool()) );
      connect( (QObject*)(m_Controls->m_EditTool), SIGNAL(clicked()), this, SLOT(OnEditTool()) );
      connect( (QObject*)(m_Controls->m_LoadStorage), SIGNAL(clicked()), this, SLOT(OnLoadStorage()) );
      connect( (QObject*)(m_Controls->m_SaveStorage), SIGNAL(clicked()), this, SLOT(OnSaveStorage()) );
      connect( (QObject*)(m_Controls->m_LoadTool), SIGNAL(clicked()), this, SLOT(OnLoadTool()) );
      connect( (QObject*)(m_Controls->m_SaveTool), SIGNAL(clicked()), this, SLOT(OnSaveTool()) );
      connect( (QObject*)(m_Controls->m_CreateNewStorage), SIGNAL(clicked()), this, SLOT(OnCreateStorage()) );


      //widget page "add tool":
      connect( (QObject*)(m_Controls->m_ToolCreationWidget), SIGNAL(Canceled()), this, SLOT(OnAddToolCancel()) );
      connect( (QObject*)(m_Controls->m_ToolCreationWidget), SIGNAL(NavigationToolFinished()), this, SLOT(OnAddToolSave()) );
    }
  }

void QmitkNavigationToolManagementWidget::Initialize(mitk::DataStorage* dataStorage)
  {
  m_DataStorage = dataStorage;
  m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage,"Tool0");
  }

void QmitkNavigationToolManagementWidget::LoadStorage(mitk::NavigationToolStorage::Pointer storageToLoad)
  {
  if(storageToLoad.IsNotNull())
    {
    m_NavigationToolStorage = storageToLoad;
    m_Controls->m_StorageName->setText(m_NavigationToolStorage->GetName().c_str());
    EnableStorageControls();
    }
  else
    {
    m_NavigationToolStorage = NULL;
    DisableStorageControls();
    }
  UpdateToolTable();
  }

//##################################################################################
//############################## slots: main widget ################################
//##################################################################################

void QmitkNavigationToolManagementWidget::OnAddTool()
  {
    if(m_NavigationToolStorage->isLocked())
    {
      MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
      return;
    }
    QString defaultIdentifier = "NavigationTool#"+QString::number(m_NavigationToolStorage->GetToolCount());
    m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage,defaultIdentifier.toStdString());
    m_edit = false;
    m_Controls->m_MainWidgets->setCurrentIndex(1);
  }

void QmitkNavigationToolManagementWidget::OnDeleteTool()
  {
    //first: some checks
    if(m_NavigationToolStorage->isLocked())
    {
      MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
      return;
    }
    else if (m_Controls->m_ToolList->currentItem() == NULL) //if no item is selected, show error message:
    {
      MessageBox("Error: Please select tool first!");
      return;
    }

    m_DataStorage->Remove(m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row())->GetDataNode());
    m_NavigationToolStorage->DeleteTool(m_Controls->m_ToolList->currentIndex().row());
    UpdateToolTable();

  }

void QmitkNavigationToolManagementWidget::OnEditTool()
  {
    if(m_NavigationToolStorage->isLocked())
    {
      MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
      return;
    }
    else if (m_Controls->m_ToolList->currentItem() == NULL) //if no item is selected, show error message:
    {
      MessageBox("Error: Please select tool first!");
      return;
    }
    mitk::NavigationTool::Pointer selectedTool = m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row());
    m_Controls->m_ToolCreationWidget->SetDefaultData(selectedTool);
    m_edit = true;
    m_Controls->m_MainWidgets->setCurrentIndex(1);
  }

void QmitkNavigationToolManagementWidget::OnCreateStorage()
  {
    QString storageName = QInputDialog::getText(NULL,"Storage Name","Name of the new tool storage:");
    if (storageName.isNull()) return;
    m_NavigationToolStorage = mitk::NavigationToolStorage::New(this->m_DataStorage);
    m_NavigationToolStorage->SetName(storageName.toStdString());
    m_Controls->m_StorageName->setText(m_NavigationToolStorage->GetName().c_str());
    EnableStorageControls();
    emit NewStorageAdded(m_NavigationToolStorage, storageName.toStdString());
  }

void QmitkNavigationToolManagementWidget::OnLoadStorage()
  {
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(m_DataStorage);
    std::string filename = QFileDialog::getOpenFileName(NULL, tr("Open Navigation Tool Storage"), "/", tr("IGT Tool Storage (*.IGTToolStorage)")).toStdString();
    if (filename == "") return;

    try
    {
      mitk::NavigationToolStorage::Pointer tempStorage = myDeserializer->Deserialize(filename);

      if (tempStorage.IsNull()) MessageBox("Error" + myDeserializer->GetErrorMessage());
      else
      {
        Poco::Path myPath = Poco::Path(filename.c_str());
        tempStorage->SetName(myPath.getFileName()); //set the filename as name for the storage, so the user can identify it
        this->LoadStorage(tempStorage);
        emit NewStorageAdded(m_NavigationToolStorage,myPath.getFileName());
      }
    }
    catch (const mitk::Exception& exception)
    {
      MessageBox(exception.GetDescription());
    }
  }

void QmitkNavigationToolManagementWidget::OnSaveStorage()
  {
    //read in filename
    QString filename = QFileDialog::getSaveFileName(NULL, tr("Save Navigation Tool Storage"), "/", tr("IGT Tool Storage (*.IGTToolStorage)"));
    if (filename.isEmpty()) return; //canceled by the user

    // add file extension if it wasn't added by the file dialog
    if ( filename.right(15) != ".IGTToolStorage" ) { filename += ".IGTToolStorage"; }

    //serialize tool storage
    mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();
    if (!mySerializer->Serialize(filename.toStdString(),m_NavigationToolStorage))
      {
      MessageBox("Error: " + mySerializer->GetErrorMessage());
      return;
      }
    Poco::Path myPath = Poco::Path(filename.toStdString());
    m_Controls->m_StorageName->setText(QString::fromStdString(myPath.getFileName()));

  }


//##################################################################################
//############################## slots: add tool widget ############################
//##################################################################################

void QmitkNavigationToolManagementWidget::OnAddToolSave()
  {
    mitk::NavigationTool::Pointer newTool = m_Controls->m_ToolCreationWidget->GetCreatedTool();

    if (m_edit) //here we edit a existing tool
      {
      mitk::NavigationTool::Pointer editedTool = m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row());
      editedTool->Graft(newTool);
      }
    else //here we create a new tool
      {
      m_NavigationToolStorage->AddTool(newTool);
      }

    UpdateToolTable();

    m_Controls->m_MainWidgets->setCurrentIndex(0);
  }

void QmitkNavigationToolManagementWidget::OnAddToolCancel()
  {
    m_Controls->m_MainWidgets->setCurrentIndex(0);
  }


//##################################################################################
//############################## private help methods ##############################
//##################################################################################
void QmitkNavigationToolManagementWidget::UpdateToolTable()
  {
  m_Controls->m_ToolList->clear();
  if(m_NavigationToolStorage.IsNull()) return;
  for(int i=0; i<m_NavigationToolStorage->GetToolCount(); i++)
    {
      QString currentTool = "Tool" + QString::number(i) + ": " + QString(m_NavigationToolStorage->GetTool(i)->GetDataNode()->GetName().c_str())+ " ";
      switch (m_NavigationToolStorage->GetTool(i)->GetTrackingDeviceType())
        {
        case mitk::ClaronMicron:
              currentTool += "(MicronTracker/"; break;
        case mitk::NDIAurora:
              currentTool += "(NDI Aurora/"; break;
        case mitk::NDIPolaris:
              currentTool += "(NDI Polaris/"; break;
      case mitk::NPOptitrack:
              currentTool += "(NP Optitrack/"; break;
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

void QmitkNavigationToolManagementWidget::DisableStorageControls()
{
  m_Controls->m_StorageName->setText("<none>");
  m_Controls->m_AddTool->setEnabled(false);
  m_Controls->m_LoadTool->setEnabled(false);
  m_Controls->m_selectedLabel->setEnabled(false);
  m_Controls->m_DeleteTool->setEnabled(false);
  m_Controls->m_EditTool->setEnabled(false);
  m_Controls->m_SaveTool->setEnabled(false);
  m_Controls->m_ToolList->setEnabled(false);
  m_Controls->m_SaveStorage->setEnabled(false);
  m_Controls->m_ToolLabel->setEnabled(false);
}

void QmitkNavigationToolManagementWidget::EnableStorageControls()
{
  m_Controls->m_AddTool->setEnabled(true);
  m_Controls->m_LoadTool->setEnabled(true);
  m_Controls->m_selectedLabel->setEnabled(true);
  m_Controls->m_DeleteTool->setEnabled(true);
  m_Controls->m_EditTool->setEnabled(true);
  m_Controls->m_SaveTool->setEnabled(true);
  m_Controls->m_ToolList->setEnabled(true);
  m_Controls->m_SaveStorage->setEnabled(true);
  m_Controls->m_ToolLabel->setEnabled(true);
}
