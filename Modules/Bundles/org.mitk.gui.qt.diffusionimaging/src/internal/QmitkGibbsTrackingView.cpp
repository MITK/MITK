/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkGibbsTrackingView.h"
#include <QmitkStdMultiWidget.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

// MITK
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkImageAccessByItk.h>
#include <mitkProgressBar.h>

// ITK
#include <itkGibbsTrackingFilter.h>
#include <itkResampleImageFilter.h>

// MISC
#include <tinyxml.h>


QmitkTrackingWorker::QmitkTrackingWorker(QmitkGibbsTrackingView* view)
  : m_View(view)
{

}

void QmitkTrackingWorker::run()
{
  m_View->m_GlobalTracker = QmitkGibbsTrackingView::GibbsTrackingFilterType::New();

  MITK_INFO << "Resampling mask images";
  // setup resampler
  typedef itk::ResampleImageFilter<QmitkGibbsTrackingView::MaskImgType, QmitkGibbsTrackingView::MaskImgType, float> ResamplerType;
  ResamplerType::Pointer resampler = ResamplerType::New();
  resampler->SetOutputSpacing( m_View->m_ItkQBallImage->GetSpacing() );
  resampler->SetOutputOrigin( m_View->m_ItkQBallImage->GetOrigin() );
  resampler->SetOutputDirection( m_View->m_ItkQBallImage->GetDirection() );
  resampler->SetSize( m_View->m_ItkQBallImage->GetLargestPossibleRegion().GetSize() );

  // resample mask image
  resampler->SetInput( m_View->m_MaskImage );
  resampler->SetDefaultPixelValue(0);
  resampler->Update();
  m_View->m_MaskImage = resampler->GetOutput();

  if (m_View->m_GfaImage.IsNotNull())
  {
    ResamplerType::Pointer resampler = ResamplerType::New();
    resampler->SetOutputSpacing( m_View->m_ItkQBallImage->GetSpacing() );
    resampler->SetOutputOrigin( m_View->m_ItkQBallImage->GetOrigin() );
    resampler->SetOutputDirection( m_View->m_ItkQBallImage->GetDirection() );
    resampler->SetSize( m_View->m_ItkQBallImage->GetLargestPossibleRegion().GetSize() );
    resampler->SetInput( m_View->m_GfaImage );
    resampler->SetDefaultPixelValue(0);
    resampler->Update();
    m_View->m_GfaImage = resampler->GetOutput();
  }

  m_View->m_GlobalTracker->SetInput0(m_View->m_ItkQBallImage.GetPointer());
  m_View->m_GlobalTracker->SetMaskImage(m_View->m_MaskImage);
  m_View->m_GlobalTracker->SetGfaImage(m_View->m_GfaImage);
  m_View->m_GlobalTracker->SetTempStart((float)m_View->m_Controls->m_StartTempSlider->value()/100);
  m_View->m_GlobalTracker->SetTempEnd((float)m_View->m_Controls->m_EndTempSlider->value()/10000);
  m_View->m_GlobalTracker->SetNumIt(m_View->m_Iterations);
  m_View->m_GlobalTracker->SetParticleWeight((float)m_View->m_Controls->m_ParticleWeightSlider->value()/10000);
  m_View->m_GlobalTracker->SetSubtractMean(m_View->m_Controls->m_MeanSubtractionCheckbox->isChecked());
  m_View->m_GlobalTracker->SetParticleWidth((float)(m_View->m_Controls->m_ParticleWidthSlider->value())/10);
  m_View->m_GlobalTracker->SetParticleLength((float)(m_View->m_Controls->m_ParticleLengthSlider->value())/10);
  m_View->m_GlobalTracker->SetInexBalance((float)m_View->m_Controls->m_InExBalanceSlider->value()/10);
  m_View->m_GlobalTracker->SetFiberLength(m_View->m_Controls->m_FiberLengthSlider->value());

  m_View->m_GlobalTracker->Update();
  m_View->m_TrackingThread.quit();
}

