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

#include "QmitkUSNavigationMarkerPlacement.h"
#include "ui_QmitkUSNavigationMarkerPlacement.h"

#include "NavigationStepWidgets/QmitkUSNavigationStepCombinedModality.h"
#include "NavigationStepWidgets/QmitkUSNavigationStepMarkerIntervention.h"
#include "NavigationStepWidgets/QmitkUSNavigationStepPlacementPlanning.h"
#include "NavigationStepWidgets/QmitkUSNavigationStepPunctuationIntervention.h"
#include "NavigationStepWidgets/QmitkUSNavigationStepTumourSelection.h"
#include "NavigationStepWidgets/QmitkUSNavigationStepZoneMarking.h"

#include "SettingsWidgets/QmitkUSNavigationCombinedSettingsWidget.h"

#include "mitkIRenderingManager.h"
#include "mitkNodeDisplacementFilter.h"
#include "mitkUSCombinedModality.h"
#include <mitkIOUtil.h>

#include "IO/mitkUSNavigationExperimentLogging.h"
#include "IO/mitkUSNavigationStepTimer.h"

#include <QDateTime>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QTimer>

#include "QmitkRenderWindow.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkStdMultiWidgetEditor.h"
#include "mitkLayoutAnnotationRenderer.h"
#include "mitkCameraController.h"
#include <vtkSmartPointer.h>

// scene serialization
#include <mitkConvert2Dto3DImageFilter.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkSceneIO.h>
#include <mitkSurface.h>

const std::string QmitkUSNavigationMarkerPlacement::VIEW_ID = "org.mitk.views.usmarkerplacement";

const char *QmitkUSNavigationMarkerPlacement::DATANAME_TUMOUR = "Tumour";
const char *QmitkUSNavigationMarkerPlacement::DATANAME_TARGETSURFACE = "Target Surface";
const char *QmitkUSNavigationMarkerPlacement::DATANAME_ZONES = "Zones";
const char *QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS = "Targets";
const char *QmitkUSNavigationMarkerPlacement::DATANAME_TARGETS_PATHS = "Target Paths";
const char *QmitkUSNavigationMarkerPlacement::DATANAME_REACHED_TARGETS = "Reached Targets";

QmitkUSNavigationMarkerPlacement::QmitkUSNavigationMarkerPlacement()
  : m_Parent(nullptr),
  m_NavigationSteps(),
  m_UpdateTimer(new QTimer(this)),
  m_ImageAndNavigationDataLoggingTimer(new QTimer(this)),
  m_StdMultiWidget(nullptr),
  m_CombinedModality(nullptr),
  m_ReinitAlreadyDone(false),
  m_IsExperimentRunning(false),
  m_CurrentApplicationName(),
  m_NavigationStepTimer(mitk::USNavigationStepTimer::New()),
  m_ExperimentLogging(mitk::USNavigationExperimentLogging::New()),
  m_IconRunning(QPixmap(":/USNavigation/record.png")),
  m_IconNotRunning(QPixmap(":/USNavigation/record-gray.png")),
  m_ResultsDirectory(),
  m_ExperimentName(),
  m_ExperimentResultsSubDirectory(),
  m_NavigationStepNames(),
  m_LoggingBackend(),
  m_USImageLoggingFilter(mitk::USImageLoggingFilter::New()),
  m_NavigationDataRecorder(mitk::NavigationDataRecorder::New()),
  m_TargetNodeDisplacementFilter(nullptr),
  m_AblationZonesDisplacementFilter(mitk::NodeDisplacementFilter::New()),
  m_AblationZonesVector(),
  m_NeedleIndex(0),
  m_MarkerIndex(1),
  m_SceneNumber(1),
  m_WarnOverlay(mitk::TextAnnotation2D::New()),
  m_NavigationDataSource(nullptr),
  m_CurrentStorage(nullptr),
  m_ListenerDeviceChanged(this, &QmitkUSNavigationMarkerPlacement::OnCombinedModalityPropertyChanged),
  ui(new Ui::QmitkUSNavigationMarkerPlacement )
{
  connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
  connect(
    m_ImageAndNavigationDataLoggingTimer, SIGNAL(timeout()), this, SLOT(OnImageAndNavigationDataLoggingTimeout()));

  // scale running (and not running) icon the specific height
  m_IconRunning = m_IconRunning.scaledToHeight(20, Qt::SmoothTransformation);
  m_IconNotRunning = m_IconNotRunning.scaledToHeight(20, Qt::SmoothTransformation);

  // set prefix for experiment logging (only keys with this prefix are taken
  // into consideration
  m_ExperimentLogging->SetKeyPrefix("USNavigation::");

  m_UpdateTimer->start(33); // every 33 Milliseconds = 30 Frames/Second
}

