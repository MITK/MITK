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

#include "QmitkSurfaceBasedInterpolator.h"

#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkProgressBar.h"
#include "mitkOperationEvent.h"
#include "mitkInteractionConst.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkDiffImageApplier.h"
#include <mitkDiffSliceOperationApplier.h>
#include "mitkUndoController.h"
#include "mitkSegTool2D.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkSliceNavigationController.h"
//#include <mitkVtkImageOverwrite.h>
//#include <mitkExtractSliceFilter.h>
#include <mitkLabelSetImage.h>
//#include <mitkImageTimeSelector.h>
//#include <mitkImageToContourModelSetFilter.h>
//#include <mitkContourUtils.h>
#include <mitkToolManagerProvider.h>

#include "QmitkStdMultiWidget.h"

#include <itkCommand.h>

#include <QMessageBox>


QmitkSurfaceBasedInterpolator::QmitkSurfaceBasedInterpolator(QWidget* parent, const char*  /*name*/) : QWidget(parent),
m_SurfaceInterpolator(mitk::SurfaceInterpolationController::GetInstance()),
m_ToolManager(NULL),
m_DataStorage(NULL),
m_Initialized(false),
m_Activated(false)
{
  m_Controls.setupUi(this);

  connect(m_Controls.m_gbControls, SIGNAL(toggled(bool)), this, SLOT(OnActivateWidget(bool)));
  connect(m_Controls.m_btAccept, SIGNAL(clicked()), this, SLOT(OnAccept3DInterpolationClicked()));

  itk::ReceptorMemberCommand<QmitkSurfaceBasedInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSurfaceBasedInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSurfaceBasedInterpolator::OnSurfaceInterpolationInfoChanged );
  m_SurfaceInterpolationInfoChangedObserverTag = m_SurfaceInterpolator->AddObserver( itk::ModifiedEvent(), command );

  m_InterpolatedSurfaceNode = mitk::DataNode::New();
  m_InterpolatedSurfaceNode->SetName( "Surface Interpolation feedback" );
  m_InterpolatedSurfaceNode->SetProperty( "color", mitk::ColorProperty::New(255.0,255.0,0.0) );
  m_InterpolatedSurfaceNode->SetProperty( "opacity", mitk::FloatProperty::New(0.5) );
  m_InterpolatedSurfaceNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
  m_InterpolatedSurfaceNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
  m_InterpolatedSurfaceNode->SetVisibility(false);

  m_3DContourNode = mitk::DataNode::New();
  m_3DContourNode->SetName( "Drawn Contours" );
  m_3DContourNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 0.0, 0.0) );
  m_3DContourNode->SetProperty( "helper object", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty( "material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  m_3DContourNode->SetProperty( "material.wireframeLineWidth", mitk::FloatProperty::New(2.0f));
  m_3DContourNode->SetProperty( "3DContourContainer", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

  connect(&m_Watcher, SIGNAL(started()), this, SLOT(StartUpdateInterpolationTimer()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnSurfaceInterpolationFinished()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(StopUpdateInterpolationTimer()));

  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(ChangeSurfaceColor()));

  this->setEnabled(false);
}

void QmitkSurfaceBasedInterpolator::Initialize(mitk::DataStorage* storage)
{
  Q_ASSERT(!storage);

  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  m_DataStorage = storage;

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  // react whenever the set of selected segmentation changes
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSurfaceBasedInterpolator>( this, &QmitkSurfaceBasedInterpolator::OnToolManagerWorkingDataModified );

  m_Initialized = true;
}

void QmitkSurfaceBasedInterpolator::Uninitialize()
{
  if (m_ToolManager.IsNotNull())
  {
    m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSurfaceBasedInterpolator>(this, &QmitkSurfaceBasedInterpolator::OnToolManagerWorkingDataModified);
  }

  m_Initialized = false;
}

QmitkSurfaceBasedInterpolator::~QmitkSurfaceBasedInterpolator()
{
  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  if(m_DataStorage->Exists(m_3DContourNode))
    m_DataStorage->Remove(m_3DContourNode);

  if(m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    m_DataStorage->Remove(m_InterpolatedSurfaceNode);

  // remove observer
  m_SurfaceInterpolator->RemoveObserver( m_SurfaceInterpolationInfoChangedObserverTag );

  delete m_Timer;
}

void QmitkSurfaceBasedInterpolator::Show3DInterpolationResult(bool status)
{
   if (m_InterpolatedSurfaceNode.IsNotNull())
      m_InterpolatedSurfaceNode->SetVisibility(status);

   if (m_3DContourNode.IsNotNull())
      m_3DContourNode->SetVisibility(status, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceBasedInterpolator::OnSurfaceInterpolationFinished()
{
  mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if (interpolatedSurface.IsNotNull() && workingNode &&
     workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))))
  {
    m_InterpolatedSurfaceNode->SetData(interpolatedSurface);
    m_3DContourNode->SetData(m_SurfaceInterpolator->GetContoursAsSurface());

    this->Show3DInterpolationResult(true);

    if( !m_DataStorage->Exists(m_InterpolatedSurfaceNode) && !m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Add(m_3DContourNode);
      m_DataStorage->Add(m_InterpolatedSurfaceNode);
    }
  }
  else if (interpolatedSurface.IsNull())
  {
    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      this->Show3DInterpolationResult(false);
    }
  }
}

void QmitkSurfaceBasedInterpolator::StartUpdateInterpolationTimer()
{
  m_Timer->start(500);
}

void QmitkSurfaceBasedInterpolator::StopUpdateInterpolationTimer()
{
  m_Timer->stop();
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(255.0,255.0,0.0));
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetRenderWindow() );
}

