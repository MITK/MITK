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
#include "QmitkUSNavigationStepTumourSelection.h"
#include "ui_QmitkUSNavigationStepTumourSelection.h"

#include "usModuleRegistry.h"

#include "mitkDataNode.h"
#include "mitkSurface.h"
#include "mitkUSCombinedModality.h"
#include "../Interactors/mitkUSZonesInteractor.h"
#include "mitkNodeDisplacementFilter.h"
#include "QmitkUSNavigationStepCombinedModality.h"

#include "../QmitkUSNavigationMarkerPlacement.h"

#include "mitkIOUtil.h"

#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkWarpScalar.h"

QmitkUSNavigationStepTumourSelection::QmitkUSNavigationStepTumourSelection(QWidget* parent) :
  QmitkUSAbstractNavigationStep(parent),
  m_targetSelectionOptional(false),
  m_SecurityDistance(0),
  m_Interactor(mitk::USZonesInteractor::New()),
  m_NodeDisplacementFilter(mitk::NodeDisplacementFilter::New()),
  m_StateMachineFilename("USZoneInteractions.xml"),
  m_ReferenceSensorIndex(1),
  m_ListenerChangeNode(this, &QmitkUSNavigationStepTumourSelection::TumourNodeChanged),
  ui(new Ui::QmitkUSNavigationStepTumourSelection)
{
  ui->setupUi(this);

  connect(ui->freezeImageButton, SIGNAL(SignalFreezed(bool)), this, SLOT(OnFreeze(bool)));
  connect(ui->tumourSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnTumourSizeChanged(int)));
  connect(ui->deleteTumourButton, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));

  m_SphereColor = mitk::Color();
  //default color: green
  m_SphereColor[0] = 0;
  m_SphereColor[1] = 255;
  m_SphereColor[2] = 0;
}
void QmitkUSNavigationStepTumourSelection::SetTumorColor(mitk::Color c)
{
  m_SphereColor = c;
}

void QmitkUSNavigationStepTumourSelection::SetTargetSelectionOptional(bool t)
{
  m_targetSelectionOptional = t;
}

QmitkUSNavigationStepTumourSelection::~QmitkUSNavigationStepTumourSelection()
{
  delete ui;
}

bool QmitkUSNavigationStepTumourSelection::OnStartStep()
{
  m_TumourNode = this->GetNamedDerivedNodeAndCreate(
    QmitkUSNavigationMarkerPlacement::DATANAME_TUMOUR,
    QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  m_TumourNode->SetColor(m_SphereColor[0], m_SphereColor[1], m_SphereColor[2]);

  // load state machine and event config for data interactor
  m_Interactor->LoadStateMachine(m_StateMachineFilename, us::ModuleRegistry::GetModule("MitkUS"));
  m_Interactor->SetEventConfig("globalConfig.xml");

  this->GetDataStorage()->ChangedNodeEvent.AddListener(m_ListenerChangeNode);

  m_TargetSurfaceNode = this->GetNamedDerivedNodeAndCreate(
    QmitkUSNavigationMarkerPlacement::DATANAME_TARGETSURFACE,
    QmitkUSNavigationMarkerPlacement::DATANAME_TUMOUR);

  // do not show the surface until this is requested
  m_TargetSurfaceNode->SetBoolProperty("visible", false);

  // make sure that scalars will be renderer on the surface
  m_TargetSurfaceNode->SetBoolProperty("scalar visibility", true);
  m_TargetSurfaceNode->SetBoolProperty("color mode", true);
  m_TargetSurfaceNode->SetBoolProperty("Backface Culling", true);

  return true;
}

bool QmitkUSNavigationStepTumourSelection::OnStopStep()
{
  // make sure that imaging isn't freezed anymore
  ui->freezeImageButton->Unfreeze();

  m_NodeDisplacementFilter->ResetNodes();

  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage(false);
  if (dataStorage.IsNotNull())
  {
    // remove target surface node from data storage, if available there
    if (m_TargetSurfaceNode.IsNotNull()) { dataStorage->Remove(m_TargetSurfaceNode); }

    dataStorage->ChangedNodeEvent.RemoveListener(m_ListenerChangeNode);
    dataStorage->Remove(m_TumourNode);
    m_TumourNode = 0;
  }

  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepTumourSelection")
    << "Removing tumour.";

  return true;
}

bool QmitkUSNavigationStepTumourSelection::OnRestartStep()
{
  ui->tumourSizeExplanationLabel->setEnabled(false);
  ui->tumourSizeLabel->setEnabled(false);
  ui->tumourSizeSlider->setEnabled(false);
  ui->deleteTumourButton->setEnabled(false);

  ui->tumourSizeSlider->blockSignals(true);
  ui->tumourSizeSlider->setValue(0);
  ui->tumourSizeSlider->blockSignals(false);


  return QmitkUSAbstractNavigationStep::OnRestartStep();
}

bool QmitkUSNavigationStepTumourSelection::OnFinishStep()
{
  // make sure that the surface has the right extent (in case the
  // tumor size was changed since the initial surface creation)
  m_TargetSurfaceNode->SetData(this->CreateTargetSurface());

  return true;
}

bool QmitkUSNavigationStepTumourSelection::OnActivateStep()
{
  m_Interactor = mitk::USZonesInteractor::New();
  m_Interactor->LoadStateMachine(m_StateMachineFilename, us::ModuleRegistry::GetModule("MitkUS"));
  m_Interactor->SetEventConfig("globalConfig.xml");

  m_NodeDisplacementFilter->ConnectTo(this->GetCombinedModality()->GetNavigationDataSource());
  m_NodeDisplacementFilter->SelectInput(1);//m_ReferenceSensorIndex

  //target selection is optional
  if (m_targetSelectionOptional) { emit SignalReadyForNextStep(); }

  return true;
}

bool QmitkUSNavigationStepTumourSelection::OnDeactivateStep()
{
  m_Interactor->SetDataNode(0);

  bool value;
  if (m_TumourNode.IsNotNull() && !(m_TumourNode->GetBoolProperty("zone.created", value) && value))
  {
    m_TumourNode->SetData(0);
  }

  // make sure that imaging isn't freezed anymore
  ui->freezeImageButton->Unfreeze();

  return true;
}

void QmitkUSNavigationStepTumourSelection::OnUpdate()
{
  if (m_NavigationDataSource.IsNull()) { return; }

  m_NavigationDataSource->Update();
  m_NodeDisplacementFilter->Update();

  bool valid = m_NavigationDataSource->GetOutput(m_ReferenceSensorIndex)->IsDataValid();

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

  ui->freezeImageButton->setEnabled(valid);

  bool created;
  if (m_TumourNode.IsNull() || !m_TumourNode->GetBoolProperty("zone.created", created) || !created)
  {
    ui->tumourSearchExplanationLabel->setEnabled(valid);
  }
}

void QmitkUSNavigationStepTumourSelection::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
  if (settingsNode.IsNull()) { return; }

  float securityDistance;
  if (settingsNode->GetFloatProperty("settings.security-distance", securityDistance))
  {
    m_SecurityDistance = securityDistance;
  }

  std::string stateMachineFilename;
  if (settingsNode->GetStringProperty("settings.interaction-concept", stateMachineFilename) && stateMachineFilename != m_StateMachineFilename)
  {
    m_StateMachineFilename = stateMachineFilename;
    m_Interactor->LoadStateMachine(stateMachineFilename, us::ModuleRegistry::GetModule("MitkUS"));
  }

  std::string referenceSensorName;
  if (settingsNode->GetStringProperty("settings.reference-name-selected", referenceSensorName))
  {
    m_ReferenceSensorName = referenceSensorName;
  }

  this->UpdateReferenceSensorName();
}

