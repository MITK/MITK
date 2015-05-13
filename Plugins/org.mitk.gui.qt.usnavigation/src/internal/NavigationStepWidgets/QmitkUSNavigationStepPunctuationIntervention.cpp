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

#include "QmitkUSNavigationStepPunctuationIntervention.h"
#include "ui_QmitkUSNavigationStepPunctuationIntervention.h"

#include "mitkNeedleProjectionFilter.h"

#include "../Widgets/QmitkZoneProgressBar.h"
#include "../USNavigationMarkerPlacement.h"

#include "usModuleRegistry.h"

#include <mitkTrackingTypes.h>

QmitkUSNavigationStepPunctuationIntervention::QmitkUSNavigationStepPunctuationIntervention(QWidget *parent) :
  QmitkUSAbstractNavigationStep(parent),
  m_NeedleProjectionFilter(mitk::NeedleProjectionFilter::New()),
  ui(new Ui::QmitkUSNavigationStepPunctuationIntervention),
  m_SphereSource(vtkSmartPointer<vtkSphereSource>::New()),
  m_OBBTree(vtkSmartPointer<vtkOBBTree>::New()),
  m_IntersectPoints(vtkSmartPointer<vtkPoints>::New())
{
  ui->setupUi(this);
  connect(ui->m_AddNewAblationZone, SIGNAL(clicked()), this, SLOT(OnAddAblationZoneClicked()));
  connect(ui->m_EnableAblationMarking, SIGNAL(clicked()), this, SLOT(OnEnableAblationZoneMarkingClicked()));
}

void QmitkUSNavigationStepPunctuationIntervention::OnEnableAblationZoneMarkingClicked()
{
  if(ui->m_EnableAblationMarking->isChecked())
    ui->m_AblationZonesBox->setEnabled(true);
  else
    ui->m_AblationZonesBox->setEnabled(false);
}

void QmitkUSNavigationStepPunctuationIntervention::OnAddAblationZoneClicked()
{
  MITK_INFO("USNavigationLogging") << "Ablation Zone Added, size: " << ui->m_AblationZoneSizeSlider->value();
  new QListWidgetItem("Ablation Zone (size: " + QString::number(ui->m_AblationZoneSizeSlider->value()) + " mm)", ui->m_AblationZonesList);
  emit AddAblationZoneClicked(ui->m_AblationZoneSizeSlider->value());
}

QmitkUSNavigationStepPunctuationIntervention::~QmitkUSNavigationStepPunctuationIntervention()
{
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage(false);
  if ( dataStorage.IsNotNull() )
  {
    // remove needle path from data storage if it is there
    mitk::DataNode::Pointer node = this->GetNamedDerivedNode
        ("Needle Path", QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
    if ( node.IsNotNull() ) { dataStorage->Remove(node); }
  }

  delete ui;
}

bool QmitkUSNavigationStepPunctuationIntervention::OnStartStep()
{
  // create node for Needle Projection
  mitk::DataNode::Pointer node = this->GetNamedDerivedNodeAndCreate
      ("Needle Path", QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  node->SetData(m_NeedleProjectionFilter->GetProjection());
  node->SetBoolProperty("show contour", true);

  return true;
}

bool QmitkUSNavigationStepPunctuationIntervention::OnRestartStep()
{
  return this->OnActivateStep();
}

bool QmitkUSNavigationStepPunctuationIntervention::OnFinishStep()
{
  mitk::DataNode::Pointer finishPunctionResult = mitk::DataNode::New();
  finishPunctionResult->SetName("PunctionResult");
  mitk::Point3D needlePos = m_NeedleProjectionFilter->GetOutput(0)->GetPosition();
  mitk::Quaternion needleRot = m_NeedleProjectionFilter->GetOutput(0)->GetOrientation();
  finishPunctionResult->SetProperty("USNavigation::TipPositionEnd", mitk::Point3dProperty::New(needlePos));
  MITK_INFO("USNavigationLogging") << "Instrument tip at end: " <<needlePos<< " / " << needleRot;
  emit SignalIntermediateResult(finishPunctionResult);
  return true;
}

bool QmitkUSNavigationStepPunctuationIntervention::OnActivateStep()
{
  this->ClearZones();

  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();


  // add progress bars for risk zone nodes
  m_ZoneNodes = dataStorage->GetDerivations(dataStorage->GetNamedNode(USNavigationMarkerPlacement::DATANAME_ZONES));

  // add zones to the widgets for risk structures
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = m_ZoneNodes->Begin();
       it != m_ZoneNodes->End(); ++it)
  {
    ui->riskStructuresRangeWidget->AddZone(it->Value());
    float rgb[3];
    it->Value()->GetColor(rgb);
    mitk::Color color;
    color.SetRed(rgb[0]);
    color.SetGreen(rgb[1]);
    color.SetBlue(rgb[2]);
    m_OldColors[it->Value()] = color;
  }

  m_NeedleProjectionFilter->SelectInput(0);

  return true;
}

void QmitkUSNavigationStepPunctuationIntervention::OnUpdate()
{
  // get navigation data source and make sure that it is not null
  mitk::NavigationDataSource::Pointer navigationDataSource =
      this->GetCombinedModality()->GetNavigationDataSource();
  if ( navigationDataSource.IsNull() )
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPunctuationIntervention")
    << "Navigation Data Source of Combined Modality must not be null.";
    mitkThrow() << "Navigation Data Source of Combined Modality must not be null.";
  }
  // update body marker
  this->UpdateBodyMarkerStatus(navigationDataSource->GetOutput(1));
  // update critical structures
  this->UpdateCriticalStructures(navigationDataSource->GetOutput(0),m_NeedleProjectionFilter->GetProjection());

  //Update Distance to US image
  mitk::Point3D point1 = m_NeedleProjectionFilter->GetProjection()->GetPoint(0);
  mitk::Point3D point2 = m_NeedleProjectionFilter->GetProjection()->GetPoint(1);
  double distance = point1.EuclideanDistanceTo(point2);
  ui->m_DistanceToUSPlane->setText(QString::number(distance) + " mm");
}

