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
#include "QmitkUltrasoundCalibration.h"
#include <QTimer>

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QmitkServiceListWidget.h>

// MITK
#include <mitkVector.h>
#include "mitkIOUtil.h"
#include <mitkBaseData.h>
#include <mitkImageGenerator.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkSceneIO.h>

// us
#include <usServiceReference.h>

// VTK
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>

#include <vtkVertexGlyphFilter.h>

#include "internal/org_mbi_gui_qt_usnavigation_Activator.h"

// sleep headers
#include <chrono>
#include <thread>

const std::string QmitkUltrasoundCalibration::VIEW_ID = "org.mitk.views.ultrasoundcalibration";

QmitkUltrasoundCalibration::QmitkUltrasoundCalibration()
  : m_PhantomConfigurationPointSet(nullptr),
    m_USDeviceChanged(this, &QmitkUltrasoundCalibration::OnUSDepthChanged)
{
  ctkPluginContext *pluginContext = mitk::PluginActivator::GetContext();

  if (pluginContext)
  {
    // to be notified about service event of an USDevice
    pluginContext->connectServiceListener(this,
                                          "OnDeviceServiceEvent",
                                          QString::fromStdString("(" + us::ServiceConstants::OBJECTCLASS() + "=" +
                                                                 us_service_interface_iid<mitk::USDevice>() + ")"));
  }
}

QmitkUltrasoundCalibration::~QmitkUltrasoundCalibration()
{
  m_Controls.m_CombinedModalityManagerWidget->blockSignals(true);
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality;
  combinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    combinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_USDeviceChanged);
  }
  m_Timer->stop();

  this->OnStopCalibrationProcess();

  this->OnStopPlusCalibration();

  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if (node.IsNotNull())
    this->GetDataStorage()->Remove(node);

  this->GetDataStorage()->Remove(m_VerificationReferencePointsDataNode);
  this->GetDataStorage()->Remove(m_SpacingNode);
  
  delete m_Timer;

  // remove observer for phantom-based point adding
  m_CalibPointsImage->RemoveAllObservers();
}

void QmitkUltrasoundCalibration::SetFocus()
{
  m_Controls.m_ToolBox->setFocus();
}

void QmitkUltrasoundCalibration::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  m_Controls.m_CombinedModalityManagerWidget->SetCalibrationLoadedNecessary(false);

  m_Timer = new QTimer(this);
  m_StreamingTimer = new QTimer(this);

  m_Controls.m_SpacingBtnFreeze->setEnabled(true);
  m_Controls.m_SpacingAddPoint->setEnabled(false);
  m_Controls.m_CalculateSpacing->setEnabled(false);

  m_SpacingPointsCount = 0;
  m_SpacingPoints = mitk::PointSet::New();
  m_SpacingNode = mitk::DataNode::New();
  m_SpacingNode->SetName("Spacing Points");
  m_SpacingNode->SetData(this->m_SpacingPoints);
  this->GetDataStorage()->Add(m_SpacingNode);

  // Pointset for Calibration Points
  m_CalibPointsTool = mitk::PointSet::New();

  // Pointset for Worldpoints
  m_CalibPointsImage = mitk::PointSet::New();

  m_CalibPointsCount = 0;

  // Evaluation Pointsets (Non-Visualized)
  m_EvalPointsImage = mitk::PointSet::New();
  m_EvalPointsTool = mitk::PointSet::New();
  m_EvalPointsProjected = mitk::PointSet::New();

  // Neelde Projection Filter
  m_NeedleProjectionFilter = mitk::NeedleProjectionFilter::New();

  // Tracking Status Widgets
  m_Controls.m_CalibTrackingStatus->ShowStatusLabels();
  m_Controls.m_EvalTrackingStatus->ShowStatusLabels();

  // General & Device Selection
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(Update()));

  // Calibration
  connect(m_Controls.m_CalibBtnFreeze, SIGNAL(clicked()), this, SLOT(SwitchFreeze())); // Freeze
  connect(m_Controls.m_CalibBtnAddPoint,
          SIGNAL(clicked()),
          this,
          SLOT(OnAddCalibPoint())); // Tracking & Image Points (Calibration)
  connect(m_Controls.m_CalibBtnCalibrate, SIGNAL(clicked()), this, SLOT(OnCalibration())); // Perform Calibration
  // Phantom-based calibration
  connect(m_Controls.m_CalibBtnLoadPhantomConfiguration,
          SIGNAL(clicked()),
          this,
          SLOT(OnLoadPhantomConfiguration())); // Phantom configuration
  connect(m_Controls.m_CalibBtnMatchAnnotationToPhantomConfiguration,
          SIGNAL(clicked()),
          this,
          SLOT(OnMatchAnnotationToPhantomConfiguration()));
  connect(m_Controls.m_CalibBtnMoveUp, SIGNAL(clicked()), this, SLOT(OnMovePhantomAnnotationsUp()));
  connect(m_Controls.m_CalibBtnMoveDown, SIGNAL(clicked()), this, SLOT(OnMovePhantomAnnotationsDown()));
  connect(m_Controls.m_CalibBtnMoveLeft, SIGNAL(clicked()), this, SLOT(OnMovePhantomAnnotationsLeft()));
  connect(m_Controls.m_CalibBtnMoveRight, SIGNAL(clicked()), this, SLOT(OnMovePhantomAnnotationsRight()));
  connect(m_Controls.m_CalibBtnRotateRight, SIGNAL(clicked()), this, SLOT(OnRotatePhantomAnnotationsRight()));
  connect(m_Controls.m_CalibBtnRotateLeft, SIGNAL(clicked()), this, SLOT(OnRotatePhantomAnnotationsLeft()));

  connect(m_Controls.m_CalibBtnPerformPhantomCalibration,
          SIGNAL(clicked()),
          this,
          SLOT(OnPhantomBasedCalibration())); // Perform phantom-based calibration
  connect(m_Controls.m_CalibBtnSavePhantomCalibration,
          SIGNAL(clicked()),
          this,
          SLOT(OnSaveCalibration())); // Save phantom-based calibration

  // Evaluation
  connect(m_Controls.m_EvalBtnStep1, SIGNAL(clicked()), this, SLOT(OnAddEvalProjectedPoint())); // Needle Projection
  connect(m_Controls.m_EvalBtnStep2, SIGNAL(clicked()), this, SLOT(SwitchFreeze()));            // Freeze
  connect(m_Controls.m_EvalBtnStep3,
          SIGNAL(clicked()),
          this,
          SLOT(OnAddEvalTargetPoint())); // Tracking & Image Points (Evaluation)
  connect(m_Controls.m_EvalBtnSave, SIGNAL(clicked()), this, SLOT(OnSaveEvaluation())); // Save Evaluation Results
  connect(m_Controls.m_CalibBtnSaveCalibration,
          SIGNAL(clicked()),
          this,
          SLOT(OnSaveCalibration()));                                       // Save Evaluation Results
  connect(m_Controls.m_BtnReset, SIGNAL(clicked()), this, SLOT(OnReset())); // Reset Pointsets

  // PLUS Calibration
  connect(m_Controls.m_GetCalibrationFromPLUS, SIGNAL(clicked()), this, SLOT(OnGetPlusCalibration()));
  connect(m_Controls.m_StartStreaming, SIGNAL(clicked()), this, SLOT(OnStartStreaming()));
  connect(m_StreamingTimer, SIGNAL(timeout()), this, SLOT(OnStreamingTimerTimeout()));
  connect(m_Controls.m_StopPlusCalibration, SIGNAL(clicked()), this, SLOT(OnStopPlusCalibration()));
  connect(m_Controls.m_SavePlusCalibration, SIGNAL(clicked()), this, SLOT(OnSaveCalibration()));
  connect(this, SIGNAL(NewConnectionSignal()), this, SLOT(OnNewConnection()));

  // Determine Spacing for Calibration of USVideoDevice
  connect(m_Controls.m_SpacingBtnFreeze, SIGNAL(clicked()), this, SLOT(OnFreezeClicked()));
  connect(m_Controls.m_SpacingAddPoint, SIGNAL(clicked()), this, SLOT(OnAddSpacingPoint()));
  connect(m_Controls.m_CalculateSpacing, SIGNAL(clicked()), this, SLOT(OnCalculateSpacing()));

  connect(m_Controls.m_CombinedModalityManagerWidget, SIGNAL(SignalReadyForNextStep()), this, SLOT(OnDeviceSelected()));
  connect(m_Controls.m_CombinedModalityManagerWidget,
          SIGNAL(SignalNoLongerReadyForNextStep()),
          this,
          SLOT(OnDeviceDeselected()));
  connect(m_Controls.m_StartCalibrationButton, SIGNAL(clicked()), this, SLOT(OnStartCalibrationProcess()));
  connect(m_Controls.m_StartPlusCalibrationButton, SIGNAL(clicked()), this, SLOT(OnStartPlusCalibration()));
  connect(m_Controls.m_CalibBtnRestartCalibration, SIGNAL(clicked()), this, SLOT(OnReset()));
  connect(m_Controls.m_CalibBtnStopCalibration, SIGNAL(clicked()), this, SLOT(OnStopCalibrationProcess()));

  connect(m_Controls.m_AddReferencePoints, SIGNAL(clicked()), this, SLOT(OnAddCurrentTipPositionToReferencePoints()));
  connect(m_Controls.m_AddCurrentPointerTipForVerification,
          SIGNAL(clicked()),
          this,
          SLOT(OnAddCurrentTipPositionForVerification()));
  connect(m_Controls.m_StartVerification, SIGNAL(clicked()), this, SLOT(OnStartVerification()));

  // initialize data storage combo box
  m_Controls.m_ReferencePointsComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_ReferencePointsComboBox->SetAutoSelectNewItems(true);
  m_Controls.m_ReferencePointsComboBox->SetPredicate(mitk::NodePredicateDataType::New("PointSet"));

  // initialize point list widget
  if (m_VerificationReferencePoints.IsNull())
  {
    m_VerificationReferencePoints = mitk::PointSet::New();
  }
  if (m_VerificationReferencePointsDataNode.IsNull())
  {
    m_VerificationReferencePointsDataNode = mitk::DataNode::New();
    m_VerificationReferencePointsDataNode->SetName("US Verification Reference Points");
    m_VerificationReferencePointsDataNode->SetData(m_VerificationReferencePoints);
    this->GetDataStorage()->Add(m_VerificationReferencePointsDataNode);
  }
  m_Controls.m_ReferencePointsPointListWidget->SetPointSetNode(m_VerificationReferencePointsDataNode);

  m_Controls.m_ToolBox->setCurrentIndex(0);
}

