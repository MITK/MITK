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
#include "QmitkBaseItemDelegate.h"
#include "QmitkNoEditItemDelegate.h"
#include "QmitkSimulationView.h"
#include <mitkISimulationService.h>
#include <mitkNodePredicateDataType.h>
#include <mitkScheduler.h>
#include <mitkSimulation.h>
#include <mitkSurface.h>
#include <mitkVtkModel.h>
#include <usModuleRegistry.h>

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

static mitk::SimulationInteractor::Pointer CreateSimulationInteractor()
{
  const us::Module* simulationModule = us::ModuleRegistry::GetModule("MitkSimulation");

  mitk::SimulationInteractor::Pointer interactor = mitk::SimulationInteractor::New();
  interactor->LoadStateMachine("Simulation.xml", simulationModule);
  interactor->SetEventConfig("SimulationConfig.xml", simulationModule);

  return interactor;
}

class Equals
{
public:
  explicit Equals(mitk::DataNode::Pointer dataNode)
    : m_DataNode(dataNode)
  {
  }

  bool operator()(const mitk::DataStorage::SetOfObjects::value_type& object)
  {
    return object == m_DataNode;
  }

private:
  mitk::DataNode::Pointer m_DataNode;
};

QmitkSimulationView::QmitkSimulationView()
  : m_SimulationService(GetService<mitk::ISimulationService>()),
    m_Interactor(CreateSimulationInteractor()),
    m_VtkModelContextMenu(NULL),
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
  m_Ui.setupUi(parent);

  m_Ui.simulationComboBox->SetDataStorage(this->GetDataStorage());
  m_Ui.simulationComboBox->SetPredicate(mitk::NodePredicateDataType::New("Simulation"));

  QAction* vtkModelModeAction = new QAction(/*QIcon(":/Qmitk/Surface_48.png"),*/ "Render to Surface", m_VtkModelContextMenu);
  vtkModelModeAction->setCheckable(true);

  m_VtkModelContextMenu = new QMenu(m_Ui.sceneTreeWidget);
  m_VtkModelContextMenu->addAction(vtkModelModeAction);

  connect(m_Ui.simulationComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnSelectedSimulationChanged(const mitk::DataNode*)));
  connect(m_Ui.animateButton, SIGNAL(toggled(bool)), this, SLOT(OnAnimateButtonToggled(bool)));
  connect(m_Ui.stepButton, SIGNAL(clicked()), this, SLOT(OnStepButtonClicked()));
  connect(m_Ui.resetButton, SIGNAL(clicked()), this, SLOT(OnResetButtonClicked()));
  connect(m_Ui.dtSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDtChanged(double)));
  connect(m_Ui.sceneTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectedBaseChanged()));
  connect(m_Ui.sceneTreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnBaseContextMenuRequested(const QPoint&)));

  if (m_Ui.simulationComboBox->GetSelectedNode().IsNotNull())
  {
    this->OnSelectedSimulationChanged(m_Ui.simulationComboBox->GetSelectedNode());
  }
  else
  {
    this->SetSimulationControlsEnabled(false);
  }
}

void QmitkSimulationView::OnAnimateButtonToggled(bool toggled)
{
  mitk::Scheduler* scheduler = m_SimulationService->GetScheduler();

  if (toggled)
  {
    mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    simulation->SetAnimationFlag(true);
    scheduler->AddProcess(simulation);

    m_Ui.stepButton->setEnabled(false);
  }
  else if (m_Selection.IsNotNull())
  {
    mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    scheduler->RemoveProcess(simulation);
    simulation->SetAnimationFlag(false);

    m_Ui.stepButton->setEnabled(true);
  }

  if (!scheduler->IsEmpty())
  {
    if (!m_Timer.isActive())
      m_Timer.start(0);
  }
  else
  {
    m_Timer.stop();
  }
}

