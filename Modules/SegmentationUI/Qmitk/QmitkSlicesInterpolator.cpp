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

#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkProgressBar.h"
#include "mitkGlobalInteraction.h"
#include "mitkOperationEvent.h"
#include "mitkInteractionConst.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkDiffImageApplier.h"
#include <mitkDiffSliceOperationApplier.h>
#include "mitkUndoController.h"
#include "mitkSegTool2D.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkSliceNavigationController.h"
#include <mitkVtkImageOverwrite.h>
#include <mitkExtractSliceFilter.h>
#include <mitkLabelSetImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToContourModelFilter.h>
#include <mitkContourUtils.h>
#include <mitkToolManagerProvider.h>
#include <itkCommand.h>

#include <QCheckBox>
#include <QPushButton>
#include <QMenu>
#include <QCursor>
#include <QVBoxLayout>
#include <QMessageBox>

QmitkSlicesInterpolator::QmitkSlicesInterpolator(QWidget* parent, const char*  /*name*/) : QWidget(parent),
m_SliceInterpolatorController( mitk::SegmentationInterpolationController::New() ),
m_SurfaceInterpolator(mitk::SurfaceInterpolationController::GetInstance()),
m_ToolManager(NULL),
m_Initialized(false),
m_LastSNC(0),
m_LastSliceIndex(0),
m_2DInterpolationEnabled(false),
m_3DInterpolationEnabled(false)
{
  m_gbControls = new QGroupBox("Interpolation", this);
  m_gbControls->setCheckable(true);
  m_gbControls->setChecked(false);

  QVBoxLayout* vboxLayout = new QVBoxLayout(m_gbControls);

  m_cbInterpolationMode = new QComboBox(m_gbControls);
  m_cbInterpolationMode->addItem("2-Dimensional");
  m_cbInterpolationMode->addItem("3-Dimensional");
  vboxLayout->addWidget(m_cbInterpolationMode);

  m_btApply2D = new QPushButton("Accept", m_gbControls);
  vboxLayout->addWidget(m_btApply2D);

  m_btApplyForAllSlices2D = new QPushButton("Accept All", m_gbControls);
  vboxLayout->addWidget(m_btApplyForAllSlices2D);

  m_btApply3D = new QPushButton("Accept", m_gbControls);
  vboxLayout->addWidget(m_btApply3D);

  m_chkShowPositionNodes = new QCheckBox("Show Position Nodes", m_gbControls);
  vboxLayout->addWidget(m_chkShowPositionNodes);

  this->HideAllInterpolationControls();

  connect(m_gbControls, SIGNAL(toggled(bool)), this, SLOT(ActivateInterpolation(bool)));
  connect(m_cbInterpolationMode, SIGNAL(currentIndexChanged(int)), this, SLOT(OnInterpolationMethodChanged(int)));
  connect(m_btApply2D, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_btApplyForAllSlices2D, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()));
  connect(m_btApply3D, SIGNAL(clicked()), this, SLOT(OnAccept3DInterpolationClicked()));
  connect(m_chkShowPositionNodes, SIGNAL(toggled(bool)), this, SLOT(OnShowMarkers(bool)));
  connect(m_chkShowPositionNodes, SIGNAL(toggled(bool)), this, SIGNAL(SignalShowMarkerNodes(bool)));

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->addWidget(m_gbControls);
  this->setLayout(layout);

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSliceInterpolationInfoChanged );
  m_InterpolationInfoChangedObserverTag = m_SliceInterpolatorController->AddObserver( itk::ModifiedEvent(), command );

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command2->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged );
  m_SurfaceInterpolationInfoChangedObserverTag = m_SurfaceInterpolator->AddObserver( itk::ModifiedEvent(), command2 );

  m_FeedbackContour = mitk::ContourModel::New();
  m_FeedbackContourNode = mitk::DataNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetName("2D interpolation preview");
  m_FeedbackContourNode->SetProperty("visible", mitk::BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("layer", mitk::IntProperty::New(1000));
  m_FeedbackContourNode->SetProperty("contour.project-onto-plane", mitk::BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("contour.width", mitk::FloatProperty::New(2.5));

  this->Disable3DRendering();

  m_InterpolatedSurfaceNode = mitk::DataNode::New();
  m_InterpolatedSurfaceNode->SetName( "3D interpolation preview" );
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

  QWidget::setContentsMargins(0, 0, 0, 0);
  if ( QWidget::layout() != NULL )
  {
    QWidget::layout()->setContentsMargins(0, 0, 0, 0);
  }

  //For running 3D Interpolation in background
  // create a QFuture and a QFutureWatcher
/*
  connect(&m_Watcher, SIGNAL(started()), this, SLOT(StartUpdateInterpolationTimer()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(OnSurfaceInterpolationFinished()));
  connect(&m_Watcher, SIGNAL(finished()), this, SLOT(StopUpdateInterpolationTimer()));
  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(ChangeSurfaceColor()));
*/
  QWidget::setEnabled(false);
}

const QmitkSlicesInterpolator::ActionToSliceDimensionMapType QmitkSlicesInterpolator::CreateActionToSliceDimension()
{
  ActionToSliceDimensionMapType actionToSliceDimension;
  foreach(mitk::SliceNavigationController* slicer, m_ControllerToDeleteObserverTag.keys())
  {
    std::string name = slicer->GetRenderer()->GetName();
    if (name == "stdmulti.widget1")
      name = "Axial (red window)";
    else if (name == "stdmulti.widget2")
      name = "Sagittal (green window)";
    else if (name == "stdmulti.widget3")
      name = "Coronal (blue window)";
    actionToSliceDimension[new QAction(QString::fromStdString(name),0)] = slicer;
  }

  return actionToSliceDimension;
}

void QmitkSlicesInterpolator::SetDataStorage( mitk::DataStorage::Pointer storage )
{
  m_DataStorage = storage;
  m_SurfaceInterpolator->SetDataStorage(storage);
}

void QmitkSlicesInterpolator::Initialize(const QList<mitk::SliceNavigationController *> &controllers)
{
  Q_ASSERT(!controllers.empty());

  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  // react whenever the set of selected segmentation changes
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );

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
    m_ControllerToTimeObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::TimeSlicedGeometryEvent(NULL,0), timeChangedCommand));

    itk::MemberCommand<QmitkSlicesInterpolator>::Pointer sliceChangedCommand = itk::MemberCommand<QmitkSlicesInterpolator>::New();
    sliceChangedCommand->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSliceChanged);
    m_ControllerToSliceObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL,0), sliceChangedCommand));
  }
  m_ActionToSliceDimensionMap = this->CreateActionToSliceDimension();

  m_Initialized = true;
}

