/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include <mitkNavigationToolWriter.h>
#include <mitkNavigationToolReader.h>
#include <mitkSTLFileReader.h>

#include "QmitkCustomVariants.h"

//#include <QtConcurrentMap>
#include "QmitkNDIToolDelegate.h"

/* VIEW MANAGEMENT */
QmitkNDIConfigurationWidget::QmitkNDIConfigurationWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL), m_Tracker(NULL), m_Source(NULL),
m_Delegate(NULL), m_SROMCellDefaultText("<click to load SROM file>"), m_RepresentatonCellDefaultText("<click to select representation>")
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
    this->HidePolarisOptionsGroupbox(true);
    this->HideAuroraOptionsGroupbox(true);
  }
}


void QmitkNDIConfigurationWidget::CreateConnections()
{
  connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
  connect(m_Controls->m_DiscoverToolsBtn, SIGNAL(clicked()), this, SLOT(OnDiscoverTools()));
  connect(m_Controls->m_AddToolBtn, SIGNAL(clicked()), this, SLOT(OnAddPassiveTool()));
  connect(m_Controls->m_DisoverDevicesBtn, SIGNAL(clicked()), this, SLOT(OnDiscoverDevices()));
  connect(m_Controls->m_ToolTable->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(UpdateTrackerFromToolTable(const QModelIndex &, const QModelIndex &)));
  connect(m_Controls->m_DisoverDevicesBtnInfo, SIGNAL(clicked()), this, SLOT(OnDisoverDevicesBtnInfo()));
  
  connect(m_Controls->m_SaveToolPushButton, SIGNAL(clicked()), this, SLOT(OnSaveTool()) );
  connect(m_Controls->m_LoadToolPushButton, SIGNAL(clicked()), this, SLOT(OnLoadTool()) );
  
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
    // show/hide options according to connected device
    if(m_Tracker->GetType() == mitk::NDIPolaris)
    {
      this->HideAuroraOptionsGroupbox(true);
      this->HidePolarisOptionsGroupbox(false);
    }
    else if(m_Tracker->GetType() == mitk::NDIAurora)
    {
      this->HidePolarisOptionsGroupbox(true);
      this->HideAuroraOptionsGroupbox(false);
    }

    this->UpdateWidgets();
    this->UpdateToolTable();
    
    connect(m_Controls->m_ToolTable, SIGNAL(cellChanged(int,int)), this, SLOT(OnTableCellChanged(int,int)));
    
    emit ToolsAdded(this->GetToolNamesList());
    emit Connected();
  }
  else
  {
    QMessageBox::warning(NULL, "Error", QString("Connection failed. Tracking device error message is '%1'").arg(m_Tracker->GetErrorMessage()));
    m_Tracker->CloseConnection();
    this->m_Tracker = NULL;
  }
}


void QmitkNDIConfigurationWidget::OnDisconnect()
{
  if (m_Tracker.IsNull())
    return;
  m_Tracker->CloseConnection();
  m_Tracker = NULL;
  
  disconnect(m_Controls->m_ToolTable, SIGNAL(cellChanged(int,int)), this, SLOT(OnTableCellChanged(int,int)));
  m_Controls->m_ToolSelectionComboBox->clear();
  
  this->UpdateToolTable();
  this->UpdateWidgets();  
  emit ToolsAdded(this->GetToolNamesList());
  emit Disconnected();

  this->HidePolarisOptionsGroupbox(true);
  this->HideAuroraOptionsGroupbox(true);
}


