/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-30 12:33:31 +0100 (Do, 30 Okt 2008) $
Version:   $Revision: 15606 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkSlicesInterpolator.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"

#include "mitkToolManager.h"
#include "mitkDataStorage.h"
#include "mitkDataTreeNodeFactory.h"
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
#include "mitkOperationEvent.h"
#include "mitkSegTool2D.h"
#include "mitkCoreObjectFactory.h"

#include <itkCommand.h>

#include <QCheckBox>
#include <QPushButton>
#include <QMenu>
#include <QCursor>

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

const std::map<QAction*, unsigned int> QmitkSlicesInterpolator::createActionToSliceDimension()
{
  std::map<QAction*, unsigned int> actionToSliceDimension;
  actionToSliceDimension[new QAction("Transversal (red window)", 0)] = 2;
  actionToSliceDimension[new QAction("Sagittal (green window)", 0)] = 0;
  actionToSliceDimension[new QAction("Coronal (blue window)", 0)] = 1;
  return actionToSliceDimension;
}

const std::map<QAction*, unsigned int> QmitkSlicesInterpolator::ACTION_TO_SLICEDIMENSION
  = QmitkSlicesInterpolator::createActionToSliceDimension();

QmitkSlicesInterpolator::QmitkSlicesInterpolator(QWidget* parent, const char* name)
:Q3VBox(parent, name),
 m_Interpolator( mitk::SegmentationInterpolationController::New() ),
 m_MultiWidget(NULL),
 m_ToolManager(NULL),
 m_Initialized(false),
 m_LastSliceDimension(2),
 m_LastSliceIndex(0),
 m_InterpolationEnabled(false)
{
  m_BtnAcceptInterpolation = new QPushButton("Accept", this);
  m_BtnAcceptInterpolation->setEnabled( false );
  connect( m_BtnAcceptInterpolation, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()) );

  m_BtnAcceptAllInterpolations = new QPushButton("Accept all interpolations", this);
  m_BtnAcceptAllInterpolations->setEnabled( false );
  connect( m_BtnAcceptAllInterpolations, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()) );

  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnInterpolationInfoChanged );
  InterpolationInfoChangedObserverTag = m_Interpolator->AddObserver( itk::ModifiedEvent(), command );

  // feedback node and its visualization properties
  m_FeedbackNode = mitk::DataTreeNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( m_FeedbackNode );

  m_FeedbackNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
  m_FeedbackNode->SetProperty( "outline binary", mitk::BoolProperty::New(true) );
  m_FeedbackNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 1.0, 1.0) );
  m_FeedbackNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(false) );
  m_FeedbackNode->SetProperty( "layer", mitk::IntProperty::New( 20 ) );
  m_FeedbackNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  m_FeedbackNode->SetProperty( "name", mitk::StringProperty::New("Interpolation feedback") );
  m_FeedbackNode->SetProperty( "opacity", mitk::FloatProperty::New(0.8) );
  m_FeedbackNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
}

void QmitkSlicesInterpolator::Initialize(mitk::ToolManager* toolManager, QmitkStdMultiWidget* multiWidget)
{
  if (m_Initialized)
  {
    std::cerr << "Repeated initialization of QmitkSlicesInterpolator is not yet implemented" << std::endl;

    // remove old observers
    if (m_ToolManager)
    {
      m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );
      m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified );
    }

    if (m_MultiWidget)
    {
      mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
      slicer->RemoveObserver( TSliceObserverTag );
      slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
      slicer->RemoveObserver( SSliceObserverTag );
      slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
      slicer->RemoveObserver( FSliceObserverTag );
    }

    return;
  }

  m_MultiWidget = multiWidget;
  m_ToolManager = toolManager;

  // set enabled only if a segmentation is selected
  mitk::DataTreeNode* node = m_ToolManager->GetWorkingData(0);
  Q3VBox::setEnabled( node != NULL );

  // react whenever the set of selected segmentation changes
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );
  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified );

  // connect to the steppers of the three multi widget widgets. after each change, call the interpolator
  mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
  m_TimeStep.resize(3);
  m_TimeStep[2] = slicer->GetTime()->GetPos();
  {
  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::MemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnTransversalTimeChanged );
  TTimeObserverTag = slicer->AddObserver( mitk::SliceNavigationController::GeometryTimeEvent(NULL, 0), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnTransversalSliceChanged );
  TSliceObserverTag = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  // connect to the steppers of the three multi widget widgets. after each change, call the interpolator
  slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
  m_TimeStep[0] = slicer->GetTime()->GetPos();
  {
  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::MemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSagittalTimeChanged );
  STimeObserverTag = slicer->AddObserver( mitk::SliceNavigationController::GeometryTimeEvent(NULL, 0), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnSagittalSliceChanged );
  SSliceObserverTag = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  // connect to the steppers of the three multi widget widgets. after each change, call the interpolator
  slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
  m_TimeStep[1] = slicer->GetTime()->GetPos();
  {
  itk::MemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::MemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnFrontalTimeChanged );
  FTimeObserverTag = slicer->AddObserver( mitk::SliceNavigationController::GeometryTimeEvent(NULL, 0), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::Pointer command = itk::ReceptorMemberCommand<QmitkSlicesInterpolator>::New();
  command->SetCallbackFunction( this, &QmitkSlicesInterpolator::OnFrontalSliceChanged );
  FSliceObserverTag = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  m_Initialized = true;
}