void QmitkSlicesInterpolator::Uninitialize()
{
  if (m_ToolManager.IsNotNull())
  {
    m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>(this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified);
  }

  foreach(mitk::SliceNavigationController* slicer, m_ControllerToSliceObserverTag.keys())
  {
    slicer->RemoveObserver(m_ControllerToDeleteObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToTimeObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToSliceObserverTag.take(slicer));
  }

  m_ActionToSliceDimensionMap.clear();

  m_Initialized = false;
}

QmitkSlicesInterpolator::~QmitkSlicesInterpolator()
{
  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  if ( m_DataStorage.IsNotNull() )
  {
    if(m_DataStorage->Exists(m_3DContourNode))
        m_DataStorage->Remove(m_3DContourNode);

    if(m_DataStorage->Exists(m_InterpolatedSurfaceNode))
        m_DataStorage->Remove(m_InterpolatedSurfaceNode);
  }

  // remove observer
  m_SliceInterpolatorController->RemoveObserver( m_InterpolationInfoChangedObserverTag );
  m_SurfaceInterpolator->RemoveObserver( m_SurfaceInterpolationInfoChangedObserverTag );

  //delete m_Timer;
}

void QmitkSlicesInterpolator::Disable3DRendering()
{
  // set explicitly visible=false for all 3D renderer (that exist already ...)
  const mitk::RenderingManager::RenderWindowVector& renderWindows = mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for (mitk::RenderingManager::RenderWindowVector::const_iterator iter = renderWindows.begin();
    iter != renderWindows.end(); ++iter)
  {
    if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == mitk::BaseRenderer::Standard3D )
      //if ( (*iter)->GetRenderer()->GetMapperID() == BaseRenderer::Standard3D )
    {
      m_FeedbackContourNode->SetProperty("visible", mitk::BoolProperty::New(false), mitk::BaseRenderer::GetInstance((*iter)));
    }
  }
}