QmitkUSNavigationMarkerPlacement::~QmitkUSNavigationMarkerPlacement()
{
  // remove listener for ultrasound device changes
  if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetUltrasoundDevice().IsNotNull())
  {
    m_CombinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
  }

  // remove listener for ultrasound device changes
  if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetUltrasoundDevice().IsNotNull())
  {
    m_CombinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
  }

  delete ui;
}

void QmitkUSNavigationMarkerPlacement::OnChangeAblationZone(int id, int newSize)
{
  if ((static_cast<int>(m_AblationZonesVector.size()) < id) || (id < 0))
  {
    return;
  }

  MITK_INFO << "Ablation Zone " << id << " changed, new size: " << newSize;

  // create a vtk sphere with given radius
  vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkSphere->SetRadius(newSize / 2);
  vtkSphere->SetCenter(0, 0, 0);
  vtkSphere->SetPhiResolution(20);
  vtkSphere->SetThetaResolution(20);
  vtkSphere->Update();

  mitk::Surface::Pointer zoneSurface = dynamic_cast<mitk::Surface *>(m_AblationZonesVector.at(id)->GetData());
  zoneSurface->SetVtkPolyData(vtkSphere->GetOutput());
}

void QmitkUSNavigationMarkerPlacement::OnAddAblationZone(int size)
{
  m_AblationZonesDisplacementFilter->SetInitialReferencePose(
    m_CombinedModality->GetNavigationDataSource()->GetOutput(m_MarkerIndex));
  mitk::DataNode::Pointer NewAblationZone = mitk::DataNode::New();

  mitk::Point3D origin = m_CombinedModality->GetNavigationDataSource()->GetOutput(m_NeedleIndex)->GetPosition();

  MITK_INFO("USNavigationLogging") << "Ablation Zone Added, initial size: " << size << ", origin: " << origin;

  mitk::Surface::Pointer zone = mitk::Surface::New();

  // create a vtk sphere with given radius
  vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkSphere->SetRadius(size / 2);
  vtkSphere->SetCenter(0, 0, 0);
  vtkSphere->SetPhiResolution(20);
  vtkSphere->SetThetaResolution(20);
  vtkSphere->Update();
  zone->SetVtkPolyData(vtkSphere->GetOutput());

  // set vtk sphere and origin to data node (origin must be set
  // again, because of the new sphere set as data)
  NewAblationZone->SetData(zone);
  NewAblationZone->GetData()->GetGeometry()->SetOrigin(origin);
  mitk::Color SphereColor = mitk::Color();
  // default color
  SphereColor[0] = 102;
  SphereColor[1] = 0;
  SphereColor[2] = 204;
  NewAblationZone->SetColor(SphereColor);
  NewAblationZone->SetOpacity(0.3);

  // set name of zone
  std::stringstream name;
  name << "Ablation Zone" << m_AblationZonesVector.size();
  NewAblationZone->SetName(name.str());

  // add zone to filter
  m_AblationZonesDisplacementFilter->AddNode(NewAblationZone);
  m_AblationZonesVector.push_back(NewAblationZone);
  this->GetDataStorage()->Add(NewAblationZone);
}

void QmitkUSNavigationMarkerPlacement::CreateQtPartControl(QWidget *parent)
{
  m_Parent = parent;
  ui->setupUi(parent);

  connect(ui->navigationProcessWidget,
    SIGNAL(SignalCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>)),
    this,
    SLOT(OnCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>)));

  connect(ui->navigationProcessWidget,
    SIGNAL(SignalSettingsChanged(itk::SmartPointer<mitk::DataNode>)),
    this,
    SLOT(OnSettingsChanged(itk::SmartPointer<mitk::DataNode>)));

  connect(ui->navigationProcessWidget,
    SIGNAL(SignalActiveNavigationStepChanged(int)),
    this,
    SLOT(OnActiveNavigationStepChanged(int)));

  connect(ui->navigationProcessWidget,
    SIGNAL(SignalActiveNavigationStepChangeRequested(int)),
    this,
    SLOT(OnNextNavigationStepInitialization(int)));

  connect(ui->startExperimentButton, SIGNAL(clicked()), this, SLOT(OnStartExperiment()));
  connect(ui->finishExperimentButton, SIGNAL(clicked()), this, SLOT(OnFinishExperiment()));
  connect(ui->m_enableNavigationLayout, SIGNAL(clicked()), this, SLOT(OnChangeLayoutClicked()));
  connect(ui->m_RenderWindowSelection, SIGNAL(valueChanged(int)), this, SLOT(OnRenderWindowSelection()));
  connect(ui->m_RefreshView, SIGNAL(clicked()), this, SLOT(OnRefreshView()));

  connect(ui->navigationProcessWidget,
    SIGNAL(SignalIntermediateResult(const itk::SmartPointer<mitk::DataNode>)),
    this,
    SLOT(OnIntermediateResultProduced(const itk::SmartPointer<mitk::DataNode>)));

  ui->navigationProcessWidget->SetDataStorage(this->GetDataStorage());

  // indicate that no experiment is running at start
  ui->runningLabel->setPixmap(m_IconNotRunning);

  ui->navigationProcessWidget->SetSettingsWidget(new QmitkUSNavigationCombinedSettingsWidget(m_Parent));
}

