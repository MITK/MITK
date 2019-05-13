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

#include "QmitkUSNavigationStepMarkerIntervention.h"
#include "ui_QmitkUSNavigationStepMarkerIntervention.h"

#include "mitkBaseRenderer.h"
#include "mitkContourModel.h"
#include "mitkNeedleProjectionFilter.h"
#include "mitkNodeDisplacementFilter.h"
#include "mitkSurface.h"
#include "mitkTextAnnotation2D.h"
#include <mitkBaseGeometry.h>
#include <mitkManualPlacementAnnotationRenderer.h>

#include "../Filter/mitkUSNavigationTargetIntersectionFilter.h"
#include "../Filter/mitkUSNavigationTargetOcclusionFilter.h"
#include "../Filter/mitkUSNavigationTargetUpdateFilter.h"

#include "../QmitkUSNavigationMarkerPlacement.h"
#include "../Widgets/QmitkZoneProgressBar.h"
#include "../mitkUSTargetPlacementQualityCalculator.h"

#include "../Interactors/mitkUSPointMarkInteractor.h"
#include "usModuleRegistry.h"

#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"

#include "mitkSurface.h"

// VTK
#include "vtkCellLocator.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkLinearTransform.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkOBBTree.h"
#include "vtkPointData.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSelectEnclosedPoints.h"
#include "vtkSmartPointer.h"
#include <vtkSphereSource.h>
#include "vtkTransformPolyDataFilter.h"
#include "vtkWarpScalar.h"


QmitkUSNavigationStepMarkerIntervention::QmitkUSNavigationStepMarkerIntervention(QWidget *parent)
  : QmitkUSAbstractNavigationStep(parent),
    m_NumberOfTargets(0),
    m_PlannedTargetsNodes(),
    m_ReachedTargetsNodes(),
    m_TargetProgressBar(new QmitkZoneProgressBar(QString::fromStdString("Target: %1 mm"), 200, 0, this)),
    m_PlannedTargetProgressBar(nullptr),
    m_CurrentTargetIndex(0),
    m_CurrentTargetReached(false),
    m_ShowPlanningColors(false),
    m_PointMarkInteractor(mitk::USPointMarkInteractor::New()),
    m_TargetNode(nullptr),
    m_TargetColorLookupTableProperty(nullptr),
    m_TargetSurface(nullptr),
    m_NeedleProjectionFilter(mitk::NeedleProjectionFilter::New()),
    m_NodeDisplacementFilter(mitk::NodeDisplacementFilter::New()),
    m_TargetUpdateFilter(mitk::USNavigationTargetUpdateFilter::New()),
    m_TargetOcclusionFilter(mitk::USNavigationTargetOcclusionFilter::New()),
    m_TargetIntersectionFilter(mitk::USNavigationTargetIntersectionFilter::New()),
    m_PlacementQualityCalculator(mitk::USTargetPlacementQualityCalculator::New()),
    m_TargetStructureWarnOverlay(mitk::TextAnnotation2D::New()),
    m_ReferenceSensorName(),
    m_NeedleSensorName(),
    m_ReferenceSensorIndex(1),
    m_NeedleSensorIndex(0),
    m_ListenerTargetCoordinatesChanged(this, &QmitkUSNavigationStepMarkerIntervention::UpdateTargetCoordinates),
    ui(new Ui::QmitkUSNavigationStepMarkerIntervention)
{
  m_ActiveTargetColor[0] = 1;
  m_ActiveTargetColor[1] = 1;
  m_ActiveTargetColor[2] = 0;
  m_InactiveTargetColor[0] = 1;
  m_InactiveTargetColor[1] = 1;
  m_InactiveTargetColor[2] = 0.5;
  m_ReachedTargetColor[0] = 0.6;
  m_ReachedTargetColor[1] = 1;
  m_ReachedTargetColor[2] = 0.6;

  ui->setupUi(this);

  connect(ui->freezeImageButton, SIGNAL(SignalFreezed(bool)), this, SLOT(OnFreeze(bool)));
  connect(ui->backToLastTargetButton, SIGNAL(clicked()), this, SLOT(OnBackToLastTargetClicked()));
  connect(ui->targetReachedButton, SIGNAL(clicked()), this, SLOT(OnTargetLeft()));
  connect(this, SIGNAL(TargetReached(int)), this, SLOT(OnTargetReached()));
  connect(this, SIGNAL(TargetLeft(int)), this, SLOT(OnTargetLeft()));
  connect(ui->riskStructuresRangeWidget,
          SIGNAL(SignalZoneViolated(const mitk::DataNode *, mitk::Point3D)),
          this,
          SLOT(OnRiskZoneViolated(const mitk::DataNode *, mitk::Point3D)));

  m_PointMarkInteractor->CoordinatesChangedEvent.AddListener(m_ListenerTargetCoordinatesChanged);

  this->GenerateTargetColorLookupTable();

  m_TargetProgressBar->SetTextFormatInvalid("Target is not on Needle Path");
  ui->targetStructuresRangeLayout->addWidget(m_TargetProgressBar);

  m_TargetUpdateFilter->SetScalarArrayIdentifier("USNavigation::ReachedTargetScores");
}

