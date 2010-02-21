/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkNDIConfigurationWidget.h"
#include <QTableWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <QFileInfo>

#include "QmitkCustomVariants.h"

//#include <QtConcurrentMap>
#include "QmitkNDIToolDelegate.h"

/* VIEW MANAGEMENT */
QmitkNDIConfigurationWidget::QmitkNDIConfigurationWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL), m_Tracker(NULL), m_Source(NULL),
m_Delegate(NULL)
{
  this->CreateQtPartControl(this);
}


QmitkNDIConfigurationWidget::~QmitkNDIConfigurationWidget()
{
  m_Controls = NULL;
  m_Tracker = NULL;
  m_Source = NULL;
}


void QmitkNDIConfigurationWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNDIConfigurationWidget;
    m_Controls->setupUi(parent);
    QStringList comPorts;
#ifdef WIN32
    comPorts << "COM1" << "COM2" << "COM3" << "COM4" << "COM5" << "COM6" << "COM7" << "COM8" << "COM9";
#else
    comPorts << "/dev/ttyS1" << "/dev/ttyS2" << "/dev/ttyS3" << "/dev/ttyS4" << "/dev/ttyS5" << "/dev/ttyUSB0" << "/dev/ttyUSB1" << "/dev/ttyUSB2" << "/dev/ttyUSB3";
#endif
    m_Controls->m_ComPortSelector->addItems(comPorts);
    m_Delegate = new QmitkNDIToolDelegate(m_Controls->m_ToolTable);
    m_Delegate->SetDataStorage(NULL);  //needs to be set later using the setter methods
    m_Delegate->SetPredicate(NULL);
    m_Delegate->SetTypes(QStringList());
    m_Controls->m_ToolTable->setItemDelegate(m_Delegate);
    this->CreateConnections();
  }
}


void QmitkNDIConfigurationWidget::CreateConnections()
{
  connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
  connect(m_Controls->m_DiscoverToolsBtn, SIGNAL(clicked()), this, SLOT(OnDiscoverTools()));
  connect(m_Controls->m_AddToolBtn, SIGNAL(clicked()), this, SLOT(OnAddPassiveTool()));
  connect(m_Controls->m_DisoverDevicesBtn, SIGNAL(clicked()), this, SLOT(OnDiscoverDevices()));
  connect(m_Controls->m_ToolTable->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(UpdateTrackerFromToolTable(const QModelIndex &, const QModelIndex &)));
}


void QmitkNDIConfigurationWidget::OnConnect()
{
  if (m_Tracker.IsNotNull())
  {
    m_Tracker->CloseConnection();
    m_Tracker = NULL;
  }
  this->CreateTracker();

  this->SetupTracker();
  bool okay = m_Tracker->OpenConnection();
  if (okay)
  {
    this->UpdateWidgets();
    this->UpdateToolTable();
    emit ToolsAdded(this->GetToolNamesList());
    emit Connected();
  }
  else
  {
    QMessageBox::warning(NULL, "Error", QString("Connection failed. Tracking device error message is '%1'").arg(m_Tracker->GetErrorMessage()));
  }
}


void QmitkNDIConfigurationWidget::OnDisconnect()
{
  if (m_Tracker.IsNull())
    return;
  m_Tracker->CloseConnection();
  m_Tracker = NULL;
  
  this->UpdateToolTable();
  this->UpdateWidgets();
  emit ToolsAdded(this->GetToolNamesList());
  emit Disconnected();
}


void QmitkNDIConfigurationWidget::UpdateWidgets()
{
  m_Controls->m_DeviceStatus->setText(this->GetStatusText());
  if (m_Tracker.IsNull())  // not connected to tracker
  {
    m_Controls->m_Connect->setText("Connect");
    disconnect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnDisconnect()));
    connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
    m_Controls->m_DiscoverToolsBtn->setDisabled(true);
    m_Controls->m_AddToolBtn->setDisabled(true);
    return;
  }

  if (m_Tracker->GetState() == mitk::TrackingDevice::Setup)
  {
    m_Controls->m_Connect->setText("Connect");
    disconnect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnDisconnect()));
    connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
    m_Controls->m_DiscoverToolsBtn->setDisabled(true);
    m_Controls->m_AddToolBtn->setDisabled(true);
    return;
  }
  if ((m_Tracker->GetState() == mitk::TrackingDevice::Ready) || (m_Tracker->GetState() == mitk::TrackingDevice::Tracking))
  {
    m_Controls->m_Connect->setText("Disconnect");
    disconnect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
    connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnDisconnect()));
    m_Controls->m_DiscoverToolsBtn->setEnabled(true);
    m_Controls->m_AddToolBtn->setEnabled(true);    
  }  
}


