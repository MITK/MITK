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

#include "QmitkUSNavigationStepPlacementPlanning.h"
#include "ui_QmitkUSNavigationStepPlacementPlanning.h"

#include "../Interactors/mitkUSPointMarkInteractor.h"
#include "../mitkUSTargetPlacementQualityCalculator.h"
#include "mitkNeedleProjectionFilter.h"
#include "mitkNodeDisplacementFilter.h"
#include "usModuleRegistry.h"

#include "../Filter/mitkUSNavigationTargetIntersectionFilter.h"
#include "../Filter/mitkUSNavigationTargetUpdateFilter.h"

#include "../QmitkUSNavigationMarkerPlacement.h"

#include "mitkLookupTableProperty.h"
#include "mitkSurface.h"
#include <mitkBaseGeometry.h>

#include "mitkLayoutAnnotationRenderer.h"
#include "mitkTextAnnotation3D.h"

#include "vtkFloatArray.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "vtkLineSource.h"
#include <vtkSphereSource.h>

#include "vtkCenterOfMass.h"
#include "vtkLinearTransform.h"
#include "vtkPoints.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkUnstructuredGrid.h"

#include "vtkMath.h"

QmitkUSNavigationStepPlacementPlanning::QmitkUSNavigationStepPlacementPlanning(QWidget *parent)
  : QmitkUSAbstractNavigationStep(parent),
    m_NumberOfTargets(0),
    m_CurrentTargetIndex(0),
    m_BodyMarkerValid(false),
    m_PointMarkInteractor(mitk::USPointMarkInteractor::New()),
    m_TargetUpdateFilter(mitk::USNavigationTargetUpdateFilter::New()),
    m_NodeDisplacementFilter(mitk::NodeDisplacementFilter::New()),
    m_NeedleProjectionFilter(mitk::NeedleProjectionFilter::New()),
    m_TargetIntersectionFilter(mitk::USNavigationTargetIntersectionFilter::New()),
    m_PlacementQualityCalculator(mitk::USTargetPlacementQualityCalculator::New()),
    m_ReferenceSensorIndex(1),
    m_NeedleSensorIndex(0),
    m_ListenerTargetCoordinatesChanged(this, &QmitkUSNavigationStepPlacementPlanning::UpdateTargetCoordinates),
    ui(new Ui::QmitkUSNavigationStepPlacementPlanning)
{
  ui->setupUi(this);

  connect(ui->freezeImageButton, SIGNAL(SignalFreezed(bool)), this, SLOT(OnFreeze(bool)));

  m_TargetUpdateFilter->SetScalarArrayIdentifier("USNavigation::PlanningPlacement");

  this->GenerateTargetColorLookupTable();
}

QmitkUSNavigationStepPlacementPlanning::~QmitkUSNavigationStepPlacementPlanning()
{
  delete ui;
}

