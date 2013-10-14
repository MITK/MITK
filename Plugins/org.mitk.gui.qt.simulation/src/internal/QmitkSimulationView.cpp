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
#include <mitkExportMitkVisitor.h>
#include <mitkIRenderingManager.h>
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
    m_SelectionWasRemovedFromDataStorage(false),
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

  m_Controls.sceneComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.sceneComboBox->SetPredicate(mitk::NodePredicateDataType::New("Simulation"));

  m_Controls.stepsRecordedLabel->hide();

  connect(m_Controls.sceneComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSelectedSceneChanged(const mitk::DataNode*)));
  connect(m_Controls.animateButton, SIGNAL(toggled(bool)), this, SLOT(OnAnimateButtonToggled(bool)));
  connect(m_Controls.stepButton, SIGNAL(clicked()), this, SLOT(OnStepButtonClicked()));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetButtonClicked()));
  connect(m_Controls.dtSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDtChanged(double)));
  connect(m_Controls.recordButton, SIGNAL(toggled(bool)), this, SLOT(OnRecordButtonToggled(bool)));
  connect(m_Controls.snapshotButton, SIGNAL(clicked()), this, SLOT(OnSnapshotButtonClicked()));

  if (m_Controls.sceneComboBox->GetSelectedNode().IsNotNull())
    this->OnSelectedSceneChanged(m_Controls.sceneComboBox->GetSelectedNode());
}

void QmitkSimulationView::OnAnimateButtonToggled(bool toggled)
{
  if (this->SetSelectionAsCurrentSimulation())
  {
    mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
    simulation->SetAnimationFlag(toggled);

    if (toggled)
    {
      m_Controls.stepButton->setEnabled(false);
      m_NextRenderWindowUpdate = QTime::currentTime().addMSecs(MsPerFrame);
      m_Timer.start(0);
    }
  }

  if (!toggled)
  {
    m_Timer.stop();
    m_Controls.stepButton->setEnabled(true);
  }
}

void QmitkSimulationView::OnDtChanged(double dt)
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
  simulation->SetDt(std::max(0.0, dt));
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

    m_Controls.stepsRecordedLabel->hide();
    m_Controls.stepsRecordedLabel->setText("0 steps recorded");
  }
  else
  {
    m_Controls.stepsRecordedLabel->show();
  }
}

void QmitkSimulationView::OnResetButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  m_Controls.dtSpinBox->setValue(simulation->GetRootNode()->getDt());
  simulation->Reset();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnSelectedSceneChanged(const mitk::DataNode* node)
{
  if (m_Controls.animateButton->isChecked())
    m_Controls.animateButton->setChecked(false);

  if (m_SelectionWasRemovedFromDataStorage)
  {
    m_SelectionWasRemovedFromDataStorage = false;
    m_Selection = NULL;
  }

  if (node != NULL)
  {
    m_Selection = m_Controls.sceneComboBox->GetSelectedNode();
    mitk::Simulation* simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    m_SimulationService->SetSimulation(simulation);

    m_Controls.sceneGroupBox->setEnabled(true);
    m_Controls.snapshotButton->setEnabled(true);
    m_Controls.dtSpinBox->setValue(simulation->GetRootNode()->getDt());
  }
  else
  {
    m_Selection = NULL;

    m_SimulationService->SetSimulation(NULL);

    m_Controls.sceneGroupBox->setEnabled(false);
    m_Controls.snapshotButton->setEnabled(false);
    m_Controls.dtSpinBox->setValue(0.0);
  }
}

void QmitkSimulationView::OnSnapshotButtonClicked()
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
  mitk::ExportMitkVisitor exportVisitor;

  simulation->GetRootNode()->executeVisitor(&exportVisitor);
}

void QmitkSimulationView::OnStep(bool renderWindowUpdate)
{
  if (!this->SetSelectionAsCurrentSimulation())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  simulation->Animate();

  if (renderWindowUpdate)
    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnStepButtonClicked()
{
  this->OnStep(true);
}

void QmitkSimulationView::SetFocus()
{
  m_Controls.animateButton->setFocus();
}

bool QmitkSimulationView::SetSelectionAsCurrentSimulation() const
{
  if (m_Selection.IsNotNull())
  {
    m_SimulationService->SetSimulation(static_cast<mitk::Simulation*>(m_Selection->GetData()));
    return true;
  }

  return false;
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
    m_NextRenderWindowUpdate = currentTime.addMSecs(MsPerFrame);
    this->OnStep(true);
  }
}