QmitkSlicesInterpolator::~QmitkSlicesInterpolator()
{
}

void QmitkSlicesInterpolator::OnToolManagerWorkingDataModified()
{
  OnInterpolationActivated( m_InterpolationEnabled ); // re-initialize if needed
}

void QmitkSlicesInterpolator::OnToolManagerReferenceDataModified()
{
  OnInterpolationActivated( m_InterpolationEnabled ); // re-initialize if needed
}


void QmitkSlicesInterpolator::OnTransversalTimeChanged(itk::Object* sender, const itk::EventObject& e)
{
  const mitk::SliceNavigationController::GeometryTimeEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent&>(e);
  m_TimeStep[2] = event.GetPos();

  if (m_LastSliceDimension == 2)
  {
    mitk::SliceNavigationController* snc = dynamic_cast<mitk::SliceNavigationController*>( sender );
    if (snc) snc->SendSlice(); // will trigger a new interpolation
    //mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget1->GetRenderWindow())->RequestUpdate(); // done in SendSlice
  }
}

void QmitkSlicesInterpolator::OnSagittalTimeChanged(itk::Object* sender, const itk::EventObject& e)
{
  const mitk::SliceNavigationController::GeometryTimeEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent&>(e);
  m_TimeStep[0] = event.GetPos();

  if (m_LastSliceDimension == 0)
  {
    mitk::SliceNavigationController* snc = dynamic_cast<mitk::SliceNavigationController*>( sender );
    if (snc) snc->SendSlice(); // will trigger a new interpolation
    //mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget2->GetRenderWindow())->RequestUpdate(); // done in SendSlice
  }
}

void QmitkSlicesInterpolator::OnFrontalTimeChanged(itk::Object* sender, const itk::EventObject& e)
{
  const mitk::SliceNavigationController::GeometryTimeEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent&>(e);
  m_TimeStep[1] = event.GetPos();

  if (m_LastSliceDimension == 1)
  {
    mitk::SliceNavigationController* snc = dynamic_cast<mitk::SliceNavigationController*>( sender );
    if (snc) snc->SendSlice(); // will trigger a new interpolation
    //mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())->RequestUpdate(); // done in SendSlice
  }
}


void QmitkSlicesInterpolator::OnTransversalSliceChanged(const itk::EventObject& e)
{
  if ( TranslateAndInterpolateChangedSlice( e, 2 ) )
  {
    mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget1->GetRenderWindow())->RequestUpdate();
  }
}

void QmitkSlicesInterpolator::OnSagittalSliceChanged(const itk::EventObject& e)
{
  if ( TranslateAndInterpolateChangedSlice( e, 0 ) )
  {
    mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget2->GetRenderWindow())->RequestUpdate();
  }
}

