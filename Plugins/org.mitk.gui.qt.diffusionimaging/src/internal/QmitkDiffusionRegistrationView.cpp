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

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkDiffusionRegistrationView.h"
#include <QmitkStdMultiWidget.h>

// MITK
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkImageAccessByItk.h>
#include <mitkProgressBar.h>
#include <mitkIOUtil.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>


#include <mitkDWIHeadMotionCorrectionFilter.h>


#define _USE_MATH_DEFINES
#include <math.h>

QmitkRegistrationWorker::QmitkRegistrationWorker(QmitkDiffusionRegistrationView* view)
  : m_View(view)
{

}

void QmitkRegistrationWorker::run()
{

  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::BValueMap BValueMap;

  unsigned int totalImagesCount;
  if( !m_View->m_IsBatch )
  {
    totalImagesCount = m_View->m_SelectedDiffusionNodes.size();
  }
  else
  {
    totalImagesCount = m_View->m_BatchList.size();
  }
  m_View->m_TotalFiles = totalImagesCount;

  QString inputPath = m_View->m_Controls->m_InputFolderTextbox->text();
  QString outputPath = m_View->m_Controls->m_OutputFolderTextbox->text();


  for(unsigned int i=0; i< totalImagesCount; i++)
  {
    if(m_View->m_IsAborted){
      m_View->m_RegistrationThread.quit();
      return;
    }

    m_View->m_CurrentFile = i+1;
    m_View->m_GlobalRegisterer = QmitkDiffusionRegistrationView::DWIHeadMotionCorrectionFilterType::New();

    //mitk::DataNode::Pointer node = m_View->m_SelectedDiffusionNodes.at(i);
    DiffusionImageType::Pointer inImage;
    mitk::DataNode::Pointer node;

    if( !m_View->m_IsBatch )
    {
      node = m_View->m_SelectedDiffusionNodes.at(i);
      inImage = dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData());
    }
    else
    {
      mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage( m_View->m_BatchList.at(i).toStdString() );
      inImage = static_cast<DiffusionImageType*>( inputImage.GetPointer() );
    }


    if(inImage.IsNull())
    {
      MITK_ERROR << "Error occured: can't get input image. \nAborting";
      return;
    }


    m_View->m_GlobalRegisterer->SetInput(inImage);

    try
    {
      m_View->m_GlobalRegisterer->Update();
    }
    catch( mitk::Exception e )
    {
      MITK_ERROR << "Internal error occured: " <<  e.what() << "\nAborting";
    }

    if( m_View->m_GlobalRegisterer->GetIsInValidState() )
    {

      if(! m_View->m_IsBatch)
      {
        DiffusionImageType::Pointer image = m_View->m_GlobalRegisterer->GetOutput();
        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = node->GetName().c_str();
        imageNode->SetName((name+"_MC").toStdString().c_str());
        m_View->GetDataStorage()->Add(imageNode);
      }
      else
      {
        QString name = m_View->m_BatchList.at(i);
        name = name.replace(".dwi", "_MC.dwi", Qt::CaseInsensitive);
        name = name.replace(inputPath, outputPath, Qt::CaseInsensitive);

        try
        {
          mitk::IOUtil::Save(m_View->m_GlobalRegisterer->GetOutput(), name.toStdString().c_str());
        }
        catch( const itk::ExceptionObject& e)
        {
          MITK_ERROR << "Catched exception: " << e.what();
          mitkThrow() << "Failed with exception from subprocess!";
        }
      }
    }

  }
  m_View->m_RegistrationThread.quit();
}


const std::string QmitkDiffusionRegistrationView::VIEW_ID = "org.mitk.views.diffusionregistrationview";

QmitkDiffusionRegistrationView::QmitkDiffusionRegistrationView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_DiffusionImage( NULL )
  , m_ThreadIsRunning(false)
  , m_Steps(100)
  , m_LastStep(0)
  , m_GlobalRegisterer(NULL)
  , m_RegistrationWorker(this)

{

  m_RegistrationWorker.moveToThread(&m_RegistrationThread);
  connect(&m_RegistrationThread, SIGNAL(started()), this, SLOT(BeforeThread()));
  connect(&m_RegistrationThread, SIGNAL(started()), &m_RegistrationWorker, SLOT(run()));
  connect(&m_RegistrationThread, SIGNAL(finished()), this, SLOT(AfterThread()));
  connect(&m_RegistrationThread, SIGNAL(terminated()), this, SLOT(AfterThread()));
  m_RegistrationTimer = new QTimer(this);

}

// Destructor
QmitkDiffusionRegistrationView::~QmitkDiffusionRegistrationView()
{
  delete m_RegistrationTimer;
}

