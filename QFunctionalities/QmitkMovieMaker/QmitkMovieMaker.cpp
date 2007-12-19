/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkMovieMaker.h"
#include "QmitkMovieMakerControls.h"
#include "QmitkStepperAdapter.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"

#include "mitkVtkPropRenderer.h"
#include "mitkGlobalInteraction.h"


#include "icon.xpm"

#include <iostream>

#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <qaction.h>
#include <qfiledialog.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qcombobox.h>



#include "qapplication.h"

#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"

#include <vtkActor.h>
#include "vtkMitkRenderProp.h"


#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkRenderWindowInteractor.h"

#include "vtkSphereSource.h"
#include "vtkDataSetMapper.h"
#include "vtkElevationFilter.h"

QmitkMovieMaker::QmitkMovieMaker( QObject *parent, const char *name,
  QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it )

: QmitkFunctionality(parent, name, it),
  m_Controls(NULL),
  m_MultiWidget(mitkStdMultiWidget),
  m_StepperAdapter(NULL), 
  m_FocusManagerCallback(0),
  m_Looping(true), 
  m_Direction(0),
  m_Aspect(0)
{

  parentWidget = parent;

  this->SetAvailability(true);
  m_Timer = new QTimer(this);
  m_Time = new QTime();

  m_FocusManagerCallback = MemberCommand::New();
  m_FocusManagerCallback->SetCallbackFunction( this, &QmitkMovieMaker::FocusChange );

  m_movieGenerator = mitk::MovieGenerator::New();

  if ( m_movieGenerator.IsNull() )
  {
    std::cerr << "Either mitk::MovieGenerator is not implemented for your";
    std::cerr << " platform or an error occurred during";
    std::cerr << " mitk::MovieGenerator::New()" << std::endl;
  }
}

QmitkMovieMaker::~QmitkMovieMaker()
{
  delete m_StepperAdapter;
  delete m_Timer;
  delete m_Time;
  //delete m_RecordingRenderer;
}

mitk::BaseController* QmitkMovieMaker::GetSpatialController()
{
  mitk::BaseRenderer* focusedRenderer =
    mitk::GlobalInteraction::GetInstance()->GetFocus();

  if ( mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget1->GetRenderWindow() ) == focusedRenderer )
  {
    return m_MultiWidget->mitkWidget1->GetController();
  }
  else if ( mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget2->GetRenderWindow() ) == focusedRenderer )
  {
    return m_MultiWidget->mitkWidget2->GetController();
  }
  else if ( mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow() ) == focusedRenderer )
  {
    return m_MultiWidget->mitkWidget3->GetController();
  }
  else if ( mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget4->GetRenderWindow() ) == focusedRenderer )
  {
    return m_MultiWidget->mitkWidget4->GetController();
  }

  return m_MultiWidget->mitkWidget4->GetController();
}

mitk::BaseController* QmitkMovieMaker::GetTemporalController()
{
  return m_MultiWidget->GetTimeNavigationController();
}

QWidget* QmitkMovieMaker::CreateMainWidget(QWidget *)
{
  return NULL;
}

QWidget* QmitkMovieMaker::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkMovieMakerControls(parent);
  
    m_StepperAdapter = new QmitkStepperAdapter(
      (QObject*) m_Controls->slidAngle, this->GetSpatialController()->GetSlice(),
      "AngleStepperToMovieMakerFunctionality"
    );

    // Initialize "Selected Window" combo box
    const mitk::RenderingManager::RenderWindowVector rwv =
      mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

    mitk::RenderingManager::RenderWindowVector::const_iterator iter;
    for (iter = rwv.begin(); iter != rwv.end(); ++iter)
    {
      m_Controls->cmbSelectedStepperWindow->insertItem(mitk::BaseRenderer::GetInstance((*iter))->GetName(), -1);
      m_Controls->cmbSelectedRecordingWindow->insertItem(mitk::BaseRenderer::GetInstance((*iter))->GetName(), -1);
    }
  }

  m_Controls->btnPause->setHidden(true);
  if(m_movieGenerator.IsNull())
      m_Controls->btnMovie->setEnabled( false );

  return m_Controls;
}