bool QmitkUSNavigationStepPlacementPlanning::OnStartStep()
{
  // create node for needle projection
  mitk::DataNode::Pointer node =
    this->GetNamedDerivedNodeAndCreate("Needle Path", QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  node->SetData(m_NeedleProjectionFilter->GetProjection());
  node->SetBoolProperty("show contour", true);

  // make sure that the targets node exist in the data storage
  this->GetNamedDerivedNodeAndCreate(QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS,
                                     QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  this->GetNamedDerivedNodeAndCreate(QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS_PATHS,
                                     QmitkUSAbstractNavigationStep::DATANAME_BASENODE);

  // listen to event of point mark interactor
  m_PointMarkInteractor->CoordinatesChangedEvent.AddListener(m_ListenerTargetCoordinatesChanged);

  return true;
}

bool QmitkUSNavigationStepPlacementPlanning::OnStopStep()
{
  // remove listener to event of point mark interactor
  m_PointMarkInteractor->CoordinatesChangedEvent.RemoveListener(m_ListenerTargetCoordinatesChanged);

  m_CurrentTargetIndex = 0;
  m_TargetUpdateFilter->Reset();
  m_NodeDisplacementFilter->ResetNodes();

  // remove the planned target nodes from the data storage
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetNodes.begin();
       it != m_PlannedTargetNodes.end();
       ++it)
  {
    dataStorage->Remove(*it);
  }
  m_PlannedTargetNodes.clear();

  // remove the planned target path nodes from the data storage
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedNeedlePaths.begin();
       it != m_PlannedNeedlePaths.end();
       ++it)
  {
    dataStorage->Remove(*it);
  }
  m_PlannedNeedlePaths.clear();

  // remove the targets node from the data storage
  mitk::DataNode::Pointer targetsNode = this->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS,
                                                                  QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  if (targetsNode.IsNotNull())
  {
    dataStorage->Remove(targetsNode);
  }

  // remove the target paths node from the data storage
  mitk::DataNode::Pointer targetPathsNode = this->GetNamedDerivedNode(
    QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS_PATHS, QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  if (targetPathsNode.IsNotNull())
  {
    dataStorage->Remove(targetPathsNode);
  }

  // make sure that the image is no longer freezed after stopping
  ui->freezeImageButton->Unfreeze();

  return true;
}

bool QmitkUSNavigationStepPlacementPlanning::OnRestartStep()
{
  this->OnStopStep();
  this->OnStartStep();

  return true;
}

bool QmitkUSNavigationStepPlacementPlanning::OnFinishStep()
{
  // create scalars showing the planned positions on the target surface
  if (!m_PlannedTargetNodes.isEmpty())
  {
    mitk::USNavigationTargetUpdateFilter::Pointer planningSurfaceFilter = mitk::USNavigationTargetUpdateFilter::New();
    planningSurfaceFilter->SetOptimalAngle(0);
    planningSurfaceFilter->SetScalarArrayIdentifier("USNavigation::PlanningScalars");
    planningSurfaceFilter->SetUseMaximumScore(true);
    planningSurfaceFilter->SetTargetStructure(m_TargetNode);

    unsigned int n = 0;
    for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetNodes.begin();
         it != m_PlannedTargetNodes.end();
         ++it)
    {
      bool isSurfaceEmpty;
      if (((*it)->GetBoolProperty("surface_empty", isSurfaceEmpty)) && isSurfaceEmpty)
      {
        // remove node from data storage if it is not fully planned
        this->GetDataStorage()->Remove(*it);
      }
      else
      {
        // set origin to filter for coloring target surface (good
        // positions are positions near to the origin)
        planningSurfaceFilter->SetControlNode(n, *it);
      }

      n++;
    }
  }

  // make sure that the image is no longer freezed after finishing
  ui->freezeImageButton->Unfreeze();

  return true;
}

bool QmitkUSNavigationStepPlacementPlanning::OnActivateStep()
{
  emit SignalReadyForNextStep();

  m_NodeDisplacementFilter->SelectInput(m_ReferenceSensorIndex);
  m_NeedleProjectionFilter->SelectInput(m_NeedleSensorIndex);

  if (m_PlannedTargetNodes.empty())
  {
    m_CurrentTargetIndex = 0;
  }
  else
  {
    m_CurrentTargetIndex = m_PlannedTargetNodes.size() - 1;
  }

  m_TargetNode = this->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_TARGETSURFACE,
                                           QmitkUSNavigationMarkerPlacement::DATANAME_TUMOUR);
  m_TargetNode->SetBoolProperty("visible", true);

  // set lookup table of tumour node
  m_TargetNode->SetProperty("LookupTable", m_TargetColorLookupTableProperty);

  m_TargetUpdateFilter->SetTargetStructure(m_TargetNode);

  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetNodes.begin();
       it != m_PlannedTargetNodes.end();
       ++it)
  {
    (*it)->SetBoolProperty("visible", true);
  }

  this->UpdateTargetDescriptions();

  // m_TargetUpdateFilter->UpdateTargetScores();

  return true;
}

bool QmitkUSNavigationStepPlacementPlanning::OnDeactivateStep()
{
  ui->freezeImageButton->Unfreeze();

  return true;
}

