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

#include "QmitkSliceBasedInterpolator.h"

#include <mitkColorProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkProgressBar.h>
#include <mitkOperationEvent.h>
#include <mitkInteractionConst.h>
#include <mitkApplyDiffImageOperation.h>
#include <mitkDiffImageApplier.h>
#include <mitkDiffSliceOperationApplier.h>
#include <mitkUndoController.h>
#include <mitkSegTool2D.h>
#include <mitkSliceNavigationController.h>
#include <mitkVtkImageOverwrite.h>
#include <mitkExtractSliceFilter.h>
#include <mitkLabelSetImage.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToContourModelSetFilter.h>
#include <mitkContourUtils.h>
#include <mitkToolManagerProvider.h>

#include "QmitkStdMultiWidget.h"

#include <itkCommand.h>

#include <QMenu>
#include <QCursor>
#include <QMessageBox>
#include <QApplication>

QmitkSliceBasedInterpolator::QmitkSliceBasedInterpolator(QWidget* parent, const char*  /*name*/) : QWidget(parent),
m_SliceInterpolatorController( mitk::SegmentationInterpolationController::New() ),
m_ToolManager(NULL),
m_DataStorage(NULL),
m_Initialized(false),
m_Activated(false),
m_LastSNC(0),
m_LastSliceIndex(0)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  connect(m_Controls.m_gbControls, SIGNAL(toggled(bool)), this, SLOT(OnActivateWidget(bool)));
  connect(m_Controls.m_btApplyForCurrentSlice, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_Controls.m_btApplyForAllSlices, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()));
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );

  itk::ReceptorMemberCommand<QmitkSliceBasedInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSliceBasedInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSliceBasedInterpolator::OnSliceInterpolationInfoChanged );
  m_InterpolationInfoChangedObserverTag = m_SliceInterpolatorController->AddObserver( itk::ModifiedEvent(), command );

  m_FeedbackContour = mitk::ContourModelSet::New();
  m_FeedbackContourNode = mitk::DataNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetName("slice-based interpolation preview");
  m_FeedbackContourNode->SetProperty("visible", mitk::BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("layer", mitk::IntProperty::New(1000));
  m_FeedbackContourNode->SetProperty("contour.project-onto-plane", mitk::BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("contour.width", mitk::FloatProperty::New(2.5));

  this->setEnabled(false);
}

const QmitkSliceBasedInterpolator::ActionToSliceDimensionMapType QmitkSliceBasedInterpolator::CreateActionToSliceDimension()
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

void QmitkSliceBasedInterpolator::Initialize(const QList<mitk::SliceNavigationController *> &controllers, mitk::DataStorage* storage)
{
  Q_ASSERT(!controllers.empty());

  Q_ASSERT(storage);

  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  m_DataStorage = storage;

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  // react whenever the set of selected segmentation changes
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSliceBasedInterpolator>( this, &QmitkSliceBasedInterpolator::OnToolManagerWorkingDataModified );

  // connect to the slice navigation controller. after each change, call the interpolator
  foreach(mitk::SliceNavigationController* slicer, controllers)
  {
    //Has to be initialized
    m_LastSNC = slicer;

    m_TimeStep.insert(slicer, slicer->GetTime()->GetPos());

    itk::MemberCommand<QmitkSliceBasedInterpolator>::Pointer deleteCommand = itk::MemberCommand<QmitkSliceBasedInterpolator>::New();
    deleteCommand->SetCallbackFunction( this, &QmitkSliceBasedInterpolator::OnSliceNavigationControllerDeleted);
    m_ControllerToDeleteObserverTag.insert(slicer, slicer->AddObserver(itk::DeleteEvent(), deleteCommand));

    itk::MemberCommand<QmitkSliceBasedInterpolator>::Pointer timeChangedCommand = itk::MemberCommand<QmitkSliceBasedInterpolator>::New();
    timeChangedCommand->SetCallbackFunction( this, &QmitkSliceBasedInterpolator::OnTimeChanged);
    m_ControllerToTimeObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::TimeSlicedGeometryEvent(NULL,0), timeChangedCommand));

    itk::MemberCommand<QmitkSliceBasedInterpolator>::Pointer sliceChangedCommand = itk::MemberCommand<QmitkSliceBasedInterpolator>::New();
    sliceChangedCommand->SetCallbackFunction( this, &QmitkSliceBasedInterpolator::OnSliceChanged);
    m_ControllerToSliceObserverTag.insert(slicer, slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL,0), sliceChangedCommand));
  }
  m_ActionToSliceDimensionMap = this->CreateActionToSliceDimension();

  m_Initialized = true;
}

