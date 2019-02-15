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
#include <mitkBaseRenderer.h>
#include <mitkIRenderingManager.h>
#include <QmitkRenderWindow.h>
#include <mitkDataStorage.h>

// VTK
#include <vtkSphereSource.h>

// Qt
#include <QMessageBox>
#include <QBoxLayout>
#include <QShortcut>

const std::string USNavigation::VIEW_ID = "org.mitk.views.usnavigation";

void USNavigation::SetFocus()
{
  //  m_Controls.buttonPerformImageProcessing->setFocus();
}

USNavigation::USNavigation()
: m_ImageAlreadySetToNode(false)
{
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
  connect( m_Controls.m_CombinedModalitySelectionWidget, SIGNAL(SignalReadyForNextStep()),
           this, SLOT(OnDeviceSelected()) );
  connect( m_Controls.m_CombinedModalitySelectionWidget, SIGNAL(SignalNoLongerReadyForNextStep()),
           this, SLOT(OnDeviceDeselected()) );

  connect( m_Controls.m_TabWidget, SIGNAL(currentChanged ( int )), this, SLOT(OnTabSwitch( int )) );

  // Zones
  connect( m_Controls.m_BtnFreeze, SIGNAL(clicked()), this, SLOT(OnFreeze()) );
  connect( m_Controls.m_ZonesWidget, SIGNAL(ZoneAdded()), this, SLOT(OnZoneAdded()) );
  // Navigation
  connect( m_Controls.m_BtnStartIntervention, SIGNAL(clicked ()), this, SLOT(OnStartIntervention()) );
  connect( m_Controls.m_BtnReset, SIGNAL(clicked ()), this, SLOT(OnReset()) );
  connect( m_Controls.m_BtnNeedleView, SIGNAL(clicked ()), this, SLOT(OnNeedleViewToogle()) );

  m_Freeze = false;
  m_IsNeedleViewActive = false;

  m_Controls.m_TabWidget->setTabEnabled(1, false);
  m_Controls.m_TabWidget->setTabEnabled(2, false);
  m_ZoneFilter = mitk::NodeDisplacementFilter::New();
  m_NeedleProjectionFilter = mitk::NeedleProjectionFilter::New();
  m_SmoothingFilter = mitk::NavigationDataSmoothingFilter::New();
  m_CameraVis = mitk::CameraVisualization::New();
  m_RangeMeterStyle = "QProgressBar:horizontal {\nborder: 1px solid gray;\nborder-radius: 3px;\nbackground: white;\npadding: 1px;\ntext-align: center;\n}\nQProgressBar::chunk:horizontal {\nbackground: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #StartColor#, stop: 0.8 #StartColor#, stop: 1 #StopColor#);\n}";
  QBoxLayout * layout = new QBoxLayout(QBoxLayout::Down, m_Controls.m_RangeBox);
  // Pedal Support for Needle View
  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_PageDown), parent);
  QObject::connect(shortcut, SIGNAL(activated()), m_Controls.m_BtnNeedleView, SLOT(animateClick()) );

  m_Controls.m_CombinedModalitySelectionWidget->OnActivateStep();
}

void USNavigation::OnDeviceSelected()
{
  m_Controls.m_BtnSelectDevices->setEnabled(true);
}

void USNavigation::OnDeviceDeselected()
{
  m_Controls.m_BtnSelectDevices->setEnabled(false);
  m_Controls.m_TabWidget->setCurrentIndex(0);
  m_Controls.m_TabWidget->setTabEnabled(1, false);
  m_Controls.m_TabWidget->setTabEnabled(2, false);
}