void QmitkUltrasoundCalibration::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                    const QList<mitk::DataNode::Pointer> & /*nodes*/)
{
}

void QmitkUltrasoundCalibration::OnTabSwitch(int index)
{
  switch (index)
  {
    case 0:
      if (m_Controls.m_ToolBox->isItemEnabled(1) || m_Controls.m_ToolBox->isItemEnabled(2))
      {
        this->OnStopCalibrationProcess();
      }
      break;
    default:;
  }
}

void QmitkUltrasoundCalibration::OnDeviceSelected()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality;
  combinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    combinedModality->GetUltrasoundDevice()->AddPropertyChangedListener(m_USDeviceChanged);

    m_Controls.m_StartCalibrationButton->setEnabled(true);
    m_Controls.m_StartPlusCalibrationButton->setEnabled(true);
    m_Controls.m_ToolBox->setItemEnabled(1, true);
    m_Controls.m_ToolBox->setItemEnabled(2, true);
  }
}

void QmitkUltrasoundCalibration::OnDeviceDeselected()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality;
  combinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    combinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_USDeviceChanged);
  }
  m_Controls.m_StartCalibrationButton->setEnabled(false);
  m_Controls.m_StartPlusCalibrationButton->setEnabled(false);
  m_Controls.m_ToolBox->setCurrentIndex(0);
  m_Controls.m_ToolBox->setItemEnabled(1, false);
  m_Controls.m_ToolBox->setItemEnabled(2, false);
}

void QmitkUltrasoundCalibration::OnAddCurrentTipPositionToReferencePoints()
{
  if (m_Controls.m_VerificationPointerChoser->GetSelectedNavigationDataSource().IsNull() ||
      (m_Controls.m_VerificationPointerChoser->GetSelectedToolID() == -1))
  {
    MITK_WARN << "No tool selected, aborting";
    return;
  }
  mitk::NavigationData::Pointer currentPointerData =
    m_Controls.m_VerificationPointerChoser->GetSelectedNavigationDataSource()->GetOutput(
      m_Controls.m_VerificationPointerChoser->GetSelectedToolID());
  mitk::Point3D currentTipPosition = currentPointerData->GetPosition();
  m_VerificationReferencePoints->InsertPoint(m_VerificationReferencePoints->GetSize(), currentTipPosition);
}

void QmitkUltrasoundCalibration::OnStartVerification()
{
  m_currentPoint = 0;
  mitk::PointSet::Pointer selectedPointSet =
    dynamic_cast<mitk::PointSet *>(m_Controls.m_ReferencePointsComboBox->GetSelectedNode()->GetData());
  m_Controls.m_CurrentPointLabel->setText("Point " + QString::number(m_currentPoint) + " of " +
                                          QString::number(selectedPointSet->GetSize()));
  m_allErrors = std::vector<double>();
  m_allReferencePoints = std::vector<mitk::Point3D>();
  for (int i = 0; i < selectedPointSet->GetSize(); i++)
  {
    m_allReferencePoints.push_back(selectedPointSet->GetPoint(i));
  }
}

void QmitkUltrasoundCalibration::OnAddCurrentTipPositionForVerification()
{
  if (m_currentPoint == -1)
  {
    MITK_WARN << "Cannot add point";
    return;
  }
  if (m_Controls.m_VerificationPointerChoser->GetSelectedNavigationDataSource().IsNull() ||
      (m_Controls.m_VerificationPointerChoser->GetSelectedToolID() == -1))
  {
    MITK_WARN << "No tool selected, aborting";
    return;
  }
  mitk::NavigationData::Pointer currentPointerData =
    m_Controls.m_VerificationPointerChoser->GetSelectedNavigationDataSource()->GetOutput(
      m_Controls.m_VerificationPointerChoser->GetSelectedToolID());
  mitk::Point3D currentTipPosition = currentPointerData->GetPosition();

  double currentError = m_allReferencePoints.at(m_currentPoint).EuclideanDistanceTo(currentTipPosition);
  MITK_INFO << "Current Error: " << currentError << " mm";
  m_allErrors.push_back(currentError);

  if (++m_currentPoint < static_cast<int>(m_allReferencePoints.size()))
  {
    m_Controls.m_CurrentPointLabel->setText("Point " + QString::number(m_currentPoint) + " of " +
                                            QString::number(m_allReferencePoints.size()));
  }
  else
  {
    m_currentPoint = -1;
    double meanError = 0;
    for (std::size_t i = 0; i < m_allErrors.size(); ++i)
    {
      meanError += m_allErrors[i];
    }
    meanError /= m_allErrors.size();

    QString result = "Finished verification! \n Verification of " + QString::number(m_allErrors.size()) +
                     " points, mean error: " + QString::number(meanError) + " mm";
    m_Controls.m_ResultsTextEdit->setText(result);
    MITK_INFO << result.toStdString();
  }
}

