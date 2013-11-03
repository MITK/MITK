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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "USNavigation.h"
#include <QFileDialog>

//MITK
//#include "mitkTransformationFileReader.h"
#include <mitkBaseRenderer.h>
#include <QmitkRenderWindow.h>

// VTK
#include <vtkSphereSource.h>

// Qt
#include <QMessageBox>
#include <QBoxLayout>
#include <QShortcut>


#include "mitkStandaloneDataStorage.h"

const std::string USNavigation::VIEW_ID = "org.mitk.views.usnavigation";


void USNavigation::SetFocus()
{
//  m_Controls.buttonPerformImageProcessing->setFocus();
}

void USNavigation::CreateQtPartControl( QWidget *parent )
{
  m_Timer = new QTimer(this);
  m_RangeMeterTimer = new QTimer(this);
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_Controls.m_ZonesWidget->SetDataStorage(this->GetDataStorage());

  // Timer
  connect( m_Timer, SIGNAL(timeout()), this, SLOT(Update()));
  connect( m_RangeMeterTimer, SIGNAL(timeout()), this, SLOT(UpdateMeters()));

  connect( m_Controls.m_BtnSelectDevices, SIGNAL(clicked()), this, SLOT(OnSelectDevices()) );
  connect( m_Controls.m_TabWidget, SIGNAL(currentChanged ( int )), this, SLOT(OnTabSwitch( int )) );
  connect( m_Controls.m_CombinedModalitiesList, SIGNAL( ServiceSelectionChanged(us::ServiceReferenceU) ), this, SLOT(OnClickDevices()) );
  connect( m_Controls.m_BtnLoadCalibration, SIGNAL(clicked()), this, SLOT(OnLoadCalibration()) );
  // Zones
  connect( m_Controls.m_BtnFreeze, SIGNAL(clicked()), this, SLOT(OnFreeze()) );
  connect( m_Controls.m_BtnAddZone, SIGNAL(clicked()), this, SLOT(OnAddZone()) );
  connect( m_Controls.m_ZoneList, SIGNAL(itemClicked ( QListWidgetItem *)), this, SLOT(OnSelectZone(QListWidgetItem * )) );
  connect( m_Controls.m_BtnSaveZone, SIGNAL(clicked ()), this, SLOT(OnSaveZone()) );
  connect( m_Controls.m_BtnDeleteZone, SIGNAL(clicked ()), this, SLOT(OnDeleteZone()) );
  // Navigation
  connect( m_Controls.m_BtnStartIntervention, SIGNAL(clicked ()), this, SLOT(OnStartIntervention()) );
  connect( m_Controls.m_BtnReset, SIGNAL(clicked ()), this, SLOT(OnReset()) );
  connect( m_Controls.m_BtnNeedleView, SIGNAL(clicked ()), this, SLOT(OnNeedleViewToogle()) );


  m_Freeze = false;
  m_IsNeedleViewActive = false;

  std::string filterOnlyCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::US_PROPKEY_CLASS + "=" + mitk::USCombinedModality::DeviceClassIdentifier + "))";
  m_Controls.m_CombinedModalitiesList->Initialize<mitk::USDevice>(mitk::USCombinedModality::US_PROPKEY_LABEL, filterOnlyCombinedModalities);

  //m_Controls.m_USDevices->Initialize<mitk::USDevice>(mitk::USDevice::US_PROPKEY_LABEL);
  //m_Controls.m_TrackingDevices->Initialize<mitk::NavigationDataSource>( mitk::NavigationDataSource::US_PROPKEY_DEVICENAME);
  //m_Controls.m_TabWidget->setTabEnabled(1, false);
  //m_Controls.m_TabWidget->setTabEnabled(2, false);
  m_ZoneFilter = mitk::NodeDisplacementFilter::New();
  m_NeedleProjectionFilter = mitk::NeedleProjectionFilter::New();
  m_SmoothingFilter = mitk::NavigationDataSmoothingFilter::New();
  m_CameraVis = mitk::CameraVisualization::New();
  m_RangeMeterStyle = "QProgressBar:horizontal {\nborder: 1px solid gray;\nborder-radius: 3px;\nbackground: white;\npadding: 1px;\ntext-align: center;\n}\nQProgressBar::chunk:horizontal {\nbackground: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #StartColor#, stop: 0.8 #StartColor#, stop: 1 #StopColor#);\n}";
  QBoxLayout * layout = new QBoxLayout(QBoxLayout::Down, m_Controls.m_RangeBox);
  // Pedal Support for Needle View
  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_PageDown), parent);
  QObject::connect(shortcut, SIGNAL(activated()), m_Controls.m_BtnNeedleView, SLOT(animateClick()) );
}