void QmitkMovieMaker::CreateConnections()
{
  if ( m_Controls )
  {
    // start / pause / stop playing
    connect( (QObject*) m_Controls, SIGNAL(StartPlaying()),
      this, SLOT(StartPlaying()) );

    connect( (QObject*) m_Controls, SIGNAL(PausePlaying()),
      this, SLOT(PausePlaying()) );

    connect( (QObject*) m_Controls, SIGNAL(StopPlaying()),
      this, SLOT(StopPlaying()) );

    // radio button group: forward, backward, ping-pong
    connect( (QObject*) m_Controls, SIGNAL(SwitchDirection(int)),
      this, SLOT(SetDirection(int)) );

    // radio button group: spatial, temporal
    connect( (QObject*) m_Controls, SIGNAL(SwitchAspect(int)),
      this, SLOT(SetAspect(int)) );

    // stepper window selection
    connect( (QObject*) m_Controls, SIGNAL(SwitchSelectedStepperWindow(int)),
      this, SLOT(SetStepperWindow(int)) );

    // recording window selection
    connect( (QObject*) m_Controls, SIGNAL(SwitchSelectedRecordingWindow(int)),
      this, SLOT(SetRecordingWindow(int)) );

    // advance the animation
    // every timer tick
    connect( (QObject*) m_Timer, SIGNAL(timeout()),
      this, SLOT(AdvanceAnimation()) );

    // movie generation
    // when the movie button is clicked 
    connect( (QObject*) m_Controls->btnMovie, SIGNAL(clicked()),
      this, SLOT(GenerateMovie()) );
      
    connect( (QObject*) m_Controls->btnScreenshot, SIGNAL(clicked()),
      this, SLOT(GenerateScreenshot()) );

    // blocking of ui elements during movie generation
    connect( (QObject*) this, SIGNAL(StartBlockControls()),
      (QObject*) m_Controls , SLOT(BlockControls()) );

    connect( (QObject*) this, SIGNAL(EndBlockControls()),
      (QObject*) m_Controls , SLOT(UnBlockControls()) );

    connect( (QObject*) this, SIGNAL(EndBlockControlsMovieDeactive()),
      (QObject*) m_Controls , SLOT(UnBlockControlsMovieDeactive()) );
  }
}

QAction* QmitkMovieMaker::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Movie Maker" ), QPixmap((const char**)icon_xpm),
    tr( "Camera Path" ), 0, parent, "MovieMaker"
  );

  return action;
}

void QmitkMovieMaker::Activated()
{
  QmitkFunctionality::Activated();
  
  m_FocusManagerObserverTag = mitk::GlobalInteraction::GetInstance()->GetFocusManager()->AddObserver( mitk::FocusEvent(), m_FocusManagerCallback);

  // Initialize steppers etc.
  this->FocusChange();
}

void QmitkMovieMaker::Deactivated()
{
  QmitkFunctionality::Deactivated();

  mitk::GlobalInteraction::GetInstance()->GetFocusManager()->RemoveObserver( m_FocusManagerObserverTag ); // remove (if tag is invalid, nothing is removed)
}

void QmitkMovieMaker::FocusChange()
{
  mitk::Stepper *stepper = this->GetAspectStepper();
  m_StepperAdapter->SetStepper( stepper );

  // Make the stepper movement non-inverted
  stepper->InverseDirectionOff();

  // Set stepping direction and aspect (spatial / temporal) for new stepper
  this->UpdateLooping();
  this->UpdateDirection();

  // Set newly focused window as active in "Selected Window" combo box
  const mitk::RenderingManager::RenderWindowVector rwv =
    mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

  int i;
  mitk::RenderingManager::RenderWindowVector::const_iterator iter;
  for (iter = rwv.begin(), i = 0; iter != rwv.end(); ++iter, ++i)
  {
    mitk::BaseRenderer* focusedRenderer =
      mitk::GlobalInteraction::GetInstance()->GetFocusManager()->GetFocused();

    if ( focusedRenderer == mitk::BaseRenderer::GetInstance((*iter)) )
    {
      m_Controls->cmbSelectedStepperWindow->setCurrentItem( i );
      m_Controls->cmbSelectedStepperWindow_activated(i);
      m_Controls->cmbSelectedRecordingWindow->setCurrentItem( i );
      m_Controls->cmbSelectedRecordWindow_activated(i);
      break;
    }
  }
}

