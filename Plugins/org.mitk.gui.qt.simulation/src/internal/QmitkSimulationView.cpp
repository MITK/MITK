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

#include "QmitkSimulationPreferencePage.h"
#include "QmitkSimulationView.h"
#include <mitkNodePredicateDataType.h>
#include <mitkSimulation.h>
#include <mitkSimulationModel.h>
#include <sofa/helper/system/PluginManager.h>
#include <sofa/simulation/common/UpdateContextVisitor.h>

QmitkSimulationView::QmitkSimulationView()
  : m_Timer(this)
{
  connect(&m_Timer, SIGNAL(timeout()), this, SLOT(OnTimerTimeout()));
  initSOFAPlugins();
}

QmitkSimulationView::~QmitkSimulationView()
{
}

void QmitkSimulationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.simulationComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.simulationComboBox->SetPredicate(mitk::NodePredicateDataType::New("Simulation"));

  connect(m_Controls.animateButton, SIGNAL(toggled(bool)), this, SLOT(OnAnimateButtonToggled(bool)));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetButtonClicked()));
  connect(m_Controls.stepButton, SIGNAL(clicked()), this, SLOT(OnStepButtonClicked()));
  connect(m_Controls.simulationComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSimulationComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(m_Controls.dtSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDTSpinBoxValueChanged(double)));
  connect(m_Controls.snapshotButton, SIGNAL(clicked()), this, SLOT(OnSnapshotButtonClicked()));
}

void QmitkSimulationView::OnAnimateButtonToggled(bool toggled)
{
  if (this->SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.simulationComboBox->GetSelectedNode()->GetData());
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

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.simulationComboBox->GetSelectedNode()->GetData());
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  rootNode->setDt(value == 0.0
    ? simulation->GetDefaultDT()
    : value);
}

void QmitkSimulationView::OnResetButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.simulationComboBox->GetSelectedNode()->GetData());
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  m_Controls.dtSpinBox->setValue(0.0);
  sofaSimulation->reset(rootNode.get());

  rootNode->setTime(0.0);
  rootNode->execute<sofa::simulation::UpdateContextVisitor>(sofa::core::ExecParams::defaultInstance());

  simulation->GetDrawTool()->Reset();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnSimulationComboBoxSelectionChanged(const mitk::DataNode* node)
{
  if (m_Controls.animateButton->isChecked())
    m_Controls.animateButton->setChecked(false);

  if (node != NULL)
  {
    m_Controls.sceneGroupBox->setEnabled(true);
    m_Controls.snapshotButton->setEnabled(true);
    static_cast<mitk::Simulation*>(node->GetData())->SetAsActiveSimulation();
  }
  else
  {
    m_Controls.sceneGroupBox->setEnabled(false);
    m_Controls.snapshotButton->setEnabled(false);
    mitk::Simulation::SetActiveSimulation(NULL);
  }
}

void QmitkSimulationView::OnSnapshotButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::DataNode::Pointer simulationDataNode = m_Controls.simulationComboBox->GetSelectedNode();
  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(simulationDataNode->GetData());

  mitk::Surface::Pointer snapshot = simulation->TakeSnapshot();

  if (snapshot.IsNull())
    return;

  mitk::DataNode::Pointer snapshotDataNode = mitk::DataNode::New();
  snapshotDataNode->SetData(snapshot);
  snapshotDataNode->SetName("Snapshot");

  this->GetDataStorage()->Add(snapshotDataNode, simulationDataNode);
}

void QmitkSimulationView::OnStepButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.simulationComboBox->GetSelectedNode()->GetData());
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  simulation->GetDrawTool()->Reset();

  sofaSimulation->animate(rootNode.get(), rootNode->getDt());

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::SetFocus()
{
  m_Controls.animateButton->setFocus();
}

bool QmitkSimulationView::SetSelectionAsCurrentSimulation() const
{
  mitk::DataNode::Pointer selectedNode = m_Controls.simulationComboBox->GetSelectedNode();

  if (selectedNode.IsNotNull())
  {
    static_cast<mitk::Simulation*>(m_Controls.simulationComboBox->GetSelectedNode()->GetData())->SetAsActiveSimulation();
    return true;
  }

  return false;
}

void QmitkSimulationView::OnTimerTimeout()
{
  this->OnStepButtonClicked();
}