void USNavigation::OnClickDevices(){

  if ( m_Controls.m_CombinedModalitiesList->GetIsServiceSelected() )
    m_Controls.m_BtnSelectDevices->setEnabled(true);
  else
    m_Controls.m_BtnSelectDevices->setEnabled(false);
}

void USNavigation::OnSelectDevices(){
  m_USDevice = dynamic_cast<mitk::USCombinedModality*>(m_Controls.m_CombinedModalitiesList->GetSelectedService<mitk::USDevice>());
  if (m_USDevice.IsNull())
  {
    MITK_WARN << "Selected device is no USCmbinedModality.";
    return;
  }

  if (! m_USDevice->GetIsActive()) m_USDevice->Activate();

  m_Tracker = m_USDevice->GetNavigationDataSource();

  // Set Calibration, if necessary (TODO: must be done for the new calibration processing)
  //if (m_LoadedCalibration.IsNotNull()) m_USDevice->setCalibration(m_LoadedCalibration);

  // Build Pipeline
  //m_SmoothingFilter->SetInput(0, m_Tracker->GetOutput());
  //m_SmoothingFilter->SetNumerOfValues(10);
  //m_ZoneFilter->SetInput(0, m_SmoothingFilter->GetOutput(0));
  m_ZoneFilter->SetInput(0, m_Tracker->GetOutput(0));
  m_ZoneFilter->SetInput(1, m_Tracker->GetOutput(1));
  m_ZoneFilter->SelectInput(1);
  m_NeedleProjectionFilter->SetInput(0, m_ZoneFilter->GetOutput(0));
  m_NeedleProjectionFilter->SetInput(1, m_ZoneFilter->GetOutput(1));
  m_NeedleProjectionFilter->SelectInput(0);
  m_NeedleProjectionFilter->SetTargetPlane(m_USDevice->GetCalibration());


  // Setting up the Camera Visualization Filter

  /* Second 3D Renderwindow currently not functional, see Bug 13465
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(this->GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderWindow());
  renderer->SetMapperID(mitk::BaseRenderer::Standard3D);
  */

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow());

  m_CameraVis->SetInput(0, m_NeedleProjectionFilter->GetOutput(0));
  m_CameraVis->SetRenderer( renderer );
  m_CameraVis->SetFocalLength(40);

  mitk::Vector3D direction;
  direction[0] = 0;
  direction[1] = 0;
  direction[2] = 1;
  m_CameraVis->SetDirectionOfProjectionInToolCoordinates(direction);
  direction[0] = -1;
  direction[1] = 0;
  direction[2] = 0;
  m_CameraVis->SetViewUpInToolCoordinates(direction);

  // Create Node for Needle Projection
  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if ( node.IsNull() )
  {
    node = mitk::DataNode::New();
    node->SetName("Needle Path");
    node->SetData(m_NeedleProjectionFilter->GetProjection());
    node->SetBoolProperty("show contour", true);
    this->GetDataStorage()->Add(node);
    node->SetFloatProperty("opacity", 0.80f );
  }

  m_Timer->start(15);

  // Create Node for US Stream
  m_USStream = mitk::DataNode::New();
  m_USStream->SetName("US Image Stream");
  m_USStream->SetFloatProperty("opacity", 0.80f );
  this->GetDataStorage()->Add(m_USStream);

  // Switch active tab to next page
  m_Controls.m_TabWidget->setTabEnabled(1, true);
  m_Controls.m_TabWidget->setTabEnabled(2, true);
  m_Controls.m_TabWidget->setCurrentIndex(1);
}

void USNavigation::OnLoadCalibration(){
  QString filename = QFileDialog::getOpenFileName( QApplication::activeWindow(),
                                                   "Load Calibration",
                                                   "",
                                                   "Calibration files *.cal" );

  //TODO:  New reader for transformations must be implemented.
  //mitk::TransformationFileReader::Pointer tReader = mitk::TransformationFileReader::New();
  //tReader->SetInputFilename(filename.toStdString());
  //bool success = tReader->DoRead();
  //if (! success){
  //  m_Controls.m_LblCalibrationLoadState->setText("Failed to load file. Unsupported format?");
  //  return;
  //}
  //m_LoadedCalibration = tReader->GetOutputTransform();
  //m_Controls.m_LblCalibrationLoadState->setText("Loaded calibration : " + filename);
  //if (m_USDevice.IsNotNull()){
  //  m_USDevice->setCalibration(m_LoadedCalibration);
  //}
}

