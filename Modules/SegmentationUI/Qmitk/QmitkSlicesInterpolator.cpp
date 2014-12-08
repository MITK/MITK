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

#include "QmitkSlicesInterpolator.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"

#include "mitkToolManager.h"
#include "mitkDataNodeFactory.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkProgressBar.h"
#include "mitkGlobalInteraction.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkInteractionConst.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkDiffImageApplier.h"
#include "mitkSegTool2D.h"
#include "mitkCoreObjectFactory.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkSliceNavigationController.h"
#include <mitkVtkImageOverwrite.h>
#include <mitkExtractSliceFilter.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageWriteAccessor.h>

#include <itkCommand.h>

#include <QCheckBox>
#include <QPushButton>
#include <QMenu>
#include <QCursor>
#include <QVBoxLayout>
#include <QMessageBox>


//#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

float SURFACE_COLOR_RGB [3] = {0.49f, 1.0f, 0.16f};

const std::map<QAction*, mitk::SliceNavigationController*> QmitkSlicesInterpolator::createActionToSliceDimension()
{
  std::map<QAction*, mitk::SliceNavigationController*> actionToSliceDimension;
  foreach(mitk::SliceNavigationController* slicer, m_ControllerToDeleteObserverTag.keys())
  {
    actionToSliceDimension[new QAction(QString::fromStdString(slicer->GetViewDirectionAsString()),0)] = slicer;
  }

  return actionToSliceDimension;
}


QmitkSlicesInterpolator::QmitkSlicesInterpolator(QWidget* parent, const char*  /*name*/)
  :QWidget(parent),
//    ACTION_TO_SLICEDIMENSION( createActionToSliceDimension() ),
    m_Interpolator( mitk::SegmentationInterpolationController::New() ),
    m_SurfaceInterpolator(mitk::SurfaceInterpolationController::GetInstance()),
    m_ToolManager(NULL),
    m_Initialized(false),
    m_LastSNC(0),
    m_LastSliceIndex(0),
    m_2DInterpolationEnabled(false),
    m_3DInterpolationEnabled(false)
{
  m_GroupBoxEnableExclusiveInterpolationMode = new QGroupBox("Interpolation", this);



  QVBoxLayout* vboxLayout = new QVBoxLayout(m_GroupBoxEnableExclusiveInterpolationMode);

  m_CmbInterpolation = new QComboBox(m_GroupBoxEnableExclusiveInterpolationMode);
  m_CmbInterpolation->addItem("Disabled");
  m_CmbInterpolation->addItem("2-Dimensional");
  m_CmbInterpolation->addItem("3-Dimensional");
  vboxLayout->addWidget(m_CmbInterpolation);

  m_BtnApply2D = new QPushButton("Confirm for single slice", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApply2D);

  m_BtnApplyForAllSlices2D = new QPushButton("Confirm for all slices", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApplyForAllSlices2D);

  m_BtnApply3D = new QPushButton("Confirm", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnApply3D);

  m_BtnReinit3DInterpolation = new QPushButton("Reinit Interpolation", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_BtnReinit3DInterpolation);

  m_ChkShowPositionNodes = new QCheckBox("Show Position Nodes", m_GroupBoxEnableExclusiveInterpolationMode);
  vboxLayout->addWidget(m_ChkShowPositionNodes);


  this->HideAllInterpolationControls();

  connect(m_CmbInterpolation, SIGNAL(currentIndexChanged(int)), this, SLOT(OnInterpolationMethodChanged(int)));
  connect(m_BtnApply2D, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_BtnApplyForAllSlices2D, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()));
  connect(m_BtnApply3D, SIGNAL(clicked()), this, SLOT(OnAccept3DInterpolationClicked()));
  connect(m_BtnReinit3DInterpolation, SIGNAL(clicked()), this, SLOT(OnReinit3DInterpolation()));
  connect(m_ChkShowPositionNodes, SIGNAL(toggled(bool)), this, SLOT(OnShowMarkers(bool)));
  connect(m_ChkShowPositionNodes, SIGNAL(toggled(bool)), this, SIGNAL(SignalShowMarkerNodes(bool)));

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->addWidget(m_GroupBoxEnableExclusiveInterpolationMode);
  this->setLayout(layout);

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnInterpolationInfoChanged );
  InterpolationInfoChangedObserverTag = m_Interpolator->AddObserver( itk::ModifiedEvent(), command );

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command2->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged );
  SurfaceInterpolationInfoChangedObserverTag = m_SurfaceInterpolator->AddObserver( itk::ModifiedEvent(), command2 );

  // feedback node and its visualization properties
  m_FeedbackNode = mitk::DataNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( m_FeedbackNode );

  m_FeedbackNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
  m_FeedbackNode->SetProperty( "outline binary", mitk::BoolProperty::New(true) );
  m_FeedbackNode->SetProperty( "color", mitk::ColorProperty::New(255.0, 255.0, 0.0) );
  m_FeedbackNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(false) );
  m_FeedbackNode->SetProperty( "layer", mitk::IntProperty::New( 20 ) );
  m_FeedbackNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  m_FeedbackNode->SetProperty( "name", mitk::StringProperty::New("Interpolation feedback") );
  m_FeedbackNode->SetProperty( "opacity", mitk::FloatProperty::New(0.8) );
  m_FeedbackNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );

  m_InterpolatedSurfaceNode = mitk::DataNode::New();
  m_InterpolatedSurfaceNode->SetProperty( "color", mitk::ColorProperty::New(SURFACE_COLOR_RGB) );
  m_InterpolatedSurfaceNode->SetProperty( "name", mitk::StringProperty::New("Surface Interpolation feedback") );
  m_InterpolatedSurfaceNode->SetProperty( "opacity", mitk::FloatProperty::New(0.5) );
  m_InterpolatedSurfaceNode->SetProperty( "line width", mitk::IntProperty::New(4) );
  m_InterpolatedSurfaceNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
  m_InterpolatedSurfaceNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
  m_InterpolatedSurfaceNode->SetVisibility(false);

  m_3DContourNode = mitk::DataNode::New();
  m_3DContourNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 0.0, 0.0) );
  m_3DContourNode->SetProperty("hidden object", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty( "name", mitk::StringProperty::New("Drawn Contours") );
  m_3DContourNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  m_3DContourNode->SetProperty("material.wireframeLineWidth", mitk::FloatProperty::New(2.0f));
  m_3DContourNode->SetProperty("3DContourContainer", mitk::BoolProperty::New(true));
  m_3DContourNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));
  m_3DContourNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

  QWidget::setContentsMargins(0, 0, 0, 0);
  if ( QWidget::layout() != NULL )
  {
    QWidget::layout()->setContentsMargins(0, 0, 0, 0);
  }

  //For running 3D Interpolation in background
  // create a QFuture and a QFutureWatcher

  connect(&m_Watcher, SIGNAL(started()), this, SLOT(StartUpdateInterpolationTimer()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnSurfaceInterpolationFinished()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(StopUpdateInterpolationTimer()));
  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(ChangeSurfaceColor()));
}

