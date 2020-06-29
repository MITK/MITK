/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSurfaceBasedInterpolatorWidget.h"

#include "mitkColorProperty.h"
#include "mitkInteractionConst.h"
#include "mitkOperationEvent.h"
#include "mitkProgressBar.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkSegTool2D.h"
#include "mitkSliceNavigationController.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkUndoController.h"
#include "mitkVtkRepresentationProperty.h"
#include <mitkToolManagerProvider.h>

#include "QmitkStdMultiWidget.h"

#include <itkCommand.h>
#include <vtkProperty.h>

#include <QMessageBox>

QmitkSurfaceBasedInterpolatorWidget::QmitkSurfaceBasedInterpolatorWidget(QWidget *parent, const char * /*name*/)
  : QWidget(parent),
    m_SurfaceBasedInterpolatorController(mitk::SurfaceBasedInterpolationController::GetInstance()),
    m_ToolManager(nullptr),
    m_Activated(false),
    m_DataStorage(nullptr)
{
  m_Controls.setupUi(this);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION);

  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSurfaceBasedInterpolatorWidget>(
    this, &QmitkSurfaceBasedInterpolatorWidget::OnToolManagerWorkingDataModified);

  connect(m_Controls.m_btStart, SIGNAL(toggled(bool)), this, SLOT(OnToggleWidgetActivation(bool)));
  connect(m_Controls.m_btAccept, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_Controls.m_cbShowPositionNodes, SIGNAL(toggled(bool)), this, SLOT(OnShowMarkers(bool)));

  itk::ReceptorMemberCommand<QmitkSurfaceBasedInterpolatorWidget>::Pointer command =
    itk::ReceptorMemberCommand<QmitkSurfaceBasedInterpolatorWidget>::New();
  command->SetCallbackFunction(this, &QmitkSurfaceBasedInterpolatorWidget::OnSurfaceInterpolationInfoChanged);
  m_SurfaceInterpolationInfoChangedObserverTag =
    m_SurfaceBasedInterpolatorController->AddObserver(itk::ModifiedEvent(), command);

  m_InterpolatedSurfaceNode = mitk::DataNode::New();
  m_InterpolatedSurfaceNode->SetName("Surface Interpolation feedback");
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(255.0, 255.0, 0.0));
  m_InterpolatedSurfaceNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
  m_InterpolatedSurfaceNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_InterpolatedSurfaceNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_InterpolatedSurfaceNode->SetVisibility(false);

  m_3DContourNode = mitk::DataNode::New();
  m_3DContourNode->SetName("Drawn Contours");
  m_3DContourNode->SetProperty("color", mitk::ColorProperty::New(0.0, 0.0, 0.0));
  m_3DContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  m_3DContourNode->SetProperty("material.wireframeLineWidth", mitk::FloatProperty::New(2.0f));
  m_3DContourNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_3DContourNode->SetVisibility(
    false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget0")));
  m_3DContourNode->SetVisibility(
    false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
  m_3DContourNode->SetVisibility(
    false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
  m_3DContourNode->SetVisibility(
    false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));

  connect(&m_Watcher, SIGNAL(started()), this, SLOT(StartUpdateInterpolationTimer()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnSurfaceInterpolationFinished()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(StopUpdateInterpolationTimer()));

  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(ChangeSurfaceColor()));

  m_Controls.m_btAccept->setEnabled(false);
  m_Controls.m_cbShowPositionNodes->setEnabled(false);

  this->setEnabled(false);
}

void QmitkSurfaceBasedInterpolatorWidget::SetDataStorage(mitk::DataStorage &storage)
{
  m_DataStorage = &storage;
}

QmitkSurfaceBasedInterpolatorWidget::~QmitkSurfaceBasedInterpolatorWidget()
{
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSurfaceBasedInterpolatorWidget>(
    this, &QmitkSurfaceBasedInterpolatorWidget::OnToolManagerWorkingDataModified);

  if (m_DataStorage->Exists(m_3DContourNode))
    m_DataStorage->Remove(m_3DContourNode);

  if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    m_DataStorage->Remove(m_InterpolatedSurfaceNode);

  // remove observer
  m_SurfaceBasedInterpolatorController->RemoveObserver(m_SurfaceInterpolationInfoChangedObserverTag);

  delete m_Timer;
}

void QmitkSurfaceBasedInterpolatorWidget::ShowInterpolationResult(bool status)
{
  if (m_InterpolatedSurfaceNode.IsNotNull())
    m_InterpolatedSurfaceNode->SetVisibility(status);

  if (m_3DContourNode.IsNotNull())
    m_3DContourNode->SetVisibility(
      status, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceBasedInterpolatorWidget::OnSurfaceInterpolationFinished()
{
  mitk::Surface::Pointer interpolatedSurface = m_SurfaceBasedInterpolatorController->GetInterpolationResult();

  if (interpolatedSurface.IsNotNull())
  {
    m_InterpolatedSurfaceNode->SetData(interpolatedSurface);
    m_3DContourNode->SetData(m_SurfaceBasedInterpolatorController->GetContoursAsSurface());
    this->ShowInterpolationResult(true);
  }
  else
  {
    m_InterpolatedSurfaceNode->SetData(nullptr);
    m_3DContourNode->SetData(nullptr);
    this->ShowInterpolationResult(false);
  }
}

void QmitkSurfaceBasedInterpolatorWidget::OnShowMarkers(bool state)
{
  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers =
    m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End();
       ++it)
  {
    it->Value()->SetProperty("helper object", mitk::BoolProperty::New(!state));
  }

  mitk::SegTool2D::Pointer manualSegmentationTool;

  unsigned int numberOfExistingTools = m_ToolManager->GetTools().size();

  for (unsigned int i = 0; i < numberOfExistingTools; i++)
  {
    manualSegmentationTool = dynamic_cast<mitk::SegTool2D *>(m_ToolManager->GetToolById(i));

    if (manualSegmentationTool)
    {
      manualSegmentationTool->SetShowMarkerNodes(state);
    }
  }
}

void QmitkSurfaceBasedInterpolatorWidget::StartUpdateInterpolationTimer()
{
  m_Timer->start(500);
}

void QmitkSurfaceBasedInterpolatorWidget::StopUpdateInterpolationTimer()
{
  m_Timer->stop();
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(255.0, 255.0, 0.0));
  mitk::RenderingManager::GetInstance()->RequestUpdate(
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))->GetRenderWindow());
}