void QmitkSurfaceBasedInterpolator::ChangeSurfaceColor()
{
  float currentColor[3];
  m_InterpolatedSurfaceNode->GetColor(currentColor);

  float yellow[3] = {255.0,255.0,0.0};

  if( currentColor[2] == yellow[2])
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(255.0,255.0,255.0));
  }
  else
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(yellow));
  }
  m_InterpolatedSurfaceNode->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetRenderWindow());
}

void QmitkSurfaceBasedInterpolator::OnToolManagerWorkingDataModified()
{
  if (!m_Initialized) return;

  mitk::DataNode* workingNode = this->m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    this->setEnabled(false);
    return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  Q_ASSERT(workingImage);

  if (workingImage->GetDimension() > 4 || workingImage->GetDimension() < 3)
  {
    this->setEnabled(false);
    return;
  }

  m_WorkingImage = workingImage;

  //Updating the current selected segmentation for the 3D interpolation
  this->SetCurrentContourListID();
}

void QmitkSurfaceBasedInterpolator::On3DInterpolationActivated(bool enabled)
{
  m_Activated = enabled;


  try
  {
    if ( m_DataStorage.IsNotNull() && m_ToolManager && m_Activated)
    {
      mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

      if (workingNode)
      {
        bool isInterpolationResult(false);
        workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);

        if ((workingNode->IsSelected() &&
             workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")))) &&
            !isInterpolationResult && m_Activated)
        {
          int ret = QMessageBox::Yes;

          if (m_SurfaceInterpolator->EstimatePortionOfNeededMemory() > 0.5)
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
            m_Future = QtConcurrent::run(this, &QmitkSurfaceBasedInterpolator::Run3DInterpolation);
            m_Watcher.setFuture(m_Future);
          }
        }
        else if (!m_Activated)
        {
          this->Show3DInterpolationResult(false);
//          m_Controls.m_btAccept->setEnabled(m_Activated);
        }
      }
      else
      {
        QWidget::setEnabled( false );
//        m_ChkShowPositionNodes->setEnabled(m_3DInterpolationEnabled);
      }
    }
    if (!m_Activated)
    {
       this->Show3DInterpolationResult(false);
//       m_Controls.m_btAccept->setEnabled(m_Activated);
    }
  }
  catch(...)
  {
    MITK_ERROR<<"Error with 3D surface interpolation!";
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceBasedInterpolator::Run3DInterpolation()
{
  m_SurfaceInterpolator->Interpolate();
}

void QmitkSurfaceBasedInterpolator:: SetCurrentContourListID()
{
  // New ContourList = hide current interpolation
  this->Show3DInterpolationResult(false);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode)
  {
    bool isInterpolationResult(false);
    workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);

    if (!isInterpolationResult)
    {
      //TODO Aufruf hier pruefen!
      mitk::Vector3D spacing = workingNode->GetData()->GetGeometry(0)->GetSpacing();
      double minSpacing (100);
      double maxSpacing (0);
      for (int i =0; i < 3; i++)
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

      m_SurfaceInterpolator->SetSegmentationImage(dynamic_cast<mitk::Image*>(workingNode->GetData()));
      m_SurfaceInterpolator->SetMaxSpacing(maxSpacing);
      m_SurfaceInterpolator->SetMinSpacing(minSpacing);
      m_SurfaceInterpolator->SetDistanceImageVolume(50000);

      mitk::Image* segmentationImage = dynamic_cast<mitk::Image*>(workingNode->GetData());
      if (segmentationImage->GetDimension() == 3)
        m_SurfaceInterpolator->SetCurrentSegmentationInterpolationList(segmentationImage);
      else
        MITK_INFO<<"3D Interpolation is only supported for 3D images at the moment!";

      if (m_Activated)
      {
        if (m_Watcher.isRunning())
          m_Watcher.waitForFinished();
        m_Future = QtConcurrent::run(this, &QmitkSurfaceBasedInterpolator::Run3DInterpolation);
        m_Watcher.setFuture(m_Future);
      }
    }
  }
}