const std::string QmitkGibbsTrackingView::VIEW_ID =
"org.mitk.views.gibbstracking";

QmitkGibbsTrackingView::QmitkGibbsTrackingView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
  , m_ThreadIsRunning(false)
  , m_GlobalTracker(NULL)
  , m_QBallImage(NULL)
  , m_MaskImage(NULL)
  , m_GfaImage(NULL)
  , m_GfaImageNode(NULL)
  , m_QBallImageNode(NULL)
  , m_ItkQBallImage(NULL)
  , m_FiberBundleNode(NULL)
  , m_TrackingWorker(this)
  , m_QBallSelected(false)
  , m_FibSelected(false)
  , m_Iterations(10000000)
  , m_LastStep(0)
{
  m_TrackingWorker.moveToThread(&m_TrackingThread);
  connect(&m_TrackingThread, SIGNAL(started()), this, SLOT(BeforeThread()));
  connect(&m_TrackingThread, SIGNAL(started()), &m_TrackingWorker, SLOT(run()));
  connect(&m_TrackingThread, SIGNAL(finished()), this, SLOT(AfterThread()));
  connect(&m_TrackingThread, SIGNAL(terminated()), this, SLOT(AfterThread()));
  m_TrackingTimer = new QTimer(this);
}

QmitkGibbsTrackingView::~QmitkGibbsTrackingView()
{
  delete m_TrackingTimer;
}

// update tracking status and generate fiber bundle
void QmitkGibbsTrackingView::TimerUpdate()
{
  mitk::ProgressBar::GetInstance()->Progress(m_GlobalTracker->GetCurrentStep()-m_LastStep);
  m_LastStep = m_GlobalTracker->GetCurrentStep();
  UpdateTrackingStatus();
  GenerateFiberBundle();
}

// tell global tractography filter to stop after current step
void QmitkGibbsTrackingView::StopGibbsTracking()
{
  if (m_GlobalTracker.IsNull())
    return;

  mitk::ProgressBar::GetInstance()->Progress(m_GlobalTracker->GetSteps()-m_LastStep);
  m_GlobalTracker->SetAbortTracking(true);
  m_Controls->m_TrackingStop->setEnabled(false);
  m_Controls->m_TrackingStop->setText("Stopping Tractography ...");
}

// update gui elements and generate fiber bundle after tracking is finished
void QmitkGibbsTrackingView::AfterThread()
{
  m_ThreadIsRunning = false;
  m_TrackingTimer->stop();

  UpdateGUI();
  UpdateTrackingStatus();
  GenerateFiberBundle();
  QString paramMessage;
  if(m_Controls->m_ParticleWeightSlider->value()==0)
  {
    m_Controls->m_ParticleWeightLabel->setText(QString::number(m_GlobalTracker->GetParticleWeight()));
    m_Controls->m_ParticleWeightSlider->setValue(m_GlobalTracker->GetParticleWeight()*10000);
    paramMessage += "Particle weight was set to " + QString::number(m_GlobalTracker->GetParticleWeight()) + "\n";
  }
  if(m_Controls->m_ParticleWidthSlider->value()==0)
  {
    m_Controls->m_ParticleWidthLabel->setText(QString::number(m_GlobalTracker->GetParticleWidth()));
    m_Controls->m_ParticleWidthSlider->setValue(m_GlobalTracker->GetParticleWidth()*10);
    paramMessage += "Particle width was set to " + QString::number(m_GlobalTracker->GetParticleWidth()) + " mm\n";
  }
  if(m_Controls->m_ParticleLengthSlider->value()==0)
  {
    m_Controls->m_ParticleLengthLabel->setText(QString::number(m_GlobalTracker->GetParticleLength()));
    m_Controls->m_ParticleLengthSlider->setValue(m_GlobalTracker->GetParticleLength()*10);
    paramMessage += "Particle length was set to " + QString::number(m_GlobalTracker->GetParticleLength()) + " mm\n";
  }

  m_FiberBundleNode = NULL;

  if (paramMessage.length()>0)
    QMessageBox::information(NULL, "Automatically selected parameters", paramMessage);
}