void QmitkSurfaceBasedInterpolatorWidget::ChangeSurfaceColor()
{
  float currentColor[3];
  m_InterpolatedSurfaceNode->GetColor(currentColor);

  float yellow[3] = {255.0, 255.0, 0.0};

  if (currentColor[2] == yellow[2])
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(255.0, 255.0, 255.0));
  }
  else
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(yellow));
  }
  m_InterpolatedSurfaceNode->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdate(
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))->GetRenderWindow());
}

void QmitkSurfaceBasedInterpolatorWidget::OnToolManagerWorkingDataModified()
{
  mitk::DataNode *workingNode = this->m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    this->setEnabled(false);
    return;
  }

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  // TODO adapt tool manager so that this check is done there, e.g. convenience function
  //  Q_ASSERT(workingImage);
  if (!workingImage)
  {
    this->setEnabled(false);
    return;
  }

  if (workingImage->GetDimension() > 4 || workingImage->GetDimension() < 3)
  {
    this->setEnabled(false);
    return;
  }

  m_WorkingImage = workingImage;

  this->setEnabled(true);
}

void QmitkSurfaceBasedInterpolatorWidget::OnRunInterpolation()
{
  m_SurfaceBasedInterpolatorController->Interpolate();
}

void QmitkSurfaceBasedInterpolatorWidget::OnToggleWidgetActivation(bool enabled)
{
  Q_ASSERT(m_ToolManager);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
    return;

  m_Controls.m_btAccept->setEnabled(enabled);
  m_Controls.m_cbShowPositionNodes->setEnabled(enabled);

  if (enabled)
    m_Controls.m_btStart->setText("Stop");
  else
    m_Controls.m_btStart->setText("Start");

  for (unsigned int i = 0; i < m_ToolManager->GetTools().size(); i++)
  {
    mitk::SegTool2D *tool = dynamic_cast<mitk::SegTool2D *>(m_ToolManager->GetToolById(i));
    if (tool)
      tool->SetEnable3DInterpolation(enabled);
  }

  if (enabled)
  {
    if (!m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      m_DataStorage->Add(m_InterpolatedSurfaceNode);
    }

    if (!m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Add(m_3DContourNode);
    }

    mitk::Vector3D spacing = m_WorkingImage->GetGeometry(0)->GetSpacing();
    double minSpacing(100);
    double maxSpacing(0);
    for (int i = 0; i < 3; i++)
    {
      if (spacing[i] < minSpacing)
      {
        minSpacing = spacing[i];
      }
      else if (spacing[i] > maxSpacing)
      {
        maxSpacing = spacing[i];
      }
    }

    m_SurfaceBasedInterpolatorController->SetWorkingImage(m_WorkingImage);
    m_SurfaceBasedInterpolatorController->SetActiveLabel(m_WorkingImage->GetActiveLabel()->GetValue());
    m_SurfaceBasedInterpolatorController->SetMaxSpacing(maxSpacing);
    m_SurfaceBasedInterpolatorController->SetMinSpacing(minSpacing);
    m_SurfaceBasedInterpolatorController->SetDistanceImageVolume(50000);

    int ret = QMessageBox::Yes;

    if (m_SurfaceBasedInterpolatorController->EstimatePortionOfNeededMemory() > 0.5)
    {
      QMessageBox msgBox;
      msgBox.setText("Due to short handed system memory the 3D interpolation may be very slow!");
      msgBox.setInformativeText("Are you sure you want to activate the 3D interpolation?");
      msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
      ret = msgBox.exec();
    }

    if (m_Watcher.isRunning())
      m_Watcher.waitForFinished();

    if (ret == QMessageBox::Yes)
    {
      m_Future = QtConcurrent::run(this, &QmitkSurfaceBasedInterpolatorWidget::OnRunInterpolation);
      m_Watcher.setFuture(m_Future);
    }
  }
  else
  {
    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      m_DataStorage->Remove(m_InterpolatedSurfaceNode);
    }
    if (m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Remove(m_3DContourNode);
    }

    mitk::UndoController::GetCurrentUndoModel()->Clear();
  }

  m_Activated = enabled;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceBasedInterpolatorWidget::OnAcceptInterpolationClicked()
{
  if (m_InterpolatedSurfaceNode.IsNotNull() && m_InterpolatedSurfaceNode->GetData())
  {
    //    m_WorkingImage->SurfaceStamp(dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData()), false);
    this->ShowInterpolationResult(false);
  }
}

void QmitkSurfaceBasedInterpolatorWidget::OnSurfaceInterpolationInfoChanged(const itk::EventObject & /*e*/)
{
  if (m_Activated)
  {
    if (m_Watcher.isRunning())
      m_Watcher.waitForFinished();

    m_Future = QtConcurrent::run(this, &QmitkSurfaceBasedInterpolatorWidget::OnRunInterpolation);
    m_Watcher.setFuture(m_Future);
  }
}