QmitkUSNavigationStepMarkerIntervention::~QmitkUSNavigationStepMarkerIntervention()
{
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage(false);
  if (dataStorage.IsNotNull())
  {
    // remove the node for the needle path
    mitk::DataNode::Pointer node =
      this->GetNamedDerivedNode("Needle Path", QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
    if (node.IsNotNull())
    {
      dataStorage->Remove(node);
    }
  }

  delete ui;

  m_PointMarkInteractor->CoordinatesChangedEvent.RemoveListener(m_ListenerTargetCoordinatesChanged);
}

bool QmitkUSNavigationStepMarkerIntervention::OnStartStep()
{
  m_NeedleProjectionFilter->SelectInput(m_NeedleSensorIndex);

  // create node for Needle Projection
  mitk::DataNode::Pointer node =
    this->GetNamedDerivedNodeAndCreate("Needle Path", QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  node->SetData(m_NeedleProjectionFilter->GetProjection());
  node->SetBoolProperty("show contour", true);

  // initialize warning overlay (and do not display it, yet)
  m_TargetStructureWarnOverlay->SetText("Warning: Needle is Inside the Target Structure.");
  m_TargetStructureWarnOverlay->SetVisibility(false);

  // set position and font size for the text overlay
  mitk::Point2D overlayPosition;
  overlayPosition.SetElement(0, 10.0f);
  overlayPosition.SetElement(1, 10.0f);
  m_TargetStructureWarnOverlay->SetPosition2D(overlayPosition);
  m_TargetStructureWarnOverlay->SetFontSize(18);

  // overlay should be red
  mitk::Color color;
  color[0] = 1;
  color[1] = 0;
  color[2] = 0;
  m_TargetStructureWarnOverlay->SetColor(color);

  mitk::ManualPlacementAnnotationRenderer::AddAnnotation(m_TargetStructureWarnOverlay.GetPointer(), "stdmulti.widget4");

  return true;
}

bool QmitkUSNavigationStepMarkerIntervention::OnStopStep()
{
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();

  // remove all reached nodes from the data storage
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_ReachedTargetsNodes.begin();
       it != m_ReachedTargetsNodes.end();
       ++it)
  {
    dataStorage->Remove(*it);
  }
  m_ReachedTargetsNodes.clear();
  m_CurrentTargetIndex = 0;

  // reset button states
  ui->freezeImageButton->setEnabled(false);
  ui->backToLastTargetButton->setEnabled(false);
  ui->targetReachedButton->setEnabled(true);

  // make sure that it is unfreezed after stopping the step
  ui->freezeImageButton->Unfreeze();

  // remove base node for reached targets from the data storage
  mitk::DataNode::Pointer reachedTargetsNode = this->GetNamedDerivedNode(
    QmitkUSAbstractNavigationStep::DATANAME_BASENODE, QmitkUSNavigationMarkerPlacement::DATANAME_REACHED_TARGETS);
  if (reachedTargetsNode.IsNotNull())
  {
    dataStorage->Remove(reachedTargetsNode);
  }

  return true;
}

bool QmitkUSNavigationStepMarkerIntervention::OnFinishStep()
{
  return true;
}

bool QmitkUSNavigationStepMarkerIntervention::OnActivateStep()
{
  this->ClearZones(); // clear risk zones before adding new ones

  // get target node from data storage and make sure that it contains data
  m_TargetNode = this->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_TARGETSURFACE,
    QmitkUSNavigationMarkerPlacement::DATANAME_TUMOUR);
  if (m_TargetNode.IsNull() || m_TargetNode->GetData() == 0)
  {
    mitkThrow() << "Target node (" << QmitkUSNavigationMarkerPlacement::DATANAME_TARGETSURFACE << ") must not be null.";
  }

  // get target data and make sure that it is a surface
  m_TargetSurface = dynamic_cast<mitk::Surface *>(m_TargetNode->GetData());
  if (m_TargetSurface.IsNull())
  {
    mitkThrow() << "Target node (" << QmitkUSNavigationMarkerPlacement::DATANAME_TARGETSURFACE
                << ") data must be of type mitk::Surface";
  }

  // delete progress bars for reinitializing them again afterwards
  if (m_PlannedTargetProgressBar)
  {
    ui->targetStructuresRangeLayout->removeWidget(m_PlannedTargetProgressBar);
    delete m_PlannedTargetProgressBar;
    m_PlannedTargetProgressBar = 0;
  }

  m_NodeDisplacementFilter->SelectInput(m_ReferenceSensorIndex);

  this->UpdateTargetProgressDisplay();

  mitk::DataNode::Pointer tumourNode = this->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_TUMOUR,
                                                                 QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  if (tumourNode.IsNotNull())
  {
    // do not show tumour node during intervention (target surface is shown)
    tumourNode->SetBoolProperty("visible", false);

    // add tumour as a risk structure
    ui->riskStructuresRangeWidget->AddZone(tumourNode);
  }

  // set target structure for target update filter
  m_TargetUpdateFilter->SetTargetStructure(m_TargetNode);
  m_TargetOcclusionFilter->SetTargetStructure(m_TargetNode);

  // set lookup table of tumour node
  m_TargetNode->SetProperty("LookupTable", m_TargetColorLookupTableProperty);

  //
  mitk::DataNode::Pointer targetsBaseNode = this->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS,
                                                                      QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  mitk::DataStorage::SetOfObjects::ConstPointer plannedTargetNodes;
  if (targetsBaseNode.IsNotNull())
  {
    plannedTargetNodes = this->GetDataStorage()->GetDerivations(targetsBaseNode);
  }
  if (plannedTargetNodes.IsNotNull() && plannedTargetNodes->Size() > 0)
  {
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = plannedTargetNodes->Begin();
         it != plannedTargetNodes->End();
         ++it)
    {
      m_PlannedTargetsNodes.push_back(it->Value());
    }

    m_PlannedTargetProgressBar = new QmitkZoneProgressBar(QString::fromStdString("Planned Target"), 200, 0);
    ui->targetStructuresRangeLayout->addWidget(m_PlannedTargetProgressBar);
  }

  // add progress bars for risk zone nodes
  mitk::DataNode::Pointer zonesBaseNode = this->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_ZONES,
                                                                    QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  // only add progress bars if the base node for zones was created
  if (zonesBaseNode.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer zoneNodes = this->GetDataStorage()->GetDerivations(zonesBaseNode);

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = zoneNodes->Begin(); it != zoneNodes->End(); ++it)
    {
      ui->riskStructuresRangeWidget->AddZone(it->Value());
    }

    m_TargetOcclusionFilter->SelectStartPositionInput(m_NeedleSensorIndex);
    m_TargetOcclusionFilter->SetObstacleStructures(zoneNodes);
  }

  return true;
}