void QmitkSliceBasedInterpolator::Uninitialize()
{
  if (m_ToolManager.IsNotNull())
  {
    m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSliceBasedInterpolator>(this, &QmitkSliceBasedInterpolator::OnToolManagerWorkingDataModified);
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

QmitkSliceBasedInterpolator::~QmitkSliceBasedInterpolator()
{
  if (m_Initialized)
  {
    // remove old observers
    this->Uninitialize();
  }

  // remove observer
  m_SliceInterpolatorController->RemoveObserver( m_InterpolationInfoChangedObserverTag );
}

void QmitkSliceBasedInterpolator::OnToolManagerWorkingDataModified()
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
}

void QmitkSliceBasedInterpolator::OnTimeChanged(itk::Object* sender, const itk::EventObject& e)
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

void QmitkSliceBasedInterpolator::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  if (this->m_Activated && m_WorkingImage.IsNotNull())
  {
    //Check whether we really have a GeometrySliceEvent
    if (!dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent*>(&e))
      return;

    mitk::SliceNavigationController* slicer = dynamic_cast<mitk::SliceNavigationController*>(sender);
    if (slicer)
    {
      this->TranslateAndInterpolateChangedSlice(e, slicer);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      //  slicer->GetRenderer()->RequestUpdate();
    }
  }
}

void QmitkSliceBasedInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e, mitk::SliceNavigationController* slicer)
{
  if (this->m_Activated && m_WorkingImage.IsNotNull())
  {
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
          this->Interpolate(plane, m_TimeStep[slicer], slicer);
        }
      }
    }
  }
}

void QmitkSliceBasedInterpolator::Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep, mitk::SliceNavigationController* slicer )
{
  int clickedSliceDimension(-1);
  int clickedSliceIndex(-1);

  // calculate real slice position, i.e. slice of the image and not slice of the TimeSlicedGeometry
  // see if timestep is needed here
  mitk::SegTool2D::DetermineAffectedImageSlice( m_WorkingImage, plane, clickedSliceDimension, clickedSliceIndex );

  mitk::Image::Pointer interpolation = m_SliceInterpolatorController->Interpolate( clickedSliceDimension, clickedSliceIndex, plane, timeStep );

  if (interpolation.IsNotNull())
  {
    mitk::ImageToContourModelSetFilter::Pointer contourExtractor = mitk::ImageToContourModelSetFilter::New();
    contourExtractor->SetInput(interpolation);

    try
    {
      contourExtractor->Update();
    }
    catch ( itk::ExceptionObject& e )
    {
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      return;
    }

    m_FeedbackContour = contourExtractor->GetOutput();
    m_FeedbackContour->DisconnectPipeline();

    m_FeedbackContourNode->SetData( m_FeedbackContour );

    const mitk::Color& color = m_WorkingImage->GetActiveLabelColor();
    m_FeedbackContourNode->SetProperty("contour.color", mitk::ColorProperty::New(color));
  }
  else
  {
    m_FeedbackContour->Clear();
  }

  m_LastSNC = slicer;
  m_LastSliceIndex = clickedSliceIndex;
}

mitk::Image::Pointer QmitkSliceBasedInterpolator::GetWorkingSlice(const mitk::PlaneGeometry* planeGeometry)
{
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
  m_undoOperation = new mitk::DiffSliceOperation(m_WorkingImage, extractor->GetVtkOutput(), slice->GetGeometry(), timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));

  slice->DisconnectPipeline();

  return slice;
}

