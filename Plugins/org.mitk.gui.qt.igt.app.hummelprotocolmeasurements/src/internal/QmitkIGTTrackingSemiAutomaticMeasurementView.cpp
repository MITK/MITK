/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkIGTTrackingSemiAutomaticMeasurementView.h"

// Qt
#include <QMessageBox>
#include <qfiledialog.h>
#include <qevent.h>
#include <QCheckBox>

// MITK
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkTrackingDeviceSourceConfigurator.h>
#include <mitkLog.h>
#include "mitkHummelProtocolEvaluation.h"

// POCO
#include <Poco/File.h>
#include <Poco/Path.h>

const std::string QmitkIGTTrackingSemiAutomaticMeasurementView::VIEW_ID = "org.mitk.views.igttrackingsemiautomaticmeasurement";

QmitkIGTTrackingSemiAutomaticMeasurementView::QmitkIGTTrackingSemiAutomaticMeasurementView()
  : QmitkFunctionality()
  , m_Controls(nullptr)
  , m_MultiWidget(nullptr)
{
  m_NextFile = 0;
  m_FilenameVector = std::vector<std::string>();
  m_Timer = new QTimer(this);
  m_logging = false;
  m_referenceValid = true;
  m_tracking = false;
  m_EvaluationFilter = mitk::NavigationDataEvaluationFilter::New();
}