void QmitkSlicesInterpolator::HideAllInterpolationControls()
{
  this->Show2DInterpolationControls(false);
  this->Show3DInterpolationControls(false);
}

void QmitkSlicesInterpolator::Show2DInterpolationControls(bool show)
{
  m_btApply2D->setVisible(show);
  m_btApplyForAllSlices2D->setVisible(show);
}

void QmitkSlicesInterpolator::Show3DInterpolationControls(bool show)
{
  m_btApply3D->setVisible(show);
  m_chkShowPositionNodes->setVisible(show);
}

void QmitkSlicesInterpolator::setChecked(bool enabled)
{
  m_gbControls->setChecked(enabled);
}

void QmitkSlicesInterpolator::ActivateInterpolation(bool enabled)
{
  if (enabled)
  {
    this->Show2DInterpolationControls(true);
    this->Activate2DInterpolation(true);
  }
  else
  {
    this->HideAllInterpolationControls();
    this->Activate2DInterpolation(false);
    mitk::UndoController::GetCurrentUndoModel()->Clear();
  }
}

void QmitkSlicesInterpolator::OnInterpolationMethodChanged(int index)
{
  switch(index)
  {
    default:
    case 0: // 2D
      m_gbControls->setTitle("2D Interpolation (Enabled)");
      this->HideAllInterpolationControls();
      this->Show2DInterpolationControls(true);
      this->Activate2DInterpolation(true);
      this->Activate3DInterpolation(false);
      break;

    case 1: // 3D
      m_gbControls->setTitle("3D Interpolation (Enabled)");
      this->HideAllInterpolationControls();
      this->Show3DInterpolationControls(true);
      this->Activate2DInterpolation(false);
      this->Activate3DInterpolation(true);
      break;
  }
}

void QmitkSlicesInterpolator::OnShowMarkers(bool state)
{
  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers =
    m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
  {
    it->Value()->SetProperty("helper object", mitk::BoolProperty::New(!state));
  }
}

void QmitkSlicesInterpolator::OnToolManagerWorkingDataModified()
{
    mitk::DataNode* workingNode = this->m_ToolManager->GetWorkingData(0);
    if (!workingNode)
    {
      this->setChecked(false);
      this->setEnabled(false);
      return;
    }

    mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
    assert(workingImage);

    if (workingImage->GetDimension() > 4 || workingImage->GetDimension() < 3)
    {
      MITK_ERROR << "slices interpolator needs a 3D or 3D+t segmentation, not 2D.";
      this->setChecked(false);
      this->setEnabled(false);
      return;
    }

    if ( m_WorkingImage != workingImage )
    {
      m_WorkingImage = workingImage;
      if (m_2DInterpolationEnabled)
      {
        m_SliceInterpolatorController->SetWorkingImage( m_WorkingImage );
        this->UpdateVisibleSuggestion();
      }
    }
}

void QmitkSlicesInterpolator::OnTimeChanged(itk::Object* sender, const itk::EventObject& e)
{
  //Check if we really have a GeometryTimeEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent*>(&e))
    return;

  mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(sender);
  Q_ASSERT(slicer);

  m_TimeStep[slicer]/* = event.GetPos()*/;

  //TODO Macht das hier wirklich Sinn????
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

  this->TranslateAndInterpolateChangedSlice(e, slicer);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  //  slicer->GetRenderer()->RequestUpdate();
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e, mitk::SliceNavigationController* slicer)
{
  if (!m_2DInterpolationEnabled) return false;

  const mitk::SliceNavigationController::GeometrySliceEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent&>(e);

  mitk::TimeGeometry* tsg = event.GetTimeGeometry();
    if (tsg && m_TimeStep.contains(slicer))
  {
    mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(tsg->GetGeometryForTimeStep(m_TimeStep[slicer]).GetPointer());
    if (slicedGeometry)
    {
      m_LastSNC = slicer;
      mitk::PlaneGeometry* plane = dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry->GetGeometry2D( event.GetPos() ));
      if (plane)
      {
        this->Interpolate( plane, m_TimeStep[slicer], slicer );
        return true;
      }
    }
  }

  return false;
}

