/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "QmitkTreeNodeSelector.h"
#include "QmitkStepperAdapter.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"

#include "mitkOpenGLRenderer.h"
#include "mitkMovieGenerator.h"
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

QmitkMovieMaker::QmitkMovieMaker( QObject *parent, const char *name,
  QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it )

: QmitkFunctionality(parent, name, it),
  m_Controls(NULL), m_StepperAdapter(NULL), m_MultiWidget(mitkStdMultiWidget),
  m_Looping(true), m_Direction(0), m_Aspect(0)
{
  this->SetAvailability(true);
  m_Timer = new QTimer(this);
  m_Time = new QTime();

  m_FocusManagerCallback = MemberCommand::New();
  m_FocusManagerCallback->SetCallbackFunction( this, &QmitkMovieMaker::FocusChange );
  mitk::GlobalInteraction::GetInstance()->GetFocusManager()->AddObserver(
    mitk::FocusEvent(), m_FocusManagerCallback
  );
}

QmitkMovieMaker::~QmitkMovieMaker()
{
  delete m_StepperAdapter;
  delete m_Timer;
  delete m_Time;
}

mitk::BaseController* QmitkMovieMaker::GetSpatialController()
{
  mitk::BaseRenderer* focusedRenderer =
    mitk::GlobalInteraction::GetInstance()->GetFocus();

  if ( m_MultiWidget->GetRenderWindow1()->GetRenderer() == focusedRenderer )
  {
    return m_MultiWidget->GetRenderWindow1()->GetController();
  }
  else if ( m_MultiWidget->GetRenderWindow2()->GetRenderer() == focusedRenderer )
  {
    return m_MultiWidget->GetRenderWindow2()->GetController();
  }
  else if ( m_MultiWidget->GetRenderWindow3()->GetRenderer() == focusedRenderer )
  {
    return m_MultiWidget->GetRenderWindow3()->GetController();
  }
  else if ( m_MultiWidget->GetRenderWindow4()->GetRenderer() == focusedRenderer )
  {
    return m_MultiWidget->GetRenderWindow4()->GetController();
  }

  return m_MultiWidget->GetRenderWindow4()->GetController();
}

mitk::BaseController* QmitkMovieMaker::GetTemporalController()
{
  return m_MultiWidget->GetTimeNavigationController();
}

QWidget* QmitkMovieMaker::CreateMainWidget(QWidget *parent)
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
    const mitk::RenderWindow::RenderWindowSet rws =
      mitk::RenderWindow::GetInstances();

    mitk::RenderWindow::RenderWindowSet::const_iterator iter;
    for (iter = rws.begin(); iter != rws.end(); ++iter)
    {
      m_Controls->cmbSelectedWindow->insertItem((*iter)->GetName(), -1);
    }
  }
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

    // window selection
    connect( (QObject*) m_Controls, SIGNAL(SwitchSelectedWindow(int)),
      this, SLOT(SetWindow(int)) );


    // advance the animation
    // every timer tick
    connect( (QObject*) m_Timer, SIGNAL(timeout()),
      this, SLOT(AdvanceAnimation()) );

    // movie generation
    // when the movie button is clickethe movie button is clicked
    connect( (QObject*) m_Controls->btnMovie, SIGNAL(clicked()),
      this, SLOT(GenerateMovie()) );
      
    connect( (QObject*) m_Controls->btnScreenshot, SIGNAL(clicked()),
      this, SLOT(GenerateScreenshot()) );

    // blocking of ui elements during movie generation
    connect( (QObject*) this, SIGNAL(StartBlockControls()),
      (QObject*) m_Controls , SLOT(BlockControls()) );

    connect( (QObject*) this, SIGNAL(EndBlockControls()),
      (QObject*) m_Controls , SLOT(UnBlockControls()) );
  }
}

QAction* QmitkMovieMaker::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "MovieMaker" ), QPixmap((const char**)icon_xpm),
    tr( "Camera Path" ), Qt::CTRL + Qt::Key_G, parent, "MovieMaker"
  );

  return action;
}

void QmitkMovieMaker::Activated()
{
  QmitkFunctionality::Activated();
  
  // Initialize steppers etc.
  this->FocusChange();
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
  const mitk::RenderWindow::RenderWindowSet rws =
    mitk::RenderWindow::GetInstances();

  int i;
  mitk::RenderWindow::RenderWindowSet::const_iterator iter;
  for (iter = rws.begin(), i = 0; iter != rws.end(); ++iter, ++i)
  {
    mitk::BaseRenderer* focusedRenderer =
      mitk::GlobalInteraction::GetInstance()->GetFocusManager()->GetFocused();

    if ( focusedRenderer == (*iter)->GetRenderer() )
    {
      m_Controls->cmbSelectedWindow->setCurrentItem( i );
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
  // Restart timer with 5 msec interval - this should be fine-grained enough
  // even for high display refresh frequencies
  m_Timer->start( 5 );

  m_Time->restart();
}

void QmitkMovieMaker::PausePlaying()
{
  m_Timer->stop();
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
  this->UpdateLooping();
  this->UpdateDirection();
}

void QmitkMovieMaker::SetWindow( int window )
{
  // Set newly selected window / renderer as focused
  const mitk::RenderWindow::RenderWindowSet rws =
    mitk::RenderWindow::GetInstances();
 
  int i;
  mitk::RenderWindow::RenderWindowSet::const_iterator iter;
  for (iter = rws.begin(), i = 0; iter != rws.end(); ++iter, ++i)
  {
    if ( i == window )
    {
      mitk::GlobalInteraction::GetInstance()->GetFocusManager()
        ->SetFocused( (*iter)->GetRenderer() );
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
  mitk::MovieGenerator::Pointer movieGenerator = mitk::MovieGenerator::New();

  if ( movieGenerator.IsNotNull() )
  {
    movieGenerator->SetStepper( this->GetAspectStepper() );
    movieGenerator->SetRenderer(
      mitk::GlobalInteraction::GetInstance()->GetFocus()
    );

    QString movieFileName = QFileDialog::getSaveFileName(
      QString::null, "Movie (*.avi)", 0, "movie file dialog", "Choose a file name"
    );

    if(movieFileName.isEmpty() == false)
    {
      movieGenerator->SetFileName( movieFileName.ascii() );
      movieGenerator->WriteMovie();
    }
  }
  else
  {
    std::cerr << "Either mitk::MovieGenerator is not implemented for your";
    std::cerr << " platform or an error occurred during";
    std::cerr << " mitk::MovieGenerator::New()" << std::endl;
  }
  emit EndBlockControls();
}

void QmitkMovieMaker::GenerateScreenshot()
{
  emit StartBlockControls();
  
  CommonFunctionality::SaveScreenshot( mitk::GlobalInteraction::GetInstance()->GetFocus()->GetRenderWindow() );
  
  emit EndBlockControls();
}