void QmitkSlicesInterpolator::SetDataStorage( mitk::DataStorage::Pointer storage )
{
  m_DataStorage = storage;
  m_SurfaceInterpolator->SetDataStorage(storage);
}

mitk::DataStorage* QmitkSlicesInterpolator::GetDataStorage()
{
  if ( m_DataStorage.IsNotNull() )
  {
    return m_DataStorage;
  }
  else
  {
    return NULL;
  }
}


void QmitkSlicesInterpolator::Initialize(mitk::ToolManager* toolManager, const QList<mitk::SliceNavigationController *> &controllers)
{
  Q_ASSERT(!controllers.empty());

  if (m_Initialized)
  {
    // remove old observers
    Uninitialize();
  }

  m_ToolManager = toolManager;

  if (m_ToolManager)
  {
    // set enabled only if a segmentation is selected
    mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
    QWidget::setEnabled( node != NULL );

    // react whenever the set of selected segmentation changes
    m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );
    m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified );

    // connect to the slice navigation controller. after each change, call the interpolator
    foreach(mitk::SliceNavigationController* slicer, controllers)
    {
      //Has to be initialized
      m_LastSNC = slicer;
      m_TimeStep.insert(slicer, slicer->GetTime()->GetPos());

      itk::MemberCommand<QmitkSlicesInterpolator>::Pointer deleteCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
      deleteCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted);
      m_ControllerToDeleteObserverTag.insert(slicer, slicer->AddObserver(itk::DeleteEvent(), deleteCommand));

      itk::MemberCommand<QmitkSlicesInterpolator>::Pointer timeChangedCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
      timeChangedCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnTimeChanged);
      m_ControllerToTimeObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::TimeGeometryEvent(NULL,0), timeChangedCommand));

      itk::MemberCommand<QmitkSlicesInterpolator>::Pointer sliceChangedCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
      sliceChangedCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSliceChanged);
      m_ControllerToSliceObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL,0), sliceChangedCommand));
    }
    ACTION_TO_SLICEDIMENSION = createActionToSliceDimension();
  }

  m_Initialized = true;
}