// start tracking timer and update gui elements before tracking is started
void QmitkGibbsTrackingView::BeforeThread()
{
  m_ThreadIsRunning = true;
  m_TrackingTime = QTime::currentTime();
  m_ElapsedTime = 0;
  m_TrackingTimer->start(1000);
  m_LastStep = 0;

  UpdateGUI();
}

// setup gui elements and signal/slot connections
void QmitkGibbsTrackingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkGibbsTrackingViewControls;
    m_Controls->setupUi( parent );

    AdvancedSettings();

    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()) );
    connect( m_Controls->m_TrackingStop, SIGNAL(clicked()), this, SLOT(StopGibbsTracking()) );
    connect( m_Controls->m_TrackingStart, SIGNAL(clicked()), this, SLOT(StartGibbsTracking()) );
    connect( m_Controls->m_SetMaskButton, SIGNAL(clicked()), this, SLOT(SetMask()) );
    connect( m_Controls->m_SetGfaButton, SIGNAL(clicked()), this, SLOT(SetGfaImage()) );
    connect( m_Controls->m_AdvancedSettingsCheckbox, SIGNAL(clicked()), this, SLOT(AdvancedSettings()) );
    connect( m_Controls->m_SaveTrackingParameters, SIGNAL(clicked()), this, SLOT(SaveTrackingParameters()) );
    connect( m_Controls->m_LoadTrackingParameters, SIGNAL(clicked()), this, SLOT(LoadTrackingParameters()) );
    connect( m_Controls->m_IterationsSlider, SIGNAL(valueChanged(int)), this, SLOT(SetIterations(int)) );
    connect( m_Controls->m_ParticleWidthSlider, SIGNAL(valueChanged(int)), this, SLOT(SetParticleWidth(int)) );
    connect( m_Controls->m_ParticleLengthSlider, SIGNAL(valueChanged(int)), this, SLOT(SetParticleLength(int)) );
    connect( m_Controls->m_InExBalanceSlider, SIGNAL(valueChanged(int)), this, SLOT(SetInExBalance(int)) );
    connect( m_Controls->m_FiberLengthSlider, SIGNAL(valueChanged(int)), this, SLOT(SetFiberLength(int)) );
    connect( m_Controls->m_ParticleWeightSlider, SIGNAL(valueChanged(int)), this, SLOT(SetParticleWeight(int)) );
    connect( m_Controls->m_StartTempSlider, SIGNAL(valueChanged(int)), this, SLOT(SetStartTemp(int)) );
    connect( m_Controls->m_EndTempSlider, SIGNAL(valueChanged(int)), this, SLOT(SetEndTemp(int)) );
  }
}

void QmitkGibbsTrackingView::SetInExBalance(int value)
{
  m_Controls->m_InExBalanceLabel->setText(QString::number((float)value/10));
}

void QmitkGibbsTrackingView::SetFiberLength(int value)
{
  m_Controls->m_FiberLengthLabel->setText(QString::number(value));
}

void QmitkGibbsTrackingView::SetParticleWeight(int value)
{
  if (value>0)
  {
    m_Controls->m_ParticleWeightLabel->setText(QString::number((float)value/10000));
    m_Controls->m_GfaFrame->setEnabled(false);
  }
  else
  {
    m_Controls->m_ParticleWeightLabel->setText("auto");
    m_Controls->m_GfaFrame->setEnabled(true);
  }
}

void QmitkGibbsTrackingView::SetStartTemp(int value)
{
  m_Controls->m_StartTempLabel->setText(QString::number((float)value/100));
}

void QmitkGibbsTrackingView::SetEndTemp(int value)
{
  m_Controls->m_EndTempLabel->setText(QString::number((float)value/10000));
}

void QmitkGibbsTrackingView::SetParticleWidth(int value)
{
  if (value>0)
    m_Controls->m_ParticleWidthLabel->setText(QString::number((float)value/10)+" mm");
  else
    m_Controls->m_ParticleWidthLabel->setText("auto");
}

