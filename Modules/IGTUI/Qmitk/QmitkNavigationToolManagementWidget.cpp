/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNavigationToolManagementWidget.h"

//mitk headers
#include "mitkTrackingTypes.h"
#include <mitkSurface.h>
#include <mitkNavigationToolReader.h>
#include <mitkNavigationToolWriter.h>
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkNavigationToolStorageSerializer.h>
#include <mitkIGTIOException.h>
#include <QmitkIGTCommonHelper.h>

//qt headers
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qsettings.h>

//poco headers
#include <Poco/Path.h>

const std::string QmitkNavigationToolManagementWidget::VIEW_ID = "org.mitk.views.navigationtoolmanagementwidget";

QmitkNavigationToolManagementWidget::QmitkNavigationToolManagementWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = nullptr;
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
  if (m_NavigationToolStorage->isLocked())
  {
    MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
    return;
  }
  mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
  std::string filename = QFileDialog::getOpenFileName(nullptr,tr("Add Navigation Tool"), QmitkIGTCommonHelper::GetLastFileLoadPath(), "*.IGTTool").toLatin1().data();
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));
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
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
  }
}

void QmitkNavigationToolManagementWidget::OnSaveTool()
{
  //if no item is selected, show error message:
  if (m_Controls->m_ToolList->currentItem() == nullptr) { MessageBox("Error: Please select tool first!"); return; }

    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = QFileDialog::getSaveFileName(nullptr,tr("Save Navigation Tool"), QmitkIGTCommonHelper::GetLastFileSavePath(), "*.IGTTool").toLatin1().data();
    QmitkIGTCommonHelper::SetLastFileSavePathByFileName(QString::fromStdString(filename));
    if (filename == "") return;
    std::string fileExt = Poco::Path(filename).getExtension();
    if (fileExt == "") { filename.append(".IGTTool"); }
    if (!myWriter->DoWrite(filename,m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row())))
      MessageBox("Error: "+ myWriter->GetErrorMessage());
}

void QmitkNavigationToolManagementWidget::CreateConnections()
{
  if (m_Controls)
  {
    //main widget page:
    connect((QObject*)(m_Controls->m_AddTool), SIGNAL(clicked()), this, SLOT(OnAddTool()));
    connect((QObject*)(m_Controls->m_DeleteTool), SIGNAL(clicked()), this, SLOT(OnDeleteTool()));
    connect((QObject*)(m_Controls->m_EditTool), SIGNAL(clicked()), this, SLOT(OnEditTool()));
    connect((QObject*)(m_Controls->m_MoveToolUp), SIGNAL(clicked()), this, SLOT(OnMoveToolUp()));
    connect((QObject*)(m_Controls->m_MoveToolDown), SIGNAL(clicked()), this, SLOT(OnMoveToolDown()));
    connect((QObject*)(m_Controls->m_LoadStorage), SIGNAL(clicked()), this, SLOT(OnLoadStorage()));
    connect((QObject*)(m_Controls->m_SaveStorage), SIGNAL(clicked()), this, SLOT(OnSaveStorage()));
    connect((QObject*)(m_Controls->m_LoadTool), SIGNAL(clicked()), this, SLOT(OnLoadTool()));
    connect((QObject*)(m_Controls->m_SaveTool), SIGNAL(clicked()), this, SLOT(OnSaveTool()));
    connect((QObject*)(m_Controls->m_CreateNewStorage), SIGNAL(clicked()), this, SLOT(OnCreateStorage()));

    connect((QObject*)(m_Controls->m_ToolList), SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(OnToolSelected()));

    //widget page "add tool":
    connect((QObject*)(m_Controls->m_ToolCreationWidget), SIGNAL(Canceled()), this, SLOT(OnAddToolCancel()));
    connect((QObject*)(m_Controls->m_ToolCreationWidget), SIGNAL(NavigationToolFinished()), this, SLOT(OnAddToolSave()));
  }
}

void QmitkNavigationToolManagementWidget::Initialize(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
  m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage, "Tool0");
}

void QmitkNavigationToolManagementWidget::LoadStorage(mitk::NavigationToolStorage::Pointer storageToLoad)
{
  if (storageToLoad.IsNotNull())
  {
    m_NavigationToolStorage = storageToLoad;
    m_Controls->m_StorageName->setText(m_NavigationToolStorage->GetName().c_str());
    EnableStorageControls();
  }
  else
  {
    m_NavigationToolStorage = nullptr;
    DisableStorageControls();
  }
  UpdateToolTable();
}

//##################################################################################
//############################## slots: main widget ################################
//##################################################################################