void QmitkUSNavigationMarkerPlacement::OnCombinedModalityPropertyChanged(const std::string &key, const std::string &)
{
  if (key == mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH)
  {
    m_ReinitAlreadyDone = false;
    this->ReinitOnImage();

    if (m_CombinedModality.IsNotNull() && !m_CombinedModality->GetIsCalibratedForCurrentStatus())
    {
      mitk::LayoutAnnotationRenderer::AddAnnotation(
        m_WarnOverlay.GetPointer(), "stdmulti.widget1", mitk::LayoutAnnotationRenderer::TopLeft);
      MITK_WARN << "No calibration available for the selected ultrasound image depth.";
    }
  }
}

void QmitkUSNavigationMarkerPlacement::SetFocus()
{
  this->ReinitOnImage();
}

void QmitkUSNavigationMarkerPlacement::OnTimeout()
{
  if (!m_StdMultiWidget)
  {
    // try to get the standard multi widget if it couldn't be got before
    mitk::IRenderWindowPart *renderWindow = this->GetRenderWindowPart();

    QmitkStdMultiWidgetEditor *multiWidgetEditor = dynamic_cast<QmitkStdMultiWidgetEditor *>(renderWindow);

    // if there is a standard multi widget now, disable the level window and
    // change the layout to 2D up and 3d down
    if (multiWidgetEditor)
    {
      m_StdMultiWidget = multiWidgetEditor->GetStdMultiWidget();
      SetTwoWindowView();
    }

    this->CreateOverlays();
  }

  if (m_CombinedModality.IsNotNull() &&
    !this->m_CombinedModality->GetIsFreezed()) // if the combined modality is freezed: do nothing
  {
    ui->navigationProcessWidget->UpdateNavigationProgress();
    m_AblationZonesDisplacementFilter->Update();

    // update the 3D window only every fourth time to speed up the rendering (at least in 2D)
    this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);

    // make sure that a reinit was performed on the image
    this->ReinitOnImage();
  }
}

void QmitkUSNavigationMarkerPlacement::OnEnableNavigationLayout()
{
  MITK_INFO << "Navigation Layout";
  // try to get the standard multi widget if it couldn't be got before
  mitk::IRenderWindowPart *renderWindow = this->GetRenderWindowPart();

  QmitkStdMultiWidgetEditor *multiWidgetEditor = dynamic_cast<QmitkStdMultiWidgetEditor *>(renderWindow);

  // if there is a standard multi widget now, disable the level window and
  // change the layout to 2D up and 3d down
  if (multiWidgetEditor)
  {
    m_StdMultiWidget = multiWidgetEditor->GetStdMultiWidget();
    SetTwoWindowView();
  }
}

void QmitkUSNavigationMarkerPlacement::OnRenderWindowSelection()
{
  SetTwoWindowView();
}

void QmitkUSNavigationMarkerPlacement::OnRefreshView()
{
  if (!ui->m_enableNavigationLayout->isChecked())
    OnResetStandardLayout();
  else
  {
    //Reinit the US Image Stream (this might be broken if there was a global reinit somewhere...)
    try
    {
      mitk::RenderingManager::GetInstance()->InitializeViews(//Reinit
        this->GetDataStorage()//GetDataStorage
        ->GetNamedNode("US Support Viewing Stream")->GetData()->GetTimeGeometry());//GetNode
    }
    catch (...)
    {
      MITK_DEBUG << "No reinit possible";
    }
    SetTwoWindowView();
  }
}