void USNavigation::Update()
{
  if (m_Freeze) return;

  // Update NeedleVisFilter only if the view is desired
  if (! m_IsNeedleViewActive)
    m_NeedleProjectionFilter->Update();
  else
    m_CameraVis->Update();
  //only Update USImage every other time
  m_UpdateImage ++;
  if (m_UpdateImage)
  {
    m_USDevice->UpdateOutputData(0);
    mitk::USImage::Pointer image = m_USDevice->GetOutput();
    m_USStream->SetData(image);
    this->RequestRenderWindowUpdate();
  }
}

//////////////////////////////// Zones //////////////////////////////////

void USNavigation::OnAddZone()
{
  mitk::Point3D target = this->GetRenderWindowPart()->GetSelectedPosition();
  mitk::DataNode::Pointer node = mitk::DataNode::New();

  this->FormatZoneNode(node, target);

  this->GetDataStorage()->Add(node);
  m_Zones.push_back(node);
  this->m_Controls.m_ZoneList->addItem(new QListWidgetItem(node->GetName().c_str()));

  this->m_ZoneFilter->AddNode(node);
}

void USNavigation::OnSelectZone( QListWidgetItem * current )
{
  if (current == NULL)
  {
    m_Controls.m_BtnSaveZone->setEnabled(false);
    m_Controls.m_BtnDeleteZone->setEnabled(false);
    m_Controls.m_InptZoneName->setText("");
  } else
  {
    m_Controls.m_BtnSaveZone->setEnabled(true);
    m_Controls.m_BtnDeleteZone->setEnabled(true);
  }

  mitk::DataNode::Pointer node = GetDataStorage()->GetNamedNode(current->text().toStdString());
  if (node.IsNull()) return;

  m_Controls.m_InptZoneName->setText(current->text());
  int size;
  node->GetIntProperty("zone.size", size);
  m_Controls.m_InptZoneSize->setValue(size);
}

void USNavigation::OnSaveZone()
{
  QString name = m_Controls.m_ZoneList->currentItem()->text();
  mitk::DataNode::Pointer node = GetDataStorage()->GetNamedNode(name.toStdString());
  this->FormatZoneNode(node, node->GetData()->GetGeometry()->GetCenter());
  m_Controls.m_ZoneList->currentItem()->setText(node->GetName().c_str());
}

void USNavigation::OnDeleteZone()
{
  QString name = m_Controls.m_ZoneList->currentItem()->text();
  m_Controls.m_ZoneList->takeItem(m_Controls.m_ZoneList->row(m_Controls.m_ZoneList->currentItem()));
  mitk::DataNode::Pointer node = GetDataStorage()->GetNamedNode(name.toStdString());
  GetDataStorage()->Remove(node);
}

void USNavigation::FormatZoneNode(mitk::DataNode::Pointer node, mitk::Point3D center)
{
  // Make Sphere
  mitk::Surface::Pointer zone= mitk::Surface::New();
  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius( m_Controls.m_InptZoneSize->value() );
  vtkData->SetCenter(0,0,0);
  vtkData->Update();
  zone->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();
  node->SetData(zone);
  node->GetData()->GetGeometry()->SetOrigin(center);

  // Set Properties
  node->SetFloatProperty("opacity", 0.60f );
  QString name = m_Controls.m_InptZoneName->text();
  QStringList takenNames;
  // Find and give Name subroutine:
  // 1) if empty, give initial name
  if (name.isNull() || name.isEmpty()) name = "Zone";
  // Fill a Q String List with all names that are already taken
  for (int i = 0; i < m_Zones.size(); i++)
    takenNames.append( QString(m_Zones.at(i)->GetName().c_str()) );
  // find unique name
  QString candidate = name;
  int counter = 2;
  while (takenNames.contains(candidate))
  {
    candidate = name + " (" + candidate.number(counter) +")";
    counter++;
  }
  node->SetName(candidate.toStdString());

  node->SetIntProperty("zone.size", m_Controls.m_InptZoneSize->value());

  // Set Color
  if (m_Controls.m_InptZoneColorBlue->isChecked())
  {
    node->SetColor(0.0f, 0.0f, 1.0f);
    node->SetStringProperty("zone.color", "#4444ff");
  }
  if (m_Controls.m_InptZoneColorRed->isChecked())
  {
    node->SetColor(1.0f, 0.0f, 0.0f);
    node->SetStringProperty("zone.color", "#ff6666");
  }
  if (m_Controls.m_InptZoneColorGreen->isChecked())
  {
    node->SetColor(0.0f, 1.0f, 0.0f);
    node->SetStringProperty("zone.color", "#44ff44");
  }
  if (m_Controls.m_InptZoneColorYellow->isChecked())
  {
    node->SetColor(1.0f, 1.0f, 0.0f);
    node->SetStringProperty("zone.color", "yellow");
  }
}



///////////////////// Range Meter ////////////////