void QmitkNavigationToolManagementWidget::OnMoveToolUp()
{
  if (m_NavigationToolStorage.IsNotNull())
  {
    //Proof, if the NavigationToolStorage is locked. If it is locked, show MessageBox to user.
    if( m_NavigationToolStorage->isLocked() )
    {
      MessageBox("The storage is locked and it cannot be modified. Maybe the tracking device which " \
        "uses this storage is connected. If you want to modify the storage please " \
        "disconnect the device first.");
      return;
    }
    int toolIndex = m_Controls->m_ToolList->currentIndex().row();
    if (toolIndex >= 0)
    {
      mitk::NavigationTool::Pointer currentNavigationTool = m_NavigationToolStorage->GetTool(toolIndex);
      if (currentNavigationTool.IsNotNull())
      {
        std::string currentIdentifier = currentNavigationTool->GetIdentifier();
        int NewNumber = m_Controls->m_ToolList->currentIndex().row() - 1;
        if (NewNumber < 0) { MITK_WARN << "Cannot move tool up because it is on the top!"; }
        else { m_NavigationToolStorage->AssignToolNumber(currentIdentifier, NewNumber); }
        UpdateToolTable();
      }
    }
  }
}

void QmitkNavigationToolManagementWidget::OnMoveToolDown()
{
  if (m_NavigationToolStorage.IsNotNull())
  {
    //Proof, if the NavigationToolStorage is locked. If it is locked, show MessageBox to user.
    if( m_NavigationToolStorage->isLocked() )
    {
      MessageBox("The storage is locked and it cannot be modified. Maybe the tracking device which " \
        "uses this storage is connected. If you want to modify the storage please " \
        "disconnect the device first.");
      return;
    }
    int toolIndex = m_Controls->m_ToolList->currentIndex().row();
    if (toolIndex >= 0)
    {
      mitk::NavigationTool::Pointer currentNavigationTool = m_NavigationToolStorage->GetTool(toolIndex);
      if (currentNavigationTool.IsNotNull())
      {
        std::string currentIdentifier = currentNavigationTool->GetIdentifier();
        unsigned int NewNumber = m_Controls->m_ToolList->currentIndex().row() + 1;
        if (NewNumber >= m_NavigationToolStorage->GetToolCount()) { MITK_WARN << "Cannot move tool down because it is the last tool in this storage!"; }
        else { m_NavigationToolStorage->AssignToolNumber(currentIdentifier, NewNumber); }
        UpdateToolTable();
      }
    }
  }
}

void QmitkNavigationToolManagementWidget::OnAddTool()
{
  if (m_NavigationToolStorage->isLocked())
  {
    MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
    return;
  }
  QString defaultIdentifier = "NavigationTool#" + QString::number(m_NavigationToolStorage->GetToolCount());
  QString defaultName = "NavigationTool" + QString::number(m_NavigationToolStorage->GetToolCount());
  m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage, defaultIdentifier.toStdString(), defaultName.toStdString());
  m_Controls->m_ToolCreationWidget->ShowToolPreview("Tool preview");
  m_edit = false;
  m_Controls->m_MainWidgets->setCurrentIndex(1);
}

void QmitkNavigationToolManagementWidget::OnDeleteTool()
{
  //first: some checks
  if (m_NavigationToolStorage->isLocked())
  {
    MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
    return;
  }
  else if (m_Controls->m_ToolList->currentItem() == nullptr) //if no item is selected, show error message:
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
  if (m_NavigationToolStorage->isLocked())
  {
    MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
    return;
  }
  else if (m_Controls->m_ToolList->currentItem() == nullptr) //if no item is selected, show error message:
  {
    MessageBox("Error: Please select tool first!");
    return;
  }
  mitk::NavigationTool::Pointer selectedTool = m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row());
  m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage, "", ""); //Initialize once again, might be called here for the first time after autodetection
  m_Controls->m_ToolCreationWidget->SetDefaultData(selectedTool);
  m_Controls->m_ToolCreationWidget->ShowToolPreview("Tool preview");
  m_edit = true;
  m_Controls->m_MainWidgets->setCurrentIndex(1);
}

void QmitkNavigationToolManagementWidget::OnCreateStorage()
{
  QString storageName = QInputDialog::getText(nullptr, "Storage Name", "Name of the new tool storage:");
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
  std::string filename = QFileDialog::getOpenFileName(nullptr, tr("Open Navigation Tool Storage"), QmitkIGTCommonHelper::GetLastFileLoadPath(), tr("IGT Tool Storage (*.IGTToolStorage)")).toStdString();
  if (filename == "") return;

  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));

  mitk::NavigationToolStorage::Pointer tempStorage = nullptr;

  try
  {
    tempStorage = myDeserializer->Deserialize(filename);
  }
  catch (const mitk::Exception& exception)
  {
    MessageBox(exception.GetDescription());
  }

  if (tempStorage.IsNotNull())
  {
    Poco::Path myPath = Poco::Path(filename.c_str());
    tempStorage->SetName(myPath.getFileName()); //set the filename as name for the storage, so the user can identify it
    this->LoadStorage(tempStorage);
    emit NewStorageAdded(m_NavigationToolStorage,myPath.getFileName());
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
  }
}