void QmitkUltrasoundCalibration::OnStartCalibrationProcess()
{
  // US Image Stream
  m_Node = dynamic_cast<mitk::DataNode*>(this->GetDataStorage()->GetNamedNode("US Viewing Stream - Image 0")->CreateAnother().GetPointer());
  m_Node->SetName("US Calibration Viewing Stream");
  this->GetDataStorage()->Add(m_Node);

  // data node for calibration point set
  m_CalibNode = mitk::DataNode::New();
  m_CalibNode->SetName("Tool Calibration Points");
  m_CalibNode->SetData(this->m_CalibPointsTool);
  this->GetDataStorage()->Add(m_CalibNode);

  // data node for world point set
  m_WorldNode = mitk::DataNode::New();
  m_WorldNode->SetName("Image Calibration Points");
  m_WorldNode->SetData(this->m_CalibPointsImage);
  this->GetDataStorage()->Add(m_WorldNode);

  m_CombinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();
  m_CombinedModality->SetCalibration(mitk::AffineTransform3D::New()); // dummy calibration because without a calibration
                                                                      // the comined modality was laggy (maybe a bug?)
  if (m_CombinedModality.IsNull())
  {
    return;
  }

  m_Tracker = m_CombinedModality->GetNavigationDataSource();

  // Construct Pipeline
  this->m_NeedleProjectionFilter->SetInput(0, m_Tracker->GetOutput(0));

  QApplication::setOverrideCursor(Qt::WaitCursor);
  // make sure that the combined modality is in connected state before using it
  if (m_CombinedModality->GetUltrasoundDevice()->GetDeviceState() < mitk::USDevice::State_Connected)
  {
    m_CombinedModality->GetUltrasoundDevice()->Connect();
  }
  if (m_CombinedModality->GetUltrasoundDevice()->GetDeviceState() < mitk::USDevice::State_Activated)
  {
    m_CombinedModality->GetUltrasoundDevice()->Activate();
  }
  QApplication::restoreOverrideCursor();

  this->SwitchFreeze();

  // Trigger the ProbeChanged method for initializing/updating the spacing of the ultrasound image correctly
  std::string probeName = m_CombinedModality->GetUltrasoundDevice()->GetCurrentProbe()->GetName();
  m_CombinedModality->GetUltrasoundDevice()->ProbeChanged(probeName);

  mitk::DataNode::Pointer usNode = this->GetDataStorage()->GetNamedNode("US Viewing Stream - Image 0");
  if (usNode.IsNotNull())
  {
    this->GetDataStorage()->Remove(usNode);
  }

  // Todo: Maybe display this elsewhere
  this->ShowNeedlePath();

  // Switch active tab to Calibration page
  m_Controls.m_ToolBox->setItemEnabled(1, true);
  m_Controls.m_ToolBox->setCurrentIndex(1);
}

void QmitkUltrasoundCalibration::OnStartPlusCalibration()
{
  if (m_CombinedModality.IsNull())
  {
    m_CombinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();
    if (m_CombinedModality.IsNull())
    {
      return;
    } // something went wrong, there is no combined modality
  }

  // setup server to send UltrasoundImages to PLUS
  mitk::IGTLServer::Pointer m_USServer = mitk::IGTLServer::New(true);
  m_USServer->SetName("EchoTrack Image Source");
  m_USServer->SetHostname("127.0.0.1");
  m_USServer->SetPortNumber(18944);

  m_USMessageProvider = mitk::IGTLMessageProvider::New();
  m_USMessageProvider->SetIGTLDevice(m_USServer);
  m_USMessageProvider->SetFPS(5);

  m_USImageToIGTLMessageFilter = mitk::ImageToIGTLMessageFilter::New();
  m_USImageToIGTLMessageFilter->ConnectTo(m_CombinedModality->GetUltrasoundDevice());
  m_USImageToIGTLMessageFilter->SetName("USImage Filter");

  // setup server to send TrackingData to PLUS
  m_TrackingServer = mitk::IGTLServer::New(true);
  m_TrackingServer->SetName("EchoTrack Tracking Source");
  m_TrackingServer->SetHostname("127.0.0.1");
  m_TrackingServer->SetPortNumber(18945);

  m_TrackingMessageProvider = mitk::IGTLMessageProvider::New();
  m_TrackingMessageProvider->SetIGTLDevice(m_TrackingServer);
  m_TrackingMessageProvider->SetFPS(5);

  m_TrackingToIGTLMessageFilter = mitk::NavigationDataToIGTLMessageFilter::New();
  m_TrackingToIGTLMessageFilter->ConnectTo(m_CombinedModality->GetTrackingDeviceDataSource());
  m_TrackingToIGTLMessageFilter->SetName("Tracker Filter");

  typedef itk::SimpleMemberCommand<QmitkUltrasoundCalibration> CurCommandType;

  CurCommandType::Pointer newConnectionCommand = CurCommandType::New();
  newConnectionCommand->SetCallbackFunction(this, &QmitkUltrasoundCalibration::OnPlusConnected);
  this->m_NewConnectionObserverTag =
    this->m_TrackingServer->AddObserver(mitk::NewClientConnectionEvent(), newConnectionCommand);

  // Open connections of both servers
  if (m_USServer->OpenConnection())
  {
    MITK_INFO << "US Server opened its connection successfully";
    m_USServer->StartCommunication();
  }
  else
  {
    MITK_INFO << "US Server could not open its connection";
  }
  if (m_TrackingServer->OpenConnection())
  {
    MITK_INFO << "Tracking Server opened its connection successfully";
    m_TrackingServer->StartCommunication();
  }
  else
  {
    MITK_INFO << "Tracking Server could not open its connection";
  }
  if (m_USMessageProvider->IsCommunicating() && m_TrackingMessageProvider->IsCommunicating())
  {
    m_Controls.m_StartPlusCalibrationButton->setEnabled(false);
    m_Controls.m_GetCalibrationFromPLUS->setEnabled(true);
    m_Controls.m_StartStreaming->setEnabled(false);
    m_Controls.m_SavePlusCalibration->setEnabled(false);
    m_Controls.m_SetupStatus->setStyleSheet("QLabel { color : green; }");
    m_Controls.m_SetupStatus->setText("Setup successfull you can now connect PLUS");
  }
  else
  {
    m_Controls.m_SetupStatus->setStyleSheet("QLabel { color : red; }");
    m_Controls.m_SetupStatus->setText("Something went wrong. Please try again");
  }
}

void QmitkUltrasoundCalibration::OnStopPlusCalibration()
{
  // closing all server and clients when PlusCalibration is finished
  if (m_USMessageProvider.IsNotNull())
  {
    if (m_USMessageProvider->IsStreaming())
    {
      m_USMessageProvider->StopStreamingOfSource(m_USImageToIGTLMessageFilter);
    }
  }
  if (m_TrackingMessageProvider.IsNotNull())
  {
    if (m_TrackingMessageProvider->IsStreaming())
    {
      m_TrackingMessageProvider->StopStreamingOfSource(m_TrackingToIGTLMessageFilter);
    }
  }
  if (m_USServer.IsNotNull())
  {
    m_USServer->CloseConnection();
  }
  if (m_TrackingServer.IsNotNull())
  {
    m_TrackingServer->CloseConnection();
  }
  if (m_TransformClient.IsNotNull())
  {
    m_TransformClient->CloseConnection();
  }
  m_Controls.m_GotCalibrationLabel->setText("");
  m_Controls.m_ConnectionStatus->setText("");
  m_Controls.m_SetupStatus->setText("");
  m_Controls.m_StartPlusCalibrationButton->setEnabled(true);
  m_StreamingTimer->stop();
  delete m_StreamingTimer;
}