void QmitkUSNavigationMarkerPlacement::SetTwoWindowView()
{
  if (m_StdMultiWidget)
  {
    m_StdMultiWidget->DisableStandardLevelWindow();
    int i, j, k;
    switch (this->ui->m_RenderWindowSelection->value())
    {
    case 1:
      mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetCameraController()->SetViewToCaudal();
      i = 2; j = 3; //other windows
      k = 1;
      break;
    case 2:
      mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetCameraController()->SetViewToSinister();
      i = 1; j = 3;
      k = 2;
      break;
    case 3:
      mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetCameraController()->SetViewToAnterior();
      i = 2; j = 1;
      k = 3;
      break;
    default:
      return;
    }
    m_StdMultiWidget->changeLayoutTo2DUpAnd3DDown(k);
    ////Crosshair invisible in 3D view
    this->GetDataStorage()->GetNamedNode("stdmulti.widget" + std::to_string(i) + ".plane")->
      SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));
    this->GetDataStorage()->GetNamedNode("stdmulti.widget" + std::to_string(j) + ".plane")->
      SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));
    this->GetDataStorage()->GetNamedNode("stdmulti.widget" + std::to_string(k) + ".plane")->
      SetBoolProperty("visible", true, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));
    this->GetDataStorage()->GetNamedNode("stdmulti.widget" + std::to_string(i) + ".plane")->
      SetIntProperty("Crosshair.Gap Size", 0);
    this->GetDataStorage()->GetNamedNode("stdmulti.widget" + std::to_string(j) + ".plane")->
      SetIntProperty("Crosshair.Gap Size", 0);
  }
}

void QmitkUSNavigationMarkerPlacement::OnResetStandardLayout()
{
  //reset render windows
  mitk::DataNode::Pointer widget1 = this->GetDataStorage()->GetNamedNode("stdmulti.widget1.plane");
  if (widget1.IsNotNull()) { widget1->SetVisibility(true); }
  mitk::DataNode::Pointer widget2 = this->GetDataStorage()->GetNamedNode("stdmulti.widget2.plane");
  if (widget2.IsNotNull()) { widget2->SetVisibility(true); }
  mitk::DataNode::Pointer widget3 = this->GetDataStorage()->GetNamedNode("stdmulti.widget3.plane");
  if (widget3.IsNotNull()) { widget3->SetVisibility(true); }
  m_StdMultiWidget->changeLayoutToDefault();
}

void QmitkUSNavigationMarkerPlacement::OnChangeLayoutClicked()
{
  if (ui->m_enableNavigationLayout->isChecked()) OnEnableNavigationLayout();
  else OnResetStandardLayout();
}

void QmitkUSNavigationMarkerPlacement::OnImageAndNavigationDataLoggingTimeout()
{
  // update filter for logging navigation data and ultrasound images
  if (m_CombinedModality.IsNotNull())
  {
    m_NavigationDataRecorder->Update();
    // get last messages for logging filer and store them
    std::vector<std::string> messages = m_LoggingBackend.GetNavigationMessages();
    std::string composedMessage = "";
    for (std::size_t i = 0; i < messages.size(); i++)
    {
      composedMessage += messages.at(i);
    }
    m_USImageLoggingFilter->AddMessageToCurrentImage(composedMessage);
    m_LoggingBackend.ClearNavigationMessages();
    // update logging filter
    m_USImageLoggingFilter->Update();
  }
}

void QmitkUSNavigationMarkerPlacement::OnStartExperiment()
{
  // get name for the experiment by a QInputDialog
  bool ok;
  if (m_ExperimentName.isEmpty())
  { // default: current date
    m_ExperimentName = QString::number(QDateTime::currentDateTime().date().year()) + "_" +
      QString::number(QDateTime::currentDateTime().date().month()) + "_" +
      QString::number(QDateTime::currentDateTime().date().day()) + "_experiment_" +
      QString::number(QDateTime::currentDateTime().time().hour()) + "." +
      QString::number(QDateTime::currentDateTime().time().minute());
  }
  m_ExperimentName = QInputDialog::getText(
    m_Parent, QString("Experiment Name"), QString("Name of the Experiment"), QLineEdit::Normal, m_ExperimentName, &ok);
  MITK_INFO("USNavigationLogging") << "Experiment started: " << m_ExperimentName.toStdString();
  if (ok && !m_ExperimentName.isEmpty())
  {
    // display error message and call the function recursivly if a directory
    // with the given name already exists
    QDir experimentResultsDir(m_ResultsDirectory + QDir::separator() + m_ExperimentName);
    if (experimentResultsDir.exists())
    {
      QMessageBox::critical(
        m_Parent, "Results Directory Exists", "The result directory already exists.\nPlease choose an other name.");
      this->OnStartExperiment();
    }
    else
    {
      QDir(m_ResultsDirectory).mkdir(m_ExperimentName);
      m_ExperimentResultsSubDirectory = m_ResultsDirectory + QDir::separator() + m_ExperimentName;

      // experiment is running now
      ui->runningLabel->setPixmap(m_IconRunning);
      ui->navigationProcessWidget->EnableInteraction(true);

      // (re)start timer for navigation step durations
      m_NavigationStepTimer->Reset();
      m_NavigationStepTimer->SetOutputFileName(
        QString(m_ExperimentResultsSubDirectory + QDir::separator() + QString("durations.cvs")).toStdString());
      m_NavigationStepTimer->SetActiveIndex(0, m_NavigationSteps.at(0)->GetTitle().toStdString());

      ui->finishExperimentButton->setEnabled(true);
      ui->startExperimentButton->setDisabled(true);

      // initialize and register logging backend
      QString loggingFilename = m_ExperimentResultsSubDirectory + QDir::separator() + "logging.txt";
      m_LoggingBackend.SetOutputFileName(loggingFilename.toStdString());
      mbilog::RegisterBackend(&m_LoggingBackend);

      // initialize and start navigation data recorder form xml recording
      m_NavigationDataRecorder->StartRecording();

      m_IsExperimentRunning = true;

      m_ImageAndNavigationDataLoggingTimer->start(1000);

      // (re)start experiment logging and set output file name
      m_ExperimentLogging->Reset();
      m_ExperimentLogging->SetFileName(
        QString(m_ExperimentResultsSubDirectory + QDir::separator() + "experiment-logging.xml").toStdString());
    }
  }
}

