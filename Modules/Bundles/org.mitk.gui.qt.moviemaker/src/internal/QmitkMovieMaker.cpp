/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 16947 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkMovieMaker.h"
//#include "QmitkMovieMakerControls.h"
#include "QmitkStepperAdapter.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"
/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 16947 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkMovieMaker.h"
//#include "QmitkMovieMakerControls.h"
#include "QmitkStepperAdapter.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"

#include "mitkVtkPropRenderer.h"
#include "mitkGlobalInteraction.h"

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

#include "vtkImageWriter.h"
#include "vtkJPEGWriter.h"
#include "vtkPNGWriter.h"
#include "vtkRenderLargeImage.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"

#include <vtkActor.h>
#include "vtkMitkRenderProp.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkRenderWindowInteractor.h"
#include <qradiobutton.h>

QmitkMovieMaker::QmitkMovieMaker(QObject *parent, const char * /*name*/)

:
  QmitkFunctionality(), m_Controls(NULL),
  m_StepperAdapter(NULL),
  m_FocusManagerCallback(0), m_Looping(true), m_Direction(0), m_Aspect(0)
{

  parentWidget = parent;

  m_Timer = new QTimer(this);
  m_Time = new QTime();

  m_FocusManagerCallback = MemberCommand::New();
  m_FocusManagerCallback->SetCallbackFunction(this, &QmitkMovieMaker::FocusChange);

  m_movieGenerator = mitk::MovieGenerator::New();

  if (m_movieGenerator.IsNull())
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
  mitk::BaseRenderer* focusedRenderer = mitk::GlobalInteraction::GetInstance()->GetFocus();

  if (mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget1->GetRenderWindow())
      == focusedRenderer)
  {
    return GetActiveStdMultiWidget()->mitkWidget1->GetController();
  }
  else if (mitk::BaseRenderer::GetInstance(
      GetActiveStdMultiWidget()->mitkWidget2->GetRenderWindow()) == focusedRenderer)
  {
    return GetActiveStdMultiWidget()->mitkWidget2->GetController();
  }
  else if (mitk::BaseRenderer::GetInstance(
      GetActiveStdMultiWidget()->mitkWidget3->GetRenderWindow()) == focusedRenderer)
  {
    return GetActiveStdMultiWidget()->mitkWidget3->GetController();
  }
  else if (mitk::BaseRenderer::GetInstance(
      GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow()) == focusedRenderer)
  {
    return GetActiveStdMultiWidget()->mitkWidget4->GetController();
  }

  return GetActiveStdMultiWidget()->mitkWidget4->GetController();
}

mitk::BaseController* QmitkMovieMaker::GetTemporalController()
{
  return GetActiveStdMultiWidget()->GetTimeNavigationController();
}