QString QmitkNDIConfigurationWidget::GetStatusText()
{
  if (m_Tracker.IsNull())
    return QString("Not connected");

  QString devName;
  switch (m_Tracker->GetType())
  {
  case mitk::NDIAurora:
    devName = "NDI Aurora";
    break;
  case mitk::NDIPolaris:
    devName = "NDI Polaris";
    break;
  case mitk::TrackingSystemNotSpecified:
  default:
    devName = "unknown tracking device";
    break;
  }
  if (m_Tracker->GetState() == mitk::TrackingDevice::Ready)
    return QString("Connected to %1 on %2. Device is ready.").arg(devName).arg(m_Tracker->GetDeviceName());
  if (m_Tracker->GetState() == mitk::TrackingDevice::Tracking)
    return QString("%1 is tracking.").arg(devName);
  return QString("");
}


void QmitkNDIConfigurationWidget::OnDiscoverTools()
{
  if (m_Tracker.IsNull())
    return;
  m_Tracker->DiscoverWiredTools();
  this->UpdateToolTable();
  emit ToolsAdded(this->GetToolNamesList());
}


void QmitkNDIConfigurationWidget::OnAddPassiveTool()
{
  if (m_Tracker.IsNull())
    this->CreateTracker();

  QStringList filenames = QFileDialog::getOpenFileNames(this, "Select NDI SROM file", QDir::currentPath(),"NDI SROM files (*.rom)");
  if (filenames.isEmpty())
    return;
  bool ok = false;
  foreach(QString fileName, filenames)
  {
    //QString toolName = QInputDialog::getText(this, "Enter a name for the tool", "Name of the tool: ", QLineEdit::Normal, QFileInfo(filename).baseName(), &ok);
    //if (ok == false || toolName.isEmpty())
    //  return;
    m_Tracker->AddTool(QFileInfo(fileName).baseName().toLatin1(), fileName.toLatin1());
  }
  emit ToolsAdded(this->GetToolNamesList());
  this->UpdateToolTable();
}


void QmitkNDIConfigurationWidget::CreateTracker()
{
  m_Tracker = mitk::NDITrackingDevice::New();
}


void QmitkNDIConfigurationWidget::SetupTracker()
{
  if (m_Tracker.IsNull())
    return;
  m_Tracker->SetDeviceName(this->GetDeviceName());
  m_Tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200);
}


std::string QmitkNDIConfigurationWidget::GetDeviceName() const
{
  if (m_Controls == NULL)
    return NULL;
  return m_Controls->m_ComPortSelector->currentText().toStdString();
}


void QmitkNDIConfigurationWidget::SetDeviceName( const char* dev )
{
  if (m_Controls == NULL)
    return;
  m_Controls->m_ComPortSelector->setCurrentIndex(m_Controls->m_ComPortSelector->findText(dev));
}