void QmitkUSNavigationStepPlacementPlanning::OnUpdate()
{
  this->UpdateTargetColors();

  // get navigation data source and make sure that it is not null
  mitk::NavigationDataSource::Pointer navigationDataSource = this->GetCombinedModality()->GetNavigationDataSource();
  if (navigationDataSource.IsNull())
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")
    ("QmitkUSNavigationStepPunctuationIntervention") << "Navigation Data Source of Combined Modality must not be null.";
    mitkThrow() << "Navigation Data Source of Combined Modality must not be null.";
  }

  navigationDataSource->Update();
  this->UpdateBodyMarkerStatus(navigationDataSource->GetOutput(m_ReferenceSensorIndex));

  ui->freezeImageButton->setEnabled(m_BodyMarkerValid);

  mitk::PointSet::Pointer needleProjectionPointSet = m_NeedleProjectionFilter->GetProjection();
  if (needleProjectionPointSet->GetSize() == 2)
  {
    m_TargetIntersectionFilter->SetTargetSurface(dynamic_cast<mitk::Surface *>(m_TargetNode->GetData()));
    m_TargetIntersectionFilter->SetLine(needleProjectionPointSet);
    m_TargetIntersectionFilter->CalculateIntersection();
    if (m_TargetIntersectionFilter->GetIsIntersecting())
    {
      // only enable button if body marker is in the tracking volume, too
      ui->placeTargetButton->setEnabled(m_BodyMarkerValid);
      ui->placeTargetButton->setToolTip("");

      if (m_PlannedTargetNodes.size() == m_NumberOfTargets - 1)
      {
        mitk::PointSet::Pointer targetPointSet = mitk::PointSet::New();

        mitk::PointSet::PointIdentifier n = 0;
        for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetNodes.begin();
             it != m_PlannedTargetNodes.end();
             ++it)
        {
          targetPointSet->InsertPoint(n++, (*it)->GetData()->GetGeometry()->GetOrigin());
        }
        targetPointSet->InsertPoint(n, m_TargetIntersectionFilter->GetIntersectionPoint());

        this->CalculatePlanningQuality(dynamic_cast<mitk::Surface *>(m_TargetNode->GetData()), targetPointSet);
      }
    }
    else
    {
      ui->placeTargetButton->setEnabled(false);
      ui->placeTargetButton->setToolTip(
        "Target cannot be placed as the needle path is not intersecting the target surface.");

      // no not show planning quality if not all nodes are planned for now
      // and there is no needle path intersection
      if (m_PlannedTargetNodes.size() < m_NumberOfTargets)
      {
        ui->angleDifferenceValue->setText("");
        ui->centersOfMassValue->setText("");
      }
    }
  }
}

void QmitkUSNavigationStepPlacementPlanning::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
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

QString QmitkUSNavigationStepPlacementPlanning::GetTitle()
{
  return "Placement Planning";
}

QmitkUSAbstractNavigationStep::FilterVector QmitkUSNavigationStepPlacementPlanning::GetFilter()
{
  FilterVector filterVector;
  filterVector.push_back(m_NodeDisplacementFilter.GetPointer());
  filterVector.push_back(m_NeedleProjectionFilter.GetPointer());
  return filterVector;
}

void QmitkUSNavigationStepPlacementPlanning::OnSetCombinedModality()
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

void QmitkUSNavigationStepPlacementPlanning::OnFreeze(bool freeze)
{
  if (freeze)
    this->GetCombinedModality()->SetIsFreezed(true);
  if (freeze)
  {
    // load state machine and event config for data interactor
    m_PointMarkInteractor->LoadStateMachine("USPointMarkInteractions.xml", us::ModuleRegistry::GetModule("MitkUS"));
    m_PointMarkInteractor->SetEventConfig("globalConfig.xml");

    this->CreateTargetNodesIfNecessary();

    m_PointMarkInteractor->SetDataNode(m_PlannedTargetNodes.at(m_CurrentTargetIndex));
  }
  else
  {
    m_PointMarkInteractor->SetDataNode(0);
  }

  if (!freeze)
    this->GetCombinedModality()->SetIsFreezed(false);
}