void QmitkSimulationView::OnBaseContextMenuRequested(const QPoint& point)
{
  typedef mitk::DataStorage::SetOfObjects SetOfObjects;

  QTreeWidgetItem* item = m_Ui.sceneTreeWidget->itemAt(point);

  if (item == NULL)
    return;

  QmitkSceneTreeWidget::Base* base = m_Ui.sceneTreeWidget->GetBaseFromItem(item);

  if (base == NULL)
    return;

  mitk::VtkModel* vtkModel = dynamic_cast<mitk::VtkModel*>(base);

  if (vtkModel == NULL)
    return;

  m_VtkModelContextMenu->actions().first()->setChecked(vtkModel->GetMode() == mitk::VtkModel::Surface);

  QAction* action = m_VtkModelContextMenu->exec(m_Ui.sceneTreeWidget->mapToGlobal(point));

  if (action == NULL)
    return;

  if (action->text() == "Render to Surface")
  {
    mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
    SetOfObjects::ConstPointer objects = dataStorage->GetSubset(NULL);

    if (action->isChecked())
    {
      vtkModel->SetMode(mitk::VtkModel::Surface);
      mitk::DataNode::Pointer dataNode = vtkModel->GetDataNode();

      if (std::find_if(objects->begin(), objects->end(), Equals(dataNode)) == objects->end())
        dataStorage->Add(dataNode, m_Selection);
    }
    else
    {
      mitk::DataNode::Pointer dataNode = vtkModel->GetDataNode();

      if (std::find_if(objects->begin(), objects->end(), Equals(dataNode)) != objects->end())
        dataStorage->Remove(dataNode);

      vtkModel->SetMode(mitk::VtkModel::OpenGL);
    }
  }
}

void QmitkSimulationView::OnDtChanged(double dt)
{
  if (m_Selection.IsNull())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
  simulation->SetDt(std::max(0.0, dt));
}

void QmitkSimulationView::OnNodeRemovedFromDataStorage(const mitk::DataNode* node)
{
  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(node->GetData());

  if (simulation.IsNotNull())
  {
    mitk::Scheduler* scheduler = m_SimulationService->GetScheduler();

    scheduler->RemoveProcess(simulation);

    if (scheduler->IsEmpty() && m_Timer.isActive())
      m_Timer.stop();

    if (m_SimulationService->GetActiveSimulation() == simulation)
      m_SimulationService->SetActiveSimulation(NULL);
  }
}

void QmitkSimulationView::OnResetButtonClicked()
{
  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());
  m_SimulationService->SetActiveSimulation(simulation);

  m_Ui.dtSpinBox->setValue(simulation->GetRootNode()->getDt());
  simulation->Reset();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkSimulationView::OnSelectedSimulationChanged(const mitk::DataNode* node)
{
  if (node != NULL)
  {
    m_Selection = m_Ui.simulationComboBox->GetSelectedNode();
    mitk::Simulation* simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    m_Ui.animateButton->setChecked(simulation->GetAnimationFlag());
    m_Ui.dtSpinBox->setValue(simulation->GetRootNode()->getDt());
    this->SetSimulationControlsEnabled(true);
  }
  else
  {
    m_Selection = NULL;

    this->SetSimulationControlsEnabled(false);
    m_Ui.animateButton->setChecked(false);
    m_Ui.dtSpinBox->setValue(0.0);
  }

  m_Interactor->SetDataNode(m_Selection);

  this->ResetSceneTreeWidget();
}

void QmitkSimulationView::OnSelectedBaseChanged()
{
  QList<QTreeWidgetItem*> selectedBaseItems = m_Ui.sceneTreeWidget->selectedItems();

  m_Ui.baseTreeWidget->OnSelectedBaseChanged(!selectedBaseItems.isEmpty()
    ? m_Ui.sceneTreeWidget->GetBaseFromItem(selectedBaseItems[0])
    : NULL);
}

void QmitkSimulationView::OnStep(bool renderWindowUpdate)
{
  mitk::Scheduler* scheduler = m_SimulationService->GetScheduler();
  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(scheduler->GetNextProcess());

  m_SimulationService->SetActiveSimulation(simulation);

  if (simulation.IsNotNull())
    simulation->Animate();

  if (renderWindowUpdate)
    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkSimulationView::OnStepButtonClicked()
{
  if (m_Selection.IsNull())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  m_SimulationService->SetActiveSimulation(simulation);
  simulation->Animate();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
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

void QmitkSimulationView::ResetSceneTreeWidget()
{
  m_Ui.sceneTreeWidget->clear();

  if (m_Selection.IsNull())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  m_Ui.sceneTreeWidget->addChild(NULL, simulation->GetRootNode().get());
  m_Ui.sceneTreeWidget->expandItem(m_Ui.sceneTreeWidget->topLevelItem(0));
}

void QmitkSimulationView::SetSimulationControlsEnabled(bool enabled)
{
  m_Ui.animateButton->setEnabled(enabled);
  m_Ui.stepButton->setEnabled(enabled);
  m_Ui.resetButton->setEnabled(enabled);
  m_Ui.dtLabel->setEnabled(enabled);
  m_Ui.dtSpinBox->setEnabled(enabled);
}

void QmitkSimulationView::SetFocus()
{
  m_Ui.animateButton->setFocus();
}