void QmitkNDIConfigurationWidget::UpdateWidgets()
{
  m_Controls->m_DeviceStatus->setText(this->GetStatusText());
  if (m_Tracker.IsNull())  // not connected to tracker
  {
    m_Controls->m_Connect->setText("Connect");
    m_Controls->m_lConnection->setText("III. Enable connection to  device  ");

    disconnect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnDisconnect()));
    connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
    m_Controls->m_DiscoverToolsBtn->setDisabled(true);
    m_Controls->m_AddToolBtn->setDisabled(true);
    return;
  }

  if (m_Tracker->GetState() == mitk::TrackingDevice::Setup)
  {
    m_Controls->m_Connect->setText("Connect");
    m_Controls->m_lConnection->setText("III. Enable connection to  device  ");
    disconnect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnDisconnect()));
    connect(m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()));
    m_Controls->m_DiscoverToolsBtn->setDisabled(true);
    m_Controls->m_AddToolBtn->setDisabled(true);
    return;
  }
  if ((m_Tracker->GetState() == mitk::TrackingDevice::Ready) || (m_Tracker->GetState() == mitk::TrackingDevice::Tracking))
  {
    m_Controls->m_Connect->setText("Disconnect");
    m_Controls->m_lConnection->setText("III. Disable connection to  device ");
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
  {
    QMessageBox::warning(NULL, "Error", QString("Connection failed. No tracking device found."));
    return;
  }
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
  {
    this->m_Tracker = NULL;
    return;
  }
  foreach(QString fileName, filenames)
  {
    //QString toolName = QInputDialog::getText(this, "Enter a name for the tool", "Name of the tool: ", QLineEdit::Normal, QFileInfo(filename).baseName(), &ok);
    //if (ok == false || toolName.isEmpty())
    //  return;
    m_Tracker->AddTool(QFileInfo(fileName).baseName().toLatin1(), fileName.toLatin1());
    m_Tracker->Modified();
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
  QString deviceName = m_Controls->m_ComPortSelector->currentText();
#if WIN32
  deviceName.prepend("\\\\.\\"); // always prepend "\\.\ to all COM ports, to be able to connect to ports > 9"
#endif
  return deviceName.toStdString();
}


void QmitkNDIConfigurationWidget::SetDeviceName( const char* dev )
{
  if (m_Controls == NULL)
    return;
  m_Controls->m_ComPortSelector->setCurrentIndex(m_Controls->m_ComPortSelector->findText(dev));
}


void QmitkNDIConfigurationWidget::UpdateToolTable()
{
  //disconnect(m_Controls->m_ToolTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(OnTableItemChanged(QTableWidgetItem*))); // stop listening to table changes
  disconnect(m_Controls->m_ToolTable->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(UpdateTrackerFromToolTable(const QModelIndex &, const QModelIndex &)));
  disconnect(m_Controls->m_ToolTable, SIGNAL( clicked ( const QModelIndex & )), this, SLOT ( OnTableItemClicked( const QModelIndex & )));
  
  m_Controls->m_ToolTable->clearContents();
  m_Controls->m_ToolTable->setRowCount(0);
  if (m_Tracker.IsNull() || (m_Controls == NULL))
    return;
    
  m_Controls->m_ToolSelectionComboBox->clear();  

  m_Controls->m_ToolTable->setRowCount(m_Tracker->GetToolCount());
  for (unsigned int i = 0; i < m_Tracker->GetToolCount(); ++i)
  {
    mitk::TrackingTool* t = m_Tracker->GetTool(i);
    if (t == NULL)
    {
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::IndexCol, new QTableWidgetItem("INVALID"));                   // Index
      continue;
    }
    
    m_Controls->m_ToolSelectionComboBox->addItem(m_Tracker->GetTool(i)->GetToolName());
        
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::IndexCol, new QTableWidgetItem(QString::number(i)));            // Index
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::NameCol, new QTableWidgetItem(t->GetToolName()));               // Name    
    if (dynamic_cast<mitk::NDIPassiveTool*>(t)->GetSROMDataLength() > 0)
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::SROMCol, new QTableWidgetItem("SROM file loaded"));           // SROM file
    else
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::SROMCol, new QTableWidgetItem(m_SROMCellDefaultText));        // SROM file
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::TypeCol, new QTableWidgetItem("<click to set type>"));          // Type
    if (t->IsEnabled())
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::StatusCol, new QTableWidgetItem("Enabled"));                  // Status
    else
      m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::StatusCol, new QTableWidgetItem("Disabled"));                 // Status
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::NodeCol, new QTableWidgetItem("<click to select node>"));       // Node

    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::RepCol, new QTableWidgetItem(m_RepresentatonCellDefaultText));       // Representation

         
    /* set read-only/editable flags */
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::IndexCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);                        // Index
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::NodeCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // Name
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::SROMCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // SROM file
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::TypeCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // Type
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::StatusCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);                       // Status
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::NodeCol)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable   | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);  // Node
  
    m_Controls->m_ToolTable->item(i, QmitkNDIToolDelegate::RepCol)->setFlags(Qt::NoItemFlags);  // Representation surface file
  }
  m_Controls->m_ToolTable->resizeColumnsToContents();
  //connect(m_Controls->m_ToolTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(OnTableItemChanged(QTableWidgetItem*))); // listen to table changes again
  connect(m_Controls->m_ToolTable->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(UpdateTrackerFromToolTable(const QModelIndex &, const QModelIndex &)));
  connect(m_Controls->m_ToolTable, SIGNAL( clicked ( const QModelIndex & )), this, SLOT ( OnTableItemClicked( const QModelIndex & )));
  
}