void QmitkMovieMaker::AdvanceAnimation()
{
  // This method is called when a timer timeout occurs. It increases the
  // stepper value according to the elapsed time and the stepper interval.
  // Note that a screen refresh is not forced, but merely requested, and may
  // occur only after more calls to AdvanceAnimation().

  mitk::Stepper* stepper = this->GetAspectStepper();

  m_StepperAdapter->SetStepper( stepper );

  int elapsedTime = m_Time->elapsed();
  m_Time->restart();

  static double increment = 0.0;
  increment = increment - static_cast< int >( increment );
  increment += elapsedTime * stepper->GetSteps()
    / ( m_Controls->spnDuration->value() * 1000.0 );

  int i, n = static_cast< int >( increment );
  for ( i = 0; i < n; ++i )
  {
    stepper->Next();
  }
}

void QmitkMovieMaker::StartPlaying()
{

  vtkSphereSource* cyl = vtkSphereSource::New();
  vtkElevationFilter* ele = vtkElevationFilter::New();
  ele->SetLowPoint(0.0, -0.5, 0.0);
  ele->SetHighPoint(0.0, 0.5, 0.0);
  ele->SetInput(cyl->GetOutput());
  vtkDataSetMapper* mapper = vtkDataSetMapper::New();
  mapper->SetInput(ele->GetOutput());
  ele->Delete();
  cyl->Delete();
  vtkActor* actor = vtkActor::New();
  actor->SetMapper(mapper);
  mapper->Delete();
  
  // ANLEGEN des QVTK Widget
  widget = new QVTKWidget(0, "testwidget");
  widget->resize(512,512);
 

  // ANLEGEN von vtkPROP und m_PropRenderer  
  vtkMitkRenderProp* renProp = vtkMitkRenderProp::New();
  m_PropRenderer = new mitk::VtkPropRenderer("test");
  
  m_PropRenderer->SetData(m_DataTreeIterator.GetPointer());
  renProp->SetPropRenderer(m_PropRenderer);
  m_PropRenderer->SetMapperID(1);
  m_PropRenderer->GetDisplayGeometry()->Fit();
  
  vtkRenderer * vtkRenderer = m_PropRenderer->GetVtkRenderer();  //renderWindow->GetRenderer()->GetVtkRenderer();//vtkRenderer::New();

  vtkRenderer->AddViewProp(renProp);
  //vtkRenderer->AddViewProp(actor);

  // vtk
  mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderWindow()->AddRenderer(vtkRenderer);
  //widget->SetRenderWindow((vtkRenderWindow*) renderWindow->GetVtkRenderWindow());
 
  
  widget->GetRenderWindow()->AddRenderer(vtkRenderer);
  
  // INTERACTION  

  /* vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);*/
  // event handling
  connections = vtkEventQtSlotConnect::New();
  connections->Connect(widget->GetRenderWindow()->GetInteractor(),
                       vtkCommand::ModifiedEvent,
                       this,SLOT(RenderSlot()) );




  widget->show();

 
}

void QmitkMovieMaker::RenderSlot( )
{
  int *i = widget->GetRenderWindow()->GetSize();
  m_PropRenderer->Resize(i[0],i[1]);

  widget->GetRenderWindow()->Render();
}

void QmitkMovieMaker::PausePlaying()
{
  
  m_Timer->stop();

  m_Controls->btnPlay->setHidden( false );
  m_Controls->btnPause->setHidden( true );
  if(m_movieGenerator.IsNull())
      m_Controls->btnMovie->setEnabled( false );
}

void QmitkMovieMaker::StopPlaying()
{
  m_Timer->stop();
  switch ( m_Direction )
  {
  case 0:
  case 2:
    this->GetAspectStepper()->First();
    break;

  case 1:
    this->GetAspectStepper()->Last();
    break;
  }

  // Reposition slider GUI element
  m_StepperAdapter->SetStepper( this->GetAspectStepper() );

  if(m_movieGenerator.IsNull())
      m_Controls->btnMovie->setEnabled( false );
}

void QmitkMovieMaker::SetLooping( bool looping )
{
  m_Looping = looping;
  this->UpdateLooping();
}

void QmitkMovieMaker::SetDirection( int direction )
{
  m_Direction = direction;
  this->UpdateDirection();
}

void QmitkMovieMaker::SetAspect( int aspect )
{
  m_Aspect = aspect;

  m_StepperAdapter->SetStepper( this->GetAspectStepper() );
  this->UpdateLooping();
  this->UpdateDirection();
}