bool QmitkUSNavigationStepMarkerIntervention::OnDeactivateStep()
{
  ui->freezeImageButton->Unfreeze();

  return true;
}

void QmitkUSNavigationStepMarkerIntervention::OnUpdate()
{
  // get navigation data source and make sure that it is not null
  mitk::NavigationDataSource::Pointer navigationDataSource = this->GetCombinedModality()->GetNavigationDataSource();
  if (navigationDataSource.IsNull())
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")
    ("QmitkUSNavigationStepMarkerIntervention") << "Navigation Data Source of Combined Modality must not be null.";
    mitkThrow() << "Navigation Data Source of Combined Modality must not be null.";
  }

  ui->riskStructuresRangeWidget->UpdateDistancesToNeedlePosition(navigationDataSource->GetOutput(m_NeedleSensorIndex));
  this->UpdateBodyMarkerStatus(navigationDataSource->GetOutput(m_ReferenceSensorIndex));

  this->UpdateTargetColors();
  this->UpdateTargetScore();

  this->UpdateTargetViolationStatus();
}

void QmitkUSNavigationStepMarkerIntervention::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
  if (settingsNode.IsNull())
  {
    return;
  }

  int numberOfTargets;
  if (settingsNode->GetIntProperty("settings.number-of-targets", numberOfTargets))
  {
    m_NumberOfTargets = numberOfTargets;
    m_TargetUpdateFilter->SetNumberOfTargets(numberOfTargets);

    m_PlacementQualityCalculator->SetOptimalAngle(m_TargetUpdateFilter->GetOptimalAngle());
  }

  std::string referenceSensorName;
  if (settingsNode->GetStringProperty("settings.reference-name-selected", referenceSensorName))
  {
    m_ReferenceSensorName = referenceSensorName;
  }

  std::string needleSensorName;
  if (settingsNode->GetStringProperty("settings.needle-name-selected", needleSensorName))
  {
    m_NeedleSensorName = needleSensorName;
  }

  this->UpdateSensorsNames();
}

QString QmitkUSNavigationStepMarkerIntervention::GetTitle()
{
  return "Computer-assisted Intervention";
}

bool QmitkUSNavigationStepMarkerIntervention::GetIsRestartable()
{
  return true;
}

