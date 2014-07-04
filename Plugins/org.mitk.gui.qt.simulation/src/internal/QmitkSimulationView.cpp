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

#include "org_mitk_gui_qt_simulation_Activator.h"
#include "QmitkSimulationView.h"
#include <mitkISimulationService.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSimulation.h>

template <class T>
static T* GetService()
{
  ctkPluginContext* context = mitk::org_mitk_gui_qt_simulation_Activator::GetContext();

  if (context == NULL)
    return NULL;

  ctkServiceReference serviceReference = context->getServiceReference<T>();

  return serviceReference
    ? context->getService<T>(serviceReference)
    : NULL;
}

QmitkSimulationView::QmitkSimulationView()
  : m_SimulationService(GetService<mitk::ISimulationService>()),
    m_Scheduler(mitk::SchedulingAlgorithm::WeightedRoundRobin),
    m_Timer(this)
{
  this->GetDataStorage()->RemoveNodeEvent.AddListener(
    mitk::MessageDelegate1<QmitkSimulationView, const mitk::DataNode*>(this, &QmitkSimulationView::OnNodeRemovedFromDataStorage));

  connect(&m_Timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
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

  connect(m_Controls.simulationComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSelectedSimulationChanged(const mitk::DataNode*)));
  connect(m_Controls.animateButton, SIGNAL(toggled(bool)), this, SLOT(OnAnimateButtonToggled(bool)));
  connect(m_Controls.stepButton, SIGNAL(clicked()), this, SLOT(OnStepButtonClicked()));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetButtonClicked()));
  connect(m_Controls.dtSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDtChanged(double)));

  if (m_Controls.simulationComboBox->GetSelectedNode().IsNotNull())
    this->OnSelectedSimulationChanged(m_Controls.simulationComboBox->GetSelectedNode());
}

void QmitkSimulationView::OnAnimateButtonToggled(bool toggled)
{
  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  if (toggled)
  {
    simulation->SetAnimationFlag(true);
    m_Scheduler.AddProcess(simulation);

    m_Controls.stepButton->setEnabled(false);
  }
  else
  {
    m_Scheduler.RemoveProcess(simulation);
    simulation->SetAnimationFlag(false);

    m_Controls.stepButton->setEnabled(true);
  }

  if (!m_Scheduler.IsEmpty())
  {
    if (!m_Timer.isActive())
      m_Timer.start(0);
  }
  else
  {
    m_Timer.stop();
  }
}

void QmitkSimulationView::OnDtChanged(double dt)
{
  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
  simulation->SetDt(std::max(0.0, dt));
}

void QmitkSimulationView::OnNodeRemovedFromDataStorage(const mitk::DataNode* node)
{
  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(node->GetData());

  if (simulation.IsNotNull())
  {
    m_Scheduler.RemoveProcess(simulation);

    if (m_Scheduler.IsEmpty() && m_Timer.isActive())
      m_Timer.stop();
  }
}

void QmitkSimulationView::OnResetButtonClicked()
{
  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
  m_SimulationService->SetActiveSimulation(simulation);

  m_Controls.dtSpinBox->setValue(simulation->GetRootNode()->getDt());
  simulation->Reset();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnSelectedSimulationChanged(const mitk::DataNode* node)
{
  if (node != NULL)
  {
    m_Selection = m_Controls.simulationComboBox->GetSelectedNode();
    mitk::Simulation* simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    m_Controls.animateButton->setChecked(simulation->GetAnimationFlag());
    m_Controls.dtSpinBox->setValue(simulation->GetRootNode()->getDt());
    m_Controls.simulationGroupBox->setEnabled(true);
  }
  else
  {
    m_Controls.simulationGroupBox->setEnabled(false);
    m_Controls.animateButton->setChecked(false);
    m_Controls.dtSpinBox->setValue(0.0);
  }
}

void QmitkSimulationView::OnStep(bool renderWindowUpdate)
{
  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(m_Scheduler.GetNextProcess());

  m_SimulationService->SetActiveSimulation(simulation);

  if (simulation.IsNotNull())
  {
    boost::chrono::high_resolution_clock::time_point t0 = boost::chrono::high_resolution_clock::now();
    simulation->Animate();
    simulation->SetElapsedTime(boost::chrono::high_resolution_clock::now() - t0);
  }

  if (renderWindowUpdate)
    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnStepButtonClicked()
{
  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  m_SimulationService->SetActiveSimulation(simulation);
  simulation->Animate();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnTimeout()
{
  QTime currentTime = QTime::currentTime();

  if (currentTime.msecsTo(m_NextRenderWindowUpdate) > 0)
  {
    this->OnStep(false);
  }
  else
  {
    m_NextRenderWindowUpdate = currentTime.addMSecs(MSecsPerFrame);
    this->OnStep(true);
  }
}

void QmitkSimulationView::SetFocus()
{
  m_Controls.animateButton->setFocus();
}
