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

#include <itkCommand.h>

#include <QCheckBox>
#include <QPushButton>
#include <QMenu>
#include <QCursor>
#include <QHBoxLayout>
#include <QMessageBox>


#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

const std::map<QAction*, unsigned int> QmitkSlicesInterpolator::createActionToSliceDimension()
{
  std::map<QAction*, unsigned int> actionToSliceDimension;
  actionToSliceDimension[new QAction("Transversal (red window)", 0)] = 2;
  actionToSliceDimension[new QAction("Sagittal (green window)", 0)] = 0;
  actionToSliceDimension[new QAction("Coronal (blue window)", 0)] = 1;
  return actionToSliceDimension;
}


QmitkSlicesInterpolator::QmitkSlicesInterpolator(QWidget* parent, const char*  /*name*/)
:QWidget(parent),
ACTION_TO_SLICEDIMENSION( createActionToSliceDimension() ),
m_Interpolator( mitk::SegmentationInterpolationController::New() ),
m_MultiWidget(NULL),
m_ToolManager(NULL),
m_Initialized(false),
m_LastSliceDimension(2),
m_LastSliceIndex(0),
m_2DInterpolationEnabled(false),
m_3DInterpolationEnabled(false)
{

  m_SurfaceInterpolator = mitk::SurfaceInterpolationController::GetInstance();
  //Changes due to intergration of 3D interpolation
  QHBoxLayout* layout = new QHBoxLayout(this);

  QGridLayout* grid = new QGridLayout(this);

  m_RBtnEnable2DInterpolation = new QRadioButton("2D",this);
  connect(m_RBtnEnable2DInterpolation, SIGNAL(toggled(bool)), this, SLOT(On2DInterpolationEnabled(bool)));
  grid->addWidget(m_RBtnEnable2DInterpolation,1,0);
  //connect

  m_RBtnEnable3DInterpolation = new QRadioButton("3D",this);
  connect(m_RBtnEnable3DInterpolation, SIGNAL(toggled(bool)), this, SLOT(On3DInterpolationEnabled(bool)));
  connect(m_RBtnEnable3DInterpolation, SIGNAL(toggled(bool)), this, SIGNAL(SignalRememberContourPositions(bool)));
  m_RBtnEnable3DInterpolation->setChecked(true);
  grid->addWidget(m_RBtnEnable3DInterpolation,0,0);
  //connect

  m_CbHideMarkers = new QCheckBox("Show Position-Nodes", this);
  m_CbHideMarkers->setChecked(true);
  connect(m_CbHideMarkers, SIGNAL(stateChanged(int)), this, SLOT(OnHideMarkers(int)));
  grid->addWidget(m_CbHideMarkers,0,2);
  //connect

  m_BtnAccept3DInterpolation = new QPushButton("Accept...", this);
  m_BtnAccept3DInterpolation->setEnabled(true); 
  grid->addWidget(m_BtnAccept3DInterpolation, 0,1);

  grid->addWidget(m_RBtnEnable2DInterpolation,0,0);
  
  m_BtnAcceptInterpolation = new QPushButton("Accept...", this);
  m_BtnAcceptInterpolation->setEnabled( false );
  connect( m_BtnAcceptInterpolation, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()) );
  //layout->addWidget( m_BtnAcceptInterpolation, 1 );
  grid->addWidget(m_BtnAcceptInterpolation,1,1);
  
  m_BtnAcceptAllInterpolations = new QPushButton("... for all slices", this);
  m_BtnAcceptAllInterpolations->setEnabled( false );
  connect( m_BtnAcceptAllInterpolations, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()) );
  //layout->addWidget( m_BtnAcceptAllInterpolations );
  grid->addWidget(m_BtnAcceptAllInterpolations,1,2);

  m_RBtnDisableInterpolation = new QRadioButton("Disable Interpolation", this);
  m_RBtnDisableInterpolation->setEnabled(true);
  grid->addWidget(m_RBtnDisableInterpolation, 2,0,1,2);

  m_GroupBoxEnableExclusiveInterpolationMode = new QGroupBox("Interpolation", this);
  m_GroupBoxEnableExclusiveInterpolationMode->setLayout(grid);
  layout->addWidget(m_GroupBoxEnableExclusiveInterpolationMode);
  
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
  //m_FeedbackNode->SetProperty( "outline binary", mitk::BoolProperty::New(true) );
  m_InterpolatedSurfaceNode->SetProperty( "color", mitk::ColorProperty::New(255.0, 255.0, 0.0) );
  //m_FeedbackNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(false) );
  //m_FeedbackNode->SetProperty( "layer", mitk::IntProperty::New( 20 ) );
  //m_FeedbackNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  m_InterpolatedSurfaceNode->SetProperty( "name", mitk::StringProperty::New("Surface Interpolation feedback") );
  m_InterpolatedSurfaceNode->SetProperty( "opacity", mitk::FloatProperty::New(0.5) );
  m_InterpolatedSurfaceNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
  
  QWidget::setContentsMargins(0, 0, 0, 0);
  if ( QWidget::layout() != NULL )
  {
    QWidget::layout()->setContentsMargins(0, 0, 0, 0);
  }
}