void QmitkNDIConfigurationWidget::UpdateToolTable()
{
  disconnect(m_Controls->m_ToolTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(OnTableItemChanged(QTableWidgetItem*))); // stop listening to table changes
  m_Controls->m_ToolTable->clearContents();
  m_Controls->m_ToolTable->setRowCount(0);
  if (m_Tracker.IsNull() || (m_Controls == NULL))
    return;

  m_Controls->m_ToolTable->setRowCount(m_Tracker->GetToolCount());
  for (unsigned int i = 0; i < m_Tracker->GetToolCount(); ++i)
  {
    mitk::TrackingTool* t = m_Tracker->GetTool(i);
    if (t == NULL)
    {
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::IndexCol, new QTableWidgetItem("INVALID"));                   // Index
      continue;
    }
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::IndexCol, new QTableWidgetItem(QString::number(i)));            // Index
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::NameCol, new QTableWidgetItem(t->GetToolName()));               // Name    
    if (dynamic_cast<mitk::NDIPassiveTool*>(t)->GetSROMDataLength() > 0)
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::SROMCol, new QTableWidgetItem("SROM file loaded"));           // SROM file
    else
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::SROMCol, new QTableWidgetItem("<click to load SROM file>"));  // SROM file
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::TypeCol, new QTableWidgetItem("<click to set type>"));          // Type
    if (t->IsEnabled())
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::StatusCol, new QTableWidgetItem("Enabled"));                  // Status
    else
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::StatusCol, new QTableWidgetItem("Disabled"));                 // Status
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::NodeCol, new QTableWidgetItem("<click to select node>"));       // Node
    /* set read-only/editable flags */
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::IndexCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);                        // Index
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::NodeCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // Name
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::SROMCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // SROM file
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::TypeCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // Type
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::StatusCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);                       // Status
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::NodeCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // Node
  }
  m_Controls->m_ToolTable->resizeColumnsToContents();
  connect(m_Controls->m_ToolTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(OnTableItemChanged(QTableWidgetItem*))); // listen to table changes again
}


void QmitkNDIConfigurationWidget::OnDiscoverDevices()
{
  PortDeviceMap portsAndDevices;
  QString status = "Scanning ";
  for (unsigned int i = 1; i < 10; ++i)
  {
    QString devName = QString("COM%1").arg(i);
    portsAndDevices[devName];
    status += devName + ", ";
  }
  status.chop(2); // remove last ", "
  status += " for NDI tracking devices...";
  m_Controls->m_DeviceStatus->setText(status);
  ScanPortsForNDITrackingDevices(portsAndDevices);
  QString result = "The following tracking devices were found:<BR/>\n";
  for (PortDeviceMap::const_iterator it = portsAndDevices.begin(); it != portsAndDevices.end(); ++it)
  {
    result += it.key() + ": ";
    switch (it.value())
    {
    case mitk::NDIPolaris:
      result += "NDI Polaris<BR/>\n";
      break;
    case mitk::NDIAurora:
      result += "NDI Aurora<BR/>\n";
      break;
    default:
      result += "No NDI tracking device found<BR/>\n";
    }
  }
  //QMessageBox::information(NULL, "Tracking Device Discovery", result);
  m_Controls->m_DeviceStatus->setText(result);
}


mitk::TrackingDeviceType QmitkNDIConfigurationWidget::ScanPort(QString port)
{
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  tracker->SetDeviceName(port.toStdString());
  return tracker->TestConnection();
}


void QmitkNDIConfigurationWidget::ScanPortsForNDITrackingDevices( PortDeviceMap& portsAndDevices )
{
  // Iterative scanning:
  for (PortDeviceMap::iterator it = portsAndDevices.begin(); it != portsAndDevices.end(); ++it)
    it.value() = this->ScanPort(it.key());
  
  // \Todo: use parallel scanning
  //QtConcurrent::blockingMap( portsAndDevices.begin(), portsAndDevices.end(), ScanPort );
  //MITK_INFO << portsAndDevices;
}


QStringList QmitkNDIConfigurationWidget::GetToolNamesList()
{
  QStringList toolNames;
  if (m_Tracker.IsNull())
    return toolNames;
  for (unsigned int i = 0; i < m_Tracker->GetToolCount(); ++i)
  {
    mitk::TrackingTool* t = m_Tracker->GetTool(i);
    if (t == NULL)
      continue;
    toolNames << t->GetToolName();
  }
  return toolNames;
}


mitk::NDITrackingDevice* QmitkNDIConfigurationWidget::GetTracker() const
{
  return m_Tracker.GetPointer();
}


void QmitkNDIConfigurationWidget::SetToolTypes(const QStringList& types)
{
  m_Delegate->SetTypes(types);
}


void QmitkNDIConfigurationWidget::SetDataStorage(mitk::DataStorage* ds)
{
  m_Delegate->SetDataStorage(ds);
}


void QmitkNDIConfigurationWidget::SetPredicate(mitk::NodePredicateBase::Pointer p)
{
  m_Delegate->SetPredicate(p);
}


