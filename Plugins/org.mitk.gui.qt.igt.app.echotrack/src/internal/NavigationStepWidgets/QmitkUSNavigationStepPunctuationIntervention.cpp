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
#include "../QmitkUSNavigationMarkerPlacement.h"

#include "usModuleRegistry.h"

#include <mitkTrackingTypes.h>

QmitkUSNavigationStepPunctuationIntervention::QmitkUSNavigationStepPunctuationIntervention(QWidget *parent) :
  QmitkUSAbstractNavigationStep(parent),
  m_Ui(new Ui::QmitkUSNavigationStepPunctuationIntervention),
  m_ZoneNodes(nullptr),
  m_NeedleProjectionFilter(mitk::NeedleProjectionFilter::New()),
  m_NeedleNavigationTool(mitk::NavigationTool::New()),
  m_OldColors(),
  m_SphereSource(vtkSmartPointer<vtkSphereSource>::New()),
  m_OBBTree(vtkSmartPointer<vtkOBBTree>::New()),
  m_IntersectPoints(vtkSmartPointer<vtkPoints>::New())
{
  m_Ui->setupUi(this);
  connect(m_Ui->m_AddNewAblationZone, SIGNAL(clicked()), this, SLOT(OnAddAblationZoneClicked()));
  connect(m_Ui->m_ShowToolAxisN, SIGNAL(stateChanged(int)), this, SLOT(OnShowToolAxisEnabled(int)));
  connect(m_Ui->m_EnableAblationMarking, SIGNAL(clicked()), this, SLOT(OnEnableAblationZoneMarkingClicked()));
  connect(m_Ui->m_AblationZoneSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnAblationZoneSizeSliderChanged(int)));
  m_Ui->m_AblationZonesBox->setVisible(false);
}

void QmitkUSNavigationStepPunctuationIntervention::SetNeedleMetaData(mitk::NavigationTool::Pointer needleNavigationTool)

{
  this->m_NeedleNavigationTool = needleNavigationTool;
}

void QmitkUSNavigationStepPunctuationIntervention::OnEnableAblationZoneMarkingClicked()
{
  if(m_Ui->m_EnableAblationMarking->isChecked())
    m_Ui->m_AblationZonesBox->setVisible(true);
  else
    m_Ui->m_AblationZonesBox->setVisible(false);
}

void QmitkUSNavigationStepPunctuationIntervention::OnAblationZoneSizeSliderChanged(int size)
{
int id = m_Ui->m_AblationZonesList->currentRow();
if (id!=-1) {emit AblationZoneChanged(id,size);}
}//

void QmitkUSNavigationStepPunctuationIntervention::OnAddAblationZoneClicked()
{
  QListWidgetItem* newItem = new QListWidgetItem("Ablation Zone (initial size: " + QString::number(m_Ui->m_AblationZoneSizeSlider->value()) + " mm)", m_Ui->m_AblationZonesList);
  newItem->setSelected(true);
  emit AddAblationZoneClicked(m_Ui->m_AblationZoneSizeSlider->value());
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

  delete m_Ui;
}

bool QmitkUSNavigationStepPunctuationIntervention::OnStartStep()
{
  // create node for Needle Projection
  mitk::DataNode::Pointer node = this->GetNamedDerivedNodeAndCreate
      ("Needle Path", QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  node->SetData(m_NeedleProjectionFilter->GetProjection());
  node->SetBoolProperty("show contour", true);
  //m_NeedleProjectionFilter->SetToolAxisForFilter(m_NeedleNavigationTool->GetToolAxis());
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
  m_ZoneNodes = dataStorage->GetDerivations(dataStorage->GetNamedNode(QmitkUSNavigationMarkerPlacement::DATANAME_ZONES));

  // add zones to the widgets for risk structures
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = m_ZoneNodes->Begin();
       it != m_ZoneNodes->End(); ++it)
  {
    m_Ui->riskStructuresRangeWidget->AddZone(it->Value());
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

void QmitkUSNavigationStepPunctuationIntervention::OnShowToolAxisEnabled(int enabled)
{
  if (enabled == 0) { m_NeedleProjectionFilter->ShowToolAxis(false); }
  else { m_NeedleProjectionFilter->ShowToolAxis(true); }
}

void QmitkUSNavigationStepPunctuationIntervention::OnUpdate()
{
  if (this->GetCombinedModality(false).IsNull()) return;
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

  m_NeedleProjectionFilter->Update();

  //Update Distance to US image
  mitk::Point3D point1 = m_NeedleProjectionFilter->GetProjection()->GetPoint(0);
  mitk::Point3D point2 = m_NeedleProjectionFilter->GetProjection()->GetPoint(1);
  double distance = point1.EuclideanDistanceTo(point2);
  m_Ui->m_DistanceToUSPlane->setText(QString::number(distance) + " mm");
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
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality(false);
  if ( combinedModality.IsNotNull() )
  {
    m_NeedleProjectionFilter->ConnectTo(combinedModality->GetNavigationDataSource());

    // set calibration of the combined modality to the needle projection filter
    mitk::AffineTransform3D::Pointer usPlaneTransform = combinedModality->GetUSPlaneTransform();
    if (usPlaneTransform.IsNotNull())
    {
      m_NeedleProjectionFilter->SetTargetPlane(usPlaneTransform);
    }
  }
  else
  {
    MITK_WARN << "CombinedModality is null!";
  }
}

void QmitkUSNavigationStepPunctuationIntervention::ClearZones()
{
  m_Ui->riskStructuresRangeWidget->ClearZones();
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
    m_Ui->bodyMarkerTrackingStatusLabel->setStyleSheet(
      "background-color: #8bff8b; margin-right: 1em; margin-left: 1em; border: 1px solid grey");
    m_Ui->bodyMarkerTrackingStatusLabel->setText("Body marker is inside the tracking volume.");
  }
  else
  {
    m_Ui->bodyMarkerTrackingStatusLabel->setStyleSheet(
          "background-color: #ff7878; margin-right: 1em; margin-left: 1em; border: 1px solid grey");
    m_Ui->bodyMarkerTrackingStatusLabel->setText("Body marker is not inside the tracking volume.");
  }

  m_Ui->riskStructuresRangeGroupBox->setEnabled(valid);
}

void QmitkUSNavigationStepPunctuationIntervention::UpdateCriticalStructures(mitk::NavigationData::Pointer needle, mitk::PointSet::Pointer path)
{
  // update the distances for the risk structures widget
  m_Ui->riskStructuresRangeWidget->UpdateDistancesToNeedlePosition(needle);

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
    mitk::Point3D point0 = path->GetPoint(0);
    mitk::Point3D point1 = path->GetPoint(1);
    if (CheckSphereLineIntersection(center,radius,point0,point1))
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

  m_OBBTree->IntersectWithLine(lineP0, lineP1, m_IntersectPoints, nullptr);

  if (m_IntersectPoints->GetNumberOfPoints() > 0) {return true;}
  else {return false;}
}