void QmitkUltrasoundCalibration::OnPlusConnected()
{
  emit NewConnectionSignal();
}

void QmitkUltrasoundCalibration::OnNewConnection()
{
  m_Controls.m_StartStreaming->setEnabled(true);
  m_Controls.m_ConnectionStatus->setStyleSheet("QLabel { color : green; }");
  m_Controls.m_ConnectionStatus->setText("Connection successfull you can now start streaming");
}

void QmitkUltrasoundCalibration::OnStreamingTimerTimeout()
{
  m_USMessageProvider->Update();
  m_TrackingMessageProvider->Update();
}

void QmitkUltrasoundCalibration::OnStartStreaming()
{
  m_USMessageProvider->StartStreamingOfSource(m_USImageToIGTLMessageFilter, 5);
  m_TrackingMessageProvider->StartStreamingOfSource(m_TrackingToIGTLMessageFilter, 5);
  m_Controls.m_StartStreaming->setEnabled(false);
  m_Controls.m_ConnectionStatus->setText("");
  m_StreamingTimer->start((1.0 / 5.0 * 1000.0));
}

void QmitkUltrasoundCalibration::OnGetPlusCalibration()
{
  m_TransformClient = mitk::IGTLClient::New(true);
  m_TransformClient->SetHostname("127.0.0.1");
  m_TransformClient->SetPortNumber(18946);
  m_TransformDeviceSource = mitk::IGTLDeviceSource::New();
  m_TransformDeviceSource->SetIGTLDevice(m_TransformClient);
  m_TransformDeviceSource->Connect();
  if (m_TransformDeviceSource->IsConnected())
  {
    MITK_INFO << "successfully connected";
    m_TransformDeviceSource->StartCommunication();
    if (m_TransformDeviceSource->IsCommunicating())
    {
      MITK_INFO << "communication started";
      mitk::IGTLMessage::Pointer receivedMessage;
      bool condition = false;
      igtl::Matrix4x4 transformPLUS;
      while (!(receivedMessage.IsNotNull() && receivedMessage->IsDataValid()))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        m_TransformDeviceSource->Update();
        receivedMessage = m_TransformDeviceSource->GetOutput();
        igtl::TransformMessage::Pointer msg =
          dynamic_cast<igtl::TransformMessage *>(m_TransformDeviceSource->GetOutput()->GetMessage().GetPointer());
        if (msg == nullptr || msg.IsNull())
        {
          MITK_INFO << "Received message could not be casted to TransformMessage. Skipping...";
          continue;
        }
        else
        {
          if (std::strcmp(msg->GetDeviceName(), "ImageToTracker") != 0)
          {
            MITK_INFO << "Was not Image to Tracker Transform. Skipping...";
            continue;
          }
          else
          {
            msg->GetMatrix(transformPLUS);
            condition = true;
            break;
          }
        }
      }
      if (condition)
      {
        this->ProcessPlusCalibration(transformPLUS);
      }
      else
      {
        m_Controls.m_GotCalibrationLabel->setStyleSheet("QLabel { color : red; }");
        m_Controls.m_GotCalibrationLabel->setText("Something went wrong. Please try again");
      }
    }
    else
    {
      MITK_INFO << " no connection";
      m_Controls.m_GotCalibrationLabel->setStyleSheet("QLabel { color : red; }");
      m_Controls.m_GotCalibrationLabel->setText("Something went wrong. Please try again");
    }
  }
  else
  {
    m_Controls.m_GotCalibrationLabel->setStyleSheet("QLabel { color : red; }");
    m_Controls.m_GotCalibrationLabel->setText("Something went wrong. Please try again");
  }
}

void QmitkUltrasoundCalibration::ProcessPlusCalibration(igtl::Matrix4x4 &imageToTracker)
{
  mitk::AffineTransform3D::Pointer imageToTrackerTransform = mitk::AffineTransform3D::New();
  itk::Matrix<mitk::ScalarType, 3, 3> rotationFloat = itk::Matrix<mitk::ScalarType, 3, 3>();
  itk::Vector<mitk::ScalarType, 3> translationFloat = itk::Vector<mitk::ScalarType, 3>();

  rotationFloat[0][0] = imageToTracker[0][0];
  rotationFloat[0][1] = imageToTracker[0][1];
  rotationFloat[0][2] = imageToTracker[0][2];
  rotationFloat[1][0] = imageToTracker[1][0];
  rotationFloat[1][1] = imageToTracker[1][1];
  rotationFloat[1][2] = imageToTracker[1][2];
  rotationFloat[2][0] = imageToTracker[2][0];
  rotationFloat[2][1] = imageToTracker[2][1];
  rotationFloat[2][2] = imageToTracker[2][2];
  translationFloat[0] = imageToTracker[0][3];
  translationFloat[1] = imageToTracker[1][3];
  translationFloat[2] = imageToTracker[2][3];

  imageToTrackerTransform->SetTranslation(translationFloat);
  imageToTrackerTransform->SetMatrix(rotationFloat);

  m_CombinedModality->SetCalibration(imageToTrackerTransform);
  m_Controls.m_ToolBox->setItemEnabled(2, true);
  m_Controls.m_SavePlusCalibration->setEnabled(true);
  m_Controls.m_GotCalibrationLabel->setStyleSheet("QLabel { color : green; }");
  m_Controls.m_GotCalibrationLabel->setText("Recieved Calibration from PLUS you can now save it");
}

void QmitkUltrasoundCalibration::OnStopCalibrationProcess()
{
  this->ClearTemporaryMembers();

  m_Timer->stop();

  this->GetDataStorage()->Remove(m_Node);
  m_Node = nullptr;

  this->GetDataStorage()->Remove(m_CalibNode);
  m_CalibNode = nullptr;

  this->GetDataStorage()->Remove(m_WorldNode);
  m_WorldNode = nullptr;

  m_Controls.m_ToolBox->setCurrentIndex(0);
}

void QmitkUltrasoundCalibration::OnDeviceServiceEvent(const ctkServiceEvent event)
{
  if (m_CombinedModality.IsNull() || event.getType() != ctkServiceEvent::MODIFIED)
  {
    return;
  }

  ctkServiceReference service = event.getServiceReference();

  QString curDepth =
    service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH)).toString();
  if (m_CurrentDepth != curDepth)
  {
    m_CurrentDepth = curDepth;
    this->OnReset();
  }
}

void QmitkUltrasoundCalibration::OnAddCalibPoint()
{
  auto world = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetSelectedPosition();

  this->m_CalibPointsImage->InsertPoint(m_CalibPointsCount, world);
  this->m_CalibPointsTool->InsertPoint(m_CalibPointsCount, this->m_FreezePoint);

  QString text = text.number(m_CalibPointsCount + 1);
  text = "Point " + text;
  this->m_Controls.m_CalibPointList->addItem(text);

  m_CalibPointsCount++;
  SwitchFreeze();
}