void QmitkUSNavigationStepPlacementPlanning::OnPlaceTargetButtonClicked()
{
  this->CreateTargetNodesIfNecessary();

  mitk::DataNode::Pointer currentNode = m_PlannedTargetNodes.at(m_CurrentTargetIndex);

  currentNode->SetData(this->CreateSphere(5));
  currentNode->SetBoolProperty("surface_empty", false);

  mitk::PointSet::Pointer needleProjection = m_NeedleProjectionFilter->GetProjection();

  m_TargetIntersectionFilter->SetTargetSurface(dynamic_cast<mitk::Surface *>(m_TargetNode->GetData()));
  m_TargetIntersectionFilter->SetLine(m_NeedleProjectionFilter->GetProjection());
  m_TargetIntersectionFilter->CalculateIntersection();
  mitk::Point3D intersectionPoint = m_TargetIntersectionFilter->GetIntersectionPoint();

  currentNode->GetData()->GetGeometry()->SetOrigin(intersectionPoint);
  m_TargetUpdateFilter->SetControlNode(m_CurrentTargetIndex, currentNode);

  mitk::PointSet::Pointer plannedPath = mitk::PointSet::New();
  m_PlannedNeedlePaths.at(m_CurrentTargetIndex)->SetData(plannedPath);
  plannedPath->SetPoint(0, needleProjection->GetPoint(0));
  plannedPath->SetPoint(1, intersectionPoint);

  if (m_CurrentTargetIndex < m_NumberOfTargets - 1)
  {
    this->OnGoToNextTarget();
  }
  else
  {
    this->UpdateTargetDescriptions();
  }

  this->ReinitNodeDisplacementFilter();

  MITK_INFO("QmitkUSAbstractNavigationStep")
  ("QmitkUSNavigationStepPlacementPlanning") << "Target " << m_CurrentTargetIndex + 1 << " planned at position "
                                             << intersectionPoint << ".";
}

void QmitkUSNavigationStepPlacementPlanning::OnGoToPreviousTarget()
{
  if (m_CurrentTargetIndex < 1)
  {
    mitkThrow() << "Cannot go to previous target as current target is first target.";
  }

  m_CurrentTargetIndex--;

  this->UpdateTargetDescriptions();
}

void QmitkUSNavigationStepPlacementPlanning::OnGoToNextTarget()
{
  m_CurrentTargetIndex++;

  this->UpdateTargetDescriptions();
}

void QmitkUSNavigationStepPlacementPlanning::OnRemoveCurrentTargetClicked()
{
  if (m_CurrentTargetIndex >= m_PlannedTargetNodes.size())
  {
    MITK_WARN << "Cannot remove current target as there aren't as much planned target nodes.";
    return;
  }

  this->GetDataStorage()->Remove(m_PlannedTargetNodes.at(m_CurrentTargetIndex));
  m_PlannedTargetNodes.remove(m_CurrentTargetIndex);
  this->GetDataStorage()->Remove(m_PlannedNeedlePaths.at(m_CurrentTargetIndex));
  m_PlannedNeedlePaths.remove(m_CurrentTargetIndex);
  this->ReinitNodeDisplacementFilter();

  for (int n = 0; n < m_PlannedTargetNodes.size(); ++n)
  {
    // set name of the target node according to its new index
    m_PlannedTargetNodes.at(n)->SetName(
      QString("Target %1").arg(n + 1, m_NumberOfTargets / 10 + 1, 10, QLatin1Char('0')).toStdString());

    m_PlannedNeedlePaths.at(n)->SetName(
      QString("Target Path %1").arg(n + 1, m_NumberOfTargets / 10 + 1, 10, QLatin1Char('0')).toStdString());
  }

  m_TargetUpdateFilter->RemovePositionOfTarget(m_CurrentTargetIndex);

  m_CurrentTargetIndex = m_PlannedTargetNodes.size();

  this->UpdateTargetDescriptions();

  MITK_INFO("QmitkUSAbstractNavigationStep")
  ("QmitkUSNavigationStepPlacementPlanning") << "Planned target " << m_CurrentTargetIndex + 1 << " removed.";
}

void QmitkUSNavigationStepPlacementPlanning::CreateTargetNodesIfNecessary()
{
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();

  while (m_PlannedTargetNodes.size() <= m_CurrentTargetIndex)
  {
    QString targetNumber =
      QString("%1").arg(m_PlannedTargetNodes.size() + 1, m_NumberOfTargets / 10 + 1, 10, QLatin1Char('0'));

    mitk::DataNode::Pointer targetNode = this->GetNamedDerivedNodeAndCreate(
      (QString("Target ") + targetNumber).toStdString().c_str(), QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS);
    targetNode->SetOpacity(0.5);
    targetNode->SetBoolProperty("surface_empty", true);
    targetNode->SetData(mitk::Surface::New());

    m_PlannedTargetNodes.push_back(targetNode);

    mitk::DataNode::Pointer targetPathNode =
      this->GetNamedDerivedNodeAndCreate((QString("Target Path ") + targetNumber).toStdString().c_str(),
                                         QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS_PATHS);
    targetPathNode->SetOpacity(0.5);
    targetPathNode->SetColor(1, 1, 1);
    targetPathNode->SetColor(1, 1, 1, nullptr, "contourcolor");
    targetPathNode->SetBoolProperty("show contour", true);
    m_PlannedNeedlePaths.push_back(targetPathNode);

    mitk::Surface::Pointer pathSurface = mitk::Surface::New();
    targetPathNode->SetData(pathSurface);
  }
}