void QmitkUSNavigationMarkerPlacement::OnFinishExperiment()
{
  this->WaitCursorOn();

  MITK_INFO("USNavigationLogging") << "Experiment finished!";
  MITK_INFO("USNavigationLogging")
    << "Position/Orientation of needle tip: "
    << (dynamic_cast<mitk::NavigationData *>(m_CombinedModality->GetTrackingDevice()->GetOutput(0)))->GetPosition();
  MITK_INFO("USNavigationLogging")
    << "Position of target: " << m_TargetNodeDisplacementFilter->GetRawDisplacementNavigationData(0)->GetPosition();
  MITK_INFO("USNavigationLogging") << "Total duration: " << m_NavigationStepTimer->GetTotalDuration();

  ui->navigationProcessWidget->FinishCurrentNavigationStep();
  m_ImageAndNavigationDataLoggingTimer->stop();

  ui->runningLabel->setPixmap(m_IconNotRunning);
  ui->navigationProcessWidget->EnableInteraction(false);

  m_NavigationStepTimer->Stop();

  // make sure that the navigation process will be start from beginning at the
  // next experiment
  ui->navigationProcessWidget->ResetNavigationProcess();

  ui->finishExperimentButton->setDisabled(true);
  ui->startExperimentButton->setEnabled(true);

  MITK_INFO("USNavigationLogging") << "Writing logging data to " << m_ExperimentResultsSubDirectory.toStdString();
  //  save ultrasound images to the file system
  QDir(m_ExperimentResultsSubDirectory).mkdir("ImageStream");
  m_USImageLoggingFilter->Update();
  m_USImageLoggingFilter->SetImageFilesExtension(".jpg");
  m_USImageLoggingFilter->SaveImages(
    QString(m_ExperimentResultsSubDirectory + QDir::separator() + "ImageStream" + QDir::separator()).toStdString());
  m_USImageLoggingFilter = mitk::USImageLoggingFilter::New();

  m_NavigationDataRecorder->StopRecording();

  // Write data to csv and xml file
  mitk::IOUtil::Save(
    m_NavigationDataRecorder->GetNavigationDataSet(),
    (QString(m_ExperimentResultsSubDirectory + QDir::separator() + "navigation-data.xml").toStdString().c_str()));
  mitk::IOUtil::Save(
    m_NavigationDataRecorder->GetNavigationDataSet(),
    (QString(m_ExperimentResultsSubDirectory + QDir::separator() + "navigation-data.csv").toStdString().c_str()));

  // write logged navigation data messages to separate file
  std::stringstream csvNavigationMessagesFilename;
  csvNavigationMessagesFilename << m_ExperimentResultsSubDirectory.toStdString() << QDir::separator().toLatin1()
    << "CSVNavigationMessagesLogFile.csv";
  MITK_INFO("USNavigationLogging") << "Writing logged navigation messages to separate csv file: "
    << csvNavigationMessagesFilename.str();
  m_LoggingBackend.WriteCSVFileWithNavigationMessages(csvNavigationMessagesFilename.str());

  mbilog::UnregisterBackend(&m_LoggingBackend);

  m_IsExperimentRunning = false;

  m_ImageAndNavigationDataLoggingTimer->stop();
  m_CombinedModality = nullptr;

  // reset scene number for next experiment
  m_SceneNumber = 1;

  this->WaitCursorOff();
  MITK_INFO("USNavigationLogging") << "Finished!";
}