void QmitkSlicesInterpolator::SetDataStorage( mitk::DataStorage& storage )
{
  m_DataStorage = &storage;
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


void QmitkSlicesInterpolator::Initialize(mitk::ToolManager* toolManager, QmitkStdMultiWidget* multiWidget)
{
  if (m_Initialized)
  {
    // remove old observers
    if (m_ToolManager)
    {
      m_ToolManager->WorkingDataChanged 
      -= mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );
      
      m_ToolManager->ReferenceDataChanged 
      -= mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified );
    }
    
    if (m_MultiWidget)
    {
      disconnect( m_MultiWidget, SIGNAL(destroyed(QObject*)), this, SLOT(OnMultiWidgetDeleted(QObject*)) );
      mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
      slicer->RemoveObserver( TSliceObserverTag );
      slicer->RemoveObserver( TTimeObserverTag );
      slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
      slicer->RemoveObserver( SSliceObserverTag );
      slicer->RemoveObserver( STimeObserverTag );
      slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
      slicer->RemoveObserver( FSliceObserverTag );
      slicer->RemoveObserver( FTimeObserverTag );
    }
    
    //return;
  }
  
  m_MultiWidget = multiWidget;
  
  connect( m_MultiWidget, SIGNAL(destroyed(QObject*)), this, SLOT(OnMultiWidgetDeleted(QObject*)) );
  
  m_ToolManager = toolManager;
  
  if (m_ToolManager)
  {
    // set enabled only if a segmentation is selected
    mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
    QWidget::setEnabled( node != NULL );
    
    // react whenever the set of selected segmentation changes
    m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerWorkingDataModified );
    m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkSlicesInterpolator>( this, &QmitkSlicesInterpolator::OnToolManagerReferenceDataModified );
    
    // connect to the steppers of the three multi widget widgets. after each change, call the interpolator
    if (m_MultiWidget)
    {
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
    }
  }
  
  m_Initialized = true;
}

QmitkSlicesInterpolator::~QmitkSlicesInterpolator()
{ 
  if (m_MultiWidget)
  {
    mitk::SliceNavigationController* slicer;
    if(m_MultiWidget->mitkWidget1 != NULL)
    {
      slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
      slicer->RemoveObserver( TSliceObserverTag );
      slicer->RemoveObserver( TTimeObserverTag );
    }
    if(m_MultiWidget->mitkWidget2 != NULL)
    {
      slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
      slicer->RemoveObserver( SSliceObserverTag );
      slicer->RemoveObserver( STimeObserverTag );
    }
    if(m_MultiWidget->mitkWidget3 != NULL)
    {
      slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
      slicer->RemoveObserver( FSliceObserverTag );
      slicer->RemoveObserver( FTimeObserverTag );
    }
  }
}