QmitkUSAbstractNavigationStep::FilterVector QmitkUSNavigationStepMarkerIntervention::GetFilter()
{
  FilterVector filter;
  filter.push_back(m_NeedleProjectionFilter.GetPointer());
  filter.push_back(m_NodeDisplacementFilter.GetPointer());
  filter.push_back(m_TargetOcclusionFilter.GetPointer());
  return filter;
}

void QmitkUSNavigationStepMarkerIntervention::OnSetCombinedModality()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality(false);
  if (combinedModality.IsNotNull())
  {
    mitk::AffineTransform3D::Pointer calibration = combinedModality->GetCalibration();
    if (calibration.IsNotNull())
    {
      m_NeedleProjectionFilter->SetTargetPlane(calibration);
    }
  }

  ui->freezeImageButton->SetCombinedModality(combinedModality, m_ReferenceSensorIndex);

  this->UpdateSensorsNames();
}

void QmitkUSNavigationStepMarkerIntervention::OnTargetReached()
{
  m_CurrentTargetReached = true;
}

void QmitkUSNavigationStepMarkerIntervention::OnTargetLeft()
{
  m_CurrentTargetReached = false;

  m_CurrentTargetIndex++;

  if (m_CurrentTargetIndex >= 0 && static_cast<unsigned int>(m_CurrentTargetIndex) >= m_NumberOfTargets)
  {
    ui->targetReachedButton->setDisabled(true);
  }

  ui->backToLastTargetButton->setEnabled(true);
  ui->freezeImageButton->setEnabled(true);

  this->UpdateTargetProgressDisplay();

  if (m_ReachedTargetsNodes.size() < m_CurrentTargetIndex)
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetName(
      (QString("Target ") + QString("%1").arg(m_CurrentTargetIndex, 2, 10, QLatin1Char('0'))).toStdString());
    this->GetDataStorage()->Add(
      node,
      this->GetNamedDerivedNodeAndCreate(QmitkUSNavigationMarkerPlacement::DATANAME_REACHED_TARGETS,
                                         QmitkUSAbstractNavigationStep::DATANAME_BASENODE));
    m_ReachedTargetsNodes.push_back(node);
  }

  mitk::DataNode::Pointer node = m_ReachedTargetsNodes.at(m_CurrentTargetIndex - 1);
  mitk::Surface::Pointer zone = mitk::Surface::New();

  // create a vtk sphere with given radius
  vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkSphere->SetRadius(5);
  vtkSphere->SetCenter(0, 0, 0);
  vtkSphere->Update();
  zone->SetVtkPolyData(vtkSphere->GetOutput());

  // set vtk sphere and origin to data node
  node->SetData(zone);
  node->GetData()->GetGeometry()->SetOrigin(
    this->GetCombinedModality()->GetNavigationDataSource()->GetOutput(m_NeedleSensorIndex)->GetPosition());
  node->SetColor(0.2, 0.9, 0.2);
  this->UpdateTargetCoordinates(node);
}

void QmitkUSNavigationStepMarkerIntervention::OnBackToLastTargetClicked()
{
  if (m_CurrentTargetIndex < 1)
  {
    MITK_WARN << "Cannot go back to last target as there is no last target.";
    return;
  }

  m_CurrentTargetIndex--;

  if (m_ReachedTargetsNodes.size() > m_CurrentTargetIndex)
  {
    this->GetDataStorage()->Remove(m_ReachedTargetsNodes.last());
    MITK_INFO("QmitkUSAbstractNavigationStep")
    ("QmitkUSNavigationStepMarkerIntervention") << "Removed Target " << m_ReachedTargetsNodes.size();
    m_ReachedTargetsNodes.pop_back();
  }

  if (m_CurrentTargetIndex == 0)
  {
    ui->backToLastTargetButton->setDisabled(true);
  }
  if (m_CurrentTargetIndex >= 0 && static_cast<unsigned int>(m_CurrentTargetIndex) < m_NumberOfTargets)
  {
    ui->targetReachedButton->setEnabled(true);
  }

  ui->freezeImageButton->setEnabled(false);
  ui->freezeImageButton->Unfreeze();

  this->UpdateTargetProgressDisplay();

  m_TargetUpdateFilter->RemovePositionOfTarget(m_CurrentTargetIndex);
}

