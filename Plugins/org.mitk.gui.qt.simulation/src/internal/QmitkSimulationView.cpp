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

  m_Controls.cmbSimulation->SetDataStorage(this->GetDataStorage());
  m_Controls.cmbSimulation->SetPredicate(mitk::NodePredicateDataType::New("Simulation"));

  connect(m_Controls.btnAnimate, SIGNAL(toggled(bool)), this, SLOT(OnAnimateButtonToggled(bool)));
  connect(m_Controls.btnResetScene, SIGNAL(clicked()), this, SLOT(OnResetSceneButtonClicked()));
  connect(m_Controls.btnStep, SIGNAL(clicked()), this, SLOT(OnStepButtonClicked()));
  connect(m_Controls.cmbSimulation, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSimulationComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(m_Controls.spnDT, SIGNAL(valueChanged(double)), this, SLOT(OnDTSpinBoxValueChanged(double)));
}

void QmitkSimulationView::OnAnimateButtonToggled(bool toggled)
{
  if (SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.cmbSimulation->GetSelectedNode()->GetData());
    sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
    sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

    rootNode->getContext()->setAnimate(toggled);

    if (toggled)
    {
      m_Controls.btnStep->setEnabled(false);
      m_Timer.start(0);
    }
  }

  if (!toggled)
  {
    m_Timer.stop();
    m_Controls.btnStep->setEnabled(true);
  }
}

void QmitkSimulationView::OnDTSpinBoxValueChanged(double value)
{
  if (SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.cmbSimulation->GetSelectedNode()->GetData());
    sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

    rootNode->setDt(value == 0.0
      ? simulation->GetDefaultDT()
      : value);
  }
}

void QmitkSimulationView::OnResetSceneButtonClicked()
{
  if (SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.cmbSimulation->GetSelectedNode()->GetData());
    sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
    sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

    m_Controls.spnDT->setValue(0.0);
    sofaSimulation->reset(rootNode.get());

    rootNode->setTime(0.0);
    rootNode->execute<sofa::simulation::UpdateContextVisitor>(sofa::core::ExecParams::defaultInstance());

    simulation->GetDrawTool()->Reset();

    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  }
}

void QmitkSimulationView::OnSimulationComboBoxSelectionChanged(const mitk::DataNode* node)
{
  if (m_Controls.btnAnimate->isChecked())
    m_Controls.btnAnimate->setChecked(false);

  if (node != NULL)
  {
    m_Controls.grpSimulation->setEnabled(true);
    static_cast<mitk::Simulation*>(node->GetData())->SetAsActiveSimulation();
  }
  else
  {
    m_Controls.grpSimulation->setEnabled(false);
    mitk::Simulation::SetActiveSimulation(NULL);
  }
}

void QmitkSimulationView::OnStepButtonClicked()
{
  if (SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Controls.cmbSimulation->GetSelectedNode()->GetData());
    sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
    sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

    simulation->GetDrawTool()->Reset();

    sofaSimulation->animate(rootNode.get(), rootNode->getDt());

    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  }
}

void QmitkSimulationView::SetFocus()
{
  m_Controls.btnAnimate->setFocus();
}

bool QmitkSimulationView::SetSelectionAsCurrentSimulation() const
{
  mitk::DataNode::Pointer selectedNode = m_Controls.cmbSimulation->GetSelectedNode();

  if (selectedNode.IsNotNull())
  {
    static_cast<mitk::Simulation*>(m_Controls.cmbSimulation->GetSelectedNode()->GetData())->SetAsActiveSimulation();
    return true;
  }

  return false;
}

void QmitkSimulationView::OnTimerTimeout()
{
  this->OnStepButtonClicked();
}