void QmitkSurfaceBasedInterpolator::OnActivateWidget(bool enabled)
{
  if (!m_Initialized) return;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode) return;

  m_Activated = enabled;

  if ( m_ToolManager.IsNotNull() )
  {
    unsigned int numberOfExistingTools = m_ToolManager->GetTools().size();
    for(unsigned int i = 0; i < numberOfExistingTools; i++)
    {
      mitk::SegTool2D* tool = dynamic_cast<mitk::SegTool2D*>(m_ToolManager->GetToolById(i));
      if (tool) tool->SetEnable3DInterpolation( m_Activated );
    }
  }

  if (m_Activated)
  {
    if (!m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      m_DataStorage->Add( m_InterpolatedSurfaceNode );
    }
    if (!m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Add( m_3DContourNode );
    }

    bool isInterpolationResult(false);
    workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);

    int ret = QMessageBox::Yes;

    if (m_SurfaceInterpolator->EstimatePortionOfNeededMemory() > 0.5)
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
      m_Future = QtConcurrent::run(this, &QmitkSurfaceBasedInterpolator::Run3DInterpolation);
      m_Watcher.setFuture(m_Future);
    }
  }
  else
  {
    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      m_DataStorage->Remove( m_InterpolatedSurfaceNode );
    }
    if (m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Remove( m_3DContourNode );
    }

    this->Show3DInterpolationResult(false);

    mitk::UndoController::GetCurrentUndoModel()->Clear();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceBasedInterpolator::OnAccept3DInterpolationClicked()
{
  if (m_InterpolatedSurfaceNode.IsNotNull() && m_InterpolatedSurfaceNode->GetData())
  {
    m_WorkingImage->SurfaceStamp(dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData()), false);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->Show3DInterpolationResult(false);
  }
}

void QmitkSurfaceBasedInterpolator::OnSurfaceInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  if (m_Activated)
  {
    if (m_Watcher.isRunning())
      m_Watcher.waitForFinished();
    m_Future = QtConcurrent::run(this, &QmitkSurfaceBasedInterpolator::Run3DInterpolation);
    m_Watcher.setFuture(m_Future);
  }
}