void QmitkUltrasoundCalibration::OnCalibration()
{
  if (m_CombinedModality.IsNull())
  {
    return;
  }

  // Compute transformation
  vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();

  transform->SetSourceLandmarks(this->ConvertPointSetToVtkPolyData(m_CalibPointsImage)->GetPoints());
  transform->SetTargetLandmarks(this->ConvertPointSetToVtkPolyData(m_CalibPointsTool)->GetPoints());

  if (!m_CombinedModality->GetIsTrackedUltrasoundActive())
  {
    if (m_Controls.m_ScaleTransform->isChecked())
    {
      transform->SetModeToSimilarity();
    } // use affine transform
    else
    {
      transform->SetModeToRigidBody();
    } // use similarity transform: scaling is not touched
    MITK_INFO << "TEST";
  }
  else
  {
    transform->SetModeToRigidBody(); // use similarity transform: scaling is not touched
  }

  transform->Modified();
  transform->Update();

  // Convert from vtk to itk data types
  itk::Matrix<mitk::ScalarType, 3, 3> rotationFloat = itk::Matrix<mitk::ScalarType, 3, 3>();
  itk::Vector<mitk::ScalarType, 3> translationFloat = itk::Vector<mitk::ScalarType, 3>();
  vtkSmartPointer<vtkMatrix4x4> m = transform->GetMatrix();
  rotationFloat[0][0] = m->GetElement(0, 0);
  rotationFloat[0][1] = m->GetElement(0, 1);
  rotationFloat[0][2] = m->GetElement(0, 2);
  rotationFloat[1][0] = m->GetElement(1, 0);
  rotationFloat[1][1] = m->GetElement(1, 1);
  rotationFloat[1][2] = m->GetElement(1, 2);
  rotationFloat[2][0] = m->GetElement(2, 0);
  rotationFloat[2][1] = m->GetElement(2, 1);
  rotationFloat[2][2] = m->GetElement(2, 2);
  translationFloat[0] = m->GetElement(0, 3);
  translationFloat[1] = m->GetElement(1, 3);
  translationFloat[2] = m->GetElement(2, 3);

  mitk::PointSet::Pointer ImagePointsTransformed = m_CalibPointsImage->Clone();
  this->ApplyTransformToPointSet(ImagePointsTransformed, transform);
  mitk::DataNode::Pointer CalibPointsImageTransformed =
    this->GetDataStorage()->GetNamedNode("Calibration Points Image (Transformed)");
  if (CalibPointsImageTransformed.IsNull())
  {
    CalibPointsImageTransformed = mitk::DataNode::New();
    CalibPointsImageTransformed->SetName("Calibration Points Image (Transformed)");
    this->GetDataStorage()->Add(CalibPointsImageTransformed);
  }
  CalibPointsImageTransformed->SetData(ImagePointsTransformed);

  // Set new calibration transform
  m_Transformation = mitk::AffineTransform3D::New();
  m_Transformation->SetTranslation(translationFloat);
  m_Transformation->SetMatrix(rotationFloat);
  MITK_INFO << "New Calibration transform: " << m_Transformation;

  mitk::SlicedGeometry3D::Pointer sliced3d = dynamic_cast<mitk::SlicedGeometry3D *>(m_Node->GetData()->GetGeometry());

  mitk::PlaneGeometry::Pointer plane = const_cast<mitk::PlaneGeometry *>(sliced3d->GetPlaneGeometry(0));

  plane->SetIndexToWorldTransform(m_Transformation);

  // Save to US-Device
  m_CombinedModality->SetCalibration(m_Transformation);
  m_Controls.m_ToolBox->setItemEnabled(2, true);

  // Save to NeedleProjectionFilter
  m_NeedleProjectionFilter->SetTargetPlane(m_Transformation);

  // Update Calibration FRE
  m_CalibrationStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
  mitk::PointSet::Pointer p1 =
    this->m_CalibPointsTool->Clone(); // We use clones to calculate statistics to avoid concurrency Problems

  // Create point set with transformed image calibration points for
  // calculating the difference of image calibration and tool
  // calibration points in one geometry space
  mitk::PointSet::Pointer p2 = mitk::PointSet::New();
  int n = 0;
  for (mitk::PointSet::PointsConstIterator it = m_CalibPointsImage->Begin(); it != m_CalibPointsImage->End(); ++it, ++n)
  {
    p2->InsertPoint(n, m_Transformation->TransformPoint(it->Value()));
  }

  m_CalibrationStatistics->SetPointSets(p1, p2);
  // QString text = text.number(m_CalibrationStatistics->GetRMS());
  QString text = QString::number(ComputeFRE(m_CalibPointsImage, m_CalibPointsTool, transform));
  MITK_INFO << "Calibration FRE: " << text.toStdString().c_str();
  m_Controls.m_EvalLblCalibrationFRE->setText(text);

  m_Node->SetStringProperty("Calibration FRE", text.toStdString().c_str());
  // Enable Button to save Calibration
  m_Controls.m_CalibBtnSaveCalibration->setEnabled(true);
}

void QmitkUltrasoundCalibration::OnLoadPhantomConfiguration()
{
  // clear all data
  ClearTemporaryMembers();
  // reset UI
  m_Controls.m_CalibBtnMatchAnnotationToPhantomConfiguration->setEnabled(false);
  m_Controls.m_RefinePhantomAnnotationsGroupBox->setEnabled(false);
  m_Controls.m_CalibBtnPerformPhantomCalibration->setEnabled(false);
  m_Controls.m_CalibBtnSavePhantomCalibration->setEnabled(false);

  // open phantom configuration
  QString fileName = QFileDialog::getOpenFileName(nullptr, "Load phantom configuration", "", "*.mps");

  // dialog closed or selection canceled
  if (fileName.isNull())
  {
    return;
  }

  m_PhantomConfigurationPointSet = dynamic_cast<mitk::PointSet *>(mitk::IOUtil::Load(fileName.toStdString()).at(0).GetPointer());

  // transform phantom fiducials to tracking space
  mitk::NavigationData::Pointer currentSensorData = this->m_Tracker->GetOutput(0)->Clone();

  for (int i = 0; i < m_PhantomConfigurationPointSet->GetSize(); i++)
  {
    mitk::Point3D phantomPoint = m_PhantomConfigurationPointSet->GetPoint(i);
    mitk::Point3D transformedPoint = currentSensorData->TransformPoint(phantomPoint);
    this->m_CalibPointsTool->InsertPoint(i, transformedPoint);
  }

  // add point set interactor for image calibration points
  mitk::PointSetDataInteractor::Pointer imageCalibrationPointSetInteractor = mitk::PointSetDataInteractor::New();
  imageCalibrationPointSetInteractor->LoadStateMachine("PointSet.xml");
  imageCalibrationPointSetInteractor->SetEventConfig("PointSetConfig.xml");
  imageCalibrationPointSetInteractor->SetDataNode(m_WorldNode);
  imageCalibrationPointSetInteractor->SetMaxPoints(m_PhantomConfigurationPointSet->GetSize());
  // Call On AddCalibPointPhantomBased() when point was added
  itk::SimpleMemberCommand<QmitkUltrasoundCalibration>::Pointer pointAddedCommand =
    itk::SimpleMemberCommand<QmitkUltrasoundCalibration>::New();
  pointAddedCommand->SetCallbackFunction(this, &QmitkUltrasoundCalibration::OnPhantomCalibPointsChanged);
  m_CalibPointsImage->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);
  m_CalibPointsImage->AddObserver(mitk::PointSetMoveEvent(), pointAddedCommand);
  // Set size of image points points
  m_WorldNode->ReplaceProperty("point 2D size", mitk::FloatProperty::New(10.0));
}