void USNavigation::OnSelectDevices(){
  m_USDevice = m_Controls.m_CombinedModalitySelectionWidget->GetSelectedCombinedModality();
  if (m_USDevice.IsNull())
  {
    MITK_WARN << "No device selected.";
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  // make sure that the combined modality is in connected state before using it
  if ( m_USDevice->GetDeviceState() < mitk::USDevice::State_Connected ) { m_USDevice->Connect(); }
  if ( m_USDevice->GetDeviceState() < mitk::USDevice::State_Activated ) { m_USDevice->Activate(); }
  QApplication::restoreOverrideCursor();

  m_Tracker = m_USDevice->GetNavigationDataSource();

  // Build Pipeline
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
    m_USDevice->Modified();
    m_USDevice->Update();
    mitk::Image::Pointer image = m_USDevice->GetOutput();
    if ( ! m_ImageAlreadySetToNode && image.IsNotNull() && image->IsInitialized() )
    {
      m_USStream->SetData(image);
      m_ImageAlreadySetToNode = true;

      // make a reinit on the ultrasound image
      mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
      if ( renderWindow != nullptr && image->GetTimeGeometry()->IsValid() )
      {
        renderWindow->GetRenderingManager()->InitializeViews(
          image->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        renderWindow->GetRenderingManager()->RequestUpdateAll();
      }
    }
    this->RequestRenderWindowUpdate();
  }
}

///////////////////// Range Meter ////////////////

void USNavigation::SetupProximityView()
{
  // make sure that the data nodes in m_Zones are exactly the same as
  // the zone nodes of the zone node widget
  m_Zones.clear();
  mitk::DataStorage::SetOfObjects::ConstPointer zoneNodes = m_Controls.m_ZonesWidget->GetZoneNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = zoneNodes->Begin();
       it != zoneNodes->End(); ++it)
  {
    m_Zones.push_back(it->Value());
  }

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

  float zoneColor[3];
  bool success = m_Zones.at(i)->GetColor(zoneColor);
  QString zoneColorString = "#555555";
  if (success)
  {
    QString zoneColorString = QString("#%1%2%3").arg(static_cast<unsigned int>(zoneColor[0]*255), 2, 16, QChar('0'))
      .arg(static_cast<unsigned int>(zoneColor[1]*255), 2, 16, QChar('0')).arg(static_cast<unsigned int>(zoneColor[2]*255), 2, 16, QChar('0'));
  }

  QProgressBar* meter = new QProgressBar();
  meter->setMinimum(0);
  meter->setMaximum(100);
  meter->setValue(0);
  QString zoneName = zone->GetName().c_str();
  meter->setFormat(zoneName + ": No Data");
  QString style = m_RangeMeterStyle;
  style = style.replace("#StartColor#", zoneColorString);
  style = style.replace("#StopColor#", zoneColorString);
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
    mitk::ScalarType distance = sqrt( pow(zoneOrigin[0] - needlePosition[0], 2) + pow(zoneOrigin[1] - needlePosition[1], 2) + pow(zoneOrigin[2] - needlePosition[2], 2) );
    // Subtract zone size
    float zoneSize;
    m_Zones.at(i)->GetFloatProperty("zone.size", zoneSize);
    distance = distance - zoneSize;

    // Prepare new Style
    float zoneColor[3];
    m_Zones.at(i)->GetColor(zoneColor);
    QString zoneColorString = QString("#%1%2%3").arg(static_cast<unsigned int>(zoneColor[0]*255), 2, 16, QChar('0'))
      .arg(static_cast<unsigned int>(zoneColor[1]*255), 2, 16, QChar('0')).arg(static_cast<unsigned int>(zoneColor[2]*255), 2, 16, QChar('0'));
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
      style = style.replace("#StartColor#", zoneColorString);
      style = style.replace("#StopColor#", "red");
      text  = text + ": " + QString::number(distance) + " mm";
      value = 100 -  100 * ((float) distance / (float )MAXRANGE);
    } // 3) Needle is away from zone
    else if (distance < MAXRANGE)
    {
      style = style.replace("#StartColor#", zoneColorString);
      style = style.replace("#StopColor#", zoneColorString);
      text  = text + ": " + QString::number(distance) + " mm";
      value = 100 -  100 * ((float) distance / (float )MAXRANGE);
    } // 4) Needle is far away from zone
    else
    {
      style = style.replace("#StartColor#", zoneColorString);
      style = style.replace("#StopColor#", zoneColorString);
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
  this->ResetRangeMeters();

  m_Controls.m_TabWidget->setCurrentIndex(2);
  this->SetupProximityView();
}

void USNavigation::OnFreeze()
{
  // Swap Freeze State
  m_Freeze = ! m_Freeze;

  if ( m_Freeze )
  {
    m_Controls.m_ZonesWidget->OnStartAddingZone();
  }
  else
  {
    m_Controls.m_ZonesWidget->OnAbortAddingZone();
  }
}

void USNavigation::OnReset()
{
  // Reset Zones
  m_ZoneFilter->ResetNodes();
  m_Zones.clear();
  m_Controls.m_ZonesWidget->OnResetZones();

  this->ResetRangeMeters();

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

void USNavigation::OnZoneAdded()
{
  if ( m_Freeze )
  {
    m_Controls.m_BtnFreeze->setChecked(false);
    m_Freeze = false;
  }

  mitk::DataStorage::SetOfObjects::ConstPointer zoneNodes = m_Controls.m_ZonesWidget->GetZoneNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = zoneNodes->Begin();
       it != zoneNodes->End(); ++it)
  {
    // add all zones to zone filter which aren't added until now
    if ( find(m_Zones.begin(), m_Zones.end(), it->Value()) == m_Zones.end() )
    {
      m_Zones.push_back(it->Value());
      m_ZoneFilter->AddNode(it->Value());
    }
  }
}

void USNavigation::ResetRangeMeters()
{
  m_RangeMeters.clear();

  QLayout* layout = m_Controls.m_RangeBox->layout();
  QLayoutItem *item;
  while((item = layout->takeAt(0))) {
    if (item->widget()) {
      delete item->widget();
    }
    delete item;
  }
}