void QmitkSlicesInterpolator::Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep, mitk::SliceNavigationController* slicer )
{
  int clickedSliceDimension(-1);
  int clickedSliceIndex(-1);

  // calculate real slice position, i.e. slice of the image and not slice of the TimeSlicedGeometry
  // see if timestep is needed here
  mitk::SegTool2D::DetermineAffectedImageSlice( m_WorkingImage, plane, clickedSliceDimension, clickedSliceIndex );

  mitk::Image::Pointer interpolation = m_SliceInterpolatorController->Interpolate( clickedSliceDimension, clickedSliceIndex, plane, timeStep );

  if (interpolation.IsNotNull())
  {
    mitk::ImageToContourModelFilter::Pointer contourExtractor = mitk::ImageToContourModelFilter::New();
    contourExtractor->SetInput(interpolation);
//    contourExtractor->SetUseProgressBar(false);

    try
    {
      contourExtractor->Update();
    }
    catch ( itk::ExceptionObject& e )
    {
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      return;
    }

    int numberOfContours = contourExtractor->GetNumberOfIndexedOutputs();

    m_FeedbackContour = contourExtractor->GetOutput(0);
    m_FeedbackContour->DisconnectPipeline();

    m_FeedbackContourNode->SetData( m_FeedbackContour );

    const mitk::Color& color = m_WorkingImage->GetActiveLabelColor();
    m_FeedbackContourNode->SetProperty("contour.color", mitk::ColorProperty::New(color));
  }
  else
  {
    m_FeedbackContour->Clear(timeStep);
  }

  m_LastSNC = slicer;
  m_LastSliceIndex = clickedSliceIndex;
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationFinished()
{
  mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if (interpolatedSurface.IsNotNull() && workingNode &&
     workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("Coronal"))))
  {
    m_btApply3D->setEnabled(true);
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
    m_btApply3D->setEnabled(false);

    if (m_DataStorage->Exists(m_InterpolatedSurfaceNode))
    {
      this->Show3DInterpolationResult(false);
    }
  }

  foreach (mitk::SliceNavigationController* slicer, m_ControllerToTimeObserverTag.keys())
  {
    slicer->GetRenderer()->RequestUpdate();
  }
}

mitk::Image::Pointer QmitkSlicesInterpolator::GetWorkingSlice(const mitk::PlaneGeometry* planeGeometry)
{
//  const mitk::PlaneGeometry* planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
  if (!planeGeometry) return NULL;

  unsigned int timeStep = m_LastSNC->GetTime()->GetPos();

  //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  //set to false to extract a slice
  reslice->SetOverwriteMode(false);
  reslice->Modified();

  //use ExtractSliceFilter with our specific vtkImageReslice for overwriting and extracting
  mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput( m_WorkingImage );
  extractor->SetTimeStep( timeStep );
  extractor->SetWorldGeometry( planeGeometry );
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry( m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep) );

  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch ( itk::ExceptionObject & excep )
  {
    MITK_ERROR << "Exception caught: " << excep.GetDescription();
    return NULL;
  }

  mitk::Image::Pointer slice = extractor->GetOutput();

  //specify the undo operation with the non edited slice
  m_undoOperation = new mitk::DiffSliceOperation(
    m_WorkingImage, extractor->GetVtkOutput(), slice->GetGeometry(), timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));

  slice->DisconnectPipeline();

  return slice;
}

