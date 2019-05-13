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

#include "QmitkUSNavigationStepZoneMarking.h"
#include "ui_QmitkUSNavigationStepZoneMarking.h"

#include "mitkNodeDisplacementFilter.h"

#include "../QmitkUSNavigationMarkerPlacement.h"

QmitkUSNavigationStepZoneMarking::QmitkUSNavigationStepZoneMarking(QWidget *parent) :
  QmitkUSAbstractNavigationStep(parent),
  m_ZoneDisplacementFilter(mitk::NodeDisplacementFilter::New()),
  m_ReferenceSensorIndex(1),
  m_CurrentlyAddingZone(false),
  ui(new Ui::QmitkUSNavigationStepZoneMarking)
{
  ui->setupUi(this);

  connect( ui->freezeButton, SIGNAL(SignalFreezed(bool)), this, SLOT(OnFreeze(bool)) );
  connect( ui->zonesWidget, SIGNAL(ZoneAdded()), this, SLOT(OnZoneAdded()) );
  connect( ui->zonesWidget, SIGNAL(ZoneRemoved()), this, SLOT(OnZoneRemoved()) );
  connect(ui->showStructureList, SIGNAL(stateChanged(int)), this, SLOT(OnShowListClicked(int)));
  ui->zonesLabel->setVisible(false);
  ui->zonesWidget->setVisible(false);
}

void QmitkUSNavigationStepZoneMarking::OnShowListClicked(int state)
{
  ui->zonesLabel->setVisible(state);
  ui->zonesWidget->setVisible(state);
}

QmitkUSNavigationStepZoneMarking::~QmitkUSNavigationStepZoneMarking()
{
  delete ui;
}

bool QmitkUSNavigationStepZoneMarking::OnStartStep()
{
  this->GetNamedDerivedNodeAndCreate(QmitkUSNavigationMarkerPlacement::DATANAME_ZONES,
                                     QmitkUSAbstractNavigationStep::DATANAME_BASENODE);

  ui->zonesWidget->SetDataStorage(this->GetDataStorage(), QmitkUSNavigationMarkerPlacement::DATANAME_ZONES);

  return true;
}

bool QmitkUSNavigationStepZoneMarking::OnStopStep()
{
  m_ZoneDisplacementFilter->ResetNodes();
  ui->zonesWidget->OnResetZones();
  m_ZoneNodes.clear();

  // remove zone nodes from the data storage
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage(false);
  if ( dataStorage.IsNotNull() )
  {
    mitk::DataNode::Pointer baseNode = dataStorage->GetNamedNode(QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
    if ( baseNode.IsNotNull() )
    {
      dataStorage->Remove(dataStorage->GetNamedDerivedNode(QmitkUSNavigationMarkerPlacement::DATANAME_ZONES, baseNode));
    }
  }

  return true;
}

bool QmitkUSNavigationStepZoneMarking::OnFinishStep()
{
  return true;
}

bool QmitkUSNavigationStepZoneMarking::OnActivateStep()
{
  m_ZoneDisplacementFilter->ConnectTo(this->GetCombinedModality()->GetNavigationDataSource());
  m_ZoneDisplacementFilter->SelectInput(m_ReferenceSensorIndex);

  emit SignalReadyForNextStep();

  return true;
}

bool QmitkUSNavigationStepZoneMarking::OnDeactivateStep()
{
  ui->freezeButton->Unfreeze();

  return true;
}

void QmitkUSNavigationStepZoneMarking::OnUpdate()
{
  if (m_NavigationDataSource.IsNull()) { return; }

  m_NavigationDataSource->Update();
  m_ZoneDisplacementFilter->Update();

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

  ui->freezeButton->setEnabled(valid);
}

void QmitkUSNavigationStepZoneMarking::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
  if ( settingsNode.IsNull() ) { return; }

  std::string stateMachineFilename;
  if ( settingsNode->GetStringProperty("settings.interaction-concept", stateMachineFilename) && stateMachineFilename != m_StateMachineFilename )
  {
    m_StateMachineFilename = stateMachineFilename;
    ui->zonesWidget->SetStateMachineFilename(stateMachineFilename);
  }

  std::string referenceSensorName;
  if ( settingsNode->GetStringProperty("settings.reference-name-selected", referenceSensorName) )
  {
    m_ReferenceSensorName = referenceSensorName;
  }

  this->UpdateReferenceSensorName();
}

QString QmitkUSNavigationStepZoneMarking::GetTitle()
{
  return "Critical Structures";
}