void QmitkNDIConfigurationWidget::OnDiscoverDevices()
{
  PortDeviceMap portsAndDevices;
  QString status = "Scanning ";
#ifdef WIN32
  QString devName;
  for (unsigned int i = 1; i < 20; ++i)
  {
    if (i<10)
    devName = QString("COM%1").arg(i);
    else
      devName = QString("\\\\.\\COM%1").arg(i); // prepend "\\.\ to COM ports >9, to be able to allow connection"
    portsAndDevices[devName];
    status += QString("COM%1").arg(i) + ", ";
  }
#else //linux/posix systems
  for(unsigned int i = 1; i < 6; ++i)
  {
    QString devName = QString("/dev/ttyS%1").arg(i);
    portsAndDevices[devName];
    status += devName + ", ";
  }
  for(unsigned int i = 0; i <7; ++i)
  {
    QString devName = QString("/dev/ttyUSB%1").arg(i);
    portsAndDevices[devName];
    status += devName + ", ";
  }
#endif

  status.chop(2); // remove last ", "
  status += " for NDI tracking devices...";
  m_Controls->m_DeviceStatus->setText(status);
  ScanPortsForNDITrackingDevices(portsAndDevices);
  m_Controls->m_ComPortSelector->clear();
  QString result = "The following tracking devices were found:<BR/>\n";
  for (PortDeviceMap::const_iterator it = portsAndDevices.begin(); it != portsAndDevices.end(); ++it)
  {
    QString tmpComPort = it.key();
    if (tmpComPort.startsWith("\\"))
    {
      tmpComPort.remove(0,4); // remove "\\.\" for nice ui visualisation
    }
    result += tmpComPort + ": ";   

    switch (it.value())
    {
    case mitk::NDIPolaris:
      result += "NDI Polaris<BR/>\n";
      m_Controls->m_ComPortSelector->addItem(tmpComPort);
      break;
    case mitk::NDIAurora:
      result += "NDI Aurora<BR/>\n";
      m_Controls->m_ComPortSelector->addItem(tmpComPort);
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


void QmitkNDIConfigurationWidget::OnTableItemClicked(const QModelIndex & topLeft )
{
  QString filename;
  QTableWidgetItem* filenameItem; 

  switch (topLeft.column())
  {
  case QmitkNDIToolDelegate::RepCol:

    filename = QFileDialog::getOpenFileName(this, "Select Surface File", QDir::currentPath(),"STL files (*.stl)");

    filenameItem = new QTableWidgetItem(filename);
    m_Controls->m_ToolTable->setItem( topLeft.row(), topLeft.column(), filenameItem );

    if(QFileInfo(filename).exists())
    {
      mitk::Surface::Pointer surface = this->LoadSurfaceFromSTLFile(filename);
      
      if(surface.IsNotNull())
        emit RepresentationChanged( topLeft.row(), surface);
    }
    break;
   default:
    break;  
  }
}


void QmitkNDIConfigurationWidget::UpdateTrackerFromToolTable(const QModelIndex & topLeft, const QModelIndex & /*bottomRight*/)
{
  //Colums ID doesn't have to be processed.
  if (topLeft.column()<1)
    return;

  if (m_Tracker.IsNull())
    return;

  if (topLeft.row() >= (int) m_Tracker->GetToolCount())
    return;

  QAbstractItemModel* model = m_Controls->m_ToolTable->model();

  //define topleft contains row and column; row 0 is tool 0; column is index =0, Name =1, SROMFileName = 2; Type = 3; Status = 4; Node (?) = 5
  //only update the changed item
  mitk::NDIPassiveTool* tool = dynamic_cast<mitk::NDIPassiveTool*> (m_Tracker->GetTool(topLeft.row()));
    if (tool == NULL)
      return;

  switch (topLeft.column())
  {
  case QmitkNDIToolDelegate::IndexCol: //index
    break;
  case QmitkNDIToolDelegate::NameCol: //name
    tool->SetToolName(model->data(model->index(topLeft.row(), 1)).toString().toLatin1());
    emit ToolsChanged();
    break;
  case QmitkNDIToolDelegate::SROMCol: //SROM File Name
    {
      QString romfile = model->data(model->index(topLeft.row(), QmitkNDIToolDelegate::SROMCol)).toString();
      if (QFileInfo(romfile).exists())
        tool->LoadSROMFile(romfile.toLatin1());
      m_Tracker->UpdateTool(tool);
      break;
    }

    //TODO: Add Node Status and Type here as well
  default:
    break;
  }

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
  for (int i = 0; i < model->rowCount(); ++i)
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
  for (int i = 0; i < model->rowCount(); ++i)
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


mitk::DataNode* QmitkNDIConfigurationWidget::GetNode( unsigned int index ) const
{
  
  if (m_Controls == NULL)
    return NULL;
  QAbstractItemModel* model = m_Controls->m_ToolTable->model();
  QVariant data = model->data(model->index(index, QmitkNDIToolDelegate::NodeCol), QmitkNDIToolDelegate::OrganNodeRole);
  return data.value<mitk::DataNode*>();
}

void QmitkNDIConfigurationWidget::HidePolarisOptionsGroupbox( bool on )
{
  m_Controls->m_gbPolarisOptions->setHidden(on);
}

void QmitkNDIConfigurationWidget::HideAuroraOptionsGroupbox( bool on )
{
  m_Controls->m_gbAuroraOptions->setHidden(on);
}

void QmitkNDIConfigurationWidget::ShowToolRepresentationColumn()
{
  int cols = m_Controls->m_ToolTable->columnCount();

  //checking if representation column is inserted at right index
  if(cols != QmitkNDIToolDelegate::RepCol) 
  {
    //throw std::exception("Representation Column is not inserted at it's designated index!");
    return;
  }
  

  m_Controls->m_ToolTable->insertColumn(cols); // insert new column at end of table
  

  m_Controls->m_ToolTable->setHorizontalHeaderItem(QmitkNDIToolDelegate::RepCol, new QTableWidgetItem(QString("Representation"))); // inser column header for new colum
  //m_Controls->m_ToolTable->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

  int rows = m_Controls->m_ToolTable->rowCount();

 // make all representation colum items not editable
  for(int i=0; i < rows; ++i) 
  {
    m_Controls->m_ToolTable->setItem(i, QmitkNDIToolDelegate::RepCol, new QTableWidgetItem("<click to select representation>"));       // Representation
    m_Controls->m_ToolTable->item(i,QmitkNDIToolDelegate::RepCol)->setFlags(Qt::NoItemFlags);
   }
   
   //connect(m_Controls->m_ToolTable, SIGNAL( clicked ( const QModelIndex & )), this, SLOT ( OnTableItemClicked( const QModelIndex & )));

}

void QmitkNDIConfigurationWidget::OnDisoverDevicesBtnInfo()
{
  QMessageBox *infoBox = new QMessageBox(this);
  infoBox->setText("Click \"Scan Ports\" to get a list of all connected NDI tracking devices. This will clear the selection menu below and add the ports for discovered NDI tracking devices. Use this function, if a port is not listed.");
  infoBox->exec();
  delete infoBox;
}


void QmitkNDIConfigurationWidget::OnTableCellChanged(int row, int column)
{
  
  if(m_Tracker.IsNull())
    return;  
  
  QString toolName;
  
  switch (column)
  {
  case QmitkNDIToolDelegate::NameCol:
    toolName = m_Controls->m_ToolTable->item(row,column)->text();
    m_Controls->m_ToolSelectionComboBox->setItemText(row, toolName);
        
    emit SignalToolNameChanged(row, toolName);
        
  break;
  
  default:
  break;   
  } 
  
}


void QmitkNDIConfigurationWidget::OnSaveTool()
{
  if(m_Tracker.IsNull() || m_Tracker->GetToolCount() <= 0)
    return;

  int currId = m_Controls->m_ToolSelectionComboBox->currentIndex();
  QString filename = QFileDialog::getSaveFileName(NULL, "Save NDI-Tool", QString(QDir::currentPath()+QString(m_Tracker->GetTool(currId)->GetToolName())),"NDI Tracking Tool file(*.ntf)");
  
  mitk::TrackingTool* selectedTool = m_Tracker->GetTool(currId);
  
  if(filename.isEmpty())
    return;

  mitk::NavigationTool::Pointer navTool = mitk::NavigationTool::New();
  
   
  mitk::NavigationToolWriter::Pointer toolWriter = mitk::NavigationToolWriter::New();
  try {
    toolWriter->DoWrite(filename.toStdString(), this->GenerateNavigationTool(selectedTool));
  }
  catch( ... )
  {    
    QMessageBox::warning(NULL, "Saving Tool Error", QString("An error occured! Could not save tool!\n\n"));
    MBI_ERROR<<"Could not save tool surface!";
    MBI_ERROR<< toolWriter->GetErrorMessage();

    QFile maybeCorruptFile(filename);

    if(maybeCorruptFile.exists())
      maybeCorruptFile.remove();
  } 

  emit SignalSavedTool(currId, filename);
}


void QmitkNDIConfigurationWidget::OnLoadTool()
{
  if(m_Tracker.IsNull() || m_Tracker->GetToolCount() <= 0)
    return;
  
  QString filename = QFileDialog::getOpenFileName(NULL, "Load NDI-Tools", QDir::currentPath(),"NDI Tracking Tool file(*.ntf)");
  int currId = m_Controls->m_ToolSelectionComboBox->currentIndex();
  
  if(filename.isEmpty())
    return;
  
  mitk::DataNode::Pointer toolNode;  
  mitk::NavigationToolReader::Pointer toolReader = mitk::NavigationToolReader::New();
  mitk::NavigationTool::Pointer navTool;

  try {
    navTool = toolReader->DoRead(filename.toStdString());
  }
  catch( ... )
  {    
    QMessageBox::warning(NULL, "Loading Tool Error", QString("An error occured! Could not load tool!\n\n"));
    MBI_ERROR<<"Could not load tool surface!";
    MBI_ERROR<< toolReader->GetErrorMessage();
  } 

  int currSelectedToolID = m_Controls->m_ToolSelectionComboBox->currentIndex();
  
  // name
  m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::NameCol)->setText(navTool->GetToolName().c_str());
  dynamic_cast<mitk::NDIPassiveTool*>(m_Tracker->GetTool(currSelectedToolID))->SetToolName(navTool->GetToolName().c_str()); // also setting name to tool directly
  
  //calibration file (.srom) filename
  m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::SROMCol)->setText(navTool->GetCalibrationFile().c_str());
  
  //type 
  if(navTool->GetType() == mitk::NavigationTool::Instrument) 
    m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::TypeCol)->setText("Instrument");
  else if(navTool->GetType() == mitk::NavigationTool::Fiducial)
    m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::TypeCol)->setText("Fiducial");
  else if(navTool->GetType() == mitk::NavigationTool::Skinmarker)
    m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::TypeCol)->setText("Skinmarker");
  else
    m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::TypeCol)->setText("Unknown");

  
  //representation
  m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::SROMCol)->setText(m_RepresentatonCellDefaultText);
  
  emit SignalLoadTool(currId, navTool->GetDataNode());
}