void QmitkMovieMaker::CreateConnections()
{
  if (m_Controls)
  {
    // start / pause / stop playing
    connect((QObject*) m_Controls->btnPlay, SIGNAL(clicked()), (QObject*) this,
        SLOT(StartPlaying()));
    connect((QObject*) m_Controls->btnPause, SIGNAL(clicked()), this, SLOT(PausePlaying()));
    connect((QObject*) m_Controls->btnStop, SIGNAL(clicked()), this, SLOT(StopPlaying()));

    connect((QObject*) m_Controls->rbtnForward, SIGNAL(clicked()), this, SLOT(RBTNForward()));
    connect((QObject*) m_Controls->rbtnBackward, SIGNAL(clicked()), this, SLOT(RBTNBackward()));
    connect((QObject*) m_Controls->rbtnPingPong, SIGNAL(clicked()), this, SLOT(RBTNPingPong()));

    // radio button group: forward, backward, ping-pong
    connect( this, SIGNAL(SwitchDirection(int)), this, SLOT(SetDirection(int)) );

    // radio button group: spatial, temporal
    connect((QObject*) m_Controls->rbtnSpatial, SIGNAL(clicked()), this, SLOT(RBTNSpatial()));
    connect((QObject*) m_Controls->rbtnTemporal, SIGNAL(clicked()), this, SLOT(RBTNTemporal()));
    connect((QObject*) m_Controls->rbtnCombined, SIGNAL(clicked()), this, SLOT(RBTNCombined()));connect( this, SIGNAL(SwitchAspect(int)), this, SLOT(SetAspect(int)) );

    // stepper window selection
    connect((QObject*) (m_Controls->cmbSelectedStepperWindow), SIGNAL ( activated ( int) ), (QObject*) this, SLOT ( SetStepperWindow (int) ) );

    // recording window selection
    connect((QObject*) (m_Controls->cmbSelectedRecordingWindow), SIGNAL ( activated ( int) ), (QObject*) this, SLOT ( SetRecordingWindow (int) ) );

    // advance the animation
    // every timer tick
    connect((QObject*) m_Timer, SIGNAL(timeout()), this, SLOT(AdvanceAnimation()));

    // movie generation
    // when the movie button is clicked
    connect((QObject*) m_Controls->btnMovie, SIGNAL(clicked()), this, SLOT(GenerateMovie()));

    connect((QObject*) m_Controls->btnScreenshot, SIGNAL(clicked()), this, SLOT(
        GenerateScreenshot()));
    connect((QObject*) m_Controls->m_HRScreenshot, SIGNAL(clicked()), this, SLOT(
        GenerateHR3DScreenshot()));

    // blocking of ui elements during movie generation
    connect((QObject*) this, SIGNAL(StartBlockControls()), (QObject*) this, SLOT(BlockControls()));

    connect((QObject*) this, SIGNAL(EndBlockControls()), (QObject*) this, SLOT(UnBlockControls()));

    connect((QObject*) this, SIGNAL(EndBlockControlsMovieDeactive()), (QObject*) this, SLOT(
        UnBlockControlsMovieDeactive()));
  }
}

void QmitkMovieMaker::Activated()
{
  QmitkFunctionality::Activated();

  // create a member command that will be executed from the observer
  itk::SimpleMemberCommand<QmitkMovieMaker>::Pointer stepperChangedCommand;
  stepperChangedCommand = itk::SimpleMemberCommand<QmitkMovieMaker>::New();
  // set the callback function of the member command
  stepperChangedCommand->SetCallbackFunction(this, &QmitkMovieMaker::UpdateGUI);
  // add an observer to the data tree node pointer connected to the above member command
  std::cout << "Add observer on insertion point node in NavigationPathController::AddObservers"
      << std::endl;
  m_StepperObserverTag = this->GetTemporalController()->GetTime()->AddObserver(
      itk::ModifiedEvent(), stepperChangedCommand);

  m_FocusManagerObserverTag
      = mitk::GlobalInteraction::GetInstance()->GetFocusManager()->AddObserver(mitk::FocusEvent(),
          m_FocusManagerCallback);
  this->UpdateGUI();
  // Initialize steppers etc.
  this->FocusChange();
}

void QmitkMovieMaker::Deactivated()
{
  QmitkFunctionality::Deactivated();
  this->GetTemporalController()->GetTime()->RemoveObserver(m_StepperObserverTag);
  mitk::GlobalInteraction::GetInstance()->GetFocusManager()->RemoveObserver(
      m_FocusManagerObserverTag); // remove (if tag is invalid, nothing is removed)
}