QString QmitkUSNavigationStepTumourSelection::GetTitle()
{
  return "Localisation of Tumour Position";
}

QmitkUSAbstractNavigationStep::FilterVector QmitkUSNavigationStepTumourSelection::GetFilter()
{
  return FilterVector(1, m_NodeDisplacementFilter.GetPointer());
}

void QmitkUSNavigationStepTumourSelection::OnFreeze(bool freezed)
{
  if (freezed) this->GetCombinedModality()->SetIsFreezed(true);

  ui->tumourSelectionExplanation1Label->setEnabled(freezed);
  ui->tumourSelectionExplanation2Label->setEnabled(freezed);

  if (freezed)
  {
    if (!m_TumourNode->GetData())
    {
      // load state machine and event config for data interactor
      m_Interactor->LoadStateMachine(m_StateMachineFilename, us::ModuleRegistry::GetModule("MitkUS"));
      m_Interactor->SetEventConfig("globalConfig.xml");

      m_Interactor->SetDataNode(m_TumourNode);
      // feed reference pose to node displacement filter
      m_NodeDisplacementFilter->SetInitialReferencePose(this->GetCombinedModality()->GetNavigationDataSource()->GetOutput(m_ReferenceSensorIndex)->Clone());
    }
  }
  else
  {
    bool value;
    if (m_TumourNode->GetBoolProperty("zone.created", value) && value)
    {
      ui->freezeImageButton->setEnabled(false);
      ui->tumourSearchExplanationLabel->setEnabled(false);
    }
  }

  if (!freezed) this->GetCombinedModality()->SetIsFreezed(false);
}

void QmitkUSNavigationStepTumourSelection::OnSetCombinedModality()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality(false);
  if (combinedModality.IsNotNull())
  {
    m_NavigationDataSource = combinedModality->GetNavigationDataSource();
  }
  else
  {
    m_NavigationDataSource = 0;
  }

  ui->freezeImageButton->SetCombinedModality(combinedModality, m_ReferenceSensorIndex);

  this->UpdateReferenceSensorName();
}

void QmitkUSNavigationStepTumourSelection::OnTumourSizeChanged(int size)
{
  m_TumourNode->SetFloatProperty("zone.size", static_cast<float>(size));
  mitk::USZonesInteractor::UpdateSurface(m_TumourNode);

  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepTumourSelection")
    << "Changing tumour radius to " << size << ".";
}

void QmitkUSNavigationStepTumourSelection::OnDeleteButtonClicked()
{
  this->OnRestartStep();
}