void QmitkGibbsTrackingView::SetParticleLength(int value)
{
  if (value>0)
    m_Controls->m_ParticleLengthLabel->setText(QString::number((float)value/10)+" mm");
  else
    m_Controls->m_ParticleLengthLabel->setText("auto");
}

void QmitkGibbsTrackingView::SetIterations(int value)
{
  switch(value)
  {
  case 0:
    m_Controls->m_IterationsLabel->setText("Iterations: 10^4");
    m_Iterations = 10000;
    break;
  case 1:
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^4");
    m_Iterations = 50000;
    break;
  case 2:
    m_Controls->m_IterationsLabel->setText("Iterations: 10^5");
    m_Iterations = 100000;
    break;
  case 3:
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^5");
    m_Iterations = 500000;
    break;
  case 4:
    m_Controls->m_IterationsLabel->setText("Iterations: 10^6");
    m_Iterations = 1000000;
    break;
  case 5:
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^6");
    m_Iterations = 5000000;
    break;
  case 6:
    m_Controls->m_IterationsLabel->setText("Iterations: 10^7");
    m_Iterations = 10000000;
    break;
  case 7:
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^7");
    m_Iterations = 50000000;
    break;
  case 8:
    m_Controls->m_IterationsLabel->setText("10^8");
    m_Iterations = 100000000;
    break;
  case 9:
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^8");
    m_Iterations = 500000000;
    break;
  case 10:
    m_Controls->m_IterationsLabel->setText("Iterations: 10^9");
    m_Iterations = 1000000000;
    break;
  case 11:
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^9");
    m_Iterations = 5000000000;
    break;
  }

}

void QmitkGibbsTrackingView::StdMultiWidgetAvailable(QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkGibbsTrackingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

// called if datamanager selection changes
void QmitkGibbsTrackingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  m_QBallSelected = false;
  m_FibSelected = false;

  // iterate all selected objects
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::QBallImage*>(node->GetData()) )
    {
      m_QBallSelected = true;
      m_QBallImageNode = node;
    }
    else if (node.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(node->GetData()))
    {
      m_FibSelected = true;
      m_FiberBundleNode = node;
    }
  }

  UpdateGUI();
}

// update gui elements displaying trackings status
void QmitkGibbsTrackingView::UpdateTrackingStatus()
{
  if (m_GlobalTracker.IsNull())
    return;

  m_ElapsedTime += m_TrackingTime.elapsed()/1000;
  m_TrackingTime.restart();
  unsigned long hours = m_ElapsedTime/3600;
  unsigned long minutes = (m_ElapsedTime%3600)/60;
  unsigned long seconds = m_ElapsedTime%60;

  m_Controls->m_ProposalAcceptance->setText(QString::number(m_GlobalTracker->GetProposalAcceptance()*100)+"%");

  m_Controls->m_TrackingTimeLabel->setText( QString::number(hours)+QString("h ")+QString::number(minutes)+QString("m ")+QString::number(seconds)+QString("s") );
  m_Controls->m_NumConnectionsLabel->setText( QString::number(m_GlobalTracker->GetNumConnections()) );
  m_Controls->m_NumParticlesLabel->setText( QString::number(m_GlobalTracker->GetNumParticles()) );
  m_Controls->m_CurrentStepLabel->setText( QString::number(100*(float)m_GlobalTracker->GetCurrentStep()/m_GlobalTracker->GetSteps())+"%" );
  m_Controls->m_AcceptedFibersLabel->setText( QString::number(m_GlobalTracker->GetNumAcceptedFibers()) );
}