void QmitkUltrasoundCalibration::OnPhantomCalibPointsChanged()
{
  int currentIndex = m_CalibPointsImage->SearchSelectedPoint();
  mitk::Point3D currentImagePoint = m_CalibPointsImage->GetPoint(currentIndex);
  UpdatePhantomAnnotationPointVisualization(currentIndex);
  // create sphere to show radius in which next point has to be placed
   this->GetDataStorage()->Remove(this->GetDataStorage()->GetNamedNode("NextPointIndicator"));
   if (currentIndex < m_CalibPointsTool->GetSize() - 1)
  {
    float distanceToNextPoint =
      m_CalibPointsTool->GetPoint(currentIndex).EuclideanDistanceTo(m_CalibPointsTool->GetPoint(currentIndex + 1));
    vtkSmartPointer<vtkSphereSource> vtkHelperSphere = vtkSmartPointer<vtkSphereSource>::New();
    vtkHelperSphere->SetCenter(currentImagePoint[0], currentImagePoint[1], currentImagePoint[2]);
    vtkHelperSphere->SetRadius(distanceToNextPoint);
    vtkHelperSphere->SetPhiResolution(40);
    vtkHelperSphere->SetThetaResolution(40);
    vtkHelperSphere->Update();
    mitk::Surface::Pointer helperSphere = mitk::Surface::New();
    helperSphere->SetVtkPolyData(vtkHelperSphere->GetOutput());
    mitk::DataNode::Pointer helperSphereNode = mitk::DataNode::New();
    helperSphereNode->SetName("NextPointIndicator");
    helperSphereNode->SetData(helperSphere);
    helperSphereNode->SetColor(0.0, 1.0, 0.0);
    this->GetDataStorage()->Add(helperSphereNode);
  }
  if (m_CalibPointsTool->GetSize() == m_CalibPointsImage->GetSize())
  {
    m_Controls.m_CalibBtnMatchAnnotationToPhantomConfiguration->setEnabled(true);
  }
}

void QmitkUltrasoundCalibration::UpdatePhantomAnnotationPointVisualization(int index)
{
  mitk::Point3D currentImagePoint = m_CalibPointsImage->GetPoint(index);
  // create sphere to show current fiducial
  std::stringstream pointName;
  pointName << "Point";
  pointName << index;
  this->GetDataStorage()->Remove(this->GetDataStorage()->GetNamedNode(pointName.str()));
  vtkSmartPointer<vtkSphereSource> vtkPointSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkPointSphere->SetCenter(currentImagePoint[0], currentImagePoint[1], currentImagePoint[2]);
  vtkPointSphere->SetRadius(5.0);
  vtkPointSphere->SetPhiResolution(40);
  vtkPointSphere->SetThetaResolution(40);
  vtkPointSphere->Update();
  mitk::Surface::Pointer pointSphere = mitk::Surface::New();
  pointSphere->SetVtkPolyData(vtkPointSphere->GetOutput());
  mitk::DataNode::Pointer sphereNode = mitk::DataNode::New();
  sphereNode->SetName(pointName.str());
  sphereNode->SetData(pointSphere);
  sphereNode->SetColor(1.0, 1.0, 0.0);
  this->GetDataStorage()->Add(sphereNode);
}

void QmitkUltrasoundCalibration::OnMatchAnnotationToPhantomConfiguration()
{
  // Transform pointset of phantom configuration to currently annotated image points
  vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();
  transform->SetModeToRigidBody();
  vtkSmartPointer<vtkPoints> toolLandmarks = this->ConvertPointSetToVtkPolyData(m_CalibPointsTool)->GetPoints();
  transform->SetSourceLandmarks(toolLandmarks);
  transform->SetTargetLandmarks(this->ConvertPointSetToVtkPolyData(m_CalibPointsImage)->GetPoints());
  transform->Update();
  // update image annotation with matched phantom configuration
  vtkSmartPointer<vtkPoints> transformedToolLandmarks = vtkSmartPointer<vtkPoints>::New();
  transform->TransformPoints(toolLandmarks, transformedToolLandmarks);
  for (int i = 0; i < transformedToolLandmarks->GetNumberOfPoints(); i++)
  {
    m_CalibPointsImage->InsertPoint(i, transformedToolLandmarks->GetPoint(i));
    UpdatePhantomAnnotationPointVisualization(i);
  }
  m_Controls.m_RefinePhantomAnnotationsGroupBox->setEnabled(true);
  m_Controls.m_CalibBtnPerformPhantomCalibration->setEnabled(true);
}

void QmitkUltrasoundCalibration::TranslatePhantomAnnotations(double tx, double ty, double tz)
{
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Translate(tx, ty, tz);
  vtkSmartPointer<vtkPoints> currentPoints = this->ConvertPointSetToVtkPolyData(m_CalibPointsImage)->GetPoints();
  vtkSmartPointer<vtkPoints> transformedPoints = vtkSmartPointer<vtkPoints>::New();
  transform->TransformPoints(currentPoints, transformedPoints);
  for (int i = 0; i < transformedPoints->GetNumberOfPoints(); i++)
  {
    m_CalibPointsImage->InsertPoint(i, transformedPoints->GetPoint(i));
    UpdatePhantomAnnotationPointVisualization(i);
  }
}

void QmitkUltrasoundCalibration::RotatePhantomAnnotations(double angle)
{
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->RotateZ(angle);
  vtkSmartPointer<vtkPoints> currentPoints = this->ConvertPointSetToVtkPolyData(m_CalibPointsImage)->GetPoints();
  vtkSmartPointer<vtkPoints> transformedPoints = vtkSmartPointer<vtkPoints>::New();
  transform->TransformPoints(currentPoints, transformedPoints);
  for (int i = 0; i < transformedPoints->GetNumberOfPoints(); i++)
  {
    m_CalibPointsImage->InsertPoint(i, transformedPoints->GetPoint(i));
    UpdatePhantomAnnotationPointVisualization(i);
  }
}

void QmitkUltrasoundCalibration::OnMovePhantomAnnotationsUp()
{
  this->TranslatePhantomAnnotations(0, -m_Image->GetGeometry()->GetSpacing()[1], 0);
}

void QmitkUltrasoundCalibration::OnMovePhantomAnnotationsDown()
{
  this->TranslatePhantomAnnotations(0, m_Image->GetGeometry()->GetSpacing()[1], 0);
}

void QmitkUltrasoundCalibration::OnMovePhantomAnnotationsLeft()
{
  this->TranslatePhantomAnnotations(-m_Image->GetGeometry()->GetSpacing()[0], 0, 0);
}

void QmitkUltrasoundCalibration::OnMovePhantomAnnotationsRight()
{
  this->TranslatePhantomAnnotations(m_Image->GetGeometry()->GetSpacing()[0], 0, 0);
}

void QmitkUltrasoundCalibration::OnRotatePhantomAnnotationsRight()
{
  mitk::BoundingBox::PointType centerOfPointSet = m_CalibPointsImage->GetGeometry()->GetBoundingBox()->GetCenter();
  this->TranslatePhantomAnnotations(-centerOfPointSet[0], -centerOfPointSet[1], -centerOfPointSet[2]);
  this->RotatePhantomAnnotations(0.5);
  this->TranslatePhantomAnnotations(centerOfPointSet[0], centerOfPointSet[1], centerOfPointSet[2]);
}

void QmitkUltrasoundCalibration::OnRotatePhantomAnnotationsLeft()
{
  mitk::BoundingBox::PointType centerOfPointSet = m_CalibPointsImage->GetGeometry()->GetBoundingBox()->GetCenter();
  this->TranslatePhantomAnnotations(-centerOfPointSet[0], -centerOfPointSet[1], -centerOfPointSet[2]);
  this->RotatePhantomAnnotations(-0.5);
  this->TranslatePhantomAnnotations(centerOfPointSet[0], centerOfPointSet[1], centerOfPointSet[2]);
}