// update Registration status and generate fiber bundle
void QmitkDiffusionRegistrationView::TimerUpdate()
{
  int currentStep = m_GlobalRegisterer->GetCurrentStep();

  mitk::ProgressBar::GetInstance()->Progress(currentStep-m_LastStep);
  UpdateRegistrationStatus();

  m_LastStep = currentStep;
}

// update gui elements after registration is finished
void QmitkDiffusionRegistrationView::AfterThread()
{

  m_ThreadIsRunning = false;
  m_RegistrationTimer->stop();

  mitk::ProgressBar::GetInstance()->Progress(m_GlobalRegisterer->GetSteps()-m_LastStep+1);
  UpdateGUI();


  if( !m_GlobalRegisterer->GetIsInValidState() )
  {
    QMessageBox::critical( NULL,  "Registration", "An internal error occured, or user canceled the Registration.\n Please check the log for details." );
    return;
  }

  UpdateRegistrationStatus();


  m_GlobalRegisterer = 0;

}

// start Registration timer and update gui elements before Registration is started
void QmitkDiffusionRegistrationView::BeforeThread()
{
  m_ThreadIsRunning = true;
  m_RegistrationTime = QTime::currentTime();
  m_ElapsedTime = 0;
  m_RegistrationTimer->start(1000);
  m_LastStep = 0;

  UpdateGUI();
}


void QmitkDiffusionRegistrationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDiffusionRegistrationViewControls;
    m_Controls->setupUi( parent );

    AdvancedSettings();

    connect( m_RegistrationTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()) );
    connect( m_Controls->m_RegistrationStopButton, SIGNAL(clicked()), this, SLOT(StopRegistration()) );
    connect( m_Controls->m_RegistrationStartButton, SIGNAL(clicked()), this, SLOT(StartRegistration()) );
    connect( m_Controls->m_AdvancedSettingsCheckbox, SIGNAL(clicked()), this, SLOT(AdvancedSettings()) );
    connect( m_Controls->m_SelectInputButton, SIGNAL(clicked()), this, SLOT(AddInputFolderName()) );
    connect( m_Controls->m_SelectOutputButton, SIGNAL(clicked()), this, SLOT(AddOutputFolderName()) );
    connect( m_Controls->m_StartBatchButton, SIGNAL(clicked()), this, SLOT(StartBatch()) );

    this->m_Parent = parent;
  }
}

// show/hide advanced settings frame
void QmitkDiffusionRegistrationView::AdvancedSettings()
{
  m_Controls->m_AdvancedFrame->setVisible(m_Controls->m_AdvancedSettingsCheckbox->isChecked());
}