void QmitkSlicesInterpolator::Uninitialize()
{
  if (m_ToolManager.IsNotNull())
  {
    m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified);
    m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified);
  }

  foreach(mitk::SliceNavigationController* slicer, m_ControllerToSliceObserverTag.keys())
  {
    slicer->RemoveObserver(m_ControllerToDeleteObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToTimeObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToSliceObserverTag.take(slicer));
  }

  ACTION_TO_SLICEDIMENSION.clear();

  m_ToolManager = NULL;

  m_Initialized = false;
}

QmitkSlicesInterpolator::~QmitkSlicesInterpolator()
{
  if (m_Initialized)
  {
    // remove old observers
    Uninitialize();
  }

  if(m_DataStorage->Exists(m_3DContourNode))
    m_DataStorage->Remove(m_3DContourNode);
  if(m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    m_DataStorage->Remove(m_InterpolatedSurfaceNode);

  // remove observer
  m_Interpolator->RemoveObserver( InterpolationInfoChangedObserverTag );
  m_SurfaceInterpolator->RemoveObserver( SurfaceInterpolationInfoChangedObserverTag );

  delete m_Timer;
}

/**
External enableization...
*/
void QmitkSlicesInterpolator::setEnabled( bool enable )
{
  QWidget::setEnabled(enable);

  //Set the gui elements of the different interpolation modi enabled
  if (enable)
  {
    if (m_2DInterpolationEnabled)
    {
      this->Show2DInterpolationControls(true);
      m_Interpolator->Activate2DInterpolation(true);
    }
    else if (m_3DInterpolationEnabled)
    {
      this->Show3DInterpolationControls(true);
      this->Show3DInterpolationResult(true);
    }
  }
  //Set all gui elements of the interpolation disabled
  else
  {
    this->HideAllInterpolationControls();
    this->Show3DInterpolationResult(false);
  }
}

void QmitkSlicesInterpolator::On2DInterpolationEnabled(bool status)
{
  OnInterpolationActivated(status);
  m_Interpolator->Activate2DInterpolation(status);
}

void QmitkSlicesInterpolator::On3DInterpolationEnabled(bool status)
{
  On3DInterpolationActivated(status);
}

void QmitkSlicesInterpolator::OnInterpolationDisabled(bool status)
{
  if (status)
  {
    OnInterpolationActivated(!status);
    On3DInterpolationActivated(!status);
    this->Show3DInterpolationResult(false);
  }
}



void QmitkSlicesInterpolator::HideAllInterpolationControls()
{
  this->Show2DInterpolationControls(false);
  this->Show3DInterpolationControls(false);
}

void QmitkSlicesInterpolator::Show2DInterpolationControls(bool show)
{
  m_BtnApply2D->setVisible(show);
  m_BtnApplyForAllSlices2D->setVisible(show);
}

void QmitkSlicesInterpolator::Show3DInterpolationControls(bool show)
{
  m_BtnApply3D->setVisible(show);
  m_ChkShowPositionNodes->setVisible(show);
  m_BtnReinit3DInterpolation->setVisible(show);
}



void QmitkSlicesInterpolator::OnInterpolationMethodChanged(int index)
{
  switch(index)
  {
    case 0: // Disabled
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle("Interpolation");
      this->HideAllInterpolationControls();
      this->OnInterpolationActivated(false);
      this->On3DInterpolationActivated(false);
      this->Show3DInterpolationResult(false);
      m_Interpolator->Activate2DInterpolation(false);
      break;

    case 1: // 2D
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle("Interpolation (Enabled)");
      this->HideAllInterpolationControls();
      this->Show2DInterpolationControls(true);
      this->OnInterpolationActivated(true);
      this->On3DInterpolationActivated(false);
      m_Interpolator->Activate2DInterpolation(true);
      break;

    case 2: // 3D
      m_GroupBoxEnableExclusiveInterpolationMode->setTitle("Interpolation (Enabled)");
      this->HideAllInterpolationControls();
      this->Show3DInterpolationControls(true);
      this->OnInterpolationActivated(false);
      this->On3DInterpolationActivated(true);
      m_Interpolator->Activate2DInterpolation(false);
      break;

    default:
      MITK_ERROR << "Unknown interpolation method!";
      m_CmbInterpolation->setCurrentIndex(0);
      break;
  }
}

void QmitkSlicesInterpolator::OnShowMarkers(bool state)
{
  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("isContourMarker"
                                                                                                                              , mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
  {
    it->Value()->SetProperty("helper object", mitk::BoolProperty::New(!state));
  }
}

void QmitkSlicesInterpolator::OnToolManagerWorkingDataModified()
{
  if (m_ToolManager->GetWorkingData(0) != 0)
  {
    m_Segmentation = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());
    m_BtnReinit3DInterpolation->setEnabled(true);
  }
  else
  {
    //If no workingdata is set, remove the interpolation feedback
    this->GetDataStorage()->Remove(m_FeedbackNode);
    m_FeedbackNode->SetData(NULL);
    this->GetDataStorage()->Remove(m_3DContourNode);
    m_3DContourNode->SetData(NULL);
    this->GetDataStorage()->Remove(m_InterpolatedSurfaceNode);
    m_InterpolatedSurfaceNode->SetData(NULL);
    m_BtnReinit3DInterpolation->setEnabled(false);
    return;
  }
  //Updating the current selected segmentation for the 3D interpolation
  SetCurrentContourListID();

  if (m_2DInterpolationEnabled)
  {
    OnInterpolationActivated( true ); // re-initialize if needed
  }
  this->CheckSupportedImageDimension();
}

void QmitkSlicesInterpolator::OnToolManagerReferenceDataModified()
{
}

void QmitkSlicesInterpolator::OnTimeChanged(itk::Object* sender, const itk::EventObject& e)
{
  //Check if we really have a GeometryTimeEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent*>(&e))
    return;

  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(sender);
  Q_ASSERT(slicer);

  m_TimeStep[slicer];

  if (m_LastSNC == slicer)
  {
    slicer->SendSlice();//will trigger a new interpolation
  }
}

void QmitkSlicesInterpolator::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  //Check whether we really have a GeometrySliceEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent*>(&e))
    return;

  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(sender);

  if (TranslateAndInterpolateChangedSlice(e, slicer))
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e, mitk::SliceNavigationController* slicer)
{
  if (!m_2DInterpolationEnabled) return false;

  try
  {
    const mitk::SliceNavigationController::GeometrySliceEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent&>(e);

    mitk::TimeGeometry* tsg = event.GetTimeGeometry();
    if (tsg && m_TimeStep.contains(slicer))
    {
      mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(tsg->GetGeometryForTimeStep(m_TimeStep[slicer]).GetPointer());
      if (slicedGeometry)
      {
        m_LastSNC = slicer;
        mitk::PlaneGeometry* plane = dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry->GetPlaneGeometry( event.GetPos() ));
        if (plane)
          Interpolate( plane, m_TimeStep[slicer], slicer );
        return true;
      }
    }
  }
  catch(std::bad_cast)
  {
    return false; // so what
  }

  return false;
}