void QmitkUSNavigationStepPlacementPlanning::UpdateTargetCoordinates(mitk::DataNode *dataNode)
{
  bool surfaceEmpty;
  if (dataNode->GetBoolProperty("surface_empty", surfaceEmpty) && surfaceEmpty)
  {
    mitk::Point3D origin = dataNode->GetData()->GetGeometry()->GetOrigin();
    dataNode->SetData(this->CreateSphere(5));
    dataNode->SetBoolProperty("surface_empty", false);
    dataNode->GetData()->GetGeometry()->SetOrigin(origin);
  }

  this->ReinitNodeDisplacementFilter();

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

  m_TargetUpdateFilter->SetControlNode(m_CurrentTargetIndex, dataNode);

  MITK_INFO("QmitkUSAbstractNavigationStep")
  ("QmitkUSNavigationStepPlacementPlanning") << "Target " << m_CurrentTargetIndex + 1 << " planned at position "
                                             << geometry->GetOrigin() << ".";

  if (ui->freezeImageButton->isChecked())
  {
    ui->freezeImageButton->Unfreeze();

    if (m_CurrentTargetIndex < m_NumberOfTargets - 1)
    {
      this->OnGoToNextTarget();
    }
  }

  this->UpdateTargetDescriptions();
}

void QmitkUSNavigationStepPlacementPlanning::UpdateTargetColors()
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

  vtkFloatArray *targetScoreScalars = dynamic_cast<vtkFloatArray *>(
    targetNodeSurface->GetVtkPolyData()->GetPointData()->GetScalars("USNavigation::PlanningPlacement"));

  if (!targetScoreScalars)
  {
    return;
  }

  unsigned int numberOfTupels = targetScoreScalars->GetNumberOfTuples();

  vtkSmartPointer<vtkFloatArray> colors = vtkSmartPointer<vtkFloatArray>::New();
  colors->SetNumberOfComponents(1);
  colors->SetNumberOfTuples(numberOfTupels);
  colors->SetName("Colors");

  double markerScore;

  for (unsigned int n = 0; n < numberOfTupels; n++)
  {
    targetScoreScalars->GetTuple(n, &markerScore);
    colors->SetTuple(n, &markerScore);
  }

  if (numberOfTupels > 0)
  {
    targetNodeSurfaceVtk->GetPointData()->SetScalars(colors);
    targetNodeSurfaceVtk->GetPointData()->Update();
  }
}

void QmitkUSNavigationStepPlacementPlanning::UpdateTargetDescriptions()
{
  ui->previousButton->setEnabled(m_CurrentTargetIndex > 0);
  ui->nextButton->setEnabled(m_CurrentTargetIndex + 1 < m_PlannedTargetNodes.size());
  ui->removeButton->setEnabled(m_PlannedTargetNodes.size() > 0);

  ui->currentTargetLabel->setText(
    QString("Planning Target %1 of %2").arg(m_CurrentTargetIndex + 1).arg(m_NumberOfTargets));

  ui->removeButton->setEnabled(m_CurrentTargetIndex < m_PlannedTargetNodes.size());

  this->CalculatePlanningQuality();
}

void QmitkUSNavigationStepPlacementPlanning::GenerateTargetColorLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetHueRange(0.0, 0.27);
  lookupTable->SetSaturationRange(1.0, 1.0);
  lookupTable->SetValueRange(1.0, 1.0);
  lookupTable->SetTableRange(0.0, 1.0);
  lookupTable->Build();

  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut->SetVtkLookupTable(lookupTable);

  m_TargetColorLookupTableProperty = mitk::LookupTableProperty::New(lut);
}

void QmitkUSNavigationStepPlacementPlanning::ReinitNodeDisplacementFilter()
{
  m_NodeDisplacementFilter->ResetNodes();
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetNodes.begin();
       it != m_PlannedTargetNodes.end();
       ++it)
  {
    if (it->IsNotNull() && (*it)->GetData() != 0)
    {
      m_NodeDisplacementFilter->AddNode(*it);
    }
  }
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedNeedlePaths.begin();
       it != m_PlannedNeedlePaths.end();
       ++it)
  {
    if (it->IsNotNull() && (*it)->GetData() != 0)
    {
      m_NodeDisplacementFilter->AddNode(*it);
    }
  }
}