void QmitkNavigationToolManagementWidget::OnSaveStorage()
{
  QFileDialog *fileDialog = new QFileDialog;
  fileDialog->setDefaultSuffix("IGTToolStorage");
  QString suffix = "IGT Tool Storage (*.IGTToolStorage)";
  // Set default file name to LastFileSavePath + storage name
  QString defaultFileName = QmitkIGTCommonHelper::GetLastFileSavePath() + "/" + QString::fromStdString(m_NavigationToolStorage->GetName());
  QString filename  = fileDialog->getSaveFileName(nullptr, tr("Save Navigation Tool Storage"), defaultFileName, suffix, &suffix);

  if (filename.isEmpty()) return; //canceled by the user

  // check file suffix
  QFileInfo file(filename);
  if(file.suffix().isEmpty()) filename += ".IGTToolStorage";

  //serialize tool storage
  mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();

  try
  {
    mySerializer->Serialize(filename.toStdString(), m_NavigationToolStorage);
  }
  catch (const mitk::IGTIOException& e)
  {
    MessageBox("Error: " + std::string(e.GetDescription()));
    return;
  }

  QmitkIGTCommonHelper::SetLastFileSavePath(file.absolutePath());
  Poco::Path myPath = Poco::Path(filename.toStdString());
  m_Controls->m_StorageName->setText(QString::fromStdString(myPath.getFileName()));
}

//##################################################################################
//############################## slots: add tool widget ############################
//##################################################################################

void QmitkNavigationToolManagementWidget::OnAddToolSave()
{
  //Proof, if the NavigationToolStorage is locked. If it is locked, show MessageBox to user.
  if( m_NavigationToolStorage->isLocked() )
  {
    MessageBox( "The storage is locked and it cannot be modified. Maybe the tracking device which " \
                "uses this storage is connected. If you want to modify the storage please " \
                "disconnect the device first.");
    return;
  }

  mitk::NavigationTool::Pointer newTool = m_Controls->m_ToolCreationWidget->GetCreatedTool();

  if (m_edit) //here we edit a existing tool
  {
    mitk::NavigationTool::Pointer editedTool = m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row());
    editedTool->Graft(newTool);
    //Keep this line, 'cause otherwise, the NavigationToolStorage wouldn't notice, that the toolStorage changed.
    m_NavigationToolStorage->UpdateMicroservice();
  }
  else //here we create a new tool
  {
    m_NavigationToolStorage->AddTool(newTool);
  }

  //Remove tool preview
  m_DataStorage->Remove(m_DataStorage->GetNamedNode("Tool preview"));

  UpdateToolTable();

  m_Controls->m_MainWidgets->setCurrentIndex(0);

  m_Controls->m_ToolInformation->setText("");
}

void QmitkNavigationToolManagementWidget::OnAddToolCancel()
{
  m_Controls->m_MainWidgets->setCurrentIndex(0);
  //Remove tool preview
  m_DataStorage->Remove(m_DataStorage->GetNamedNode("Tool preview"));
}

void QmitkNavigationToolManagementWidget::OnToolSelected()
{
  QString _label = "Information for tool " + m_Controls->m_ToolList->currentItem()->text() + "\n";
  _label.append(QString(m_NavigationToolStorage->GetTool(m_Controls->m_ToolList->currentIndex().row())->GetStringWithAllToolInformation().c_str()));
  m_Controls->m_ToolInformation->setText(_label);
}

//##################################################################################
//############################## private help methods ##############################
//##################################################################################
void QmitkNavigationToolManagementWidget::UpdateToolTable()
{
  m_Controls->m_ToolList->clear();
  if (m_NavigationToolStorage.IsNull()) return;
  for (unsigned int i = 0; i < m_NavigationToolStorage->GetToolCount(); i++)
  {
    QString currentTool = "Tool" + QString::number(i) + ": " + QString(m_NavigationToolStorage->GetTool(i)->GetToolName().c_str()) + " ";

    currentTool += "(" + QString::fromStdString(m_NavigationToolStorage->GetTool(i)->GetTrackingDeviceType()) + "/";

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

void QmitkNavigationToolManagementWidget::MessageBox(const std::string& s)
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
  m_Controls->m_MoveToolUp->setEnabled(false);
  m_Controls->m_MoveToolDown->setEnabled(false);
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
  m_Controls->m_MoveToolUp->setEnabled(true);
  m_Controls->m_MoveToolDown->setEnabled(true);
  m_Controls->m_selectedLabel->setEnabled(true);
  m_Controls->m_DeleteTool->setEnabled(true);
  m_Controls->m_EditTool->setEnabled(true);
  m_Controls->m_SaveTool->setEnabled(true);
  m_Controls->m_ToolList->setEnabled(true);
  m_Controls->m_SaveStorage->setEnabled(true);
  m_Controls->m_ToolLabel->setEnabled(true);
}