void QmitkUSNavigationStepMarkerIntervention::OnFreeze(bool freezed)
{
  if (freezed)
  {
    this->GetCombinedModality()->SetIsFreezed(true);
    // load state machine and event config for data interactor
    m_PointMarkInteractor->LoadStateMachine("USPointMarkInteractions.xml", us::ModuleRegistry::GetModule("MitkUS"));
    m_PointMarkInteractor->SetEventConfig("globalConfig.xml");

    if (m_CurrentTargetIndex < 1)
    {
      mitkThrow() << "Current target index has to be greater zero when freeze button is clicked.";
    }

    if (m_ReachedTargetsNodes.size() < m_CurrentTargetIndex)
    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetName(
        (QString("Target ") + QString("%1").arg(m_CurrentTargetIndex, 2, 10, QLatin1Char('0'))).toStdString());
      this->GetDataStorage()->Add(
        node,
        this->GetNamedDerivedNodeAndCreate(QmitkUSNavigationMarkerPlacement::DATANAME_REACHED_TARGETS,
                                           QmitkUSAbstractNavigationStep::DATANAME_BASENODE));
      m_ReachedTargetsNodes.push_back(node);
    }

    m_PointMarkInteractor->SetDataNode(m_ReachedTargetsNodes.last());
  }
  else
  {
    m_PointMarkInteractor->SetDataNode(0);
    this->GetCombinedModality()->SetIsFreezed(false);
  }
}

void QmitkUSNavigationStepMarkerIntervention::OnShowPlanningView(bool show)
{
  m_ShowPlanningColors = show;
}

void QmitkUSNavigationStepMarkerIntervention::OnRiskZoneViolated(const mitk::DataNode *node, mitk::Point3D position)
{
  MITK_INFO << "Risk zone (" << node->GetName() << ") violated at position " << position << ".";
}

void QmitkUSNavigationStepMarkerIntervention::ClearZones()
{
  ui->riskStructuresRangeWidget->ClearZones();

  // remove all reached target nodes from the data storage and clear the list
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
  for (QVector<mitk::DataNode::Pointer>::iterator it = m_ReachedTargetsNodes.begin(); it != m_ReachedTargetsNodes.end();
       ++it)
  {
    if (it->IsNotNull())
    {
      dataStorage->Remove(*it);
    }
  }
  m_ReachedTargetsNodes.clear();
}

void QmitkUSNavigationStepMarkerIntervention::UpdateTargetCoordinates(mitk::DataNode *dataNode)
{
  m_NodeDisplacementFilter->ResetNodes();
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_ReachedTargetsNodes.begin();
       it != m_ReachedTargetsNodes.end();
       ++it)
  {
    if (it->IsNotNull() && (*it)->GetData() != 0)
    {
      m_NodeDisplacementFilter->AddNode(*it);
    }
  }

  mitk::BaseData *baseData = dataNode->GetData();
  if (!baseData)
  {
    mitkThrow() << "Data of the data node must not be null.";
  }

  mitk::BaseGeometry::Pointer geometry = baseData->GetGeometry();
  if (geometry.IsNull())
  {
    mitkThrow() << "Geometry of the data node must not be null.";
  }

  m_TargetUpdateFilter->SetControlNode(m_CurrentTargetIndex - 1, dataNode);

  if (m_PlannedTargetsNodes.size() > m_CurrentTargetIndex - 1)
  {
    m_PlannedTargetsNodes.at(m_CurrentTargetIndex - 1)->SetVisibility(false);
  }

  MITK_INFO("QmitkUSAbstractNavigationStep")
  ("QmitkUSNavigationStepMarkerIntervention") << "Target " << m_CurrentTargetIndex << " reached at position "
                                              << geometry->GetOrigin();

  this->CalculateTargetPlacementQuality();
}

void QmitkUSNavigationStepMarkerIntervention::UpdateBodyMarkerStatus(mitk::NavigationData::Pointer bodyMarker)
{
  if (bodyMarker.IsNull())
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")
    ("QmitkUSNavigationStepMarkerIntervention")
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

  ui->targetStructuresRangeGroupBox->setEnabled(valid);
  ui->riskStructuresRangeGroupBox->setEnabled(valid);
}

void QmitkUSNavigationStepMarkerIntervention::GenerateTargetColorLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetHueRange(0.0, 0.33);
  lookupTable->SetSaturationRange(1.0, 1.0);
  lookupTable->SetValueRange(1.0, 1.0);
  lookupTable->SetTableRange(0.0, 1.0);
  lookupTable->Build();

  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut->SetVtkLookupTable(lookupTable);

  m_TargetColorLookupTableProperty = mitk::LookupTableProperty::New(lut);
}