void QmitkUSNavigationStepTumourSelection::TumourNodeChanged(const mitk::DataNode* dataNode)
{
  // only changes of tumour node are of interest
  if (dataNode != m_TumourNode) { return; }

  float size;
  dataNode->GetFloatProperty("zone.size", size);

  ui->tumourSizeSlider->setValue(static_cast<int>(size));

  bool created;
  if (dataNode->GetBoolProperty("zone.created", created) && created)
  {
    if (ui->freezeImageButton->isChecked())
    {
      m_NodeDisplacementFilter->AddNode(const_cast<mitk::DataNode*>(dataNode));

      m_TargetSurfaceNode->SetData(this->CreateTargetSurface());
      m_NodeDisplacementFilter->AddNode(m_TargetSurfaceNode);

      MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepTumourSelection")
        << "Tumour created with center " << dataNode->GetData()->GetGeometry()->GetOrigin()
        << " and radius " << size << ".";

      mitk::DataNode::Pointer tumourResultNode = mitk::DataNode::New();
      tumourResultNode->SetName("TumourResult");
      tumourResultNode->SetProperty("USNavigation::TumourCenter",
        mitk::Point3dProperty::New(dataNode->GetData()->GetGeometry()->GetOrigin()));
      tumourResultNode->SetProperty("USNavigation::TumourRadius", mitk::DoubleProperty::New(size));

      emit SignalIntermediateResult(tumourResultNode);

      ui->freezeImageButton->Unfreeze();
    }

    ui->tumourSearchExplanationLabel->setEnabled(false);
    ui->tumourSizeExplanationLabel->setEnabled(true);
    ui->tumourSizeLabel->setEnabled(true);
    ui->tumourSizeSlider->setEnabled(true);
    ui->deleteTumourButton->setEnabled(true);

    emit SignalReadyForNextStep();
  }
}

mitk::Surface::Pointer QmitkUSNavigationStepTumourSelection::CreateTargetSurface()
{
  mitk::Surface::Pointer tumourSurface = dynamic_cast<mitk::Surface*>(m_TumourNode->GetData());

  if (tumourSurface.IsNull())
  {
    MITK_WARN << "No target selected, cannot create surface!";
    return mitk::Surface::New(); //return a empty surface in this case...
  }

  // make a deep copy of the tumour surface polydata
  vtkSmartPointer<vtkPolyData> tumourSurfaceVtk = vtkSmartPointer<vtkPolyData>::New();
  tumourSurfaceVtk->DeepCopy(tumourSurface->GetVtkPolyData());

  // create scalars for moving every point the same size onto its normal vector
  vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
  int numberOfPoints = tumourSurfaceVtk->GetNumberOfPoints();
  scalars->SetNumberOfTuples(numberOfPoints);

  // set scalars for warp filter
  for (vtkIdType i = 0; i < numberOfPoints; ++i) { scalars->SetTuple1(i, m_SecurityDistance * 10); }
  tumourSurfaceVtk->GetPointData()->SetScalars(scalars);

  vtkSmartPointer<vtkWarpScalar> warpScalar = vtkSmartPointer<vtkWarpScalar>::New();
  warpScalar->SetInputData(tumourSurfaceVtk);
  warpScalar->SetScaleFactor(1); // use the scalars themselves
  warpScalar->Update();
  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = warpScalar->GetPolyDataOutput();

  // set the moved points to the deep copied tumour surface; this is
  // necessary as setting the targetSurfaceVtk as polydata for the
  // targetSurface would result in flat shading for the surface (seems
  // to be a bug in MITK or VTK)
  tumourSurfaceVtk->SetPoints(targetSurfaceVtk->GetPoints());

  mitk::Surface::Pointer targetSurface = mitk::Surface::New();
  targetSurface->SetVtkPolyData(tumourSurfaceVtk);
  targetSurface->GetGeometry()->SetOrigin(tumourSurface->GetGeometry()->GetOrigin());

  return targetSurface;
}

itk::SmartPointer<mitk::NodeDisplacementFilter> QmitkUSNavigationStepTumourSelection::GetTumourNodeDisplacementFilter()
{
  return m_NodeDisplacementFilter;
}

void QmitkUSNavigationStepTumourSelection::UpdateReferenceSensorName()
{
  if (m_NavigationDataSource.IsNull()) { return; }

  if (!m_ReferenceSensorName.empty())
  {
    try
    {
      m_ReferenceSensorIndex = m_NavigationDataSource->GetOutputIndex(m_ReferenceSensorName);
    }
    catch (const std::exception &e)
    {
      MITK_WARN("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepTumourSelection")
        << "Cannot get index for reference sensor name: " << e.what();
    }
  }

  if (this->GetNavigationStepState() >= QmitkUSAbstractNavigationStep::State_Active)
  {
    MITK_INFO << "############### " << m_NodeDisplacementFilter->GetNumberOfIndexedInputs();
    m_NodeDisplacementFilter->SelectInput(m_ReferenceSensorIndex);
  }

  ui->freezeImageButton->SetCombinedModality(this->GetCombinedModality(false), m_ReferenceSensorIndex);
}