void QmitkMovieMaker::FocusChange()
{
  mitk::Stepper *stepper = this->GetAspectStepper();
  m_StepperAdapter->SetStepper(stepper);

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

    if (focusedRenderer == mitk::BaseRenderer::GetInstance((*iter)))
    {
      m_Controls->cmbSelectedStepperWindow->setCurrentIndex(i);
      //      this->cmbSelectedStepperWindow_activated(i);
      this->SetStepperWindow(i);
      m_Controls->cmbSelectedRecordingWindow->setCurrentIndex(i);
      //      this->cmbSelectedRecordWindow_activated(i);
      this->SetRecordingWindow(i);
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

  m_StepperAdapter->SetStepper(stepper);

  int elapsedTime = m_Time->elapsed();
  m_Time->restart();

  static double increment = 0.0;
  increment = increment - static_cast<int> (increment);
  increment += elapsedTime * stepper->GetSteps() / (m_Controls->spnDuration->value() * 1000.0);

  int i, n = static_cast<int> (increment);
  for (i = 0; i < n; ++i)
  {
    stepper->Next();
  }
}

void QmitkMovieMaker::RenderSlot()
{
  int *i = widget->GetRenderWindow()->GetSize();
  m_PropRenderer->Resize(i[0], i[1]);

  widget->GetRenderWindow()->Render();
}

void QmitkMovieMaker::PausePlaying()
{

  m_Controls->slidAngle->setDisabled(false);
  m_Controls->btnMovie->setEnabled(true);
  m_Controls->btnPlay->setEnabled(true);
  m_Controls->btnScreenshot->setEnabled(true);

  m_Timer->stop();

  m_Controls->btnPlay->setHidden(false);
  m_Controls->btnPause->setHidden(true);
  if (m_movieGenerator.IsNull())
    m_Controls->btnMovie->setEnabled(false);
}

void QmitkMovieMaker::StopPlaying()
{
  m_Controls->slidAngle->setDisabled(false);
  m_Controls->btnMovie->setEnabled(true);
  m_Controls->btnPlay->setEnabled(true);
  m_Controls->btnScreenshot->setEnabled(true);

  m_Controls->btnPlay->setHidden(false);
  m_Controls->btnPause->setHidden(true);

  m_Timer->stop();
  switch (m_Direction)
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
  m_StepperAdapter->SetStepper(this->GetAspectStepper());

  if (m_movieGenerator.IsNull())
    m_Controls->btnMovie->setEnabled(false);

}

void QmitkMovieMaker::SetLooping(bool looping)
{
  m_Looping = looping;
  this->UpdateLooping();
}

void QmitkMovieMaker::SetDirection(int direction)
{
  m_Direction = direction;
  this->UpdateDirection();
}

void QmitkMovieMaker::SetAspect(int aspect)
{
  m_Aspect = aspect;

  m_StepperAdapter->SetStepper(this->GetAspectStepper());
  this->UpdateLooping();
  this->UpdateDirection();
}

void QmitkMovieMaker::SetStepperWindow(int window)
{
  // Set newly selected window / renderer as focused
  const mitk::RenderingManager::RenderWindowVector rwv =
      mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

  int i;
  mitk::RenderingManager::RenderWindowVector::const_iterator iter;
  for (iter = rwv.begin(), i = 0; iter != rwv.end(); ++iter, ++i)
  {
    if (i == window)
    {
      mitk::GlobalInteraction::GetInstance()->GetFocusManager() ->SetFocused(
          mitk::BaseRenderer::GetInstance((*iter)));
      break;
    }
  }
}

void QmitkMovieMaker::SetRecordingWindow(int window)
{
  // Set newly selected window for recording
  const mitk::RenderingManager::RenderWindowVector rwv =
      mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

  int i;
  mitk::RenderingManager::RenderWindowVector::const_iterator iter;
  for (iter = rwv.begin(), i = 0; iter != rwv.end(); ++iter, ++i)
  {
    if (i == window)
    {
      m_RecordingRenderer = mitk::BaseRenderer::GetInstance((*iter));
      break;
    }
  }
}

void QmitkMovieMaker::UpdateLooping()
{
  this->GetAspectStepper()->SetAutoRepeat(m_Looping);
}

void QmitkMovieMaker::UpdateDirection()
{
  mitk::Stepper* stepper = this->GetAspectStepper();

  switch (m_Direction)
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
  }
  else if (m_Aspect == 2)
  {
    if (m_Stepper.IsNull())
    {
      int rel = m_Controls->spatialTimeRelation->value();
      int timeRepeat = 1;
      int sliceRepeat = 1;
      if (rel < 0)
      {
        sliceRepeat = -rel;
      }
      else if (rel > 0)
      {
        timeRepeat = rel;
      }
      m_Stepper = mitk::MultiStepper::New();
      m_Stepper->AddStepper(this->GetSpatialController()->GetSlice(), sliceRepeat);
      m_Stepper->AddStepper(this->GetTemporalController()->GetTime(), timeRepeat);
    }
    return m_Stepper.GetPointer();
  }
  else
  {
    // should never get here
    return 0;
  }
}