QmitkIGTTrackingSemiAutomaticMeasurementView::~QmitkIGTTrackingSemiAutomaticMeasurementView()
{
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::CreateResults()
{
  QString LogFileName = m_Controls->m_OutputPath->text() + "_results.log";
  mitk::LoggingBackend::Unregister();
  mitk::LoggingBackend::SetLogFile(LogFileName.toStdString().c_str());
  mitk::LoggingBackend::Register();

  double RMSmean = 0;
  for (std::size_t i = 0; i < m_RMSValues.size(); ++i)
  {
    MITK_INFO << "RMS at " << this->m_FilenameVector.at(i) << ": " << m_RMSValues.at(i);
    RMSmean += m_RMSValues.at(i);
  }
  RMSmean /= m_RMSValues.size();
  MITK_INFO << "RMS mean over " << m_RMSValues.size() << " values: " << RMSmean;

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName("Tracking Results");
  newNode->SetData(this->m_MeanPoints);
  this->GetDataStorage()->Add(newNode);

  std::vector<mitk::HummelProtocolEvaluation::HummelProtocolDistanceError> results5cmDistances;
  if (m_Controls->m_mediumVolume->isChecked())
    mitk::HummelProtocolEvaluation::Evaluate5cmDistances(m_MeanPoints, mitk::HummelProtocolEvaluation::medium, results5cmDistances);
  else if (m_Controls->m_smallVolume->isChecked())
    mitk::HummelProtocolEvaluation::Evaluate5cmDistances(m_MeanPoints, mitk::HummelProtocolEvaluation::small, results5cmDistances);
  else if (m_Controls->m_standardVolume->isChecked())
    mitk::HummelProtocolEvaluation::Evaluate5cmDistances(m_MeanPoints, mitk::HummelProtocolEvaluation::standard, results5cmDistances);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::CreateQtPartControl(QWidget *parent)
{
  // build up qt view, unless already done
  if (!m_Controls)
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkIGTTrackingSemiAutomaticMeasurementViewControls;
    m_Controls->setupUi(parent);

    //buttons
    connect(m_Controls->m_LoadMeasurementToolStorage, SIGNAL(clicked()), this, SLOT(OnLoadMeasurementStorage()));
    connect(m_Controls->m_LoadReferenceToolStorage, SIGNAL(clicked()), this, SLOT(OnLoadReferenceStorage()));
    connect(m_Controls->m_StartTracking, SIGNAL(clicked()), this, SLOT(OnStartTracking()));
    connect(m_Controls->m_LoadList, SIGNAL(clicked()), this, SLOT(OnMeasurementLoadFile()));
    connect(m_Controls->m_StartNextMeasurement, SIGNAL(clicked()), this, SLOT(StartNextMeasurement()));
    connect(m_Controls->m_ReapeatLastMeasurement, SIGNAL(clicked()), this, SLOT(RepeatLastMeasurement()));
    connect(m_Controls->m_SetReference, SIGNAL(clicked()), this, SLOT(OnSetReference()));
    connect(m_Controls->m_UseReferenceTrackingSystem, SIGNAL(toggled(bool)), this, SLOT(OnUseReferenceToggled(bool)));
    connect(m_Controls->m_CreateResults, SIGNAL(clicked()), this, SLOT(CreateResults()));

    //event filter
    qApp->installEventFilter(this);

    //timers
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(UpdateTimer()));
  }

  //initialize some view
  m_Controls->m_StopTracking->setEnabled(false);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::MultiWidgetAvailable(QmitkAbstractMultiWidget &multiWidget)
{
  m_MultiWidget = dynamic_cast<QmitkStdMultiWidget*>(&multiWidget);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::MultiWidgetNotAvailable()
{
  m_MultiWidget = nullptr;
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnUseReferenceToggled(bool state)
{
  if (state)
  {
    m_Controls->m_ReferenceBox->setEnabled(true);
    m_Controls->m_SetReference->setEnabled(true);
  }

  else
  {
    m_Controls->m_ReferenceBox->setEnabled(false);
    m_Controls->m_SetReference->setEnabled(false);
  }
}

mitk::NavigationToolStorage::Pointer QmitkIGTTrackingSemiAutomaticMeasurementView::ReadStorage(std::string file)
{
  mitk::NavigationToolStorage::Pointer returnValue;

  //initialize tool storage
  returnValue = mitk::NavigationToolStorage::New();

  //read tool storage from disk
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  returnValue = myDeserializer->Deserialize(file);
  if (returnValue.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText(myDeserializer->GetErrorMessage().c_str());
    msgBox.exec();

    returnValue = nullptr;
  }

  return returnValue;
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnSetReference()
{
  //initialize reference
  m_ReferenceStartPositions = std::vector<mitk::Point3D>();
  m_ReferenceTrackingDeviceSource->Update();
  QString Label = "Positions At Start: ";
  for (unsigned int i = 0; i < m_ReferenceTrackingDeviceSource->GetNumberOfOutputs(); ++i)
  {
    mitk::Point3D position = m_ReferenceTrackingDeviceSource->GetOutput(i)->GetPosition();
    Label = Label + "Tool" + QString::number(i) + ":[" + QString::number(position[0]) + ":" + QString::number(position[1]) + ":" + QString::number(position[1]) + "] ";
    m_ReferenceStartPositions.push_back(position);
  }
  m_Controls->m_ReferencePosAtStart->setText(Label);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnLoadMeasurementStorage()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Toolfile"), "/", tr("All Files (*.*)"));
  if (filename.isNull()) return;

  m_MeasurementStorage = ReadStorage(filename.toStdString());

  //update label
  Poco::Path myPath = Poco::Path(filename.toStdString()); //use this to seperate filename from path
  QString toolLabel = QString("Tool Storage: ") + QString::number(m_MeasurementStorage->GetToolCount()) + " Tools from " + myPath.getFileName().c_str();
  m_Controls->m_MeasurementToolStorageLabel->setText(toolLabel);

  //update status widget
  m_Controls->m_ToolStatusWidget->RemoveStatusLabels();
  m_Controls->m_ToolStatusWidget->PreShowTools(m_MeasurementStorage);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnLoadReferenceStorage()
{
  //read in filename
  static QString oldFile;
  if (oldFile.isNull()) oldFile = "/";
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Toolfile"), oldFile, tr("All Files (*.*)"));
  if (filename.isNull()) return;
  oldFile = filename;

  m_ReferenceStorage = ReadStorage(filename.toStdString());

  //update label
  Poco::Path myPath = Poco::Path(filename.toStdString()); //use this to seperate filename from path
  QString toolLabel = QString("Tool Storage: ") + QString::number(m_ReferenceStorage->GetToolCount()) + " Tools from " + myPath.getFileName().c_str();
  m_Controls->m_ReferenceToolStorageLabel->setText(toolLabel);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnStartTracking()
{
  //check if everything is ready to start tracking
  if (m_MeasurementStorage.IsNull())
  {
    MessageBox("Error: No measurement tools loaded yet!");
    return;
  }
  else if (m_ReferenceStorage.IsNull() && m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    MessageBox("Error: No refernce tools loaded yet!");
    return;
  }
  else if (m_MeasurementStorage->GetToolCount() == 0)
  {
    MessageBox("Error: No way to track without tools!");
    return;
  }
  else if (m_Controls->m_UseReferenceTrackingSystem->isChecked() && (m_ReferenceStorage->GetToolCount() == 0))
  {
    MessageBox("Error: No way to track without tools!");
    return;
  }

  //build the first IGT pipeline (MEASUREMENT)
  mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory1 = mitk::TrackingDeviceSourceConfigurator::New(this->m_MeasurementStorage, this->m_Controls->m_MeasurementTrackingDeviceConfigurationWidget->GetTrackingDevice());
  m_MeasurementTrackingDeviceSource = myTrackingDeviceSourceFactory1->CreateTrackingDeviceSource(this->m_MeasurementToolVisualizationFilter);
  if (m_MeasurementTrackingDeviceSource.IsNull())
  {
    MessageBox(myTrackingDeviceSourceFactory1->GetErrorMessage());
    return;
  }
  //connect the tool visualization widget
  for (unsigned int i = 0; i < m_MeasurementTrackingDeviceSource->GetNumberOfOutputs(); ++i)
  {
    m_Controls->m_ToolStatusWidget->AddNavigationData(m_MeasurementTrackingDeviceSource->GetOutput(i));
    m_EvaluationFilter->SetInput(i, m_MeasurementTrackingDeviceSource->GetOutput(i));
  }
  m_Controls->m_ToolStatusWidget->ShowStatusLabels();
  m_Controls->m_ToolStatusWidget->SetShowPositions(true);
  m_Controls->m_ToolStatusWidget->SetShowQuaternions(true);

  //build the second IGT pipeline (REFERENCE)
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory2 = mitk::TrackingDeviceSourceConfigurator::New(this->m_ReferenceStorage, this->m_Controls->m_ReferenceDeviceConfigurationWidget->GetTrackingDevice());
    m_ReferenceTrackingDeviceSource = myTrackingDeviceSourceFactory2->CreateTrackingDeviceSource();
    if (m_ReferenceTrackingDeviceSource.IsNull())
    {
      MessageBox(myTrackingDeviceSourceFactory2->GetErrorMessage());
      return;
    }
  }

  //initialize tracking
  try
  {
    m_MeasurementTrackingDeviceSource->Connect();
    m_MeasurementTrackingDeviceSource->StartTracking();
    if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
    {
      m_ReferenceTrackingDeviceSource->Connect();
      m_ReferenceTrackingDeviceSource->StartTracking();
    }
  }
  catch (...)
  {
    MessageBox("Error while starting the tracking device!");
    return;
  }

  //set reference
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked()) OnSetReference();

  //start timer
  m_Timer->start(1000 / (m_Controls->m_SamplingRate->value()));

  m_Controls->m_StartTracking->setEnabled(false);
  m_Controls->m_StartTracking->setEnabled(true);

  m_tracking = true;
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnStopTracking()
{
  if (this->m_logging) FinishMeasurement();
  m_MeasurementTrackingDeviceSource->Disconnect();
  m_MeasurementTrackingDeviceSource->StopTracking();
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    m_ReferenceTrackingDeviceSource->Disconnect();
    m_ReferenceTrackingDeviceSource->StopTracking();
  }
  m_Timer->stop();
  m_Controls->m_StartTracking->setEnabled(true);
  m_Controls->m_StartTracking->setEnabled(false);
  m_tracking = false;
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::OnMeasurementLoadFile()
{
  m_FilenameVector = std::vector<std::string>();
  m_FilenameVector.clear();
  m_NextFile = 0;

  //read in filename
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Measurement Filename List"), "/", tr("All Files (*.*)"));
  if (filename.isNull()) return;

  //define own locale
  std::locale C("C");
  setlocale(LC_ALL, "C");

  //read file
  std::ifstream file;
  file.open(filename.toStdString().c_str(), std::ios::in);
  if (file.good())
  {
    //read out file
    file.seekg(0L, std::ios::beg);  // move to begin of file
    while (!file.eof())
    {
      std::string buffer;
      std::getline(file, buffer);    // read out file line by line
      if (buffer.size() > 0) m_FilenameVector.push_back(buffer);
    }
  }

  //fill list at GUI
  m_Controls->m_MeasurementList->clear();
  for (unsigned int i = 0; i < m_FilenameVector.size(); i++) { new QListWidgetItem(tr(m_FilenameVector.at(i).c_str()), m_Controls->m_MeasurementList); }

  //update label next measurement
  std::stringstream label;
  label << "Next Measurement: " << m_FilenameVector.at(0);
  m_Controls->m_NextMeasurement->setText(label.str().c_str());

  //reset results files
  m_MeanPoints = mitk::PointSet::New();
  m_RMSValues = std::vector<double>();
  m_EvaluationFilter = mitk::NavigationDataEvaluationFilter::New();
  if (m_MeasurementToolVisualizationFilter.IsNotNull()) m_EvaluationFilter->SetInput(0, m_MeasurementToolVisualizationFilter->GetOutput(0));
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::UpdateTimer()
{
  if (m_EvaluationFilter.IsNotNull() && m_logging) m_EvaluationFilter->Update();
  else m_MeasurementToolVisualizationFilter->Update();

  m_Controls->m_ToolStatusWidget->Refresh();

  //update reference
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    m_ReferenceTrackingDeviceSource->Update();
    QString Label = "Current Positions: ";
    bool distanceThresholdExceeded = false;
    for (unsigned int i = 0; i < m_ReferenceTrackingDeviceSource->GetNumberOfOutputs(); ++i)
    {
      mitk::Point3D position = m_ReferenceTrackingDeviceSource->GetOutput(i)->GetPosition();
      Label = Label + "Tool" + QString::number(i) + ":[" + QString::number(position[0]) + ":" + QString::number(position[1]) + ":" + QString::number(position[1]) + "] ";
      if (position.EuclideanDistanceTo(m_ReferenceStartPositions.at(i)) > m_Controls->m_ReferenceThreshold->value()) distanceThresholdExceeded = true;
    }
    m_Controls->m_ReferenceCurrentPos->setText(Label);
    if (distanceThresholdExceeded)
    {
      m_Controls->m_ReferenceOK->setText("<html><body style=\" font-family:\'MS Shell Dlg 2\'; font-size:28pt; font-weight:400; font-style:normal;\">NOT OK!</body></html>");
      m_referenceValid = false;
    }
    else
    {
      m_Controls->m_ReferenceOK->setText("<html><body style=\" font-family:\'MS Shell Dlg 2\'; font-size:28pt; font-weight:400; font-style:normal;\">OK</body></html>");
      m_referenceValid = true;
    }
  }

  //update logging
  if (m_logging)
  {
    //check for missing objects
    if (m_MeasurementLoggingFilterXML.IsNull() ||
      m_MeasurementLoggingFilterCSV.IsNull()
      )
    {
      return;
    }

    //log/measure
    m_MeasurementLoggingFilterXML->Update();
    m_MeasurementLoggingFilterCSV->Update();

    if (m_Controls->m_UseReferenceTrackingSystem->isChecked() &&
      m_ReferenceLoggingFilterXML.IsNotNull() &&
      m_ReferenceLoggingFilterCSV.IsNotNull())
    {
      m_ReferenceLoggingFilterXML->Update();
      m_ReferenceLoggingFilterCSV->Update();
    }
    m_loggedFrames++;
    LogAdditionalCSVFile();

    //check if all frames are logged ... if yes finish the measurement
    if (m_loggedFrames > m_Controls->m_SamplesPerMeasurement->value()) { FinishMeasurement(); }

    //update logging label
    QString loggingLabel = "Collected Samples: " + QString::number(m_loggedFrames);
    m_Controls->m_CollectedSamples->setText(loggingLabel);
  }
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::StartNextMeasurement()
{
  if (this->m_NextFile >= static_cast<int>(m_FilenameVector.size()))
  {
    MessageBox("Last Measurement reached!");
    return;
  }

  m_loggedFrames = 0;
  m_logging = true;

  //check if directory exists, if not create one
  Poco::File myPath(std::string(m_Controls->m_OutputPath->text().toUtf8()).c_str());
  if (!myPath.exists()) myPath.createDirectory();

  QString LogFileName = m_Controls->m_OutputPath->text() + QString(m_FilenameVector.at(m_NextFile).c_str()) + ".log";
  mitk::LoggingBackend::Unregister();
  mitk::LoggingBackend::SetLogFile(LogFileName.toStdString().c_str());
  mitk::LoggingBackend::Register();

  //initialize logging filters
  m_MeasurementLoggingFilterXML = mitk::NavigationDataRecorderDeprecated::New();
  m_MeasurementLoggingFilterXML->SetRecordingMode(mitk::NavigationDataRecorderDeprecated::NormalFile);
  m_MeasurementLoggingFilterCSV = mitk::NavigationDataRecorderDeprecated::New();
  m_MeasurementLoggingFilterCSV->SetRecordingMode(mitk::NavigationDataRecorderDeprecated::NormalFile);
  m_MeasurementLoggingFilterXML->SetOutputFormat(mitk::NavigationDataRecorderDeprecated::xml);
  m_MeasurementLoggingFilterCSV->SetOutputFormat(mitk::NavigationDataRecorderDeprecated::csv);
  QString MeasurementFilenameXML = m_Controls->m_OutputPath->text() + QString(m_FilenameVector.at(m_NextFile).c_str()) + ".xml";
  QString MeasurementFilenameCSV = m_Controls->m_OutputPath->text() + QString(m_FilenameVector.at(m_NextFile).c_str()) + ".csv";
  m_MeasurementLoggingFilterXML->SetFileName(MeasurementFilenameXML.toStdString());
  m_MeasurementLoggingFilterCSV->SetFileName(MeasurementFilenameCSV.toStdString());
  m_MeasurementLoggingFilterXML->SetRecordCountLimit(m_Controls->m_SamplesPerMeasurement->value());
  m_MeasurementLoggingFilterCSV->SetRecordCountLimit(m_Controls->m_SamplesPerMeasurement->value());

  if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    m_ReferenceLoggingFilterXML = mitk::NavigationDataRecorderDeprecated::New();
    m_ReferenceLoggingFilterXML->SetRecordingMode(mitk::NavigationDataRecorderDeprecated::NormalFile);
    m_ReferenceLoggingFilterCSV = mitk::NavigationDataRecorderDeprecated::New();
    m_ReferenceLoggingFilterCSV->SetRecordingMode(mitk::NavigationDataRecorderDeprecated::NormalFile);
    m_ReferenceLoggingFilterXML->SetOutputFormat(mitk::NavigationDataRecorderDeprecated::xml);
    m_ReferenceLoggingFilterCSV->SetOutputFormat(mitk::NavigationDataRecorderDeprecated::csv);
    QString ReferenceFilenameXML = m_Controls->m_OutputPath->text() + "Reference_" + QString(m_FilenameVector.at(m_NextFile).c_str()) + ".xml";
    QString ReferenceFilenameCSV = m_Controls->m_OutputPath->text() + "Reference_" + QString(m_FilenameVector.at(m_NextFile).c_str()) + ".csv";
    m_ReferenceLoggingFilterXML->SetFileName(ReferenceFilenameXML.toStdString());
    m_ReferenceLoggingFilterCSV->SetFileName(ReferenceFilenameCSV.toStdString());
    m_ReferenceLoggingFilterXML->SetRecordCountLimit(m_Controls->m_SamplesPerMeasurement->value());
    m_ReferenceLoggingFilterCSV->SetRecordCountLimit(m_Controls->m_SamplesPerMeasurement->value());
  }

  //start additional csv logging
  StartLoggingAdditionalCSVFile(m_FilenameVector.at(m_NextFile));

  //connect filter
  for (unsigned int i = 0; i < m_MeasurementToolVisualizationFilter->GetNumberOfOutputs(); ++i)
  {
    m_MeasurementLoggingFilterXML->AddNavigationData(m_MeasurementToolVisualizationFilter->GetOutput(i));
    m_MeasurementLoggingFilterCSV->AddNavigationData(m_MeasurementToolVisualizationFilter->GetOutput(i));
  }
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked()) for (unsigned int i = 0; i < m_ReferenceTrackingDeviceSource->GetNumberOfOutputs(); ++i)
  {
    m_ReferenceLoggingFilterXML->AddNavigationData(m_ReferenceTrackingDeviceSource->GetOutput(i));
    m_ReferenceLoggingFilterCSV->AddNavigationData(m_ReferenceTrackingDeviceSource->GetOutput(i));
  }

  //start filter
  m_MeasurementLoggingFilterXML->StartRecording();
  m_MeasurementLoggingFilterCSV->StartRecording();
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    m_ReferenceLoggingFilterXML->StartRecording();
    m_ReferenceLoggingFilterCSV->StartRecording();
  }

  //disable all buttons
  DisableAllButtons();

  //update label next measurement
  std::stringstream label;
  if ((m_NextFile + 1) >= static_cast<int>(m_FilenameVector.size())) label << "Next Measurement: <none>";
  else label << "Next Measurement: " << m_FilenameVector.at(m_NextFile + 1);
  m_Controls->m_NextMeasurement->setText(label.str().c_str());

  //update label last measurement
  std::stringstream label2;
  label2 << "Last Measurement: " << m_FilenameVector.at(m_NextFile);
  m_Controls->m_LastMeasurement->setText(label2.str().c_str());

  m_NextFile++;
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::RepeatLastMeasurement()
{
  m_NextFile--;
  StartNextMeasurement();
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::MessageBox(std::string s)
{
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::DisableAllButtons()
{
  m_Controls->m_LoadList->setEnabled(false);
  m_Controls->m_StartNextMeasurement->setEnabled(false);
  m_Controls->m_ReapeatLastMeasurement->setEnabled(false);
  m_Controls->m_SamplingRate->setEnabled(false);
  m_Controls->m_SamplesPerMeasurement->setEnabled(false);
  m_Controls->m_ReferenceThreshold->setEnabled(false);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::EnableAllButtons()
{
  m_Controls->m_LoadList->setEnabled(true);
  m_Controls->m_StartNextMeasurement->setEnabled(true);
  m_Controls->m_ReapeatLastMeasurement->setEnabled(true);
  m_Controls->m_SamplingRate->setEnabled(true);
  m_Controls->m_SamplesPerMeasurement->setEnabled(true);
  m_Controls->m_ReferenceThreshold->setEnabled(true);
}

void QmitkIGTTrackingSemiAutomaticMeasurementView::FinishMeasurement()
{
  m_logging = false;

  m_MeasurementLoggingFilterXML->StopRecording();
  m_MeasurementLoggingFilterCSV->StopRecording();
  if (m_Controls->m_UseReferenceTrackingSystem->isChecked())
  {
    m_ReferenceLoggingFilterXML->StopRecording();
    m_ReferenceLoggingFilterCSV->StopRecording();
  }
  StopLoggingAdditionalCSVFile();

  int id = m_NextFile - 1;
  mitk::Point3D positionMean = m_EvaluationFilter->GetPositionMean(0);
  MITK_INFO << "Evaluated " << m_EvaluationFilter->GetNumberOfAnalysedNavigationData(0) << " samples.";
  double rms = m_EvaluationFilter->GetPositionErrorRMS(0);
  MITK_INFO << "RMS: " << rms;
  MITK_INFO << "Position Mean: " << positionMean;
  m_MeanPoints->SetPoint(id, positionMean);
  if (static_cast<int>(m_RMSValues.size()) <= id) m_RMSValues.push_back(rms);
  else m_RMSValues[id] = rms;

  m_EvaluationFilter->ResetStatistic();

  EnableAllButtons();
}

void  QmitkIGTTrackingSemiAutomaticMeasurementView::StartLoggingAdditionalCSVFile(std::string filePostfix)
{
  //write logfile header
  QString header = "Nr;MITK_Time;Valid_Reference;";
  QString tool = QString("MeasureTool_") + QString(m_MeasurementTrackingDeviceSource->GetOutput(0)->GetName());
  header = header + tool + "[x];" + tool + "[y];" + tool + "[z];" + tool + "[qx];" + tool + "[qy];" + tool + "[qz];" + tool + "[qr]\n";

  //open logfile and write header
  m_logFileCSV.open(std::string(m_Controls->m_OutputPath->text().toUtf8()).append("/LogFileCombined").append(filePostfix.c_str()).append(".csv").c_str(), std::ios::out);
  m_logFileCSV << header.toStdString().c_str();
}

void  QmitkIGTTrackingSemiAutomaticMeasurementView::LogAdditionalCSVFile()
{
  mitk::Point3D pos = m_MeasurementTrackingDeviceSource->GetOutput(0)->GetPosition();
  mitk::Quaternion rot = m_MeasurementTrackingDeviceSource->GetOutput(0)->GetOrientation();
  std::string valid = "";
  if (m_referenceValid) valid = "true";
  else valid = "false";
  std::stringstream timestamp;
  timestamp << m_MeasurementTrackingDeviceSource->GetOutput(0)->GetTimeStamp();
  QString dataSet = QString::number(m_loggedFrames) + ";" + QString(timestamp.str().c_str()) + ";" + QString(valid.c_str()) + ";" + QString::number(pos[0]) + ";" + QString::number(pos[1]) + ";" + QString::number(pos[2]) + ";" + QString::number(rot.x()) + ";" + QString::number(rot.y()) + ";" + QString::number(rot.z()) + ";" + QString::number(rot.r()) + "\n";
  m_logFileCSV << dataSet.toStdString();
}

void  QmitkIGTTrackingSemiAutomaticMeasurementView::StopLoggingAdditionalCSVFile()
{
  m_logFileCSV.close();
}

bool QmitkIGTTrackingSemiAutomaticMeasurementView::eventFilter(QObject *, QEvent *ev)
{
  if (ev->type() == QEvent::KeyPress)
  {
    QKeyEvent *k = (QKeyEvent *)ev;
    bool down = k->key() == 16777239;

    if (down && m_tracking && !m_logging)
      StartNextMeasurement();
  }

  return false;
}