void QmitkSlicesInterpolator::OnAcceptInterpolationClicked()
{
  if (m_WorkingImage.IsNotNull() && m_FeedbackContourNode->GetData())
  {
    const mitk::PlaneGeometry* planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
    mitk::Image::Pointer sliceImage = this->GetWorkingSlice(planeGeometry);
    if (sliceImage.IsNull()) return;

    unsigned int timeStep = m_LastSNC->GetTime()->GetPos();

//    const mitk::PlaneGeometry* planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
//    if (!planeGeometry) return;

    mitk::ContourModel::Pointer projectedContour = mitk::ContourModel::New();
    const mitk::Geometry3D* sliceGeometry = sliceImage->GetGeometry(timeStep);
    mitk::ContourUtils::ProjectContourTo2DSlice( sliceGeometry, m_FeedbackContour, projectedContour );
    mitk::ContourUtils::FillContourInSlice( projectedContour, sliceImage, m_WorkingImage->GetActiveLabelIndex() );

    //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
    vtkSmartPointer<mitkVtkImageOverwrite> overwrite = vtkSmartPointer<mitkVtkImageOverwrite>::New();
    overwrite->SetInputSlice(sliceImage->GetVtkImageData());
    //set overwrite mode to true to write back to the image volume
    overwrite->SetOverwriteMode(true);
    overwrite->Modified();

    mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(overwrite);
    extractor->SetInput( m_WorkingImage );
    extractor->SetTimeStep( timeStep );
    extractor->SetWorldGeometry( planeGeometry );
    extractor->SetVtkOutputRequest(true);
    extractor->SetResliceTransformByGeometry( m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep) );

    extractor->Modified();

    try
    {
      extractor->Update();
    }
    catch ( itk::ExceptionObject & excep )
    {
      MITK_ERROR << "Exception caught: " << excep.GetDescription();
      return;
    }

    //the image was modified within the pipeline, but not marked so
    m_WorkingImage->Modified();

    int clickedSliceDimension(-1);
    int clickedSliceIndex(-1);
    mitk::SegTool2D::DetermineAffectedImageSlice( m_WorkingImage, planeGeometry, clickedSliceDimension, clickedSliceIndex );
    m_SliceInterpolatorController->SetChangedSlice( sliceImage, clickedSliceDimension, clickedSliceIndex, timeStep );

    //specify the undo operation with the edited slice
    m_doOperation = new mitk::DiffSliceOperation(
      m_WorkingImage, extractor->GetVtkOutput(),sliceImage->GetGeometry(), timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));

    //create an operation event for the undo stack
    mitk::OperationEvent* undoStackItem = new mitk::OperationEvent(
      mitk::DiffSliceOperationApplier::GetInstance(), m_doOperation, m_undoOperation, "Slice Interpolation" );

    //add it to the undo controller
    mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );

    //clear the pointers as the operation are stored in the undocontroller and also deleted from there
    m_undoOperation = NULL;
    m_doOperation = NULL;

    m_FeedbackContour->Clear();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSlicesInterpolator::AcceptAllInterpolations(mitk::SliceNavigationController* slicer)
{
  // Since we need to shift the plane it must be clone so that the original plane isn't altered
  mitk::PlaneGeometry::Pointer reslicePlane = slicer->GetCurrentPlaneGeometry()->Clone();
  unsigned int timeStep = slicer->GetTime()->GetPos();

  int sliceDimension(-1);
  int sliceIndex(-1);
  mitk::SegTool2D::DetermineAffectedImageSlice( m_WorkingImage, reslicePlane, sliceDimension, sliceIndex );

  unsigned int zslices = m_WorkingImage->GetDimension( sliceDimension );
  mitk::ProgressBar::GetInstance()->AddStepsToDo(zslices);

  mitk::Point3D origin = reslicePlane->GetOrigin();

  for (unsigned int idx = 0; idx < zslices; ++idx)
  {
    // Transforming the current origin of the reslice plane
    // so that it matches the one of the next slice
    m_WorkingImage->GetSlicedGeometry()->WorldToIndex(origin, origin);
    origin[sliceDimension] = idx;
    m_WorkingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
    reslicePlane->SetOrigin(origin);

    mitk::Image::Pointer interpolation = m_SliceInterpolatorController->Interpolate( sliceDimension, idx, reslicePlane, timeStep );

    if (interpolation.IsNotNull())
    {
      mitk::ImageToContourModelFilter::Pointer contourExtractor = mitk::ImageToContourModelFilter::New();
      contourExtractor->SetInput(interpolation);
//      contourExtractor->SetUseProgressBar(false);

      try
      {
        contourExtractor->Update();
      }
      catch ( itk::ExceptionObject& e )
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        return;
      }

      // todo: consider all contours
      //int numberOfContours = contourExtractor->GetNumberOfIndexedOutputs();

      m_FeedbackContour = contourExtractor->GetOutput(0);
      m_FeedbackContour->DisconnectPipeline();

      m_FeedbackContourNode->SetData( m_FeedbackContour );

      mitk::Image::Pointer sliceImage = this->GetWorkingSlice(reslicePlane);
      if (sliceImage.IsNull()) return;

      mitk::ContourModel::Pointer projectedContour = mitk::ContourModel::New();
      const mitk::Geometry3D* sliceGeometry = sliceImage->GetGeometry();
      mitk::ContourUtils::ProjectContourTo2DSlice( sliceGeometry, m_FeedbackContour, projectedContour );
      if (projectedContour.IsNull()) return;

      mitk::ContourUtils::FillContourInSlice( projectedContour, sliceImage, m_WorkingImage->GetActiveLabelIndex() );

      //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
      vtkSmartPointer<mitkVtkImageOverwrite> overwrite = vtkSmartPointer<mitkVtkImageOverwrite>::New();
      overwrite->SetInputSlice(sliceImage->GetVtkImageData());
      //set overwrite mode to true to write back to the image volume
      overwrite->SetOverwriteMode(true);
      overwrite->Modified();

      mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(overwrite);
      extractor->SetInput( m_WorkingImage );
      extractor->SetTimeStep( timeStep );
      extractor->SetWorldGeometry( reslicePlane );
      extractor->SetVtkOutputRequest(true);
      extractor->SetResliceTransformByGeometry( m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep) );

      extractor->Modified();

      try
      {
        extractor->Update();
      }
      catch ( itk::ExceptionObject& e )
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        return;
      }

      m_WorkingImage->Modified();

      m_SliceInterpolatorController->SetChangedSlice( interpolation, sliceDimension, idx, timeStep );

      mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
    }

    mitk::ProgressBar::GetInstance()->Progress();
  }

  //the image was modified within the pipeline, but not marked so
  m_WorkingImage->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSlicesInterpolator::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  std::map<QAction*, mitk::SliceNavigationController*>::const_iterator it;
  for(it = m_ActionToSliceDimensionMap.begin(); it != m_ActionToSliceDimensionMap.end(); it++)
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
      return;

    s2iFilter->SetImage(dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData()));
    s2iFilter->Update();

    mitk::DataNode* segmentationNode = m_ToolManager->GetWorkingData(0);
    segmentationNode->SetData(s2iFilter->GetOutput());
    m_cbInterpolationMode->setCurrentIndex(0);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->Show3DInterpolationResult(false);
  }
}