void QmitkUSNavigationMarkerPlacement::OnCombinedModalityChanged(
  itk::SmartPointer<mitk::USCombinedModality> combinedModality)
{
  // remove old listener for ultrasound device changes
  if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetUltrasoundDevice().IsNotNull())
  {
    m_CombinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
  }

  m_CombinedModality = combinedModality;
  m_ReinitAlreadyDone = false;

  // add a listener for ultrasound device changes
  if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetUltrasoundDevice().IsNotNull())
  {
    m_CombinedModality->GetUltrasoundDevice()->AddPropertyChangedListener(m_ListenerDeviceChanged);
  }

  // update navigation data recorder for using the new combined modality
  mitk::NavigationDataSource::Pointer navigationDataSource = combinedModality->GetNavigationDataSource();
  m_NavigationDataRecorder->ConnectTo(navigationDataSource);
  m_NavigationDataRecorder->ResetRecording();

  // update ultrasound image logging filter for using the new combined modality
  mitk::USDevice::Pointer ultrasoundImageSource = combinedModality->GetUltrasoundDevice();
  for (unsigned int n = 0; n < ultrasoundImageSource->GetNumberOfIndexedOutputs(); ++n)
  {
    m_USImageLoggingFilter->SetInput(n, ultrasoundImageSource->GetOutput(n));
  }

  // update ablation zone filter for using the new combined modality
  for (unsigned int n = 0; n < navigationDataSource->GetNumberOfIndexedOutputs(); ++n)
  {
    m_AblationZonesDisplacementFilter->SetInput(n, navigationDataSource->GetOutput(n));
  }
  m_AblationZonesDisplacementFilter->SelectInput(m_MarkerIndex);

  // make sure that a reinit is done for the new images
  this->ReinitOnImage();
}

void QmitkUSNavigationMarkerPlacement::OnSettingsChanged(itk::SmartPointer<mitk::DataNode> settings)
{
  std::string applicationName;
  if (!settings->GetStringProperty("settings.application", applicationName))
  {
    // set default application if the string property is not available
    applicationName = "Marker Placement";
  }

  // create navigation step widgets according to the selected application
  if (applicationName != m_CurrentApplicationName)
  {
    m_CurrentApplicationName = applicationName;

    QmitkUSNavigationProcessWidget::NavigationStepVector navigationSteps;
    if (applicationName == "Puncture")
    {
      QmitkUSNavigationStepCombinedModality* stepCombinedModality =
        new QmitkUSNavigationStepCombinedModality(m_Parent);
      QmitkUSNavigationStepTumourSelection* stepTumourSelection =
        new QmitkUSNavigationStepTumourSelection(m_Parent);
      stepTumourSelection->SetTargetSelectionOptional(true);
      m_TargetNodeDisplacementFilter = stepTumourSelection->GetTumourNodeDisplacementFilter();
      QmitkUSNavigationStepZoneMarking* stepZoneMarking =
        new QmitkUSNavigationStepZoneMarking(m_Parent);
      QmitkUSNavigationStepPunctuationIntervention* stepIntervention =
        new QmitkUSNavigationStepPunctuationIntervention(m_Parent);

      connect(stepIntervention, SIGNAL(AddAblationZoneClicked(int)), this, SLOT(OnAddAblationZone(int)));
      connect(stepIntervention, SIGNAL(AblationZoneChanged(int, int)), this, SLOT(OnChangeAblationZone(int, int)));

      m_NavigationStepNames = std::vector<QString>();
      navigationSteps.push_back(stepCombinedModality);
      m_NavigationStepNames.push_back("Combined Modality Initialization");
      navigationSteps.push_back(stepTumourSelection);
      m_NavigationStepNames.push_back("Target Selection");
      navigationSteps.push_back(stepZoneMarking);
      m_NavigationStepNames.push_back("Critical Structure Marking");
      navigationSteps.push_back(stepIntervention);
      m_NavigationStepNames.push_back("Intervention");
    }
    else if (applicationName == "Marker Placement")
    {
      QmitkUSNavigationStepCombinedModality *stepCombinedModality = new QmitkUSNavigationStepCombinedModality(m_Parent);
      QmitkUSNavigationStepTumourSelection *stepTumourSelection = new QmitkUSNavigationStepTumourSelection(m_Parent);
      m_TargetNodeDisplacementFilter = stepTumourSelection->GetTumourNodeDisplacementFilter();
      QmitkUSNavigationStepZoneMarking *stepZoneMarking = new QmitkUSNavigationStepZoneMarking(m_Parent);
      QmitkUSNavigationStepPlacementPlanning *stepPlacementPlanning =
        new QmitkUSNavigationStepPlacementPlanning(m_Parent);
      QmitkUSNavigationStepMarkerIntervention *stepMarkerIntervention =
        new QmitkUSNavigationStepMarkerIntervention(m_Parent);

      m_NavigationStepNames = std::vector<QString>();
      navigationSteps.push_back(stepCombinedModality);
      m_NavigationStepNames.push_back("Combined Modality Initialization");
      navigationSteps.push_back(stepTumourSelection);
      m_NavigationStepNames.push_back("Target Selection");
      navigationSteps.push_back(stepZoneMarking);
      m_NavigationStepNames.push_back("Critical Structure Marking");
      navigationSteps.push_back(stepPlacementPlanning);
      m_NavigationStepNames.push_back("Placement Planning");
      navigationSteps.push_back(stepMarkerIntervention);
      m_NavigationStepNames.push_back("Marker Intervention");
    }

    // set navigation step widgets to the process widget
    ui->navigationProcessWidget->SetNavigationSteps(navigationSteps);

    for (QmitkUSNavigationProcessWidget::NavigationStepIterator it = m_NavigationSteps.begin();
      it != m_NavigationSteps.end();
      ++it)
    {
      delete *it;
    }
    m_NavigationSteps.clear();
    m_NavigationSteps = navigationSteps;
  }

  // initialize gui according to the experiment mode setting
  bool experimentMode = false;
  settings->GetBoolProperty("settings.experiment-mode", experimentMode);
  ui->startExperimentButton->setVisible(experimentMode);
  ui->finishExperimentButton->setVisible(experimentMode);
  ui->runningLabel->setVisible(experimentMode);
  if (experimentMode && !m_IsExperimentRunning)
  {
    ui->navigationProcessWidget->ResetNavigationProcess();
    ui->navigationProcessWidget->EnableInteraction(false);
    ui->runningLabel->setPixmap(m_IconNotRunning);
  }
  else if (!experimentMode)
  {
    if (m_IsExperimentRunning)
    {
      this->OnFinishExperiment();
    }
    ui->navigationProcessWidget->EnableInteraction(true);
  }

  // get the results directory from the settings and use home directory if
  // there is no results directory configured
  std::string resultsDirectory;
  if (settings->GetStringProperty("settings.experiment-results-directory", resultsDirectory))
  {
    m_ResultsDirectory = QString::fromStdString(resultsDirectory);
  }
  else
  {
    m_ResultsDirectory = QDir::homePath();
  }

  // make sure that the results directory exists
  QDir resultsDirectoryQDir = QDir(m_ResultsDirectory);
  if (!resultsDirectoryQDir.exists())
  {
    resultsDirectoryQDir.mkpath(m_ResultsDirectory);
  }

  MITK_INFO("USNavigation") << "Results Directory: " << m_ResultsDirectory.toStdString();
}