void QmitkUltrasoundCalibration::OnPhantomBasedCalibration()
{
  // perform calibration
  OnCalibration();
  m_Controls.m_CalibBtnSavePhantomCalibration->setEnabled(true);
}

void QmitkUltrasoundCalibration::OnAddEvalTargetPoint()
{
  auto world = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetSelectedPosition();

  this->m_EvalPointsImage->InsertPoint(m_EvalPointsImage->GetSize(), world);
  this->m_EvalPointsTool->InsertPoint(m_EvalPointsTool->GetSize(), this->m_FreezePoint);

  QString text = text.number(this->m_EvalPointsTool->GetSize());
  this->m_Controls.m_EvalLblNumTargetPoints->setText(text);

  // Update FREs
  // Update Evaluation FRE, but only if it contains more than one point (will crash otherwise)
  if ((m_EvalPointsProjected->GetSize() > 1) && (m_EvalPointsTool->GetSize() > 1))
  {
    m_EvaluationStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
    m_ProjectionStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
    mitk::PointSet::Pointer p1 =
      this->m_EvalPointsTool->Clone(); // We use clones to calculate statistics to avoid concurrency Problems
    mitk::PointSet::Pointer p2 = this->m_EvalPointsImage->Clone();
    mitk::PointSet::Pointer p3 = this->m_EvalPointsProjected->Clone();
    m_EvaluationStatistics->SetPointSets(p1, p2);
    m_ProjectionStatistics->SetPointSets(p1, p3);
    QString evalText = evalText.number(m_EvaluationStatistics->GetRMS());
    QString projText = projText.number(m_ProjectionStatistics->GetRMS());
    m_Controls.m_EvalLblEvaluationFRE->setText(evalText);
    m_Controls.m_EvalLblProjectionFRE->setText(projText);
  }
  SwitchFreeze();
}

void QmitkUltrasoundCalibration::OnAddEvalProjectedPoint()
{
  MITK_WARN << "Projection Evaluation may currently be inaccurate.";
  // TODO: Verify correct Evaluation. Is the Point that is added really current?
  mitk::Point3D projection = this->m_NeedleProjectionFilter->GetProjection()->GetPoint(1);
  m_EvalPointsProjected->InsertPoint(m_EvalPointsProjected->GetSize(), projection);
  QString text = text.number(this->m_EvalPointsProjected->GetSize());
  this->m_Controls.m_EvalLblNumProjectionPoints->setText(text);
}

void QmitkUltrasoundCalibration::OnSaveEvaluation()
{
  // Filename without suffix
  QString filename = m_Controls.m_EvalFilePath->text() + "//" + m_Controls.m_EvalFilePrefix->text();

  MITK_WARN << "CANNOT SAVE, ABORTING!";
  /* not working any more TODO!
  mitk::PointSetWriter::Pointer psWriter = mitk::PointSetWriter::New();
  psWriter->SetInput(0, m_CalibPointsImage);
  psWriter->SetInput(1, m_CalibPointsTool);
  psWriter->SetInput(2, m_EvalPointsImage);
  psWriter->SetInput(3, m_EvalPointsTool);
  psWriter->SetInput(4, m_EvalPointsProjected);
  psWriter->SetFileName(filename.toStdString() + ".xml");
  psWriter->Write();
  */

  // TODO: New writer for transformations must be implemented.
  /*
  mitk::TransformationFileWriter::Pointer tWriter = mitk::TransformationFileWriter::New();
  tWriter->SetInput(0, m_CalibPointsImage);
  tWriter->SetInput(1, m_CalibPointsTool);
  tWriter->SetInput(2, m_EvalPointsImage);
  tWriter->SetInput(3, m_EvalPointsTool);
  tWriter->SetInput(4, m_EvalPointsProjected);
  tWriter->SetOutputFilename(filename.toStdString() + ".txt");
  tWriter->DoWrite(this->m_Transformation);
  */
}

void QmitkUltrasoundCalibration::OnSaveCalibration()
{
  m_Controls.m_GotCalibrationLabel->setText("");
  QString filename =
    QFileDialog::getSaveFileName(QApplication::activeWindow(), "Save Calibration", "", "Calibration files *.cal");

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
  {
    MITK_WARN << "Cannot open file '" << filename.toStdString() << "' for writing.";
    return;
  }

  std::string calibrationSerialization = m_CombinedModality->SerializeCalibration();

  QTextStream outStream(&file);
  outStream << QString::fromStdString(calibrationSerialization);

  // save additional information
  if (m_Controls.m_saveAdditionalCalibrationLog->isChecked())
  {
    mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
    QString filenameScene = filename + "_mitkScene.mitk";
    mitk::NodePredicateNot::Pointer isNotHelperObject =
      mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = this->GetDataStorage()->GetSubset(isNotHelperObject);
    mySceneIO->SaveScene(nodesToBeSaved, this->GetDataStorage(), filenameScene.toStdString().c_str());
  }
}

void QmitkUltrasoundCalibration::OnReset()
{
  this->ClearTemporaryMembers();

  if (m_Transformation.IsNull())
  {
    m_Transformation = mitk::AffineTransform3D::New();
  }
  m_Transformation->SetIdentity();

  if (m_Node.IsNotNull() && (m_Node->GetData() != nullptr) && (m_Node->GetData()->GetGeometry() != nullptr))
  {
    mitk::SlicedGeometry3D::Pointer sliced3d = dynamic_cast<mitk::SlicedGeometry3D *>(m_Node->GetData()->GetGeometry());
    mitk::PlaneGeometry::Pointer plane = const_cast<mitk::PlaneGeometry *>(sliced3d->GetPlaneGeometry(0));
    plane->SetIndexToWorldTransform(m_Transformation);
  }

  QString text1 = text1.number(this->m_EvalPointsTool->GetSize());
  this->m_Controls.m_EvalLblNumTargetPoints->setText(text1);
  QString text2 = text2.number(this->m_EvalPointsProjected->GetSize());
  this->m_Controls.m_EvalLblNumProjectionPoints->setText(text2);
}