void QmitkSlicesInterpolator::Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep, mitk::SliceNavigationController* slicer )
{
  if (m_ToolManager)
  {
    mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
    if (node)
    {
      m_Segmentation = dynamic_cast<mitk::Image*>(node->GetData());
      if (m_Segmentation)
      {
        int clickedSliceDimension(-1);
        int clickedSliceIndex(-1);

        // calculate real slice position, i.e. slice of the image and not slice of the TimeSlicedGeometry
        mitk::SegTool2D::DetermineAffectedImageSlice( m_Segmentation, plane, clickedSliceDimension, clickedSliceIndex );

        mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( clickedSliceDimension, clickedSliceIndex, plane, timeStep );
        m_FeedbackNode->SetData( interpolation );

        m_LastSNC = slicer;
        m_LastSliceIndex = clickedSliceIndex;
      }
    }
  }
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationFinished()
{
  mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if(interpolatedSurface.IsNotNull() && workingNode &&
     workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))))
  {
    m_BtnApply3D->setEnabled(true);
    m_InterpolatedSurfaceNode->SetData(interpolatedSurface);
    m_3DContourNode->SetData(m_SurfaceInterpolator->GetContoursAsSurface());

    this->Show3DInterpolationResult(true);

    if( !m_DataStorage->Exists(m_InterpolatedSurfaceNode) )
    {
      m_DataStorage->Add(m_InterpolatedSurfaceNode);
    }
    if (!m_DataStorage->Exists(m_3DContourNode))
    {
      m_DataStorage->Add(m_3DContourNode, workingNode);
    }

  }
  else if (interpolatedSurface.IsNull())
  {
    m_BtnApply3D->setEnabled(false);

    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      this->Show3DInterpolationResult(false);
    }
  }

  m_BtnReinit3DInterpolation->setEnabled(true);

  foreach (mitk::SliceNavigationController* slicer, m_ControllerToTimeObserverTag.keys())
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