void QmitkUSNavigationMarkerPlacement::OnActiveNavigationStepChanged(int index)
{
  // update navigation step timer each time the active navigation step changes
  m_NavigationStepTimer->SetActiveIndex(index, m_NavigationSteps.at(index)->GetTitle().toStdString());
  if (static_cast<int>(m_NavigationStepNames.size()) <= index)
  {
    MITK_INFO("USNavigationLogging") << "Someting went wrong: unknown navigation step!";
  }
  else
  {
    MITK_INFO("USNavigationLogging") << "Navigation step finished/changed, next step: "
      << this->m_NavigationStepNames.at(index).toStdString()
      << "; duration until now: " << m_NavigationStepTimer->GetTotalDuration();
  }
}

void QmitkUSNavigationMarkerPlacement::OnNextNavigationStepInitialization(int index)
{
  MITK_DEBUG << "Next Step: " << m_NavigationSteps.at(index)->GetTitle().toStdString();

  if (m_NavigationSteps.at(index)->GetTitle().toStdString() == "Computer-assisted Intervention")
  {
    QmitkUSNavigationStepPunctuationIntervention* navigationStepPunctuationIntervention = static_cast<QmitkUSNavigationStepPunctuationIntervention*>(m_NavigationSteps.at(index));
    if (navigationStepPunctuationIntervention != nullptr)
    {
      if (m_CurrentStorage.IsNull()) { this->UpdateToolStorage(); }
      if (m_CurrentStorage.IsNull() || (m_CurrentStorage->GetTool(m_NeedleIndex).IsNull()))
      {
        MITK_WARN << "Found null pointer when setting the tool axis, aborting";
      }
      else
      {
        navigationStepPunctuationIntervention->SetNeedleMetaData(m_CurrentStorage->GetTool(m_NeedleIndex));
        MITK_DEBUG << "Needle axis vector: " << m_CurrentStorage->GetTool(m_NeedleIndex)->GetToolAxis();
      }
    }
  }
}