void QmitkSlicesInterpolator::OnAcceptAllPopupActivated(QAction* action)
{
  ActionToSliceDimensionMapType::const_iterator iter = m_ActionToSliceDimensionMap.find( action );
  if (iter != m_ActionToSliceDimensionMap.end())
  {
    mitk::SliceNavigationController* slicer = iter->second;
    this->AcceptAllInterpolations( slicer );
  }
}

void QmitkSlicesInterpolator::Activate2DInterpolation(bool on)
{
  if (m_WorkingImage.IsNull()) return;

  m_2DInterpolationEnabled = on;

  emit Signal2DInterpolationEnabled(m_2DInterpolationEnabled);

  if ( m_DataStorage.IsNotNull() )
  {
    if (on && !m_DataStorage->Exists(m_FeedbackContourNode))
    {
      m_DataStorage->Add( m_FeedbackContourNode );
    }
    else if (!on && m_DataStorage->Exists(m_FeedbackContourNode))
    {
      m_DataStorage->Remove( m_FeedbackContourNode );
    }
  }

  if (on)
  {
    m_SliceInterpolatorController->SetWorkingImage( m_WorkingImage );
    this->UpdateVisibleSuggestion();
    return;
  }

/*
  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  if (referenceNode)
  {
    mitk::Image* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
    m_SliceInterpolatorController->SetReferenceImage( referenceImage ); // may be NULL
  }
*/
}