void QmitkUSNavigationStepMarkerIntervention::UpdateTargetColors()
{
  if (m_TargetNode.IsNull())
  {
    return;
  }

  m_TargetNode->SetColor(1, 1, 1);

  mitk::BaseData *targetNodeData = m_TargetNode->GetData();
  if (targetNodeData == 0)
  {
    return;
  }

  mitk::Surface::Pointer targetNodeSurface = dynamic_cast<mitk::Surface *>(targetNodeData);
  vtkSmartPointer<vtkPolyData> targetNodeSurfaceVtk = targetNodeSurface->GetVtkPolyData();

  vtkPointData *targetPointData = targetNodeSurface->GetVtkPolyData()->GetPointData();

  vtkFloatArray *scalars = dynamic_cast<vtkFloatArray *>(targetPointData->GetScalars("USNavigation::Occlusion"));
  vtkFloatArray *targetScoreScalars;

  if (m_ShowPlanningColors)
  {
    targetScoreScalars = dynamic_cast<vtkFloatArray *>(targetPointData->GetScalars("USNavigation::PlanningScalars"));
  }
  else
  {
    targetScoreScalars =
      dynamic_cast<vtkFloatArray *>(targetPointData->GetScalars("USNavigation::ReachedTargetScores"));
  }

  if (!scalars || !targetScoreScalars)
  {
    return;
  }

  unsigned int numberOfTupels = scalars->GetNumberOfTuples();

  vtkSmartPointer<vtkFloatArray> colors = vtkSmartPointer<vtkFloatArray>::New();
  colors->SetNumberOfComponents(1);
  colors->SetNumberOfTuples(numberOfTupels);
  colors->SetName("Colors");

  double color, intersection, markerScore;

  for (unsigned int n = 0; n < numberOfTupels; n++)
  {
    scalars->GetTuple(n, &intersection);
    targetScoreScalars->GetTuple(n, &markerScore);

    if (intersection > 0)
    {
      color = 0;
    }
    else
    {
      color = markerScore;
    }

    colors->SetTuple(n, &color);
  }

  if (numberOfTupels > 0)
  {
    targetNodeSurfaceVtk->GetPointData()->SetScalars(colors);
    targetNodeSurfaceVtk->GetPointData()->Update();
  }
}

void QmitkUSNavigationStepMarkerIntervention::UpdateTargetScore()
{
  if (m_NeedleProjectionFilter->GetProjection()->GetSize() != 2)
  {
    return;
  }

  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = m_TargetSurface->GetVtkPolyData();

  m_TargetIntersectionFilter->SetTargetSurface(m_TargetSurface);
  m_TargetIntersectionFilter->SetLine(m_NeedleProjectionFilter->GetProjection());

  m_TargetIntersectionFilter->CalculateIntersection();

  if (m_TargetIntersectionFilter->GetIsIntersecting())
  {
    vtkFloatArray *scalars =
      dynamic_cast<vtkFloatArray *>(targetSurfaceVtk->GetPointData()->GetScalars("USNavigation::ReachedTargetScores"));
    double score;
    scalars->GetTuple(m_TargetIntersectionFilter->GetIntersectionNearestSurfacePointId(), &score);

    double color[3];
    m_TargetColorLookupTableProperty->GetLookupTable()->GetVtkLookupTable()->GetColor(score, color);

    float colorF[3];
    colorF[0] = color[0];
    colorF[1] = color[1];
    colorF[2] = color[2];
    m_TargetProgressBar->SetColor(colorF);
    m_TargetProgressBar->SetBorderColor(colorF);
    m_TargetProgressBar->setValue(m_TargetIntersectionFilter->GetDistanceToIntersection());

    if (m_PlannedTargetProgressBar)
    {
      vtkFloatArray *scalars =
        dynamic_cast<vtkFloatArray *>(targetSurfaceVtk->GetPointData()->GetScalars("USNavigation::PlanningScalars"));
      if (scalars)
      {
        double score;
        scalars->GetTuple(m_TargetIntersectionFilter->GetIntersectionNearestSurfacePointId(), &score);

        double color[3];
        m_TargetColorLookupTableProperty->GetLookupTable()->GetVtkLookupTable()->GetColor(score, color);

        float colorF[3];
        colorF[0] = color[0];
        colorF[1] = color[1];
        colorF[2] = color[2];
        m_PlannedTargetProgressBar->SetColor(colorF);
        m_PlannedTargetProgressBar->SetBorderColor(colorF);
        m_PlannedTargetProgressBar->SetTextFormatValid("Planned Target: %1 mm");

        mitk::Point3D intersectionPoint = m_TargetIntersectionFilter->GetIntersectionPoint();

        mitk::ScalarType minDistance = -1;
        for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetsNodes.begin();
             it != m_PlannedTargetsNodes.end();
             ++it)
        {
          mitk::ScalarType distance =
            intersectionPoint.EuclideanDistanceTo((*it)->GetData()->GetGeometry()->GetOrigin());
          if (minDistance < 0 || distance < minDistance)
          {
            minDistance = distance;
          }
        }

        m_PlannedTargetProgressBar->setValue(minDistance);
      }
    }
  }
  else
  {
    m_TargetProgressBar->setValueInvalid();
  }
}