void QmitkSlicesInterpolator::On2DInterpolationEnabled(bool status)
{

    OnInterpolationActivated(status);

}

void QmitkSlicesInterpolator::On3DInterpolationEnabled(bool status)
{

  On3DInterpolationActivated(status);
}

void QmitkSlicesInterpolator::OnInterpolationDisabled(bool status)
{
  OnInterpolationActivated(status);
  On3DInterpolationActivated(status);
}

void QmitkSlicesInterpolator::OnHideMarkers(int state)
{
  bool hide (!m_CbHideMarkers->isChecked());

  mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("isContourMarker"
    , mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
  {
    m_DataStorage->Remove(it->Value());
    it->Value()->SetProperty("helper object", mitk::BoolProperty::New(hide));
    m_DataStorage->Add( it->Value(), m_ToolManager->GetWorkingData(0));
  }

   mitk::DataStorage::SetOfObjects::ConstPointer contours3D = m_DataStorage->GetSubset(mitk::NodePredicateProperty::New("3DContourContainer"
    , mitk::BoolProperty::New(true)));

  if(!hide)
  {
    if(contours3D->empty())
    {
      mitk::DataNode::Pointer contourNode = mitk::DataNode::New();
      contourNode->SetData(m_SurfaceInterpolator->GetContoursAsSurface());
      contourNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 0.0, 0.0) );
      contourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
      contourNode->SetProperty( "name", mitk::StringProperty::New("Drawn Contours") );
      contourNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
      contourNode->SetProperty("material.wireframeLineWidth", mitk::FloatProperty::New(2.0f));
      contourNode->SetProperty("3DContourContainer", mitk::BoolProperty::New(true));
      m_DataStorage->Add(contourNode, m_ToolManager->GetWorkingData(0));
    }
    else
    {
      contours3D->at(0)->SetVisibility(true);
    }    
  }
  else if(!contours3D->empty())
  {
    contours3D->at(0)->SetVisibility(false);
  }

  //Perhaps think of a way to just hide markers instead of removing them
  //m_CbHideMarkers->setEnabled(false);
}

void QmitkSlicesInterpolator::OnToolManagerWorkingDataModified()
{
  OnInterpolationActivated( m_2DInterpolationEnabled ); // re-initialize if needed
  On3DInterpolationActivated( m_3DInterpolationEnabled);
}

void QmitkSlicesInterpolator::OnToolManagerReferenceDataModified()
{
  OnInterpolationActivated( m_2DInterpolationEnabled ); // re-initialize if needed
  On3DInterpolationActivated( m_3DInterpolationEnabled);
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
    if (m_MultiWidget)
    {
      mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget1->GetRenderWindow())->RequestUpdate();
    }
  }
}

void QmitkSlicesInterpolator::OnSagittalSliceChanged(const itk::EventObject& e)
{
  if ( TranslateAndInterpolateChangedSlice( e, 0 ) )
  {
    if (m_MultiWidget)
    {
      mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget2->GetRenderWindow())->RequestUpdate();
    }
  }
}

void QmitkSlicesInterpolator::OnFrontalSliceChanged(const itk::EventObject& e)
{
  if ( TranslateAndInterpolateChangedSlice( e, 1 ) )
  {
    if (m_MultiWidget)
    {
      mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())->RequestUpdate();
    }
  }
}

bool QmitkSlicesInterpolator::TranslateAndInterpolateChangedSlice(const itk::EventObject& e, unsigned int windowID)
{
  if (!m_2DInterpolationEnabled) return false;
  
  try
  {
    const mitk::SliceNavigationController::GeometrySliceEvent& event = dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent&>(e);
    
    mitk::TimeSlicedGeometry* tsg = event.GetTimeSlicedGeometry();
    if (tsg && m_TimeStep.size() > windowID)
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
        mitk::Image::Pointer interpolation = m_Interpolator->Interpolate( clickedSliceDimension, clickedSliceIndex, timeStep );
        
        m_FeedbackNode->SetData( interpolation );
        m_LastSliceDimension = clickedSliceDimension;
        m_LastSliceIndex = clickedSliceIndex;
      }
    }
  }
}