void QmitkSlicesInterpolator::Run3DInterpolation()
{
  m_SurfaceInterpolator->Interpolate();
}

void QmitkSlicesInterpolator::StartUpdateInterpolationTimer()
{
 // m_Timer->start(500);
}

void QmitkSlicesInterpolator::StopUpdateInterpolationTimer()
{
 // m_Timer->stop();
  m_InterpolatedSurfaceNode->SetProperty("color", mitk::ColorProperty::New(255.0,255.0,0.0));
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("3D"))->GetRenderWindow());
}

void QmitkSlicesInterpolator::ChangeSurfaceColor()
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
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("3D"))->GetRenderWindow());
}

void QmitkSlicesInterpolator::Activate3DInterpolation(bool on)
{
  m_3DInterpolationEnabled = on;

  try
  {
    if ( m_DataStorage.IsNotNull() && m_ToolManager && m_3DInterpolationEnabled)
    {
      mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

      if (workingNode)
      {
        bool isInterpolationResult(false);
        workingNode->GetBoolProperty("3DInterpolationResult",isInterpolationResult);

        if ((workingNode->IsSelected() &&
             workingNode->IsVisible(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("Coronal")))) &&
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

//          if (m_Watcher.isRunning())
//            m_Watcher.waitForFinished();

          if (ret == QMessageBox::Yes)
          {
//            m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
//            m_Watcher.setFuture(m_Future);
          }
          else
          {
            m_cbInterpolationMode->setCurrentIndex(0);
          }
        }
        else if (!m_3DInterpolationEnabled)
        {
          this->Show3DInterpolationResult(false);
          m_btApply3D->setEnabled(m_3DInterpolationEnabled);
        }
      }
      else
      {
        QWidget::setEnabled( false );
        m_chkShowPositionNodes->setEnabled(m_3DInterpolationEnabled);
      }
    }
    if (!m_3DInterpolationEnabled)
    {
       this->Show3DInterpolationResult(false);
       m_btApply3D->setEnabled(m_3DInterpolationEnabled);
    }
  }
  catch(...)
  {
    MITK_ERROR<<"Error with 3D surface interpolation!";
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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
        this->TranslateAndInterpolateChangedSlice(event, m_LastSNC);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
}

void QmitkSlicesInterpolator::OnSliceInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  // something (e.g. undo) changed the interpolation info, we should refresh our display
  this->UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::OnSurfaceInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  if(m_3DInterpolationEnabled)
  {
//    if (m_Watcher.isRunning())
//      m_Watcher.waitForFinished();
//    m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
//    m_Watcher.setFuture(m_Future);
  }
}

void QmitkSlicesInterpolator::SetCurrentContourListID()
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

      bool isVisible (workingNode->IsVisible(m_LastSNC->GetRenderer()));
      if (isVisible && !isInterpolationResult)
      {
        QWidget::setEnabled( true );

        //TODO Aufruf hier pruefen!
        mitk::Vector3D spacing = workingNode->GetData()->GetGeometry( m_LastSNC->GetTime()->GetPos() )->GetSpacing();
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

        m_SurfaceInterpolator->SetCurrentSegmentationInterpolationList(dynamic_cast<mitk::Image*>(workingNode->GetData()));

        if (m_3DInterpolationEnabled)
        {
//          if (m_Watcher.isRunning())
//            m_Watcher.waitForFinished();
//          m_Future = QtConcurrent::run(this, &QmitkSlicesInterpolator::Run3DInterpolation);
//          m_Watcher.setFuture(m_Future);
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