void USNavigation::SetupProximityView()
{
  QLayout * layout = m_Controls.m_RangeBox->layout();
  for (int i = 0; i < m_Zones.size(); i++)
  {
    QProgressBar * meter = CreateRangeMeter(i);
    layout->addWidget(meter);
    m_RangeMeters.push_back(meter);
  }
  m_RangeMeterTimer->start(100);
}

QProgressBar* USNavigation::CreateRangeMeter(int i)
{
  mitk::DataNode::Pointer zone = m_Zones.at(i);
  std::string zoneColor;
  bool success = zone->GetStringProperty("zone.color", zoneColor);
  if (!success) zoneColor = "grey";

  QProgressBar* meter = new QProgressBar();
  meter->setMinimum(0);
  meter->setMaximum(100);
  meter->setValue(0);
  QString zoneName = zone->GetName().c_str();
  meter->setFormat(zoneName + ": No Data");
  QString style = m_RangeMeterStyle;
  style = style.replace("#StartColor#", zoneColor.c_str());
  style = style.replace("#StopColor#", zoneColor.c_str());
  meter->setStyleSheet(style);
  meter->setVisible(true);
  return meter;
}

void USNavigation::UpdateMeters()
{
  // Get NeedlePosition
  mitk::NavigationData::Pointer needle = this->m_Tracker->GetOutput(0);
  if (! needle->IsDataValid()) return;
  mitk::Point3D needlePosition = needle->GetPosition();
  // Update each meter
  for (int i = 0; i < m_Zones.size(); i++)
  {
    mitk::Point3D zoneOrigin = m_Zones.at(i)->GetData()->GetGeometry()->GetOrigin();
    // calculate absolute distance
    int distance = sqrt( pow(zoneOrigin[0] - needlePosition[0], 2) + pow(zoneOrigin[1] - needlePosition[1], 2) + pow(zoneOrigin[2] - needlePosition[2], 2) );
    // Subtract zone size
    int zoneSize;
    m_Zones.at(i)->GetIntProperty("zone.size", zoneSize);
    distance = distance - zoneSize;

    // Prepare new Style
    std::string zoneColor;
    m_Zones.at(i)->GetStringProperty("zone.color", zoneColor);
    QString style = m_RangeMeterStyle;
    QString text =  m_Zones.at(i)->GetName().c_str();
    int value = 0;
    // There Are now four possible Outcomes
    // 1) Needle is inside zone (bad news)
    if (distance < 0)
    {
      style = style.replace("#StartColor#", "red");
      style = style.replace("#StopColor#", "red");
      text  = text + ": VIOLATED";
      value = 100;
    } // 2) Needle is close to Zone
    else if (distance < WARNRANGE)
    {
      style = style.replace("#StartColor#", zoneColor.c_str());
      style = style.replace("#StopColor#", "red");
      text  = text + ": " + QString::number(distance) + " mm";
      value = 100 -  100 * ((float) distance / (float )MAXRANGE);
    } // 3) Needle is away from zone
    else if (distance < MAXRANGE)
        {
      style = style.replace("#StartColor#", zoneColor.c_str());
      style = style.replace("#StopColor#", zoneColor.c_str());
      text  = text + ": " + QString::number(distance) + " mm";
      value = 100 -  100 * ((float) distance / (float )MAXRANGE);
    } // 4) Needle is far away from zone
    else
    {
      style = style.replace("#StartColor#", zoneColor.c_str());
      style = style.replace("#StopColor#", zoneColor.c_str());
      text  = text + ": " + QString::number(distance) + " mm";
      value = 0;
    }

    QProgressBar * meter = this->m_RangeMeters.at(i);
    meter->setStyleSheet(style);
    meter->setFormat(text);
    meter->setValue(value);
  }
}


/////////////////// Intervention ///////////////////////

void USNavigation::OnStartIntervention()
{
  this->m_Controls.m_TabWidget->setCurrentIndex(2);
  this->SetupProximityView();
}

void USNavigation::OnFreeze()
{
  // Swap Freeze State
  m_Freeze = ! m_Freeze;
}

void USNavigation::OnReset()
{
  // Reset Zones
  m_ZoneFilter->ResetNodes();
  for (int i; i < m_Zones.size(); i++)
    this->GetDataStorage()->Remove(m_Zones.at(i));
  m_Zones.clear();
  m_Controls.m_ZoneList->clear();

  // Reset RangeMeters
  QLayout* layout = m_Controls.m_RangeBox->layout();
  QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

  m_RangeMeters.clear();
  m_RangeMeterTimer->stop();

}

void USNavigation::OnNeedleViewToggle()
{
  m_IsNeedleViewActive = m_Controls.m_BtnNeedleView->isChecked();

  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if ( node.IsNotNull() )
    node->SetVisibility( ! m_IsNeedleViewActive );


}