mitk::NavigationTool::Pointer QmitkNDIConfigurationWidget::GenerateNavigationTool(mitk::TrackingTool* tool)
{
  mitk::NavigationTool::Pointer navTool = mitk::NavigationTool::New();
  mitk::NDIPassiveTool::Pointer passiveTool = dynamic_cast<mitk::NDIPassiveTool*>(tool);

  if(passiveTool.IsNull())
    throw std::runtime_error("Could not cast TrackingTool to PassiveTool");

  int currSelectedToolID = m_Controls->m_ToolSelectionComboBox->currentIndex();

  QString sromFileName = m_Controls->m_ToolTable->item(currSelectedToolID, QmitkNDIToolDelegate::SROMCol)->text();
  QString surfaceFileName = m_Controls->m_ToolTable->item(currSelectedToolID, QmitkNDIToolDelegate::RepCol)->text();
  
  //calibration file (.srom) filename
  QFile sromFile(sromFileName);
  if(sromFile.exists())
    navTool->SetCalibrationFile(sromFileName.toStdString());

  //serial number
  navTool->SetSerialNumber(passiveTool->GetSerialNumber());

  // name and surface as dataNode
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  
  mitk::Surface::Pointer toolSurface;
  
  try{
   toolSurface = this->LoadSurfaceFromSTLFile(surfaceFileName);
  }
  catch( ... )
  {
    QMessageBox::warning(NULL, "Loading Surface Error", QString("An error occured! Could not load surface from .stl file!\n\n"));
    MBI_ERROR<<"Could not load .stl tool surface!";
  }


  if(toolSurface.IsNotNull())
  {
    node->SetData(toolSurface); 
    node->SetName(tool->GetToolName());
  }
  
  navTool->SetDataNode(node);
  
  // type
  mitk::NavigationTool::NavigationToolType type;
  QString currentToolType = m_Controls->m_ToolTable->item(currSelectedToolID,QmitkNDIToolDelegate::TypeCol)->text();

  if(currentToolType.compare("Instrument") == 0)
    type = mitk::NavigationTool::Instrument;
  else if(currentToolType.compare("Fiducial") == 0)
    type = mitk::NavigationTool::Fiducial;
  else if(currentToolType.compare("Skinmarker") == 0)
    type = mitk::NavigationTool::Skinmarker;
  else
    type = mitk::NavigationTool::Unknown;

  navTool->SetType(type);

  return navTool;
}


mitk::Surface::Pointer QmitkNDIConfigurationWidget::LoadSurfaceFromSTLFile(QString surfaceFilename)
{
  mitk::Surface::Pointer toolSurface;
  
  QFile surfaceFile(surfaceFilename);
  if(surfaceFile.exists())
  {
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    
    try{
      stlReader->SetFileName(surfaceFilename.toStdString().c_str());
      stlReader->Update();//load surface
      toolSurface = stlReader->GetOutput();
    }
    catch(std::exception& e )
    {
      MBI_ERROR<<"Could not load surface for tool!";
      MBI_ERROR<< e.what();
      throw e;
    }
  }
  
  return toolSurface;
}