void QmitkNDIConfigurationWidget::SetTagPropertyName( const std::string& name )
{
  m_Delegate->SetTagPropertyName(name);
}


void QmitkNDIConfigurationWidget::SetTagProperty( mitk::BaseProperty::Pointer prop )
{
  m_Delegate->SetTagProperty(prop);
}


void QmitkNDIConfigurationWidget::UpdateTrackerFromToolTable(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
  //Colums ID doesn't have to be processed.
  if (topLeft.column()<1)
    return;

  if (m_Tracker.IsNull())
    return;
  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  for (unsigned int i = 0; i < m_Controls->m_ToolTable->rowCount(); ++i)
  {
    mitk::NDIPassiveTool* tool = dynamic_cast<mitk::NDIPassiveTool*> (m_Tracker->GetTool(i));
    if (tool == NULL)
      continue;
    tool->SetToolName(model->data(model->index(i, 1)).toString().toLatin1());
    QString romfile = model->data(model->index(i, QmitkNDIToolDelegate::SROMCol)).toString();
    if (romfile.isEmpty())
      continue;
    if (QFileInfo(romfile).exists())
      tool->LoadSROMFile(romfile.toLatin1());
  }
  m_Tracker->InitializeWiredTools();
}


const QString QmitkNDIConfigurationWidget::GetToolType( unsigned int index ) const
{
  if (m_Controls == NULL)
    return QString("");

  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  QModelIndex modelIndex = model->index(index, QmitkNDIToolDelegate::TypeCol);
  if (modelIndex.isValid() == false)
    return QString("");

  return model->data(modelIndex).toString();
}


const QString QmitkNDIConfigurationWidget::GetToolName( unsigned int index ) const
{
  if (m_Controls == NULL)
    return QString("");

  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  QModelIndex modelIndex = model->index(index, QmitkNDIToolDelegate::NameCol);
  if (modelIndex.isValid() == false)
    return QString("");

  return model->data(modelIndex).toString();
}


QMap<QString, unsigned int> QmitkNDIConfigurationWidget::GetToolAndTypes() const
{
  QMap<QString, unsigned int> map;
  if (m_Controls == NULL)
    return map;
  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  for (unsigned int i = 0; i < model->rowCount(); ++i)
  {
    QModelIndex indexIndex = model->index(i, QmitkNDIToolDelegate::IndexCol);
    QModelIndex typeIndex = model->index(i, QmitkNDIToolDelegate::TypeCol);
    if ((indexIndex.isValid() == false) || (typeIndex.isValid() == false))
      continue;
    map.insert(model->data(typeIndex).toString(), model->data(indexIndex).toUInt());
  }
  return map;
}


QList<unsigned int> QmitkNDIConfigurationWidget::GetToolsByToolType( QString toolType ) const
{
  QList<unsigned int> list;
  if (m_Controls == NULL)
      return list;
  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  for (unsigned int i = 0; i < model->rowCount(); ++i)
  {
    QModelIndex indexIndex = model->index(i, QmitkNDIToolDelegate::IndexCol);
    QModelIndex typeIndex = model->index(i, QmitkNDIToolDelegate::TypeCol);
    if ((indexIndex.isValid() == false) || (typeIndex.isValid() == false))
      continue;
    if (model->data(typeIndex).toString() == toolType)
      list.append(model->data(indexIndex).toUInt());
  }
  return list;
}


mitk::DataTreeNode* QmitkNDIConfigurationWidget::GetNode( unsigned int index ) const
{
  
  if (m_Controls == NULL)
    return NULL;
  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  QVariant data = model->data(model->index(index, QmitkNDIToolDelegate::NodeCol), QmitkNDIToolDelegate::OrganNodeRole);
  return data.value<mitk::DataTreeNode*>();
}


void QmitkNDIConfigurationWidget::HideDiscoverDeviceButton( bool on )
{
  m_Controls->m_DisoverDevicesBtn->setHidden(on);
}


void QmitkNDIConfigurationWidget::HideDiscoverWiredToolseButton( bool on )
{
  m_Controls->m_DiscoverToolsBtn->setHidden(on);
}


void QmitkNDIConfigurationWidget::HideAddPassiveToolButton( bool on )
{
  m_Controls->m_AddToolBtn->setHidden(on);
}