void QmitkUSNavigationStepPunctuationIntervention::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
  if ( settingsNode.IsNull() ) { return; }
}

QString QmitkUSNavigationStepPunctuationIntervention::GetTitle()
{
  return "Computer-assisted Intervention";
}

bool QmitkUSNavigationStepPunctuationIntervention::GetIsRestartable()
{
  return false;
}

QmitkUSNavigationStepPunctuationIntervention::FilterVector QmitkUSNavigationStepPunctuationIntervention::GetFilter()
{
  return FilterVector(1, m_NeedleProjectionFilter.GetPointer());
}

void QmitkUSNavigationStepPunctuationIntervention::OnSetCombinedModality()
{
  mitk::USCombinedModality::Pointer combinedModality = this->GetCombinedModality(false);
  if ( combinedModality.IsNotNull() )
  {
    // set calibration of the combined modality to the needle projection filter
    mitk::AffineTransform3D::Pointer calibration = combinedModality->GetCalibration();
    if ( calibration.IsNotNull() )
    {
      m_NeedleProjectionFilter->SetTargetPlane(calibration);
    }
  }
}

void QmitkUSNavigationStepPunctuationIntervention::ClearZones()
{
  ui->riskStructuresRangeWidget->ClearZones();
}

void QmitkUSNavigationStepPunctuationIntervention::UpdateBodyMarkerStatus(mitk::NavigationData::Pointer bodyMarker)
{
  if ( bodyMarker.IsNull() )
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPunctuationIntervention")
      << "Current Navigation Data for body marker of Combined Modality must not be null.";
    mitkThrow() << "Current Navigation Data for body marker of Combined Modality must not be null.";
  }

  bool valid = bodyMarker->IsDataValid();

  // update body marker status label
  if (valid)
  {
    ui->bodyMarkerTrackingStatusLabel->setStyleSheet(
      "background-color: #8bff8b; margin-right: 1em; margin-left: 1em; border: 1px solid grey");
    ui->bodyMarkerTrackingStatusLabel->setText("Body marker is inside the tracking volume.");
  }
  else
  {
    ui->bodyMarkerTrackingStatusLabel->setStyleSheet(
          "background-color: #ff7878; margin-right: 1em; margin-left: 1em; border: 1px solid grey");
    ui->bodyMarkerTrackingStatusLabel->setText("Body marker is not inside the tracking volume.");
  }

  ui->riskStructuresRangeGroupBox->setEnabled(valid);
}

void QmitkUSNavigationStepPunctuationIntervention::UpdateCriticalStructures(mitk::NavigationData::Pointer needle, mitk::PointSet::Pointer path)
{
  // update the distances for the risk structures widget
  ui->riskStructuresRangeWidget->UpdateDistancesToNeedlePosition(needle);

  //iterate through all zones
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = m_ZoneNodes->Begin();
       it != m_ZoneNodes->End(); ++it)
  {
    mitk::DataNode::Pointer currentNode = it->Value();
    //get center point and radius
    float radius = -1;
    mitk::Point3D center;
    currentNode->GetFloatProperty("zone.size", radius);
    center = currentNode->GetData()->GetGeometry()->GetIndexToWorldTransform()->GetTranslation();
    if (CheckSphereLineIntersection(center,radius,path->GetPoint(0),path->GetPoint(1)))
      {currentNode->SetColor(mitk::IGTColor_WARNING);}
    else
      {currentNode->SetColor(m_OldColors[currentNode]);}
  }
}

bool QmitkUSNavigationStepPunctuationIntervention::CheckSphereLineIntersection(mitk::Point3D& sphereOrigin, float& sphereRadius, mitk::Point3D& lineStart, mitk::Point3D& lineEnd)
{
  double center[3] = {sphereOrigin[0],sphereOrigin[1],sphereOrigin[2]};
  m_SphereSource->SetCenter(center);
  m_SphereSource->SetRadius(sphereRadius);
  m_SphereSource->Update();

  m_OBBTree->SetDataSet(m_SphereSource->GetOutput());
  m_OBBTree->BuildLocator();

  double lineP0[3] = {lineStart[0], lineStart[1], lineStart[2]};
  double lineP1[3] = {lineEnd[0], lineEnd[1], lineEnd[2]};

  m_OBBTree->IntersectWithLine(lineP0, lineP1, m_IntersectPoints, NULL);

  if (m_IntersectPoints->GetNumberOfPoints() > 0) {return true;}
  else {return false;}
}