// update gui elements (enable/disable elements and set tooltips)
void QmitkGibbsTrackingView::UpdateGUI()
{
  if (!m_ThreadIsRunning && m_QBallSelected)
  {
    m_Controls->m_TrackingStop->setEnabled(false);
    m_Controls->m_TrackingStart->setEnabled(true);
    m_Controls->m_LoadTrackingParameters->setEnabled(true);
    m_Controls->m_MaskFrame->setEnabled(true);
    m_Controls->m_GfaFrame->setEnabled(true);
    m_Controls->m_IterationsSlider->setEnabled(true);
    m_Controls->m_AdvancedFrame->setEnabled(true);
    m_Controls->m_TrackingStop->setText("Stop Tractography");
    m_Controls->m_TrackingStart->setToolTip("Start tractography. No further change of parameters possible.");
    m_Controls->m_TrackingStop->setToolTip("");
  }
  else if (!m_ThreadIsRunning)
  {
    m_Controls->m_TrackingStop->setEnabled(false);
    m_Controls->m_TrackingStart->setEnabled(false);
    m_Controls->m_LoadTrackingParameters->setEnabled(true);
    m_Controls->m_MaskFrame->setEnabled(true);
    m_Controls->m_GfaFrame->setEnabled(true);
    m_Controls->m_IterationsSlider->setEnabled(true);
    m_Controls->m_AdvancedFrame->setEnabled(true);
    m_Controls->m_TrackingStop->setText("Stop Tractography");
    m_Controls->m_TrackingStart->setToolTip("No Q-Ball image selected.");
    m_Controls->m_TrackingStop->setToolTip("");
  }
  else
  {
    m_Controls->m_TrackingStop->setEnabled(true);
    m_Controls->m_TrackingStart->setEnabled(false);
    m_Controls->m_LoadTrackingParameters->setEnabled(false);
    m_Controls->m_MaskFrame->setEnabled(false);
    m_Controls->m_GfaFrame->setEnabled(false);
    m_Controls->m_IterationsSlider->setEnabled(false);
    m_Controls->m_AdvancedFrame->setEnabled(false);
    m_Controls->m_AdvancedFrame->setVisible(false);
    m_Controls->m_AdvancedSettingsCheckbox->setChecked(false);
    m_Controls->m_TrackingStart->setToolTip("Tracking in progress.");
    m_Controls->m_TrackingStop->setToolTip("Stop tracking and display results.");
  }
}

// show/hide advanced settings frame
void QmitkGibbsTrackingView::AdvancedSettings()
{
  m_Controls->m_AdvancedFrame->setVisible(m_Controls->m_AdvancedSettingsCheckbox->isChecked());
}

// set mask image data node
void QmitkGibbsTrackingView::SetMask()
{
  std::vector<mitk::DataNode*> nodes = GetDataManagerSelection();
  if (nodes.empty())
  {
    m_MaskImageNode = NULL;
    m_Controls->m_MaskImageEdit->setText("N/A");
    return;
  }

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
      it != nodes.end();
      ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()))
    {
      m_MaskImageNode = node;
      m_Controls->m_MaskImageEdit->setText(node->GetName().c_str());
      return;
    }
  }
}

// set gfa image data node
void QmitkGibbsTrackingView::SetGfaImage()
{
  std::vector<mitk::DataNode*> nodes = GetDataManagerSelection();
  if (nodes.empty())
  {
    m_GfaImageNode = NULL;
    m_Controls->m_GfaImageEdit->setText("N/A");
    return;
  }

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
      it != nodes.end();
      ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()))
    {
      m_GfaImageNode = node;
      m_Controls->m_GfaImageEdit->setText(node->GetName().c_str());
      return;
    }
  }
}

// cast image to float
template<class InputImageType>
void QmitkGibbsTrackingView::CastToFloat(InputImageType* image, mitk::Image::Pointer outImage)
{
  typedef itk::CastImageFilter<InputImageType, MaskImgType> ItkCastFilter;
  typename ItkCastFilter::Pointer itkCaster = ItkCastFilter::New();
  itkCaster->SetInput(image);
  itkCaster->Update();
  outImage->InitializeByItk(itkCaster->GetOutput());
  outImage->SetVolume(itkCaster->GetOutput()->GetBufferPointer());
}

