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

#include "QmitkSimulationView.h"
#include <mitkIRenderingManager.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSimulation.h>
#include <mitkSimulationModel.h>

QmitkSimulationView::QmitkSimulationView()
  : m_SelectionWasRemovedFromDataStorage(false),
    m_Timer(this)
{
  this->GetDataStorage()->RemoveNodeEvent.AddListener(
    mitk::MessageDelegate1<QmitkSimulationView, const mitk::DataNode*>(this, &QmitkSimulationView::OnNodeRemovedFromDataStorage));

  connect(&m_Timer, SIGNAL(timeout()), this, SLOT(OnTimerTimeout()));
}

QmitkSimulationView::~QmitkSimulationView()
{
  this->GetDataStorage()->RemoveNodeEvent.RemoveListener(
    mitk::MessageDelegate1<QmitkSimulationView, const mitk::DataNode*>(this, &QmitkSimulationView::OnNodeRemovedFromDataStorage));
}

void QmitkSimulationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.simulationComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.simulationComboBox->SetPredicate(mitk::NodePredicateDataType::New("Simulation"));

  m_Controls.stepsRecordedLabel->hide();

  connect(m_Controls.animateButton, SIGNAL(toggled(bool)), this, SLOT(OnAnimateButtonToggled(bool)));
  connect(m_Controls.recordButton, SIGNAL(toggled(bool)), this, SLOT(OnRecordButtonToggled(bool)));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetButtonClicked()));
  connect(m_Controls.stepButton, SIGNAL(clicked()), this, SLOT(OnStepButtonClicked()));
  connect(m_Controls.simulationComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSimulationComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(m_Controls.dtSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDTSpinBoxValueChanged(double)));
  connect(m_Controls.snapshotButton, SIGNAL(clicked()), this, SLOT(OnSnapshotButtonClicked()));

  if (m_Controls.simulationComboBox->GetSelectedNode().IsNotNull())
    this->OnSimulationComboBoxSelectionChanged(m_Controls.simulationComboBox->GetSelectedNode());
}

void QmitkSimulationView::OnAnimateButtonToggled(bool toggled)
{
  if (this->SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Selection->GetData());
    sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
    sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

    rootNode->getContext()->setAnimate(toggled);

    if (toggled)
    {
      m_Controls.stepButton->setEnabled(false);
      m_Timer.start(0);
    }
  }

  if (!toggled)
  {
    m_Timer.stop();
    m_Controls.stepButton->setEnabled(true);
  }
}

void QmitkSimulationView::OnDTSpinBoxValueChanged(double value)
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Selection->GetData());
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  rootNode->setDt(std::max(0.0, value));
}

void QmitkSimulationView::OnNodeRemovedFromDataStorage(const mitk::DataNode* node)
{
  if (m_Selection.IsNotNull() && m_Selection.GetPointer() == node)
    m_SelectionWasRemovedFromDataStorage = true;
}

void QmitkSimulationView::OnRecordButtonToggled(bool toggled)
{
  if (!toggled)
  {
    if (m_Record.IsNotNull())
    {
      mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
      dataNode->SetData(m_Record);
      dataNode->SetName(m_Record->GetTimeSteps() == 1 ? "Snapshot" : "Record");

      this->GetDataStorage()->Add(dataNode, m_Selection);
      mitk::RenderingManager::GetInstance()->InitializeViews(m_Record->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

      m_Record = NULL;
    }

    m_Controls.stepsRecordedLabel->hide();
    m_Controls.stepsRecordedLabel->setText("0 steps recorded");
  }
  else if (toggled)
  {
    m_Controls.stepsRecordedLabel->show();
  }
}

void QmitkSimulationView::OnResetButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  if (m_Controls.recordButton->isChecked())
    m_Controls.recordButton->setChecked(false);

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Selection->GetData());
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  m_Controls.dtSpinBox->setValue(simulation->GetDefaultDT());
  sofaSimulation->reset(rootNode.get());

  rootNode->setTime(0.0);
  sofaSimulation->updateContext(rootNode.get());

  simulation->GetDrawTool()->Reset();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnSimulationComboBoxSelectionChanged(const mitk::DataNode* node)
{
  if (m_Controls.animateButton->isChecked())
    m_Controls.animateButton->setChecked(false);

  if (m_SelectionWasRemovedFromDataStorage)
  {
    m_SelectionWasRemovedFromDataStorage = false;
    m_Selection = NULL;
  }

  if (m_Controls.recordButton->isChecked())
    m_Controls.recordButton->setChecked(false);

  if (node != NULL)
  {
    m_Selection = m_Controls.simulationComboBox->GetSelectedNode();
    mitk::Simulation* simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    m_Controls.sceneGroupBox->setEnabled(true);
    m_Controls.snapshotButton->setEnabled(true);

    simulation->SetAsActiveSimulation();
    m_Controls.dtSpinBox->setValue(simulation->GetDefaultDT());
  }
  else
  {
    m_Selection = NULL;

    m_Controls.sceneGroupBox->setEnabled(false);
    m_Controls.snapshotButton->setEnabled(false);

    mitk::Simulation::SetActiveSimulation(NULL);
    m_Controls.dtSpinBox->setValue(0.0);
  }
}

void QmitkSimulationView::OnSnapshotButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Selection->GetData());

  mitk::Surface::Pointer snapshot = simulation->TakeSnapshot();

  if (snapshot.IsNull())
    return;

  mitk::DataNode::Pointer snapshotDataNode = mitk::DataNode::New();
  snapshotDataNode->SetData(snapshot);
  snapshotDataNode->SetName("Snapshot");

  this->GetDataStorage()->Add(snapshotDataNode, m_Selection);
}

void QmitkSimulationView::OnStepButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Selection->GetData());
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  simulation->GetDrawTool()->Reset();

  sofaSimulation->animate(rootNode.get(), rootNode->getDt());

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);

  if (m_Controls.recordButton->isChecked())
  {
    if (m_Record.IsNull())
      m_Record = mitk::Surface::New();

    if (simulation->AppendSnapshot(m_Record))
    {
      unsigned int numSteps = m_Record->GetTimeSteps();
      QString plural = numSteps != 1 ? "s" : "";

      m_Controls.stepsRecordedLabel->setText(QString("%1 step%2 recorded").arg(numSteps).arg(plural));
    }
    else if (m_Record->GetTimeSteps() == 1)
    {
      m_Record = NULL;
    }
  }
}

void QmitkSimulationView::SetFocus()
{
  m_Controls.animateButton->setFocus();
}

bool QmitkSimulationView::SetSelectionAsCurrentSimulation() const
{
  if (m_Selection.IsNotNull())
  {
    static_cast<mitk::Simulation*>(m_Selection->GetData())->SetAsActiveSimulation();
    return true;
  }

  return false;
}

void QmitkSimulationView::OnTimerTimeout()
{
  this->OnStepButtonClicked();
}