void QmitkUSNavigationStepPlacementPlanning::CalculatePlanningQuality()
{
  if (m_PlannedTargetNodes.size() != m_NumberOfTargets)
  {
    ui->angleDifferenceLabel->setEnabled(false);
    ui->centersOfMassLabel->setEnabled(false);
    ui->allTargetsPlannedLabel->setEnabled(false);
    ui->angleDifferenceValue->setText("");
    ui->centersOfMassValue->setText("");
    return;
  }

  ui->angleDifferenceLabel->setEnabled(true);
  ui->centersOfMassLabel->setEnabled(true);
  ui->allTargetsPlannedLabel->setEnabled(true);

  mitk::PointSet::Pointer targetPointSet = mitk::PointSet::New();

  mitk::PointSet::PointIdentifier n = 0;
  for (QVector<itk::SmartPointer<mitk::DataNode>>::iterator it = m_PlannedTargetNodes.begin();
       it != m_PlannedTargetNodes.end();
       ++it)
  {
    targetPointSet->InsertPoint(n++, (*it)->GetData()->GetGeometry()->GetOrigin());
  }

  mitk::DataNode::Pointer planningQualityResult =
    this->CalculatePlanningQuality(dynamic_cast<mitk::Surface *>(m_TargetNode->GetData()), targetPointSet);
  emit SignalIntermediateResult(planningQualityResult);
}

mitk::DataNode::Pointer QmitkUSNavigationStepPlacementPlanning::CalculatePlanningQuality(
  mitk::Surface::Pointer targetSurface, mitk::PointSet::Pointer targetPoints)
{
  if (targetSurface.IsNull())
  {
    mitkThrow() << "Target surface must not be null.";
  }

  m_PlacementQualityCalculator->SetTargetSurface(targetSurface);
  m_PlacementQualityCalculator->SetTargetPoints(targetPoints);
  m_PlacementQualityCalculator->Update();

  mitk::DataNode::Pointer planningQualityResult = mitk::DataNode::New();
  planningQualityResult->SetName("PlanningQuality");

  double centersOfMassDistance = m_PlacementQualityCalculator->GetCentersOfMassDistance();
  ui->centersOfMassValue->setText(QString::number(centersOfMassDistance, 103, 2) + " mm");
  planningQualityResult->SetFloatProperty("USNavigation::CentersOfMassDistance", centersOfMassDistance);

  if (m_PlannedTargetNodes.size() > 1)
  {
    double meanAnglesDifference = m_PlacementQualityCalculator->GetMeanAngleDifference();
    ui->angleDifferenceValue->setText(QString::number(meanAnglesDifference, 103, 2) + QString::fromLatin1(" °"));

    planningQualityResult->SetFloatProperty("USNavigation::MeanAngleDifference", meanAnglesDifference);
    planningQualityResult->SetProperty(
      "USNavigation::AngleDifferences",
      mitk::GenericProperty<mitk::VnlVector>::New(m_PlacementQualityCalculator->GetAngleDifferences()));
  }
  else
  {
    ui->angleDifferenceValue->setText("not valid for one point");
  }

  return planningQualityResult;
}

itk::SmartPointer<mitk::Surface> QmitkUSNavigationStepPlacementPlanning::CreateSphere(float)
{
  mitk::Surface::Pointer surface = mitk::Surface::New();

  // create a vtk sphere with fixed radius
  vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkSphere->SetRadius(5);
  vtkSphere->SetCenter(0, 0, 0);
  vtkSphere->Update();
  surface->SetVtkPolyData(vtkSphere->GetOutput());

  return surface;
}

void QmitkUSNavigationStepPlacementPlanning::UpdateBodyMarkerStatus(mitk::NavigationData::Pointer bodyMarker)
{
  if (bodyMarker.IsNull())
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")
    ("QmitkUSNavigationStepPunctuationIntervention")
      << "Current Navigation Data for body marker of Combined Modality must not be null.";
    mitkThrow() << "Current Navigation Data for body marker of Combined Modality must not be null.";
  }

  m_BodyMarkerValid = bodyMarker->IsDataValid();

  // update body marker status label
  if (m_BodyMarkerValid)
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
}

void QmitkUSNavigationStepPlacementPlanning::UpdateSensorsNames()
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
