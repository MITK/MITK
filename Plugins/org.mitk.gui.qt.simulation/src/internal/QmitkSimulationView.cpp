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

QmitkSimulationView::QmitkSimulationView()
  : m_SimulationService(GetService<mitk::ISimulationService>()),
    m_Interactor(CreateSimulationInteractor()),
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
  connect(m_Controls.sceneTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectedBaseChanged()));
  connect(m_Controls.pushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));

  if (m_Controls.simulationComboBox->GetSelectedNode().IsNotNull())
  {
    this->OnSelectedSimulationChanged(m_Controls.simulationComboBox->GetSelectedNode());
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

    m_Controls.stepButton->setEnabled(false);
  }
  else if (m_Selection.IsNotNull())
  {
    mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

    scheduler->RemoveProcess(simulation);
    simulation->SetAnimationFlag(false);

    m_Controls.stepButton->setEnabled(true);
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
    this->SetSimulationControlsEnabled(true);
  }
  else
  {
    m_Selection = NULL;

    this->SetSimulationControlsEnabled(false);
    m_Controls.animateButton->setChecked(false);
    m_Controls.dtSpinBox->setValue(0.0);
  }

  m_Interactor->SetDataNode(m_Selection);

  this->ResetSceneTreeWidget();
}

void QmitkSimulationView::OnSelectedBaseChanged()
{
  QList<QTreeWidgetItem*> selectedBaseItems = m_Controls.sceneTreeWidget->selectedItems();

  m_Controls.baseTreeWidget->OnSelectedBaseChanged(!selectedBaseItems.isEmpty()
    ? m_Controls.sceneTreeWidget->GetBaseFromItem(selectedBaseItems[0])
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
    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkSimulationView::OnStepButtonClicked()
{
  if (m_Selection.IsNull())
    return;

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

void QmitkSimulationView::ResetSceneTreeWidget()
{
  m_Controls.sceneTreeWidget->clear();

  if (m_Selection.IsNull())
    return;

  mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(m_Selection->GetData());

  m_Controls.sceneTreeWidget->addChild(NULL, simulation->GetRootNode().get());
  m_Controls.sceneTreeWidget->expandItem(m_Controls.sceneTreeWidget->topLevelItem(0));
}

void QmitkSimulationView::SetSimulationControlsEnabled(bool enabled)
{
  m_Controls.animateButton->setEnabled(enabled);
  m_Controls.stepButton->setEnabled(enabled);
  m_Controls.resetButton->setEnabled(enabled);
  m_Controls.dtLabel->setEnabled(enabled);
  m_Controls.dtSpinBox->setEnabled(enabled);
}

void QmitkSimulationView::SetFocus()
{
  m_Controls.animateButton->setFocus();
}

void QmitkSimulationView::OnButtonClicked()
{
}

/*#include <QmitkRenderWindow.h>
#include <vtkImageShiftScale.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

void QmitkSimulationView::OnButtonClicked()
{
  vtkRenderWindow* renderWindow = this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetVtkRenderWindow();

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();

  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->SetInputBufferTypeToZBuffer();

  vtkSmartPointer<vtkImageShiftScale> imageShiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();

  imageShiftScaleFilter->SetInputConnection(windowToImageFilter->GetOutputPort());
  imageShiftScaleFilter->SetOutputScalarTypeToUnsignedChar();
  imageShiftScaleFilter->SetShift(0);
  imageShiftScaleFilter->SetScale(-255);

  vtkSmartPointer<vtkPNGWriter> pngWriter = vtkSmartPointer<vtkPNGWriter>::New();

  pngWriter->SetInputConnection(imageShiftScaleFilter->GetOutputPort());
  pngWriter->SetFileName("C:\\Users\\Stefan\\Desktop\\DepthBuffer.png");
  pngWriter->Write();
}*/