void QmitkMovieMaker::SetStepperWindow( int window )
{
  // Set newly selected window / renderer as focused
  const mitk::RenderingManager::RenderWindowVector rwv =
    mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
 
  int i;
  mitk::RenderingManager::RenderWindowVector::const_iterator iter;
  for (iter = rwv.begin(), i = 0; iter != rwv.end(); ++iter, ++i)
  {
    if ( i == window )
    {
      mitk::GlobalInteraction::GetInstance()->GetFocusManager()
        ->SetFocused( mitk::BaseRenderer::GetInstance((*iter)) );
      break;
    }
  }
}
void QmitkMovieMaker::SetRecordingWindow( int window )
{
  // Set newly selected window for recording
  const mitk::RenderingManager::RenderWindowVector rwv =
    mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
 
  int i;
  mitk::RenderingManager::RenderWindowVector::const_iterator iter;
  for (iter = rwv.begin(), i = 0; iter != rwv.end(); ++iter, ++i)
  {
    if ( i == window )
    {
      m_RecordingRenderer = mitk::BaseRenderer::GetInstance( (*iter) );
      break;
    }
  }
}

void QmitkMovieMaker::UpdateLooping()
{
 this->GetAspectStepper()->SetAutoRepeat( m_Looping );
}

void QmitkMovieMaker::UpdateDirection()
{
  mitk::Stepper* stepper = this->GetAspectStepper();

  switch ( m_Direction )
  {
  case 0:
    stepper->InverseDirectionOff();
    stepper->PingPongOff();
    break;

  case 1:
    stepper->InverseDirectionOn();
    stepper->PingPongOff();
    break;

  case 2:
    stepper->PingPongOn();
    break;
  }
}

mitk::Stepper* QmitkMovieMaker::GetAspectStepper()
{
  if (m_Aspect == 0)
  {
    m_Stepper = NULL;
    return this->GetSpatialController()->GetSlice();
  }
  else if (m_Aspect == 1) 
  {
    m_Stepper = NULL;
    return this->GetTemporalController()->GetTime();
  } else if (m_Aspect == 2) {
    if (m_Stepper.IsNull()) {
      int rel = m_Controls->spatialTimeRelation->value();
      int timeRepeat =1 ;
      int sliceRepeat = 1;
      if (rel < 0 ) {
        sliceRepeat = -rel;
      } else if (rel > 0) {
        timeRepeat = rel;
      }
      m_Stepper = mitk::MultiStepper::New();
      m_Stepper->AddStepper(this->GetSpatialController()->GetSlice(),sliceRepeat);
      m_Stepper->AddStepper(this->GetTemporalController()->GetTime(),timeRepeat);
    }
    return m_Stepper.GetPointer();
  } else {
    // should never get here
  return 0;
  }
}

void QmitkMovieMaker::GenerateMovie()
{
  emit StartBlockControls();
  
  // provide the movie generator with the stepper and rotate the camera each step
  if ( m_movieGenerator.IsNotNull() )
  {
    m_movieGenerator->SetStepper( this->GetAspectStepper() );
    m_movieGenerator->SetRenderer(m_RecordingRenderer);
    m_movieGenerator->SetFrameRate(static_cast<unsigned int>( 360 / ( m_Controls->spnDuration->value() ) ) );

    QString movieFileName = QFileDialog::getSaveFileName(
      QString::null, "Movie (*.avi)", 0, "movie file dialog", "Choose a file name"
    );

    if(movieFileName.isEmpty() == false)
    {
      m_movieGenerator->SetFileName( movieFileName.ascii() );
      m_movieGenerator->WriteMovie();
    }

    emit EndBlockControls();
  }
  else
  {
    std::cerr << "Either mitk::MovieGenerator is not implemented for your";
    std::cerr << " platform or an error occurred during";
    std::cerr << " mitk::MovieGenerator::New()" << std::endl;

    emit EndBlockControlsMovieDeactive();
  }
}

void QmitkMovieMaker::GenerateScreenshot()
{
  emit StartBlockControls();
  
  CommonFunctionality::SaveScreenshot( mitk::GlobalInteraction::GetInstance()->GetFocus()->GetRenderWindow() );
  
  if(m_movieGenerator.IsNotNull())
    emit EndBlockControls();
  else
    emit EndBlockControlsMovieDeactive();
}