void QmitkSlicesInterpolator::InterpolateSurface()
{
  m_InterpolatedSurfaceNode->SetData(m_SurfaceInterpolator->Interpolate());
  OnHideMarkers(m_CbHideMarkers->checkState());
  if (m_MultiWidget)
  {
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
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
    
    //making interpolation separately undoable
    mitk::UndoStackItem::IncCurrObjectEventId();
    mitk::UndoStackItem::IncCurrGroupEventId();
    mitk::UndoStackItem::ExecuteIncrement(); 
    
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
void QmitkSlicesInterpolator::FinishInterpolation(int windowID)
{
  //this redirect is for calling from outside
  
  if (windowID < 0)
    OnAcceptAllInterpolationsClicked();
  else
    AcceptAllInterpolations( (unsigned int)windowID );
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
      //else
      //{
      //  m_DataStorage->Remove( m_FeedbackNode );
      //}
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
      //m_FeedbackNode->ReplaceProperty( "color", workingNode->GetProperty("color") ); // use the same color as the original image (but outline - see constructor)
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

void QmitkSlicesInterpolator::On3DInterpolationActivated(bool on)
{
  m_3DInterpolationEnabled = on;
  
  try
  {
    if ( m_DataStorage.IsNotNull() )
    {
      if (on && !m_DataStorage->Exists(m_InterpolatedSurfaceNode))
      {
        m_DataStorage->Add( m_InterpolatedSurfaceNode );
      }
      //else
      //{
      //  m_DataStorage->Remove( m_InterpolatedSurfaceNode );
      //}
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
    
    m_BtnAccept3DInterpolation->setEnabled( on );
    m_InterpolatedSurfaceNode->SetVisibility( on );
    m_CbHideMarkers->setEnabled(on);
    
    if (!on)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return;
    }
    
    if (workingNode)
    {

      //Evtl wo anders hin...!!!
      mitk::Vector3D spacing = workingNode->GetData()->GetGeometry( m_MultiWidget->GetRenderWindow3()->GetRenderer()->GetTimeStep() )->GetSpacing();
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

      m_SurfaceInterpolator->SetWorkingImage(dynamic_cast<mitk::Image*>(workingNode->GetData()));
      m_SurfaceInterpolator->SetMaxSpacing(maxSpacing);
      m_SurfaceInterpolator->SetMinSpacing(minSpacing);

      //if (m_SurfaceInterpolator->DataSetHasChanged())
      //InterpolateSurface();
      //m_InterpolatedSurfaceNode->ReplaceProperty( "color", workingNode->GetProperty("color") ); // use the same color as the original image (but outline - see constructor)
      //mitk::Image* segmentation = dynamic_cast<mitk::Image*>(workingNode->GetData());
      //if (segmentation)
      //{
      //  m_Interpolator->SetSegmentationVolume( segmentation );
      //  
      //  if (referenceNode)
      //  {
      //    mitk::Image* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
      //    m_Interpolator->SetReferenceVolume( referenceImage ); // may be NULL
      //  }
      //}
    }
  }
  
  //UpdateVisibleSuggestion();
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
  if (m_2DInterpolationEnabled)
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
    InterpolateSurface();
}

bool QmitkSlicesInterpolator::GetSliceForWindowsID(unsigned windowID, int& sliceDimension, int& sliceIndex)
{
  mitk::BaseRenderer* renderer(NULL);
  
  // find sliceDimension for windowID:
  //   windowID 2: transversal window = renderWindow1
  //   windowID 1: frontal window = renderWindow3
  //   windowID 0: sagittal window = renderWindow2
  if ( m_MultiWidget )
  {
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

void QmitkSlicesInterpolator::OnMultiWidgetDeleted(QObject*)
{
  if (m_MultiWidget)
  {
    m_MultiWidget = NULL;
  }
}