QmitkUSAbstractNavigationStep::FilterVector QmitkUSNavigationStepZoneMarking::GetFilter()
{
  return FilterVector(1, m_ZoneDisplacementFilter.GetPointer());
}

void QmitkUSNavigationStepZoneMarking::OnFreeze(bool freezed)
{
  if (freezed) this->GetCombinedModality()->SetIsFreezed(true);

  ui->zoneAddingExplanationLabel->setEnabled(freezed);

  if ( freezed )
  {
    m_CurrentlyAddingZone = true;
    ui->zonesWidget->OnStartAddingZone();
    // feed reference pose to node displacement filter
      m_ZoneDisplacementFilter->SetInitialReferencePose(this->GetCombinedModality()->GetNavigationDataSource()->GetOutput(m_ReferenceSensorIndex)->Clone());
  }
  else if ( m_CurrentlyAddingZone )
  {
    m_CurrentlyAddingZone = false;
    ui->zonesWidget->OnAbortAddingZone();
  }

  if (!freezed) this->GetCombinedModality()->SetIsFreezed(false);
}

void QmitkUSNavigationStepZoneMarking::OnZoneAdded()
{
  m_CurrentlyAddingZone = false;

  ui->freezeButton->Unfreeze();

  ui->zoneAddingExplanationLabel->setEnabled(ui->freezeButton->isChecked());

  mitk::DataStorage::SetOfObjects::ConstPointer zoneNodesSet = ui->zonesWidget->GetZoneNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = zoneNodesSet->Begin();
       it != zoneNodesSet->End(); ++it)
  {
    // add all zones to zone filter which aren't added until now
    if ( std::find(m_ZoneNodes.begin(), m_ZoneNodes.end(), it->Value()) == m_ZoneNodes.end() )
    {
      // logging center point and radius
      float radius = -1;
      it->Value()->GetFloatProperty("zone.size", radius);
      MITK_INFO("QmitkUSNavigationStepZoneMarking")("QmitkUSAbstractNavigationStep")
        << "Risk zone (" << it->Value()->GetName() << ") added with center "
        << it->Value()->GetData()->GetGeometry()->GetOrigin() << " and radius " << radius << ".";

      m_ZoneNodes.push_back(it->Value());
      m_ZoneDisplacementFilter->AddNode(it->Value());
    }
  }
}

void QmitkUSNavigationStepZoneMarking::OnZoneRemoved()
{
  mitk::DataStorage::SetOfObjects::ConstPointer zoneNodesSet = ui->zonesWidget->GetZoneNodes();

  for ( int n = m_ZoneNodes.size() - 1; n >= 0; --n )
  {
    bool found = false;

    // test if the node can be found in the set of zone nodes
    for (mitk::DataStorage::SetOfObjects::ConstIterator itSet = zoneNodesSet->Begin();
       itSet != zoneNodesSet->End(); ++itSet)
    {
      if ( m_ZoneNodes.at(n) == itSet->Value() ) { found = true; break; }
    }

    if ( ! found )
    {
      MITK_INFO("QmitkUSNavigationStepZoneMarking")("QmitkUSAbstractNavigationStep")
        << "Risk zone (" << m_ZoneNodes.at(n)->GetName() << ") removed.";

      m_ZoneNodes.erase(m_ZoneNodes.begin()+n);
      m_ZoneDisplacementFilter->RemoveNode(n);
    }
  }
}

void QmitkUSNavigationStepZoneMarking::OnSetCombinedModality()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality(false);
  if (combinedModality.IsNotNull())
  {
    m_NavigationDataSource = combinedModality->GetNavigationDataSource();
  }

  ui->freezeButton->SetCombinedModality(combinedModality, m_ReferenceSensorIndex);

  this->UpdateReferenceSensorName();
}

void QmitkUSNavigationStepZoneMarking::UpdateReferenceSensorName()
{
  if ( m_NavigationDataSource.IsNull() ) { return; }

  if ( ! m_ReferenceSensorName.empty() )
  {
    try
    {
      //m_ReferenceSensorIndex = m_NavigationDataSource->GetOutputIndex(m_ReferenceSensorName);
      m_ReferenceSensorIndex = 1;
    }
    catch ( const std::exception &e )
    {
      MITK_WARN("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepZoneMarking")
        << "Cannot get index for reference sensor name: " << e.what();
    }
  }

  if ( this->GetNavigationStepState() >= QmitkUSAbstractNavigationStep::State_Active )
  {
    m_ZoneDisplacementFilter->SelectInput(m_ReferenceSensorIndex);
  }

  ui->freezeButton->SetCombinedModality(this->GetCombinedModality(false), m_ReferenceSensorIndex);
}