// check for mask and qbi and start tracking thread
void QmitkGibbsTrackingView::StartGibbsTracking()
{
  if(m_ThreadIsRunning)
  {
    MITK_WARN("QmitkGibbsTrackingView")<<"Thread already running!";
    return;
  }

  if (!m_QBallSelected)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Warning", "Please load and select a qball image before starting image processing.");
    return;
  }

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = m_QBallImageNode->GetData();
  if (!data)
    return;

  // test if this data item is an image or not (could also be a surface or something totally different)
  m_QBallImage = dynamic_cast<mitk::QBallImage*>( data );
  if (m_QBallImage.IsNull())
    return;

  // cast qbi to itk
  m_ItkQBallImage = ItkQBallImgType::New();
  mitk::CastToItkImage<ItkQBallImgType>(m_QBallImage, m_ItkQBallImage);

  // mask image found?
  // catch exceptions thrown by the itkAccess macros
  try{
    if(m_Controls->m_MaskImageEdit->text().compare("N/A") != 0)
    {
      m_MaskImage = 0;
      if (dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData()))

        mitk::CastToItkImage<MaskImgType>(dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData()),
                                          m_MaskImage);
    }
  }
  catch(...)
  {
    QMessageBox::warning(NULL, "Warning", "Incompatible mask image chosen. Processing without masking.");
    //reset mask image
    m_MaskImage = NULL;
  }


  // if no mask image is selected generate it
  if( m_MaskImage.IsNull() )
  {
    m_MaskImage = MaskImgType::New();
    m_MaskImage->SetSpacing( m_ItkQBallImage->GetSpacing() );   // Set the image spacing
    m_MaskImage->SetOrigin( m_ItkQBallImage->GetOrigin() );     // Set the image origin
    m_MaskImage->SetDirection( m_ItkQBallImage->GetDirection() );  // Set the image direction
    m_MaskImage->SetLargestPossibleRegion( m_ItkQBallImage->GetLargestPossibleRegion());
    m_MaskImage->SetBufferedRegion( m_ItkQBallImage->GetLargestPossibleRegion() );
    m_MaskImage->Allocate();

    itk::ImageRegionIterator<MaskImgType> it (m_MaskImage, m_MaskImage->GetLargestPossibleRegion() );
    for (it = it.Begin(); !it.IsAtEnd(); ++it)
    {
      it.Set(1);
    }
  }

  // gfa image found?
  // catch exceptions thrown by the itkAccess macros
  try{
    if(m_Controls->m_GfaImageEdit->text().compare("N/A") != 0)
    {
      m_GfaImage = 0;
      if (dynamic_cast<mitk::Image*>(m_GfaImageNode->GetData()))
        mitk::CastToItkImage<MaskImgType>(dynamic_cast<mitk::Image*>(m_GfaImageNode->GetData()), m_GfaImage);
    }
  }
  catch(...)
  {
    QMessageBox::warning(NULL, "Warning", "Incompatible GFA image chosen. Processing without GFA image.");
    m_GfaImage = NULL;
  }

  unsigned int steps = m_Iterations/10000;
  if (steps<10)
    steps = 10;

  m_LastStep = 0;
  mitk::ProgressBar::GetInstance()->AddStepsToDo(steps);

  // start worker thread
  m_TrackingThread.start(QThread::LowestPriority);
}