void QmitkSlicesInterpolator::OnAcceptInterpolationClicked()
{
  if (m_Segmentation && m_FeedbackNode->GetData())
  {
    //making interpolation separately undoable
    mitk::UndoStackItem::IncCurrObjectEventId();
    mitk::UndoStackItem::IncCurrGroupEventId();
    mitk::UndoStackItem::ExecuteIncrement();

    //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
    vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

    // Set slice as input
    mitk::Image::Pointer slice = dynamic_cast<mitk::Image*>(m_FeedbackNode->GetData());
    reslice->SetInputSlice(slice->GetSliceData()->GetVtkImageAccessor(slice)->GetVtkImageData());
    //set overwrite mode to true to write back to the image volume
    reslice->SetOverwriteMode(true);
    reslice->Modified();

    mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
    extractor->SetInput( m_Segmentation );
    unsigned int timestep = m_LastSNC->GetTime()->GetPos();
    extractor->SetTimeStep( timestep );
    extractor->SetWorldGeometry( m_LastSNC->GetCurrentPlaneGeometry() );
    extractor->SetVtkOutputRequest(true);
    extractor->SetResliceTransformByGeometry( m_Segmentation->GetTimeGeometry()->GetGeometryForTimeStep( timestep ) );

    extractor->Modified();
    extractor->Update();

    //the image was modified within the pipeline, but not marked so
    m_Segmentation->Modified();
    m_Segmentation->GetVtkImageData()->Modified();

    m_FeedbackNode->SetData(NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSlicesInterpolator::AcceptAllInterpolations(mitk::SliceNavigationController* slicer)
{
  /*
   * What exactly is done here:
   * 1. We create an empty diff image for the current segmentation
   * 2. All interpolated slices are written into the diff image
   * 3. Then the diffimage is applied to the original segmentation
   */
  if (m_Segmentation)
  {
    //making interpolation separately undoable
    mitk::UndoStackItem::IncCurrObjectEventId();
    mitk::UndoStackItem::IncCurrGroupEventId();
    mitk::UndoStackItem::ExecuteIncrement();

    mitk::Image::Pointer image3D = m_Segmentation;
    unsigned int timeStep( slicer->GetTime()->GetPos() );
    if (m_Segmentation->GetDimension() == 4)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput( m_Segmentation );
      timeSelector->SetTimeNr( timeStep );
      timeSelector->Update();
      image3D = timeSelector->GetOutput();
    }
    // create a empty diff image for the undo operation
    mitk::Image::Pointer diffImage = mitk::Image::New();
    diffImage->Initialize( image3D );

    // Create scope for ImageWriteAccessor so that the accessor is destroyed
    // after the image is initialized. Otherwise later image access will lead to an error
    {
      mitk::ImageWriteAccessor imAccess(diffImage);

      // Set all pixels to zero
      mitk::PixelType pixelType( mitk::MakeScalarPixelType<unsigned char>()  );
      memset( imAccess.GetData(), 0, (pixelType.GetBpe() >> 3) * diffImage->GetDimension(0) * diffImage->GetDimension(1) * diffImage->GetDimension(2) );
    }


    // Since we need to shift the plane it must be clone so that the original plane isn't altered
    mitk::PlaneGeometry::Pointer reslicePlane = slicer->GetCurrentPlaneGeometry()->Clone();

    int sliceDimension(-1);
    int sliceIndex(-1);
    mitk::SegTool2D::DetermineAffectedImageSlice( m_Segmentation, reslicePlane, sliceDimension, sliceIndex );

    unsigned int zslices = m_Segmentation->GetDimension( sliceDimension );
    mitk::ProgressBar::GetInstance()->AddStepsToDo(zslices);

    mitk::Point3D origin = reslicePlane->GetOrigin();
    unsigned int totalChangedSlices(0);

    for (unsigned int sliceIndex = 0; sliceIndex < zslices; ++sliceIndex)
    {
      // Transforming the current origin of the reslice plane
      // so that it matches the one of the next slice
      m_Segmentation->GetSlicedGeometry()->WorldToIndex(origin, origin);
      origin[sliceDimension] = sliceIndex;
      m_Segmentation->GetSlicedGeometry()->IndexToWorld(origin, origin);
      reslicePlane->SetOrigin(origin);
      //Set the slice as 'input'
      mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( sliceDimension, sliceIndex, reslicePlane, timeStep );

      if (interpolation.IsNotNull()) // we don't check if interpolation is necessary/sensible - but m_Interpolator does
      {
        //Setting up the reslicing pipeline which allows us to write the interpolation results back into
        //the image volume
        vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

        //set overwrite mode to true to write back to the image volume
        reslice->SetInputSlice(interpolation->GetSliceData()->GetVtkImageAccessor(interpolation)->GetVtkImageData());
        reslice->SetOverwriteMode(true);
        reslice->Modified();

        mitk::ExtractSliceFilter::Pointer diffslicewriter =  mitk::ExtractSliceFilter::New(reslice);
        diffslicewriter->SetInput( diffImage );
        diffslicewriter->SetTimeStep( timeStep );
        diffslicewriter->SetWorldGeometry(reslicePlane);
        diffslicewriter->SetVtkOutputRequest(true);
        diffslicewriter->SetResliceTransformByGeometry( diffImage->GetTimeGeometry()->GetGeometryForTimeStep( timeStep ) );

        diffslicewriter->Modified();
        diffslicewriter->Update();
        ++totalChangedSlices;
      }
      mitk::ProgressBar::GetInstance()->Progress();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    if (totalChangedSlices > 0)
    {
      // store undo stack items
      if ( true )
      {
        // create do/undo operations
        mitk::ApplyDiffImageOperation* doOp = new mitk::ApplyDiffImageOperation( mitk::OpTEST, m_Segmentation, diffImage, timeStep );
        mitk::ApplyDiffImageOperation* undoOp = new mitk::ApplyDiffImageOperation( mitk::OpTEST, m_Segmentation, diffImage, timeStep );
        undoOp->SetFactor( -1.0 );
        std::stringstream comment;
        comment << "Confirm all interpolations (" << totalChangedSlices << ")";
        mitk::OperationEvent* undoStackItem = new mitk::OperationEvent( mitk::DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, comment.str() );
        mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );

        // acutally apply the changes here to the original image
        mitk::DiffImageApplier::GetInstanceForUndo()->ExecuteOperation( doOp );
      }
    }

    m_FeedbackNode->SetData(NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSlicesInterpolator::FinishInterpolation(mitk::SliceNavigationController* slicer)
{
  //this redirect is for calling from outside

  if (slicer == NULL)
    OnAcceptAllInterpolationsClicked();
  else
    AcceptAllInterpolations( slicer );
}

void QmitkSlicesInterpolator::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  std::map<QAction*, mitk::SliceNavigationController*>::const_iterator it;
  for(it = ACTION_TO_SLICEDIMENSION.begin(); it != ACTION_TO_SLICEDIMENSION.end(); it++)
    orientationPopup.addAction(it->first);

  connect( &orientationPopup, SIGNAL(triggered(QAction*)), this, SLOT(OnAcceptAllPopupActivated(QAction*)) );

  orientationPopup.exec( QCursor::pos() );
}

void QmitkSlicesInterpolator::OnAccept3DInterpolationClicked()
{
  if (m_InterpolatedSurfaceNode.IsNotNull() && m_InterpolatedSurfaceNode->GetData())
  {
    mitk::SurfaceToImageFilter::Pointer s2iFilter = mitk::SurfaceToImageFilter::New();
    s2iFilter->MakeOutputBinaryOn();
    s2iFilter->SetInput(dynamic_cast<mitk::Surface*>(m_InterpolatedSurfaceNode->GetData()));

    // check if ToolManager holds valid ReferenceData
    if (m_ToolManager->GetReferenceData(0) == NULL || m_ToolManager->GetWorkingData(0) == NULL)
    {
        return;
    }
    s2iFilter->SetImage(dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData()));
    s2iFilter->Update();

    mitk::DataNode* segmentationNode = m_ToolManager->GetWorkingData(0);
    mitk::Image* oldSeg = dynamic_cast<mitk::Image*>(segmentationNode->GetData());
    mitk::Image::Pointer newSeg = s2iFilter->GetOutput();
    if (oldSeg)
      m_SurfaceInterpolator->ReplaceInterpolationSession(oldSeg, newSeg);
    else
      return;

    segmentationNode->SetData(newSeg);
    m_CmbInterpolation->setCurrentIndex(0);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    mitk::DataNode::Pointer segSurface = mitk::DataNode::New();
    float rgb[3];
    segmentationNode->GetColor(rgb);
    segSurface->SetColor(rgb);
    segSurface->SetData(m_InterpolatedSurfaceNode->GetData());
    std::stringstream stream;
    stream << segmentationNode->GetName();
    stream << "_";
    stream << "3D-interpolation";
    segSurface->SetName(stream.str());
    segSurface->SetProperty( "opacity", mitk::FloatProperty::New(0.7) );
    segSurface->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(true));
    segSurface->SetProperty( "3DInterpolationResult", mitk::BoolProperty::New(true));
    m_DataStorage->Add(segSurface, segmentationNode);
    this->Show3DInterpolationResult(false);
  }
}

void QmitkSlicesInterpolator::OnReinit3DInterpolation()
{
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("3DContourContainer", mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer contourNodes = m_DataStorage->GetDerivations( m_ToolManager->GetWorkingData(0), pred);
  if (contourNodes->Size() != 0)
  {
    m_3DContourNode = contourNodes->at(0);
  }
  else
  {
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Reinitialize surface interpolation");
    errorInfo.setIcon(QMessageBox::Information);
    errorInfo.setText("No contours available for the selected segmentation!");
    errorInfo.exec();
  }
  mitk::Surface::Pointer contours = dynamic_cast<mitk::Surface*>(m_3DContourNode->GetData());
  if (contours)
    mitk::SurfaceInterpolationController::GetInstance()->ReinitializeInterpolation(contours);
  m_BtnReinit3DInterpolation->setEnabled(false);
}

void QmitkSlicesInterpolator::OnAcceptAllPopupActivated(QAction* action)
{
  try
  {
    std::map<QAction*, mitk::SliceNavigationController*>::const_iterator iter = ACTION_TO_SLICEDIMENSION.find( action );
    if (iter != ACTION_TO_SLICEDIMENSION.end())
    {
      mitk::SliceNavigationController* slicer = iter->second;
      AcceptAllInterpolations( slicer );
    }
  }
  catch(...)
  {
    /* Showing message box with possible memory error */
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Interpolation Process");
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText("An error occurred during interpolation. Possible cause: Not enough memory!");
    errorInfo.exec();

    //additional error message on std::cerr
    std::cerr << "Ill construction in " __FILE__ " l. " << __LINE__ << std::endl;
  }
}

void QmitkSlicesInterpolator::OnInterpolationActivated(bool on)
{
  m_2DInterpolationEnabled = on;

  try
  {
    if ( m_DataStorage.IsNotNull() )
    {
      if (on && !m_DataStorage->Exists(m_FeedbackNode))
      {
        m_DataStorage->Add( m_FeedbackNode );
      }
    }
  }
  catch(...)
  {
    // don't care (double add/remove)
  }

  if (m_ToolManager)
  {
    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
    mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
    QWidget::setEnabled( workingNode != NULL );

    m_BtnApply2D->setEnabled( on );
    m_FeedbackNode->SetVisibility( on );

    if (!on)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return;
    }

    if (workingNode)
    {
      mitk::Image* segmentation = dynamic_cast<mitk::Image*>(workingNode->GetData());
      if (segmentation)
      {
        m_Interpolator->SetSegmentationVolume( segmentation );

        if (referenceNode)
        {
          mitk::Image* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
          m_Interpolator->SetReferenceVolume( referenceImage ); // may be NULL
        }
      }
    }
  }

  UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::Run3DInterpolation()
{
  m_SurfaceInterpolator->Interpolate();
}

void QmitkSlicesInterpolator::StartUpdateInterpolationTimer()
{
  m_Timer->start(500);
}

void QmitkSlicesInterpolator::StopUpdateInterpolationTimer()
{
  m_Timer->stop();
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(SURFACE_COLOR_RGB));
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetRenderWindow());
}

void QmitkSlicesInterpolator::ChangeSurfaceColor()
{
  float currentColor[3];
  m_InterpolatedSurfaceNode->GetColor(currentColor);

  if( currentColor[2] == SURFACE_COLOR_RGB[2])
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(1.0f,1.0f,1.0f));
  }
  else
  {
    m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(SURFACE_COLOR_RGB));
  }
  m_InterpolatedSurfaceNode->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetRenderWindow());
}