void QmitkUltrasoundCalibration::Update()
{
  // Update Tracking Data
  std::vector<mitk::NavigationData::Pointer> *datas = new std::vector<mitk::NavigationData::Pointer>();
  datas->push_back(m_Tracker->GetOutput());
  m_Controls.m_CalibTrackingStatus->SetNavigationDatas(datas);
  m_Controls.m_CalibTrackingStatus->Refresh();
  m_Controls.m_EvalTrackingStatus->SetNavigationDatas(datas);
  m_Controls.m_EvalTrackingStatus->Refresh();

  m_CombinedModality->Modified();
  m_CombinedModality->Update();

  // Update US Image
  mitk::Image::Pointer image = m_CombinedModality->GetOutput();
  // make sure that always the current image is set to the data node
  if (image.IsNotNull() && m_Node->GetData() != image.GetPointer() && image->IsInitialized())
  {
    m_Node->SetData(image);
  }

  // Update Needle Projection
  m_NeedleProjectionFilter->Update();

  // only update 2d window because it is faster
  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void QmitkUltrasoundCalibration::SwitchFreeze()
{
  m_Controls.m_CalibBtnAddPoint->setEnabled(false); // generally deactivate
  // We use the activity state of the timer to determine whether we are currently viewing images
  if (!m_Timer->isActive()) // Activate Imaging
  {
    // if (m_Node) m_Node->ReleaseData();
    if (m_CombinedModality.IsNull())
    {
      m_Timer->stop();
      return;
    }

    m_CombinedModality->Update();
    m_Image = m_CombinedModality->GetOutput();
    if (m_Image.IsNotNull() && m_Image->IsInitialized())
    {
      m_Node->SetData(m_Image);
    }

    std::vector<mitk::NavigationData::Pointer> datas;
    datas.push_back(m_Tracker->GetOutput());
    m_Controls.m_CalibTrackingStatus->SetNavigationDatas(&datas);
    m_Controls.m_CalibTrackingStatus->ShowStatusLabels();
    m_Controls.m_CalibTrackingStatus->Refresh();

    m_Controls.m_EvalTrackingStatus->SetNavigationDatas(&datas);
    m_Controls.m_EvalTrackingStatus->ShowStatusLabels();
    m_Controls.m_EvalTrackingStatus->Refresh();

    int interval = 40;
    m_Timer->setInterval(interval);
    m_Timer->start();

    m_CombinedModality->SetIsFreezed(false);
  }
  else if (this->m_Tracker->GetOutput(0)->IsDataValid())
  {
    // deactivate Imaging
    m_Timer->stop();
    // Remember last tracking coordinates
    m_FreezePoint = this->m_Tracker->GetOutput(0)->GetPosition();
    m_Controls.m_CalibBtnAddPoint->setEnabled(true); // activate only, if valid point is set

    m_CombinedModality->SetIsFreezed(true);
  }
}

void QmitkUltrasoundCalibration::ShowNeedlePath()
{
  // Init Filter
  this->m_NeedleProjectionFilter->SelectInput(0);

  // Create Node for Pointset
  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if (node.IsNull())
  {
    node = mitk::DataNode::New();
    node->SetName("Needle Path");
    node->SetData(m_NeedleProjectionFilter->GetProjection());
    node->SetBoolProperty("show contour", true);
    this->GetDataStorage()->Add(node);
  }
}

void QmitkUltrasoundCalibration::ClearTemporaryMembers()
{
  m_CalibPointsTool->Clear();
  m_CalibPointsImage->Clear();
  m_CalibPointsCount = 0;

  m_EvalPointsImage->Clear();
  m_EvalPointsTool->Clear();
  m_EvalPointsProjected->Clear();

  this->m_Controls.m_CalibPointList->clear();

  m_SpacingPoints->Clear();
  m_Controls.m_SpacingPointsList->clear();
  m_SpacingPointsCount = 0;
}

vtkSmartPointer<vtkPolyData> QmitkUltrasoundCalibration::ConvertPointSetToVtkPolyData(mitk::PointSet::Pointer PointSet)
{
  vtkSmartPointer<vtkPolyData> returnValue = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for (int i = 0; i < PointSet->GetSize(); i++)
  {
    double point[3] = {PointSet->GetPoint(i)[0], PointSet->GetPoint(i)[1], PointSet->GetPoint(i)[2]};
    points->InsertNextPoint(point);
  }
  vtkSmartPointer<vtkPolyData> temp = vtkSmartPointer<vtkPolyData>::New();
  temp->SetPoints(points);

  vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  vertexFilter->SetInputData(temp);
  vertexFilter->Update();

  returnValue->ShallowCopy(vertexFilter->GetOutput());

  return returnValue;
}

double QmitkUltrasoundCalibration::ComputeFRE(mitk::PointSet::Pointer imageFiducials,
                                              mitk::PointSet::Pointer realWorldFiducials,
                                              vtkSmartPointer<vtkLandmarkTransform> transform)
{
  if (imageFiducials->GetSize() != realWorldFiducials->GetSize())
    return -1;
  double FRE = 0;
  for (int i = 0; i < imageFiducials->GetSize(); ++i)
  {
    itk::Point<double> current_image_fiducial_point = imageFiducials->GetPoint(i);
    if (transform != nullptr)
    {
      current_image_fiducial_point = transform->TransformPoint(
        imageFiducials->GetPoint(i)[0], imageFiducials->GetPoint(i)[1], imageFiducials->GetPoint(i)[2]);
    }
    double cur_error_squared = current_image_fiducial_point.SquaredEuclideanDistanceTo(realWorldFiducials->GetPoint(i));
    FRE += cur_error_squared;
  }

  FRE = sqrt(FRE / (double)imageFiducials->GetSize());

  return FRE;
}

void QmitkUltrasoundCalibration::ApplyTransformToPointSet(mitk::PointSet::Pointer pointSet,
                                                          vtkSmartPointer<vtkAbstractTransform> transform)
{
  for (int i = 0; i < pointSet->GetSize(); ++i)
  {
    itk::Point<double> current_point_transformed = itk::Point<double>();
    current_point_transformed =
      transform->TransformPoint(pointSet->GetPoint(i)[0], pointSet->GetPoint(i)[1], pointSet->GetPoint(i)[2]);
    pointSet->SetPoint(i, current_point_transformed);
  }
}

void QmitkUltrasoundCalibration::OnFreezeClicked()
{
  if (m_CombinedModality.IsNull())
  {
    return;
  }

  if (m_CombinedModality->GetIsFreezed())
  {
    // device was already frozen so we need to delete all spacing points because they need to be collected all at once
    // no need to check if all four points are already collected, because if that's the case you can no longer click the
    // Freeze button
    m_SpacingPoints->Clear();
    m_Controls.m_SpacingPointsList->clear();
    m_SpacingPointsCount = 0;
    m_Controls.m_SpacingAddPoint->setEnabled(false);
  }
  else
  {
    m_Controls.m_SpacingAddPoint->setEnabled(true);
  }
  SwitchFreeze();
}

void QmitkUltrasoundCalibration::OnAddSpacingPoint()
{
  auto point = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetSelectedPosition();

  this->m_SpacingPoints->InsertPoint(m_SpacingPointsCount, point);

  QString text = text.number(m_SpacingPointsCount + 1);
  text = "Point " + text;

  this->m_Controls.m_SpacingPointsList->addItem(text);

  m_SpacingPointsCount++;

  if (m_SpacingPointsCount == 4) // now we have all 4 points needed
  {
    m_Controls.m_SpacingAddPoint->setEnabled(false);
    m_Controls.m_CalculateSpacing->setEnabled(true);
    m_Controls.m_SpacingBtnFreeze->setEnabled(false);
  }
}

void QmitkUltrasoundCalibration::OnCalculateSpacing()
{
  mitk::Point3D horizontalOne = m_SpacingPoints->GetPoint(0);
  mitk::Point3D horizontalTwo = m_SpacingPoints->GetPoint(1);
  mitk::Point3D verticalOne = m_SpacingPoints->GetPoint(2);
  mitk::Point3D verticalTwo = m_SpacingPoints->GetPoint(3);

  // Get the distances between the points in the image
  double xDistance = horizontalOne.EuclideanDistanceTo(horizontalTwo);
  double yDistance = verticalOne.EuclideanDistanceTo(verticalTwo);

  // Calculate the spacing of the image and fill a vector with it
  double xSpacing = 30 / xDistance;
  double ySpacing = 20 / yDistance;

  m_CombinedModality->GetUltrasoundDevice()->SetSpacing(xSpacing, ySpacing);

  // Now that the spacing is set clear all stuff and return to Calibration
  m_SpacingPoints->Clear();
  m_Controls.m_SpacingPointsList->clear();
  m_SpacingPointsCount = 0;
  m_CombinedModality->SetIsFreezed(false);
}

void QmitkUltrasoundCalibration::OnUSDepthChanged(const std::string &key, const std::string &)
{
  // whenever depth of USImage is changed the spacing should no longer be overwritten
  if (key == mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH)
  {
  }
}