// generate mitkFiberBundle from tracking filter output
void QmitkGibbsTrackingView::GenerateFiberBundle()
{
  if (m_GlobalTracker.IsNull() || m_ItkQBallImage.IsNull() || m_QBallImage.IsNull() || (!m_Controls->m_VisualizationCheckbox->isChecked() && m_ThreadIsRunning))
    return;

  m_FiberBundle = mitk::FiberBundle::New();

  typedef std::vector< itk::Point<float, 3> > FiberTractType;
  typedef std::vector< FiberTractType > FiberBundleType;

  FiberBundleType* fiberBundle = m_GlobalTracker->GetFiberBundle();

  for (int i=0; i<fiberBundle->size(); i++)
  {
    FiberTractType* tract = &fiberBundle->at(i);
    for (int j=0; j<tract->size(); j++)
      m_FiberBundle->PushPoint(i, tract->at(j));
  }
  m_FiberBundle->initFiberGroup();

  float bounds[] = {0,0,0};
  bounds[0] = m_ItkQBallImage->GetLargestPossibleRegion().GetSize().GetElement(0);
  bounds[1] = m_ItkQBallImage->GetLargestPossibleRegion().GetSize().GetElement(1);
  bounds[2] = m_ItkQBallImage->GetLargestPossibleRegion().GetSize().GetElement(2);
  m_FiberBundle->SetBounds(bounds);
  m_FiberBundle->SetGeometry(m_QBallImage->GetGeometry());

  if (m_FiberBundleNode.IsNotNull()){
    GetDefaultDataStorage()->Remove(m_FiberBundleNode);
    m_FiberBundleNode = 0;
  }
  m_FiberBundleNode = mitk::DataNode::New();
  m_FiberBundleNode->SetData(m_FiberBundle);

  QString name(m_QBallImageNode->GetName().c_str());
  name += "_FiberBundle";
  m_FiberBundleNode->SetName(name.toStdString());
  m_FiberBundleNode->SetVisibility(true);

  if(m_QBallImageNode.IsNull())
    GetDataStorage()->Add(m_FiberBundleNode);
  else
    GetDataStorage()->Add(m_FiberBundleNode, m_QBallImageNode);
}

// save current tracking paramters as xml file (.gtp)
void QmitkGibbsTrackingView::SaveTrackingParameters()
{
  TiXmlDocument documentXML;
  TiXmlDeclaration* declXML = new TiXmlDeclaration( "1.0", "", "" );
  documentXML.LinkEndChild( declXML );

  TiXmlElement* mainXML = new TiXmlElement("global_tracking_parameter_file");
  mainXML->SetAttribute("file_version",  "0.1");
  documentXML.LinkEndChild(mainXML);

  TiXmlElement* paramXML = new TiXmlElement("parameter_set");
  paramXML->SetAttribute("iterations", QString::number(m_Iterations).toStdString());
  paramXML->SetAttribute("particle_length", QString::number((float)m_Controls->m_ParticleLengthSlider->value()/10).toStdString());
  paramXML->SetAttribute("particle_width", QString::number((float)m_Controls->m_ParticleWidthSlider->value()/10).toStdString());
  paramXML->SetAttribute("particle_weight", QString::number((float)m_Controls->m_ParticleWeightSlider->value()/10000).toStdString());
  paramXML->SetAttribute("temp_start", QString::number((float)m_Controls->m_StartTempSlider->value()/100).toStdString());
  paramXML->SetAttribute("temp_end", QString::number((float)m_Controls->m_EndTempSlider->value()/10000).toStdString());
  paramXML->SetAttribute("inexbalance", QString::number((float)m_Controls->m_InExBalanceSlider->value()/10).toStdString());
  paramXML->SetAttribute("fiber_length", QString::number(m_Controls->m_FiberLengthSlider->value()).toStdString());
  mainXML->LinkEndChild(paramXML);
  QString filename = QFileDialog::getSaveFileName(
        0,
        tr("Save Parameters"),
        QDir::currentPath()+"/param.gtp",
        tr("Global Tracking Parameters (*.gtp)") );

  if(filename.isEmpty() || filename.isNull())
    return;
  if(!filename.endsWith(".gtp"))
    filename += ".gtp";
  documentXML.SaveFile( filename.toStdString() );
}