void QmitkSlicesInterpolator::OnFrontalSliceChanged(const itk::EventObject& e)
{
  if ( TranslateAndInterpolateChangedSlice( e, 1 ) )
  {
    mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())->RequestUpdate();
  }
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e, unsigned int windowID)
{
  if (!m_InterpolationEnabled) return false;

  try
  {
    const mitk::SliceNavigationController::GeometrySliceEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent&>(e);

    mitk::TimeSlicedGeometry* tsg = event.GetTimeSlicedGeometry();
    if (tsg)
    {
      mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(tsg->GetGeometry3D(m_TimeStep[windowID]));
      if (slicedGeometry)
      {
        mitk::PlaneGeometry* plane = dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry->GetGeometry2D( event.GetPos() ));
        Interpolate( plane, m_TimeStep[windowID] );
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

void QmitkSlicesInterpolator::Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep )
{
  mitk::DataTreeNode* node = m_ToolManager->GetWorkingData(0);
  if (node)
  {
    m_Segmentation = dynamic_cast<mitk::Image*>(node->GetData());
    if (m_Segmentation)
    {
      int clickedSliceDimension(-1);
      int clickedSliceIndex(-1);

      // calculate real slice position, i.e. slice of the image and not slice of the TimeSlicedGeometry
      mitk::SegTool2D::DetermineAffectedImageSlice( m_Segmentation, plane, clickedSliceDimension, clickedSliceIndex );
      mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( clickedSliceDimension, clickedSliceIndex, timeStep );

      m_FeedbackNode->SetData( interpolation );
      m_LastSliceDimension = clickedSliceDimension;
      m_LastSliceIndex = clickedSliceIndex;
    }
  }
}

void QmitkSlicesInterpolator::OnAcceptInterpolationClicked()
{
  if (m_Segmentation && m_FeedbackNode->GetData())
  {
    mitk::UndoStackItem::IncCurrObjectEventId();
    mitk::UndoStackItem::IncCurrGroupEventId();
    mitk::UndoStackItem::ExecuteIncrement(); // oh well designed undo stack, how do I love thee? let me count the ways... done

    mitk::OverwriteSliceImageFilter::Pointer slicewriter = mitk::OverwriteSliceImageFilter::New();
    slicewriter->SetInput( m_Segmentation );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( dynamic_cast<mitk::Image*>(m_FeedbackNode->GetData()) );
    slicewriter->SetSliceDimension( m_LastSliceDimension );
    slicewriter->SetSliceIndex( m_LastSliceIndex );
    slicewriter->SetTimeStep( m_TimeStep[m_LastSliceDimension] );
    slicewriter->Update();
    m_FeedbackNode->SetData(NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSlicesInterpolator::AcceptAllInterpolations(unsigned int windowID)
{
  // first creates a 3D diff image, then applies this diff to the segmentation
  if (m_Segmentation)
  {
    int sliceDimension(-1);
    int dummySliceIndex(-1);
    if (!GetSliceForWindowsID(windowID, sliceDimension, dummySliceIndex))
    {
      return; // cannot determine slice orientation
    }

    mitk::UndoStackItem::IncCurrObjectEventId();
    mitk::UndoStackItem::IncCurrGroupEventId();
    mitk::UndoStackItem::ExecuteIncrement(); // oh well designed undo stack, how do I love thee? let me count the ways... done

    // create a diff image for the undo operation
    mitk::Image::Pointer diffImage = mitk::Image::New();
    diffImage->Initialize( m_Segmentation );
    mitk::PixelType pixelType( typeid(short signed int) );
    diffImage->Initialize( pixelType, 3, m_Segmentation->GetDimensions() );

    memset( diffImage->GetData(), 0, (pixelType.GetBpe() >> 3) * diffImage->GetDimension(0) * diffImage->GetDimension(1) * diffImage->GetDimension(2) );
    // now the diff image is all 0

    unsigned int timeStep( m_TimeStep[windowID] );

    // a slicewriter to create the diff image
    mitk::OverwriteSliceImageFilter::Pointer diffslicewriter = mitk::OverwriteSliceImageFilter::New();
    diffslicewriter->SetCreateUndoInformation( false );
    diffslicewriter->SetInput( diffImage );
    diffslicewriter->SetSliceDimension( sliceDimension );
    diffslicewriter->SetTimeStep( timeStep );

    unsigned int totalChangedSlices(0);
    unsigned int zslices = m_Segmentation->GetDimension( sliceDimension );
    mitk::ProgressBar::GetInstance()->AddStepsToDo(zslices);
    for (unsigned int sliceIndex = 0; sliceIndex < zslices; ++sliceIndex)
    {
      mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( sliceDimension, sliceIndex, timeStep );
      if (interpolation.IsNotNull()) // we don't check if interpolation is necessary/sensible - but m_Interpolator does
      {
        diffslicewriter->SetSliceImage( interpolation );
        diffslicewriter->SetSliceIndex( sliceIndex );
        diffslicewriter->Update();
        ++totalChangedSlices;
      }
      mitk::ProgressBar::GetInstance()->Progress();
    }

    if (totalChangedSlices > 0)
    {
      // store undo stack items
      if ( true )
      {
        // create do/undo operations (we don't execute the doOp here, because it has already been executed during calculation of the diff image
        mitk::ApplyDiffImageOperation* doOp = new mitk::ApplyDiffImageOperation( mitk::OpTEST, m_Segmentation, diffImage, timeStep );
        mitk::ApplyDiffImageOperation* undoOp = new mitk::ApplyDiffImageOperation( mitk::OpTEST, m_Segmentation, diffImage, timeStep );
        undoOp->SetFactor( -1.0 );
        std::stringstream comment;
        comment << "Accept all interpolations (" << totalChangedSlices << ")";
        mitk::OperationEvent* undoStackItem = new mitk::OperationEvent( mitk::DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, comment.str() );
        mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );

        // acutally apply the changes here
        mitk::DiffImageApplier::GetInstanceForUndo()->ExecuteOperation( doOp );
      }
    }

    m_FeedbackNode->SetData(NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkSlicesInterpolator::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  std::map<QAction*, unsigned int>::const_iterator it;
  for(it = ACTION_TO_SLICEDIMENSION.begin(); it != ACTION_TO_SLICEDIMENSION.end(); it++)
    orientationPopup.addAction(it->first);

  connect( &orientationPopup, SIGNAL(triggered(QAction*)), this, SLOT(OnAcceptAllPopupActivated(QAction*)) );

  orientationPopup.exec( QCursor::pos() );
}

void QmitkSlicesInterpolator::OnAcceptAllPopupActivated(QAction* action)
{
  try
  {
    std::map<QAction*, unsigned int>::const_iterator iter = ACTION_TO_SLICEDIMENSION.find( action );
    if (iter != ACTION_TO_SLICEDIMENSION.end())
    {
      int windowID = iter->second;
      AcceptAllInterpolations( windowID );
    }
  }
  catch(...)
  {
    std::cerr << "Ill construction in " __FILE__ " l. " << __LINE__ << std::endl;
  }
}

void QmitkSlicesInterpolator::OnInterpolationActivated(bool on)
{
  m_InterpolationEnabled = on;

  try
  {
    if (on)
    {
      mitk::DataStorage::GetInstance()->Add( m_FeedbackNode );
    }
    else
    {
      mitk::DataStorage::GetInstance()->Remove( m_FeedbackNode );
    }
  }
  catch(...)
  {
    // don't care (double add/remove)
  }

  mitk::DataTreeNode* workingNode = m_ToolManager->GetWorkingData(0);
  mitk::DataTreeNode* referenceNode = m_ToolManager->GetReferenceData(0);
  Q3VBox::setEnabled( workingNode != NULL );

  m_BtnAcceptAllInterpolations->setEnabled( on );
  m_BtnAcceptInterpolation->setEnabled( on );
  m_FeedbackNode->SetVisibility( on );

  if (!on)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return;
  }

  if (workingNode)
  {
    m_FeedbackNode->ReplaceProperty( "color", workingNode->GetProperty("color") ); // use the same color as the original image (but outline - see constructor)
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

  UpdateVisibleSuggestion();
}

void QmitkSlicesInterpolator::EnableInterpolation(bool on)
{
  // only to be called from the outside world
  // just a redirection to OnInterpolationActivated
  OnInterpolationActivated(on);
}

void QmitkSlicesInterpolator::UpdateVisibleSuggestion()
{
  // determine which one is the current view, try to do an initial interpolation
  mitk::BaseRenderer* renderer = mitk::GlobalInteraction::GetInstance()->GetFocus();
  if (renderer && renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
  {
    const mitk::TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const mitk::TimeSlicedGeometry*>( renderer->GetWorldGeometry() );
    if (timeSlicedGeometry)
    {
      mitk::SliceNavigationController::GeometrySliceEvent event( const_cast<mitk::TimeSlicedGeometry*>(timeSlicedGeometry), renderer->GetSlice() );

      std::string s;
      if ( renderer->GetCurrentWorldGeometry2DNode() && renderer->GetCurrentWorldGeometry2DNode()->GetName(s) )
      {
             if (s == "widget1Plane")
        {
          TranslateAndInterpolateChangedSlice( event, 2 );
        }
        else if (s == "widget2Plane")
        {
          TranslateAndInterpolateChangedSlice( event, 0 );
        }
        else if (s == "widget3Plane")
        {
          TranslateAndInterpolateChangedSlice( event, 1 );
        }
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

bool QmitkSlicesInterpolator::GetSliceForWindowsID(unsigned windowID, int& sliceDimension, int& sliceIndex)
{
  mitk::BaseRenderer* renderer(NULL);

  // find sliceDimension for windowID:
  //   windowID 2: transversal window = renderWindow1
  //   windowID 1: frontal window = renderWindow3
  //   windowID 0: sagittal window = renderWindow2
  switch (windowID)
  {
  case 2:
  default:
    renderer = m_MultiWidget->mitkWidget1->GetRenderer();
    break;
  case 1:
    renderer = m_MultiWidget->mitkWidget3->GetRenderer();
    break;
  case 0:
    renderer = m_MultiWidget->mitkWidget2->GetRenderer();
    break;
  }

  if ( m_Segmentation && renderer && renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
  {
    const mitk::TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const mitk::TimeSlicedGeometry*>( renderer->GetWorldGeometry() );
    if (timeSlicedGeometry)
    {
      mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(timeSlicedGeometry->GetGeometry3D(m_TimeStep[windowID]));
      if (slicedGeometry)
      {
        mitk::PlaneGeometry* plane = dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry->GetGeometry2D( renderer->GetSlice() ));
        Interpolate( plane, m_TimeStep[windowID] );
        return mitk::SegTool2D::DetermineAffectedImageSlice( m_Segmentation, plane, sliceDimension, sliceIndex );
      }
    }
  }

  return false;
}