void QmitkUSNavigationStepMarkerIntervention::UpdateTargetProgressDisplay()
{
  QString description;
  if (m_CurrentTargetIndex >= static_cast<int>(m_NumberOfTargets))
  {
    description = "All Targets Reached";
    if (m_TargetProgressBar)
    {
      m_TargetProgressBar->hide();
    }
  }
  else
  {
    description = QString("Distance to Target ") + QString::number(m_CurrentTargetIndex + 1) + QString(" of ") +
                  QString::number(m_NumberOfTargets);
    if (m_TargetProgressBar)
    {
      m_TargetProgressBar->show();
    }
  }

  ui->targetStructuresRangeGroupBox->setTitle(description);
}

void QmitkUSNavigationStepMarkerIntervention::UpdatePlannedTargetProgressDisplay()
{
  // make sure that the needle projection consists of two points
  if (m_NeedleProjectionFilter->GetProjection()->GetSize() != 2)
  {
    return;
  }

  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = m_TargetSurface->GetVtkPolyData();

  m_TargetIntersectionFilter->SetTargetSurface(m_TargetSurface);
  m_TargetIntersectionFilter->SetLine(m_NeedleProjectionFilter->GetProjection());

  m_TargetIntersectionFilter->CalculateIntersection();

  // update target progress bar according to the color of the intersection
  // point on the target surface and the distance to the intersection
  if (m_TargetIntersectionFilter->GetIsIntersecting())
  {
    vtkFloatArray *scalars = dynamic_cast<vtkFloatArray *>(targetSurfaceVtk->GetPointData()->GetScalars("Colors"));
    double score;
    scalars->GetTuple(m_TargetIntersectionFilter->GetIntersectionNearestSurfacePointId(), &score);

    double color[3];
    m_TargetColorLookupTableProperty->GetLookupTable()->GetVtkLookupTable()->GetColor(score, color);

    float colorF[3];
    colorF[0] = color[0];
    colorF[1] = color[1];
    colorF[2] = color[2];
    m_TargetProgressBar->SetColor(colorF);
    m_TargetProgressBar->SetBorderColor(colorF);
    m_TargetProgressBar->setValue(m_TargetIntersectionFilter->GetDistanceToIntersection());
  }
  else
  {
    float red[3] = {0.6f, 0.0f, 0.0f};
    m_TargetProgressBar->SetBorderColor(red);
    m_TargetProgressBar->setValueInvalid();
  }
}

void QmitkUSNavigationStepMarkerIntervention::UpdateTargetViolationStatus()
{
  // transform vtk polydata according to mitk geometry
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(0, m_TargetSurface->GetVtkPolyData());
  transformFilter->SetTransform(m_TargetSurface->GetGeometry()->GetVtkTransform());
  transformFilter->Update();

  vtkSmartPointer<vtkSelectEnclosedPoints> enclosedPoints = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
  enclosedPoints->Initialize(transformFilter->GetOutput());

  mitk::Point3D needleTip = m_NeedleProjectionFilter->GetProjection()->GetPoint(0);

  // show warning if the needle tip is inside the target surface
  if (enclosedPoints->IsInsideSurface(needleTip[0], needleTip[1], needleTip[2]))
  {
    if (!m_TargetStructureWarnOverlay->IsVisible(NULL))
    {
      m_TargetStructureWarnOverlay->SetVisibility(true);

      mitk::DataNode::Pointer targetViolationResult = mitk::DataNode::New();
      targetViolationResult->SetName("TargetViolation");
      targetViolationResult->SetProperty("USNavigation::TargetViolationPoint", mitk::Point3dProperty::New(needleTip));

      emit SignalIntermediateResult(targetViolationResult);
    }

    MITK_INFO("QmitkUSAbstractNavigationStep")
    ("QmitkUSNavigationStepMarkerIntervention") << "Target surface violated at " << needleTip << ".";
  }
  else
  {
    m_TargetStructureWarnOverlay->SetVisibility(false);
  }
}