void QmitkMovieMaker::GenerateMovie()
{
  emit StartBlockControls();

  // provide the movie generator with the stepper and rotate the camera each step
  if (m_movieGenerator.IsNotNull())
  {
    m_movieGenerator->SetStepper(this->GetAspectStepper());
    m_movieGenerator->SetRenderer(m_RecordingRenderer);
    m_movieGenerator->SetFrameRate(static_cast<unsigned int> (360
        / (m_Controls->spnDuration->value())));

    //    QString movieFileName = QFileDialog::getSaveFileName( QString::null, "Movie (*.avi)", 0, "movie file dialog", "Choose a file name" );

    QString movieFileName = QFileDialog::getSaveFileName(0, "Choose a file name", QString::null,
        "Movie (*.avi)", 0, 0);

    if (movieFileName.isEmpty() == false)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      m_movieGenerator->SetFileName(movieFileName.toAscii());
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

  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");
  
  vtkRenderer* renderer = mitk::GlobalInteraction::GetInstance()->GetFocus()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, 1, fileName);

  if (m_movieGenerator.IsNotNull())
    emit EndBlockControls();
  else
    emit EndBlockControlsMovieDeactive();
}

void QmitkMovieMaker::GenerateHR3DScreenshot()
{
  emit StartBlockControls();

  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");
  
  // only works correctly for 3D RenderWindow
  vtkRenderer* renderer = m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, 4, fileName);

  if (m_movieGenerator.IsNotNull())
    emit EndBlockControls();
  else
    emit EndBlockControlsMovieDeactive();
}

void QmitkMovieMaker::UpdateGUI()
{
  int bla = this->GetTemporalController()->GetTime()->GetSteps();
  if (bla < 2)
  {
    m_Controls->rbtnTemporal->setEnabled(false);
    m_Controls->rbtnCombined->setEnabled(false);
    m_Controls->spatialTimeRelation->setEnabled(false);
  }
  else
  {
    m_Controls->rbtnTemporal->setEnabled(true);
    m_Controls->rbtnCombined->setEnabled(true);
    m_Controls->spatialTimeRelation->setEnabled(true);
  }

}

void QmitkMovieMaker::DataStorageChanged()
{
  //  UpdateGUI();
}

void QmitkMovieMaker::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkMovieMakerControls;
    m_Controls->setupUi(parent);

    m_StepperAdapter = new QmitkStepperAdapter((QObject*) m_Controls->slidAngle,
        this->GetSpatialController()->GetSlice(), "AngleStepperToMovieMakerFunctionality");

    // Initialize "Selected Window" combo box
    const mitk::RenderingManager::RenderWindowVector rwv =
        mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

    m_Controls->cmbSelectedStepperWindow->setInsertPolicy(QComboBox::InsertAtTop);
    m_Controls->cmbSelectedRecordingWindow->setInsertPolicy(QComboBox::InsertAtTop);

    mitk::RenderingManager::RenderWindowVector::const_iterator iter;
    for (iter = rwv.begin(); iter != rwv.end(); ++iter)
    {
      m_Controls->cmbSelectedStepperWindow->insertItem(-1,
          mitk::BaseRenderer::GetInstance((*iter))->GetName());
      m_Controls->cmbSelectedRecordingWindow->insertItem(-1, mitk::BaseRenderer::GetInstance(
          (*iter))->GetName());
    }

    m_Controls->btnPause->setHidden(true);
    if (m_movieGenerator.IsNull())
      m_Controls->btnMovie->setEnabled(false);
  }

  this->CreateConnections();

}