void QmitkDiffusionRegistrationView::OnSelectionChanged( berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes )
{

  if (m_ThreadIsRunning)
    return;

  bool foundDwiVolume = false;
  QString tempSelectedNames = "";
  m_DiffusionImage = NULL;
  m_SelectedDiffusionNodes.clear();

  // iterate selection
  for( int i=0; i<nodes.size(); i++)
  {
    mitk::DataNode::Pointer node = nodes.at(i);

    if( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
    {
      foundDwiVolume = true;
      m_SelectedDiffusionNodes.push_back(node);

      if(m_SelectedDiffusionNodes.size() > 0){tempSelectedNames += "\n";}
      tempSelectedNames += (node->GetName().c_str());
    }

  }


  m_Controls->m_RegistrationStartButton->setEnabled(foundDwiVolume);

  if (foundDwiVolume)
  {
    m_Controls->m_DiffusionImageLabel->setText(tempSelectedNames);
    m_Controls->m_InputData->setTitle("Input Data");
  }
  else
  {
    m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_InputData->setTitle("Please Select Input Data");
  }

  UpdateGUI();

}


// update gui elements displaying Registrations status
void QmitkDiffusionRegistrationView::UpdateRegistrationStatus()
{
  if (m_GlobalRegisterer.IsNull())
    return;

  m_ElapsedTime += m_RegistrationTime.elapsed()/1000;
  m_RegistrationTime.restart();
  unsigned long hours = m_ElapsedTime/3600;
  unsigned long minutes = (m_ElapsedTime%3600)/60;
  unsigned long seconds = m_ElapsedTime%60;

  m_Controls->m_RegistrationTimeLabel->setText( QString::number(hours)+QString("h ")+QString::number(minutes)+QString("m ")+QString::number(seconds)+QString("s") );
  m_Controls->m_CurrentStepLabel->setText( QString::number((int)(100*(float)(m_GlobalRegisterer->GetCurrentStep()-1)/m_GlobalRegisterer->GetSteps()))+"%" );
  m_Controls->m_CurrentFileLabel->setText( QString::number(m_CurrentFile)+" / "+QString::number(m_TotalFiles) );
}

void QmitkDiffusionRegistrationView::UpdateGUI()
{

  if (!m_ThreadIsRunning && (m_SelectedDiffusionNodes.size() > 0) )
  {
    m_Controls->m_RegistrationStopButton->setEnabled(false);
    m_Controls->m_RegistrationStartButton->setEnabled(true);
    m_Controls->m_StartBatchButton->setEnabled(true);
    m_Controls->m_AdvancedFrame->setEnabled(true);
    m_Controls->m_RegistrationStopButton->setText("Stop");
    m_Controls->m_RegistrationStartButton->setToolTip("Start Registration");
    m_Controls->m_RegistrationStopButton->setToolTip("");
  }
  else if (!m_ThreadIsRunning)
  {
    m_Controls->m_RegistrationStopButton->setEnabled(false);
    m_Controls->m_RegistrationStartButton->setEnabled(false);
    m_Controls->m_StartBatchButton->setEnabled(true);
    m_Controls->m_AdvancedFrame->setEnabled(true);
    m_Controls->m_RegistrationStopButton->setText("Stop");
    m_Controls->m_RegistrationStartButton->setToolTip("No Diffusion image selected.");
    m_Controls->m_RegistrationStopButton->setToolTip("");
  }
  else
  {
    m_Controls->m_RegistrationStopButton->setEnabled(true);
    m_Controls->m_RegistrationStartButton->setEnabled(false);
    m_Controls->m_StartBatchButton->setEnabled(false);
    m_Controls->m_AdvancedFrame->setEnabled(false);
    m_Controls->m_RegistrationStartButton->setToolTip("Registration in progress.");
    m_Controls->m_RegistrationStopButton->setToolTip("Cancel Registration");
  }


}

void QmitkDiffusionRegistrationView::SetFocus()
{
  m_Controls->m_RegistrationStartButton->setFocus();
}


void QmitkDiffusionRegistrationView::StartRegistration()
{

  if(m_ThreadIsRunning)
  {
    MITK_WARN("QmitkDiffusionRegistrationView")<<"Thread already running!";
    return;
  }
  m_GlobalRegisterer = NULL;

  if (m_SelectedDiffusionNodes.size()<1)
  {
    QMessageBox::information( NULL, "Warning", "Please load and select a diffusion image before starting image processing.");
    return;
  }

  m_IsBatch = false;
  m_IsAborted = false;

  m_Controls->m_RegistrationStartButton->setEnabled(false);
  m_Controls->m_StartBatchButton->setEnabled(false);

  // start worker thread
  m_RegistrationThread.start(QThread::NormalPriority);

  return;
}

void QmitkDiffusionRegistrationView::StopRegistration()
{

  if (m_GlobalRegisterer.IsNull())
    return;

  m_IsAborted = true;
  m_GlobalRegisterer->SetAbortRegistration(true);
  m_Controls->m_RegistrationStopButton->setEnabled(false);
  m_Controls->m_RegistrationStopButton->setText("Stopping ...");

  return;
}

void QmitkDiffusionRegistrationView::AddInputFolderName()
{

  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( m_Parent, QString("Select the input folder with DWI files within") );
  w->setFileMode( QFileDialog::Directory );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  m_Controls->m_InputFolderTextbox->setText(w->selectedFiles()[0]);
}

void QmitkDiffusionRegistrationView::AddOutputFolderName()
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( m_Parent, QString("Select the output folder") );
  w->setFileMode( QFileDialog::Directory );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  m_Controls->m_OutputFolderTextbox->setText(w->selectedFiles()[0]);
}

void QmitkDiffusionRegistrationView::StartBatch()
{
  QString inputPath = m_Controls->m_InputFolderTextbox->text();
  QString outputPath = m_Controls->m_OutputFolderTextbox->text();

  if(inputPath == outputPath){
    QMessageBox::information( NULL, "Error", "Input and Output folders can't be the same");
    return;
  }

  QStringList list, filters;
  filters<<"*.dwi";
  QDirIterator dirIterator(inputPath,
                           filters,
                           QDir::Files|QDir::NoSymLinks);

  while (dirIterator.hasNext())
  {
    dirIterator.next();

    list.append(dirIterator.fileInfo().absoluteFilePath());
    std::cout << dirIterator.fileInfo().absoluteFilePath().toStdString() << endl;
    m_BatchList = list;

    m_IsBatch = true;
    m_IsAborted = false;

    if(m_ThreadIsRunning)
    {
      MITK_WARN("QmitkDiffusionRegistrationView")<<"Thread already running!";
      return;
    }
    m_GlobalRegisterer = NULL;

    if (m_BatchList.size()<1)
    {
      QMessageBox::information( NULL, "Error", "No diffusion images were found in the selected input folder.");
      return;
    }

    m_Controls->m_RegistrationStartButton->setEnabled(false);
    m_Controls->m_StartBatchButton->setEnabled(false);

    // start worker thread
    m_RegistrationThread.start(QThread::NormalPriority);
  }
}