void QmitkSliceBasedInterpolator::OnActivateWidget(bool enabled)
{
  if (!m_Initialized) return;

  m_Activated = enabled;

  if ( m_ToolManager.IsNotNull() )
  {
    unsigned int numberOfExistingTools = m_ToolManager->GetTools().size();
    for(unsigned int i = 0; i < numberOfExistingTools; i++)
    {
      mitk::SegTool2D* tool = dynamic_cast<mitk::SegTool2D*>(m_ToolManager->GetToolById(i));
      if (tool) tool->SetEnable2DInterpolation( m_Activated );
    }
  }

  if (m_Activated)
  {
    if (!m_DataStorage->Exists(m_FeedbackContourNode))
    {
      m_DataStorage->Add( m_FeedbackContourNode );
    }
    m_SliceInterpolatorController->SetWorkingImage( m_WorkingImage );
    this->UpdateVisibleSuggestion();
  }
  else
  {
    if (m_DataStorage->Exists(m_FeedbackContourNode))
    {
      m_DataStorage->Remove( m_FeedbackContourNode );
    }

    mitk::UndoController::GetCurrentUndoModel()->Clear();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedInterpolator::OnAcceptInterpolationClicked()
{
  if (m_WorkingImage.IsNotNull() && m_FeedbackContourNode->GetData())
  {
    const mitk::PlaneGeometry* planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
    if (!planeGeometry) return;
    mitk::Image::Pointer sliceImage = this->GetWorkingSlice(planeGeometry);
    if (sliceImage.IsNull()) return;

    unsigned int timeStep = m_LastSNC->GetTime()->GetPos();

    int numberOfContours = m_FeedbackContour->GetSize();
    for (int i=0; i<numberOfContours; i++)
    {
 //     mitk::ContourModel::Pointer projectedContour = mitk::ContourModel::New();
//      const mitk::Geometry3D* sliceGeometry = sliceImage->GetGeometry(timeStep);
//      mitk::ContourUtils::ProjectContourTo2DSlice( sliceGeometry, m_FeedbackContour->GetContourModelAt(i), projectedContour );
      mitk::ContourUtils::FillContourInSlice( m_FeedbackContour->GetContourModelAt(i), sliceImage, m_WorkingImage->GetActiveLabelIndex() );
    }

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

void QmitkSliceBasedInterpolator::AcceptAllInterpolations(mitk::SliceNavigationController* slicer)
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
      mitk::ImageToContourModelSetFilter::Pointer contourExtractor = mitk::ImageToContourModelSetFilter::New();
      contourExtractor->SetInput(interpolation);

      try
      {
        contourExtractor->Update();
      }
      catch ( itk::ExceptionObject& e )
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        return;
      }

      m_FeedbackContour = contourExtractor->GetOutput();
      m_FeedbackContour->DisconnectPipeline();

      m_FeedbackContourNode->SetData( m_FeedbackContour );

      mitk::Image::Pointer sliceImage = this->GetWorkingSlice(reslicePlane);
      if (sliceImage.IsNull()) return;

      int numberOfContours = m_FeedbackContour->GetSize();
      for (int i=0; i<numberOfContours; i++)
      {
//         mitk::ContourModel::Pointer projectedContour = mitk::ContourModel::New();
//         const mitk::Geometry3D* sliceGeometry = sliceImage->GetGeometry();
//         mitk::ContourUtils::ProjectContourTo2DSlice( sliceGeometry, m_FeedbackContour->GetContourModelAt(i), projectedContour );
         mitk::ContourUtils::FillContourInSlice( m_FeedbackContour->GetContourModelAt(i), sliceImage, m_WorkingImage->GetActiveLabelIndex() );
      }

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

void QmitkSliceBasedInterpolator::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  std::map<QAction*, mitk::SliceNavigationController*>::const_iterator it;
  for(it = m_ActionToSliceDimensionMap.begin(); it != m_ActionToSliceDimensionMap.end(); it++)
    orientationPopup.addAction(it->first);

  connect( &orientationPopup, SIGNAL(triggered(QAction*)), this, SLOT(OnAcceptAllPopupActivated(QAction*)) );

  orientationPopup.exec( QCursor::pos() );
}

void QmitkSliceBasedInterpolator::OnShowInformation(bool on)
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkSliceBasedInterpolator::OnAcceptAllPopupActivated(QAction* action)
{
  ActionToSliceDimensionMapType::const_iterator iter = m_ActionToSliceDimensionMap.find( action );
  if (iter != m_ActionToSliceDimensionMap.end())
  {
    mitk::SliceNavigationController* slicer = iter->second;
    this->AcceptAllInterpolations( slicer );
  }
}

void QmitkSliceBasedInterpolator::UpdateVisibleSuggestion()
{
  if (this->m_Activated && m_LastSNC)
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

void QmitkSliceBasedInterpolator::OnSliceInterpolationInfoChanged(const itk::EventObject& /*e*/)
{
  // something (e.g. undo) changed the interpolation info, we should refresh our display
  this->UpdateVisibleSuggestion();
}

void QmitkSliceBasedInterpolator::OnSliceNavigationControllerDeleted(const itk::Object *sender, const itk::EventObject& /*e*/)
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

void QmitkSliceBasedInterpolator::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkSliceBasedInterpolator::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkSliceBasedInterpolator::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}