void QmitkSlicesInterpolator::On3DInterpolationActivated(bool on)
{
  m_3DInterpolationEnabled = on;

  this->CheckSupportedImageDimension();
  try
  {
    if ( m_DataStorage.IsNotNull() && m_ToolManager && m_3DInterpolationEnabled)
    {
      mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

      if (workingNode)
      {
        bool isInterpolationResult(false);
        workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);
        mitk::NodePredicateAnd::Pointer pred = mitk::NodePredicateAnd::New(mitk::NodePredicateProperty::New("3DInterpolationResult", mitk::BoolProperty::New(true)),
                                                                           mitk::NodePredicateDataType::New("Surface"));
        mitk::DataStorage::SetOfObjects::ConstPointer interpolationResults =
            m_DataStorage->GetDerivations(workingNode, pred);

        for (unsigned int i = 0; i < interpolationResults->Size(); ++i)
        {
          mitk::DataNode::Pointer currNode = interpolationResults->at(i);
          if (currNode.IsNotNull())
            m_DataStorage->Remove(currNode);
        }

        if ((workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")))) &&
            !isInterpolationResult && m_3DInterpolationEnabled)
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
            m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
            m_Watcher.setFuture(m_Future);
          }
          else
          {
            m_CmbInterpolation->setCurrentIndex(0);
          }
        }
        else if (!m_3DInterpolationEnabled)
        {
          this->Show3DInterpolationResult(false);
          m_BtnApply3D->setEnabled(m_3DInterpolationEnabled);
        }
      }
      else
      {
        QWidget::setEnabled( false );
        m_ChkShowPositionNodes->setEnabled(m_3DInterpolationEnabled);
      }
    }
    if (!m_3DInterpolationEnabled)
    {
       this->Show3DInterpolationResult(false);
       m_BtnApply3D->setEnabled(m_3DInterpolationEnabled);
    }
  }
  catch(...)
  {
    MITK_ERROR<<"Error with 3D surface interpolation!";
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::EnableInterpolation(bool on)
{
  // only to be called from the outside world
  // just a redirection to OnInterpolationActivated
  OnInterpolationActivated(on);
}

void QmitkSlicesInterpolator::Enable3DInterpolation(bool on)
{
  // only to be called from the outside world
  // just a redirection to OnInterpolationActivated
  On3DInterpolationActivated(on);
}

void QmitkSlicesInterpolator::UpdateVisibleSuggestion()
{
  if (m_2DInterpolationEnabled && m_LastSNC)
  {
    // determine which one is the current view, try to do an initial interpolation
    mitk::BaseRenderer* renderer = m_LastSNC->GetRenderer();
    if (renderer && renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
    {
      const mitk::TimeGeometry* timeGeometry = dynamic_cast<const mitk::TimeGeometry*>( renderer->GetWorldGeometry() );
      if (timeGeometry)
      {
        mitk::SliceNavigationController::GeometrySliceEvent event( const_cast<mitk::TimeGeometry*>(timeGeometry), renderer->GetSlice() );

        TranslateAndInterpolateChangedSlice(event, m_LastSNC);
      }
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::OnInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  // something (e.g. undo) changed the interpolation info, we should refresh our display
  UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  if(m_3DInterpolationEnabled)
  {
    if (m_Watcher.isRunning())
      m_Watcher.waitForFinished();
    m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
    m_Watcher.setFuture(m_Future);
  }
}

void QmitkSlicesInterpolator:: SetCurrentContourListID()
{
  // New ContourList = hide current interpolation
  Show3DInterpolationResult(false);

  if ( m_DataStorage.IsNotNull() && m_ToolManager && m_LastSNC )
  {
    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

    if (workingNode)
    {
      bool isInterpolationResult(false);
      workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);

      if (!isInterpolationResult)
      {
        QWidget::setEnabled( true );

        // In case the time is not valid use 0 to access the time geometry of the working node
        unsigned int time_position = 0;
        if( m_LastSNC->GetTime() != NULL )
          time_position = m_LastSNC->GetTime()->GetPos();

        mitk::Vector3D spacing = workingNode->GetData()->GetGeometry( time_position )->GetSpacing();
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

        m_SurfaceInterpolator->SetMaxSpacing(maxSpacing);
        m_SurfaceInterpolator->SetMinSpacing(minSpacing);
        m_SurfaceInterpolator->SetDistanceImageVolume(50000);

        mitk::Image* segmentationImage = dynamic_cast<mitk::Image*>(workingNode->GetData());
        if (segmentationImage->GetDimension() == 3)
          m_SurfaceInterpolator->SetCurrentInterpolationSession(segmentationImage);
        else
          MITK_INFO<<"3D Interpolation is only supported for 3D images at the moment!";

        if (m_3DInterpolationEnabled)
        {
          if (m_Watcher.isRunning())
            m_Watcher.waitForFinished();
          m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
          m_Watcher.setFuture(m_Future);
        }
      }
    }
    else
    {
      QWidget::setEnabled(false);
    }
  }
}

void QmitkSlicesInterpolator::Show3DInterpolationResult(bool status)
{
   if (m_InterpolatedSurfaceNode.IsNotNull())
      m_InterpolatedSurfaceNode->SetVisibility(status);

   if (m_3DContourNode.IsNotNull())
      m_3DContourNode->SetVisibility(status, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::CheckSupportedImageDimension()
{
  if (m_ToolManager->GetWorkingData(0))
    m_Segmentation = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());

  if (m_3DInterpolationEnabled && m_Segmentation && m_Segmentation->GetDimension() != 3)
  {
    QMessageBox info;
    info.setWindowTitle("3D Interpolation Process");
    info.setIcon(QMessageBox::Information);
    info.setText("3D Interpolation is only supported for 3D images at the moment!");
    info.exec();
    m_CmbInterpolation->setCurrentIndex(0);
  }
}

void QmitkSlicesInterpolator::OnSliceNavigationControllerDeleted(const itk::Object *sender, const itk::EventObject& /*e*/)
{
  //Don't know how to avoid const_cast here?!
  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(const_cast<itk::Object*>(sender));
  if (slicer)
  {
    m_ControllerToTimeObserverTag.remove(slicer);
    m_ControllerToSliceObserverTag.remove(slicer);
    m_ControllerToDeleteObserverTag.remove(slicer);
  }
}