void QmitkGibbsTrackingView::UpdateIteraionsGUI(unsigned long iterations)
{
  switch(iterations)
  {
  case 10000:
    m_Controls->m_IterationsSlider->setValue(0);
    m_Controls->m_IterationsLabel->setText("Iterations: 10^4");
    break;
  case 50000:
    m_Controls->m_IterationsSlider->setValue(1);
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^4");
    break;
  case 100000:
    m_Controls->m_IterationsSlider->setValue(2);
    m_Controls->m_IterationsLabel->setText("Iterations: 10^5");
    break;
  case 500000:
    m_Controls->m_IterationsSlider->setValue(3);
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^5");
    break;
  case 1000000:
    m_Controls->m_IterationsSlider->setValue(4);
    m_Controls->m_IterationsLabel->setText("Iterations: 10^6");
    break;
  case 5000000:
    m_Controls->m_IterationsSlider->setValue(5);
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^6");
    break;
  case 10000000:
    m_Controls->m_IterationsSlider->setValue(6);
    m_Controls->m_IterationsLabel->setText("Iterations: 10^7");
    break;
  case 50000000:
    m_Controls->m_IterationsSlider->setValue(7);
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^7");
    break;
  case 100000000:
    m_Controls->m_IterationsSlider->setValue(8);
    m_Controls->m_IterationsLabel->setText("Iterations: 10^8");
    break;
  case 500000000:
    m_Controls->m_IterationsSlider->setValue(9);
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^8");
    break;
  case 1000000000:
    m_Controls->m_IterationsSlider->setValue(10);
    m_Controls->m_IterationsLabel->setText("Iterations: 10^9");
    break;
  case 5000000000:
    m_Controls->m_IterationsSlider->setValue(11);
    m_Controls->m_IterationsLabel->setText("Iterations: 5x10^9");
    break;
  }
}

// load current tracking paramters from xml file (.gtp)
void QmitkGibbsTrackingView::LoadTrackingParameters()
{
  QString filename = QFileDialog::getOpenFileName(0, tr("Load Parameters"), QDir::currentPath(), tr("Global Tracking Parameters (*.gtp)") );
  if(filename.isEmpty() || filename.isNull())
    return;

  TiXmlDocument doc( filename.toStdString() );
  doc.LoadFile();

  TiXmlHandle hDoc(&doc);
  TiXmlElement* pElem;
  TiXmlHandle hRoot(0);

  pElem = hDoc.FirstChildElement().Element();
  hRoot = TiXmlHandle(pElem);
  pElem = hRoot.FirstChildElement("parameter_set").Element();

  QString iterations(pElem->Attribute("iterations"));
  m_Iterations = iterations.toULong();
  UpdateIteraionsGUI(m_Iterations);

  QString particleLength(pElem->Attribute("particle_length"));
  float pLength = particleLength.toFloat();
  QString particleWidth(pElem->Attribute("particle_width"));
  float pWidth = particleWidth.toFloat();

  if (pLength==0)
    m_Controls->m_ParticleLengthLabel->setText("auto");
  else
    m_Controls->m_ParticleLengthLabel->setText(particleLength+" mm");
  if (pWidth==0)
    m_Controls->m_ParticleWidthLabel->setText("auto");
  else
    m_Controls->m_ParticleWidthLabel->setText(particleWidth+" mm");

  m_Controls->m_ParticleWidthSlider->setValue(pWidth*10);
  m_Controls->m_ParticleLengthSlider->setValue(pLength*10);

  QString partWeight(pElem->Attribute("particle_weight"));
  m_Controls->m_ParticleWeightSlider->setValue(partWeight.toFloat()*10000);
  m_Controls->m_ParticleWeightLabel->setText(partWeight);

  QString startTemp(pElem->Attribute("temp_start"));
  m_Controls->m_StartTempSlider->setValue(startTemp.toFloat()*100);
  m_Controls->m_StartTempLabel->setText(startTemp);

  QString endTemp(pElem->Attribute("temp_end"));
  m_Controls->m_EndTempSlider->setValue(endTemp.toFloat()*10000);
  m_Controls->m_EndTempLabel->setText(endTemp);

  QString inExBalance(pElem->Attribute("inexbalance"));
  m_Controls->m_InExBalanceSlider->setValue(inExBalance.toFloat()*10);
  m_Controls->m_InExBalanceLabel->setText(inExBalance);


  QString fiberLength(pElem->Attribute("fiber_length"));
  m_Controls->m_FiberLengthSlider->setValue(fiberLength.toInt());
  m_Controls->m_FiberLengthLabel->setText(fiberLength);
}