void QmitkUSNavigationStepMarkerIntervention::CalculateTargetPlacementQuality()
{
  // clear quality display if there aren't all targets reached
  if (m_ReachedTargetsNodes.size() != static_cast<int>(m_NumberOfTargets))
  {
    ui->placementQualityGroupBox->setEnabled(false);
    ui->angleDifferenceValue->setText("");
    ui->centersOfMassValue->setText("");
    return;
  }

  ui->placementQualityGroupBox->setEnabled(true);

  mitk::Surface::Pointer targetSurface = dynamic_cast<mitk::Surface *>(m_TargetNode->GetData());
  if (targetSurface.IsNull())
  {
    mitkThrow() << "Target surface must not be null.";
  }

  m_PlacementQualityCalculator->SetTargetSurface(targetSurface);
  mitk::PointSet::Pointer targetPointSet = mitk::PointSet::New();

  // copy the origins of all reached target nodes into a point set
  // for the quality calculator
  mitk::PointSet::PointIdentifier n = 0;
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_ReachedTargetsNodes.begin();
       it != m_ReachedTargetsNodes.end();
       ++it)
  {
    targetPointSet->InsertPoint(n++, (*it)->GetData()->GetGeometry()->GetOrigin());
  }
  m_PlacementQualityCalculator->SetTargetPoints(targetPointSet);

  m_PlacementQualityCalculator->Update();

  double centersOfMassDistance = m_PlacementQualityCalculator->GetCentersOfMassDistance();
  ui->centersOfMassValue->setText(QString::number(centersOfMassDistance, 103, 2) + " mm");

  double meanAnglesDifference = m_PlacementQualityCalculator->GetMeanAngleDifference();
  ui->angleDifferenceValue->setText(QString::number(meanAnglesDifference, 103, 2) + QString::fromLatin1(" °"));

  // create an intermediate result of the placement quality
  mitk::DataNode::Pointer placementQualityResult = mitk::DataNode::New();
  placementQualityResult->SetName("PlacementQuality");
  placementQualityResult->SetFloatProperty("USNavigation::CentersOfMassDistance", centersOfMassDistance);
  placementQualityResult->SetFloatProperty("USNavigation::MeanAngleDifference", meanAnglesDifference);
  placementQualityResult->SetProperty(
    "USNavigation::AngleDifferences",
    mitk::GenericProperty<mitk::VnlVector>::New(m_PlacementQualityCalculator->GetAngleDifferences()));

  if (m_PlannedTargetsNodes.size() == static_cast<int>(m_NumberOfTargets))
  {
    mitk::VnlVector reachedPlannedDifferences;
    double reachedPlannedDifferencesSum = 0;
    double reachedPlannedDifferencesMax = 0;
    reachedPlannedDifferences.set_size(m_NumberOfTargets);

    // get sum and maximum of the planning / reality differences
    for (unsigned int n = 0; n < m_NumberOfTargets; ++n)
    {
      mitk::ScalarType distance =
        m_PlannedTargetsNodes.at(n)->GetData()->GetGeometry()->GetOrigin().EuclideanDistanceTo(
          m_ReachedTargetsNodes.at(n)->GetData()->GetGeometry()->GetOrigin());

      reachedPlannedDifferences.put(n, distance);
      reachedPlannedDifferencesSum += distance;

      if (distance > reachedPlannedDifferencesMax)
      {
        reachedPlannedDifferencesMax = distance;
      }
    }

    // add distances between planning and reality to the quality intermediate result
    placementQualityResult->SetProperty("USNavigation::PlanningRealityDistances",
                                        mitk::GenericProperty<mitk::VnlVector>::New(reachedPlannedDifferences));
    placementQualityResult->SetProperty(
      "USNavigation::MeanPlanningRealityDistance",
      mitk::DoubleProperty::New(reachedPlannedDifferencesSum / static_cast<double>(m_NumberOfTargets)));
    placementQualityResult->SetProperty("USNavigation::MaximumPlanningRealityDistance",
                                        mitk::DoubleProperty::New(reachedPlannedDifferencesMax));
  }

  emit SignalIntermediateResult(placementQualityResult);
}

void QmitkUSNavigationStepMarkerIntervention::UpdateSensorsNames()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality(false);
  if (combinedModality.IsNull())
  {
    return;
  }

  mitk::NavigationDataSource::Pointer navigationDataSource = combinedModality->GetNavigationDataSource();
  if (navigationDataSource.IsNull())
  {
    return;
  }

  if (!m_NeedleSensorName.empty())
  {
    try
    {
      m_NeedleSensorIndex = navigationDataSource->GetOutputIndex(m_NeedleSensorName);
    }
    catch (const std::exception &e)
    {
      MITK_WARN("QmitkUSAbstractNavigationStep")
      ("QmitkUSNavigationStepPlacementPlanning") << "Cannot get index for needle sensor name: " << e.what();
    }
  }
  if (this->GetNavigationStepState() >= QmitkUSAbstractNavigationStep::State_Active)
  {
    m_NeedleProjectionFilter->SelectInput(m_NeedleSensorIndex);
  }

  if (!m_ReferenceSensorName.empty())
  {
    try
    {
      m_ReferenceSensorIndex = navigationDataSource->GetOutputIndex(m_ReferenceSensorName);
    }
    catch (const std::exception &e)
    {
      MITK_WARN("QmitkUSAbstractNavigationStep")
      ("QmitkUSNavigationStepPlacementPlanning") << "Cannot get index for reference sensor name: " << e.what();
    }
  }
  if (this->GetNavigationStepState() >= QmitkUSAbstractNavigationStep::State_Active)
  {
    m_NodeDisplacementFilter->SelectInput(m_ReferenceSensorIndex);
  }

  ui->freezeImageButton->SetCombinedModality(combinedModality, m_ReferenceSensorIndex);
}