void QmitkMovieMaker::StartPlaying()
{
  m_Controls->slidAngle->setDisabled(true);
  m_Controls->btnMovie->setEnabled(false);
  m_Controls->btnPlay->setEnabled(false);
  m_Controls->btnScreenshot->setEnabled(false);

  // Restart timer with 5 msec interval - this should be fine-grained enough
  // even for high display refresh frequencies
  m_Timer->start(5);

  m_Time->restart();

  m_Controls->btnPlay->setHidden(true);
  m_Controls->btnPause->setHidden(false);
  if (m_movieGenerator.IsNull())
    m_Controls->btnMovie->setEnabled(false);

}

void QmitkMovieMaker::RBTNForward()
{
  emit SwitchDirection(0);
}

void QmitkMovieMaker::RBTNBackward()
{
  emit SwitchDirection(1);
}

void QmitkMovieMaker::RBTNPingPong()
{
  emit SwitchDirection(2);
}

void QmitkMovieMaker::RBTNSpatial()
{
  emit SwitchAspect(0);
}

void QmitkMovieMaker::RBTNTemporal()
{
  emit SwitchAspect(1);
}

void QmitkMovieMaker::RBTNCombined()
{
  emit SwitchAspect(2);
}

void QmitkMovieMaker::BlockControls()
{
  BlockControls(true);
}

void QmitkMovieMaker::UnBlockControls()
{
  BlockControls(false);
}

void QmitkMovieMaker::UnBlockControlsMovieDeactive()
{
  BlockControls(false);

  m_Controls->btnMovie->setEnabled(false);
}

void QmitkMovieMaker::BlockControls(bool blocked)
{
  m_Controls->slidAngle->setDisabled(blocked);
  m_Controls->spnDuration->setEnabled(!blocked);
  m_Controls->btnPlay->setEnabled(!blocked);
  m_Controls->btnMovie->setEnabled(!blocked);
  m_Controls->btnScreenshot->setEnabled(!blocked);
}

void QmitkMovieMaker::StdMultiWidgetAvailable(QmitkStdMultiWidget&  stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
  m_Parent->setEnabled(true);
}

void QmitkMovieMaker::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
  m_Parent->setEnabled(false);
}

void QmitkMovieMaker::TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName)
{
  if ((renderer == NULL) ||(magnificationFactor < 1) || fileName.isEmpty())
    return;

  bool doubleBuffering( renderer->GetRenderWindow()->GetDoubleBuffer() );
  renderer->GetRenderWindow()->DoubleBufferOff();

  vtkImageWriter* fileWriter;

  QFileInfo fi(fileName);
  QString suffix = fi.suffix();
  if (suffix.compare("png", Qt::CaseInsensitive) == 0)
  {
    fileWriter = vtkPNGWriter::New();
  }
  else  // default is jpeg
  {
    vtkJPEGWriter* w = vtkJPEGWriter::New();
    w->SetQuality(100);
    w->ProgressiveOff();
    fileWriter = w;
  }
  vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
  magnifier->SetInput(renderer);
  magnifier->SetMagnification(magnificationFactor);
  //magnifier->Update();
  fileWriter->SetInput(magnifier->GetOutput());
  fileWriter->SetFileName(fileName.toLatin1());

  // vtkRenderLargeImage has problems with different layers, therefore we have to 
  // temporarily deactivate all other layers.
  // we set the background to white, because it is nicer than black...
  double oldBackground[3];
  renderer->GetBackground(oldBackground);
  double white[] = {1.0, 1.0, 1.0};
  renderer->SetBackground(white);
  m_MultiWidget->DisableColoredRectangles();
  m_MultiWidget->DisableDepartmentLogo();
  m_MultiWidget->DisableGradientBackground();

  fileWriter->Write();  
  fileWriter->Delete();  

  m_MultiWidget->EnableColoredRectangles();
  m_MultiWidget->EnableDepartmentLogo();
  m_MultiWidget->EnableGradientBackground();
  renderer->SetBackground(oldBackground);
  
  renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}