void QmitkUSNavigationMarkerPlacement::OnIntermediateResultProduced(const itk::SmartPointer<mitk::DataNode> resultsNode)
{
  // intermediate results only matter during an experiment
  if (!m_IsExperimentRunning)
  {
    return;
  }

  this->WaitCursorOn();

  // set results node to the experiment logging (for saving contents to the
  // file system)
  m_ExperimentLogging->SetResult(resultsNode);

  std::string resultsName;
  if (!resultsNode->GetName(resultsName))
  {
    MITK_WARN << "Could not get name of current results node.";
    return;
  }

  // save the mitk scene
  std::string scenefile = QString(m_ExperimentResultsSubDirectory + QDir::separator() +
    QString("Scene %1 - ").arg(m_SceneNumber++, 2, 10, QChar('0')) +
    QString::fromStdString(resultsName).replace(":", "_") + ".mitk")
    .toStdString();
  MITK_INFO << "Saving Scene File: " << scenefile;

  mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();
  mitk::NodePredicateNot::Pointer isNotHelperObject =
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
  mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = this->GetDataStorage()->GetSubset(isNotHelperObject);

  this->Convert2DImagesTo3D(nodesToBeSaved);

  sceneIO->SaveScene(nodesToBeSaved, this->GetDataStorage(), scenefile);

  this->WaitCursorOff();
}

void QmitkUSNavigationMarkerPlacement::ReinitOnImage()
{
  if (!m_ReinitAlreadyDone && m_CombinedModality.IsNotNull())
  {
    // make sure that the output is already calibrated correctly
    // (if the zoom level was changed recently)
    m_CombinedModality->Modified();
    m_CombinedModality->Update();

    mitk::Image::Pointer image = m_CombinedModality->GetOutput();
    if (image.IsNotNull() && image->IsInitialized())
    {
      // make a reinit on the ultrasound image
      mitk::IRenderWindowPart *renderWindow = this->GetRenderWindowPart();
      if (renderWindow != NULL && image->GetTimeGeometry()->IsValid())
      {
        renderWindow->GetRenderingManager()->InitializeViews(
          image->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
        renderWindow->GetRenderingManager()->RequestUpdateAll();
      }

      this->RequestRenderWindowUpdate();

      m_ReinitAlreadyDone = true;
    }
  }
}

void QmitkUSNavigationMarkerPlacement::Convert2DImagesTo3D(mitk::DataStorage::SetOfObjects::ConstPointer nodes)
{
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it != nodes->End(); ++it)
  {
    if (it->Value()->GetData() && strcmp(it->Value()->GetData()->GetNameOfClass(), "Image") == 0)
    {
      // convert image to 3d image if it is 2d at the moment
      mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(it->Value()->GetData());
      if (image.IsNotNull() && image->GetDimension() == 2 && !image->GetGeometry()->Is2DConvertable())
      {
        mitk::Convert2Dto3DImageFilter::Pointer convert2DTo3DImageFilter = mitk::Convert2Dto3DImageFilter::New();
        convert2DTo3DImageFilter->SetInput(image);
        convert2DTo3DImageFilter->Update();
        it->Value()->SetData(convert2DTo3DImageFilter->GetOutput());
      }
    }
  }
}

void QmitkUSNavigationMarkerPlacement::CreateOverlays()
{
  // initialize warning overlay (and do not display it, yet)
  m_WarnOverlay->SetText("Warning: No calibration available for current depth.");

  // set position and font size for the text overlay
  // (nonesense postition as a layouter is used, but it ignored
  // the overlay without setting a position here)
  mitk::Point2D overlayPosition;
  overlayPosition.SetElement(0, -50.0f);
  overlayPosition.SetElement(1, -50.0f);
  m_WarnOverlay->SetPosition2D(overlayPosition);
  m_WarnOverlay->SetFontSize(22);
  m_WarnOverlay->SetColor(1, 0, 0); // overlay should be red
}

void QmitkUSNavigationMarkerPlacement::UpdateToolStorage()
{
  if (m_NavigationDataSource.IsNull()) { m_NavigationDataSource = m_CombinedModality->GetNavigationDataSource(); }
  if (m_NavigationDataSource.IsNull()) { MITK_WARN << "Found an invalid navigation data source object!"; }
  us::ModuleContext* context = us::GetModuleContext();
  std::string id = m_NavigationDataSource->US_PROPKEY_ID;
  std::string filter = "(" + mitk::NavigationToolStorage::US_PROPKEY_SOURCE_ID + "=" + id + ")";
  // Get Storage
  std::vector<us::ServiceReference<mitk::NavigationToolStorage> > refs = context->GetServiceReferences<mitk::NavigationToolStorage>();
  m_CurrentStorage = context->GetService(refs.front());

  if (m_CurrentStorage.IsNull())
  {
    MITK_WARN << "Found an invalid storage object!";
  }
  else if (m_CurrentStorage->GetToolCount() != m_NavigationDataSource->GetNumberOfOutputs()) //there is something wrong with the storage
  {
    MITK_WARN << "Found a tool storage, but it has not the same number of tools like the NavigationDataSource. This storage won't be used because it isn't the right one.";
    m_CurrentStorage = NULL;
  }
}
