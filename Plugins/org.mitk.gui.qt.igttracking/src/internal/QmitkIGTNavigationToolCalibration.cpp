/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <numeric>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkIGTNavigationToolCalibration.h"

// mitk
#include <mitkNavigationToolWriter.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkTrackingDevice.h>
#include <mitkTrackingTool.h>
#include <mitkQuaternionAveraging.h>
#include <mitkPivotCalibration.h>
#include <mitkNavigationData.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

// vtk
#include <vtkSphereSource.h>

const std::string QmitkIGTNavigationToolCalibration::VIEW_ID = "org.mitk.views.igtnavigationtoolcalibration";

QmitkIGTNavigationToolCalibration::QmitkIGTNavigationToolCalibration()
{
  m_ToolTransformationWidget = new QmitkInteractiveTransformationWidget();
}

QmitkIGTNavigationToolCalibration::~QmitkIGTNavigationToolCalibration()
{
  //The following code is required due to a bug in the point list widget.
  //If this is removed, MITK crashes when closing the view:
  m_Controls.m_RegistrationLandmarkWidget->SetPointSetNode(nullptr);
  m_Controls.m_CalibrationLandmarkWidget->SetPointSetNode(nullptr);

  //clean up data storage
  this->GetDataStorage()->Remove(m_ToolTipPointPreview);


  delete m_ToolTransformationWidget;
}

void QmitkIGTNavigationToolCalibration::SetFocus()
{

}

void QmitkIGTNavigationToolCalibration::OnToolCalibrationMethodChanged(int index)
{
  //if pivot calibration (3) or manual(0) is chosen only calibration pointer is needed
  if (index == 0 || index == 3) {

    if (!CheckInitialization(false)) {
      return;
    }
  }
  else{
    if (!CheckInitialization()) { return; }
  }

  UpdateManualToolTipCalibrationView();
  m_Controls.m_CalibrationMethodsWidget->setCurrentIndex(index);
  m_IndexCurrentCalibrationMethod = index;
}

void QmitkIGTNavigationToolCalibration::CreateQtPartControl(QWidget *parent)
{
  m_TrackingTimer = new QTimer(this);

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.m_SetToolToCalibrate, SIGNAL(clicked()), this, SLOT(SetToolToCalibrate()));
  connect(m_Controls.m_SetPointer, SIGNAL(clicked()), this, SLOT(SetCalibrationPointer()));
  connect(m_TrackingTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackingTimer()));
  connect(m_Controls.m_AddLandmark, SIGNAL(clicked()), this, SLOT(AddLandmark()));
  connect(m_Controls.m_SaveCalibratedTool, SIGNAL(clicked()), this, SLOT(SaveCalibratedTool()));
  connect(m_Controls.m_AddPivotPose, SIGNAL(clicked()), this, SLOT(OnAddPivotPose()));
  connect(m_Controls.m_ComputePivot, SIGNAL(clicked()), this, SLOT(OnComputePivot()));
  connect(m_Controls.m_UseComputedPivotPoint, SIGNAL(clicked()), this, SLOT(OnUseComputedPivotPoint()));
  connect(m_Controls.m_StartEditTooltipManually, SIGNAL(clicked()), this, SLOT(OnStartManualToolTipCalibration()));
  connect(m_Controls.m_GetPositions, SIGNAL(clicked()), this, SLOT(OnGetPositions()));
  connect(m_Controls.m_ToolAxis_X, SIGNAL(valueChanged(double)), this, SLOT(OnToolAxisSpinboxChanged()));
  connect(m_Controls.m_ToolAxis_Y, SIGNAL(valueChanged(double)), this, SLOT(OnToolAxisSpinboxChanged()));
  connect(m_Controls.m_ToolAxis_Z, SIGNAL(valueChanged(double)), this, SLOT(OnToolAxisSpinboxChanged()));
  connect(m_Controls.m_CalibrateToolAxis, SIGNAL(clicked()), this, SLOT(OnCalibrateToolAxis()));
  connect((QObject*)(m_ToolTransformationWidget), SIGNAL(EditToolTipFinished(mitk::AffineTransform3D::Pointer)), this,
    SLOT(OnManualEditToolTipFinished(mitk::AffineTransform3D::Pointer)));
  connect(m_Controls.m_CalibrationMethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnToolCalibrationMethodChanged(int)));

  connect((QObject*)(m_Controls.m_RunCalibrationButton), SIGNAL(clicked()), (QObject*) this, SLOT(OnRunSingleRefToolCalibrationClicked()));
  connect((QObject*)(m_Controls.m_CollectNavigationDataButton), SIGNAL(clicked()), (QObject*) this, SLOT(OnLoginSingleRefToolNavigationDataClicked()));
  connect((QObject*)(m_Controls.m_SetNewToolTipPosButton), SIGNAL(clicked()), (QObject*) this, SLOT(OnSetNewToolTipPosButtonClicked()));

  m_IDToolToCalibrate = -1;
  m_IDCalibrationPointer = -1;
  m_IndexCurrentCalibrationMethod = -1;
  m_OnLoginSingleRefToolNavigationDataClicked = false;
  m_NumberOfNavigationDataCounter = 0;
  m_NumberOfNavigationData = -1;

  //for pivot calibration
  m_OnAddPivotPoseClicked = false;
  PivotCount = 0;
  m_PivotPoses = std::vector<mitk::NavigationData::Pointer>();

  m_CalibrationLandmarks = mitk::PointSet::New();
  m_CalibrationLandmarksNode = mitk::DataNode::New();
  m_CalibrationLandmarksNode->SetData(m_CalibrationLandmarks);
  m_Controls.m_CalibrationLandmarkWidget->SetPointSetNode(m_CalibrationLandmarksNode);

  m_RegistrationLandmarks = mitk::PointSet::New();
  m_RegistrationLandmarksNode = mitk::DataNode::New();
  m_RegistrationLandmarksNode->SetData(m_RegistrationLandmarks);
  m_Controls.m_RegistrationLandmarkWidget->SetPointSetNode(m_RegistrationLandmarksNode);

  m_ToolSurfaceInToolCoordinatesDataNode = mitk::DataNode::New();
  m_ToolSurfaceInToolCoordinatesDataNode->SetName("ToolSurface(ToolCoordinates)");

  m_LoggedNavigationDataDifferences = std::vector< mitk::NavigationData::Pointer >();
}


void QmitkIGTNavigationToolCalibration::OnRunSingleRefToolCalibrationClicked()
{
  if (!CheckInitialization()) { return; }

  mitk::NavigationData::Pointer ToolTipTransform = mitk::NavigationData::New();

  if (m_Controls.m_CalibratePosition->isChecked())
  {
    //1: Compute mean translational offset vector
    m_ResultOffsetVector.Fill(0);
    for (std::vector<mitk::Point3D>::iterator vecIter = m_LoggedNavigationDataOffsets.begin(); vecIter != m_LoggedNavigationDataOffsets.end(); vecIter++)
    {
      m_ResultOffsetVector[0] = m_ResultOffsetVector[0] + (*vecIter)[0];
      m_ResultOffsetVector[1] = m_ResultOffsetVector[1] + (*vecIter)[1];
      m_ResultOffsetVector[2] = m_ResultOffsetVector[2] + (*vecIter)[2];
    }
    m_ResultOffsetVector[0] = m_ResultOffsetVector[0] / m_LoggedNavigationDataOffsets.size();
    m_ResultOffsetVector[1] = m_ResultOffsetVector[1] / m_LoggedNavigationDataOffsets.size();
    m_ResultOffsetVector[2] = m_ResultOffsetVector[2] / m_LoggedNavigationDataOffsets.size();

    this->m_Controls.m_ResultOfCalibration->setText(
      QString("x: ") + QString(QString::number(m_ResultOffsetVector[0], 103, 3)) +
      QString("; y: ") + (QString::number(m_ResultOffsetVector[1], 103, 3)) +
      QString("; z: ") + (QString::number(m_ResultOffsetVector[2], 103, 3)));

    ToolTipTransform->SetPosition(m_ResultOffsetVector);
  }


  if (m_Controls.m_CalibrateOrientation->isChecked())
  {
    //2: Compute mean orientation
    mitk::Quaternion meanOrientation;
    std::vector <mitk::Quaternion> allOrientations = std::vector <mitk::Quaternion>();
    for (std::size_t i = 0; i < m_LoggedNavigationDataDifferences.size(); i++) { allOrientations.push_back(m_LoggedNavigationDataDifferences.at(i)->GetOrientation()); }
    meanOrientation = mitk::QuaternionAveraging::CalcAverage(allOrientations);
    this->m_Controls.m_ResultOfCalibrationOrientation->setText(
      QString("qx: ") + QString(QString::number(meanOrientation.x(), 103, 3)) +
      QString("; qy: ") + (QString::number(meanOrientation.y(), 103, 3)) +
      QString("; qz: ") + (QString::number(meanOrientation.z(), 103, 3)) +
      QString("; qr: ") + (QString::number(meanOrientation.r(), 103, 3)));

    ToolTipTransform->SetOrientation(meanOrientation);
  }

  MITK_INFO << "Computed calibration: ";
  MITK_INFO << "Translation Vector: " << ToolTipTransform->GetPosition();
  MITK_INFO << "Quaternion: (" << ToolTipTransform->GetOrientation() << ")";
  MITK_INFO << "Euler Angles [rad]: (" << ToolTipTransform->GetOrientation().rotation_euler_angles() << ")";
  MITK_INFO << "Matrix:";
  vnl_matrix_fixed<double, 3, 3> rotMatrix = ToolTipTransform->GetOrientation().rotation_matrix_transpose();
  MITK_INFO << rotMatrix[0][0] << " " << rotMatrix[0][1] << " " << rotMatrix[0][2] << std::endl;
  MITK_INFO << rotMatrix[1][0] << " " << rotMatrix[1][1] << " " << rotMatrix[1][2] << std::endl;
  MITK_INFO << rotMatrix[2][0] << " " << rotMatrix[2][1] << " " << rotMatrix[2][2] << std::endl;

  //3: write everything into the final tool tip transform and save it as member (it will be written to the tool later on)
  mitk::NavigationData::Pointer ToolTipInTrackingCoordinates = mitk::NavigationData::New();
  ToolTipInTrackingCoordinates->Compose(ToolTipTransform);
  ToolTipInTrackingCoordinates->Compose(m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate));
  ShowToolTipPreview(ToolTipInTrackingCoordinates);
  m_Controls.m_SetNewToolTipPosButton->setEnabled(true);
  m_ComputedToolTipTransformation = ToolTipTransform;

}

void QmitkIGTNavigationToolCalibration::OnLoginSingleRefToolNavigationDataClicked()
{

  if (!CheckInitialization()) { return; }

  //reset old data
  m_LoggedNavigationDataOffsets.clear();
  m_LoggedNavigationDataDifferences.clear();

  m_OnLoginSingleRefToolNavigationDataClicked = true;
  m_Controls.m_CollectNavigationDataButton->setEnabled(false);
  m_NumberOfNavigationData = m_Controls.m_NumberOfNavigationDataToCollect->value();
  MITK_INFO << "Collecting " << m_NumberOfNavigationData << " NavigationData ... " << endl;
}

void QmitkIGTNavigationToolCalibration::LoginSingleRefToolNavigationData()
{
  if (!CheckInitialization()) { return; }

  if (m_NumberOfNavigationDataCounter < m_NumberOfNavigationData)
  {
    //update label text
    QString labelText = "Collecting Data: " + QString::number(m_NumberOfNavigationDataCounter);
    m_Controls.m_CollectionStatus->setText(labelText);

    mitk::NavigationData::Pointer referenceTool = m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer);
    mitk::NavigationData::Pointer toolToCalibrate = m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate);

    //compute difference:
    // differenceND = toolToCalibrate^-1 * referenceTool
    mitk::NavigationData::Pointer differenceND = mitk::NavigationData::New();
    differenceND->Compose(referenceTool);
    differenceND->Compose(toolToCalibrate->GetInverse());

    //inverse mode...
    if (m_Controls.m_InvertQuaternions->isChecked())
    {
      // negate identity matrix to directly show parameters that will set up in NDI 6D Software Architect
      differenceND = differenceND->GetInverse();
    }

    //save difference in member
    m_LoggedNavigationDataOffsets.push_back(differenceND->GetPosition());
    m_LoggedNavigationDataDifferences.push_back(differenceND);
    m_NumberOfNavigationDataCounter++;
  }

  if (m_NumberOfNavigationDataCounter == m_NumberOfNavigationData)
  {
    m_NumberOfNavigationDataCounter = 0;
    m_OnLoginSingleRefToolNavigationDataClicked = false;
    m_Controls.m_CollectNavigationDataButton->setEnabled(true);
    m_Controls.m_RunCalibrationButton->setEnabled(true);
    MITK_INFO << "Collecting " << m_NumberOfNavigationData << " NavigationData ... Finished" << endl;
    QString labelText = "Collected " + QString::number(m_NumberOfNavigationData) + " data samples!";
    m_Controls.m_CollectionStatus->setText(labelText);
  }
}

void QmitkIGTNavigationToolCalibration::OnSetNewToolTipPosButtonClicked()
{
  ApplyToolTipTransform(m_ComputedToolTipTransformation);
  RemoveToolTipPreview();
}

void QmitkIGTNavigationToolCalibration::ClearOldPivot()
{
  mitk::NavigationData::Pointer tempND = mitk::NavigationData::New();
  this->ApplyToolTipTransform(tempND);
  UpdateManualToolTipCalibrationView();
  //m_ManualToolTipEditWidget->hide(); //TODO
  this->GetDataStorage()->Remove(m_ToolSurfaceInToolCoordinatesDataNode);
}
void QmitkIGTNavigationToolCalibration::OnAddPivotPose()
{
  ClearOldPivot();
  //When the collect Poses Button is Clicked
  m_OnAddPivotPoseClicked = true;
  m_NumberOfNavigationData = m_Controls.m_PosesToCollect->value();

}

void QmitkIGTNavigationToolCalibration::AddPivotPose()
{
  //Save the poses to be used in computation
  if (PivotCount < m_NumberOfNavigationData)
  {
    mitk::NavigationData::Pointer currentPose = mitk::NavigationData::New();
    currentPose->Graft(m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_IDToolToCalibrate));
    m_PivotPoses.push_back(currentPose);
    m_Controls.m_PoseNumber->setText(QString::number(m_PivotPoses.size()));
    PivotCount++;
  }
  if (PivotCount == m_NumberOfNavigationData)
  {
    m_OnAddPivotPoseClicked = false;
  }
}

void QmitkIGTNavigationToolCalibration::OnComputePivot()
{

  mitk::PivotCalibration::Pointer myPivotCalibration = mitk::PivotCalibration::New();
  for (std::size_t i = 0; i < this->m_PivotPoses.size(); i++)
  {
    myPivotCalibration->AddNavigationData(m_PivotPoses.at(i));
  }
  QString resultString;
  if (myPivotCalibration->ComputePivotResult())
  {
    mitk::NavigationData::Pointer markerTransformationTrackingCoordinates = m_PivotPoses.at(0);

    //Get computed pivot transfromation in tool coordinates

    mitk::NavigationData::Pointer ToolTipToTool = mitk::NavigationData::New();
    ToolTipToTool->SetPosition(myPivotCalibration->GetResultPivotPoint());
    ToolTipToTool->SetOrientation(mitk::Quaternion(0,0,0,1));
    mitk::NavigationData::Pointer TrackerToTool = mitk::NavigationData::New();
    TrackerToTool->SetOrientation(markerTransformationTrackingCoordinates->GetOrientation());
    TrackerToTool->SetPosition(markerTransformationTrackingCoordinates->GetPosition());
    TrackerToTool->Compose(ToolTipToTool);

    // Compute pivot point in relation to marker transformation for preview
    mitk::NavigationData::Pointer ToolTipToTracker = mitk::NavigationData::New();
    ToolTipToTracker->Compose(ToolTipToTool);
    ToolTipToTracker->Compose(markerTransformationTrackingCoordinates);

    //add the preview node to the data storage
    ShowToolTipPreview(ToolTipToTracker);

    //parse result string
    resultString = QString("Pivot computation succeeded!\n")
      + QString("RMS Error: ") + QString::number(myPivotCalibration->GetResultRMSError()) + QString("\n")
      + QString("Pivot Point: ") + QString::number(myPivotCalibration->GetResultPivotPoint()[0]) + ";" + QString::number(myPivotCalibration->GetResultPivotPoint()[1]) + ";" + QString::number(myPivotCalibration->GetResultPivotPoint()[2]) + QString("\n");

    //finally: save results to member variable
    m_ComputedToolTipTransformation = ToolTipToTool;


    //enable button to use the computed point with the tool
    m_Controls.m_UseComputedPivotPoint->setEnabled(true);
  }
  else
  {
    resultString = "Pivot computation failed!";
  }
  MITK_INFO << resultString.toStdString().c_str();
  m_Controls.m_ResultText->setText(resultString);

}
void QmitkIGTNavigationToolCalibration::UpdatePivotCount()
{
  PivotCount = 0;
  while (!m_PivotPoses.empty())
  {
    m_PivotPoses.pop_back();
  }
  m_Controls.m_PoseNumber->setText(QString::number(PivotCount));
}

void QmitkIGTNavigationToolCalibration::OnUseComputedPivotPoint()
{
  RemoveToolTipPreview();
  QString resultString = QString("Pivoted tool tip transformation was written to the tool ") + m_ToolToCalibrate->GetToolName().c_str();
  ApplyToolTipTransform(m_ComputedToolTipTransformation, resultString.toStdString());
  m_Controls.m_ResultText->setText(resultString);
  UpdatePivotCount();
}

void QmitkIGTNavigationToolCalibration::ApplyToolTipTransform(mitk::NavigationData::Pointer ToolTipTransformInToolCoordinates, std::string message)
{
  if (!CheckInitialization(false)) { return; }

  //Update tool in tool storage
  m_ToolToCalibrate->SetToolTipPosition(ToolTipTransformInToolCoordinates->GetPosition());
  m_ToolToCalibrate->SetToolAxisOrientation(ToolTipTransformInToolCoordinates->GetOrientation());

  //And also update tracking device, so the transform is directly used
  mitk::TrackingDeviceSource::Pointer trackingDeviceSource;
  try
  {
    trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_NavigationDataSourceOfToolToCalibrate.GetPointer());
    mitk::TrackingTool::Pointer TrackingToolToCalibrate = trackingDeviceSource->GetTrackingDevice()->GetTool(m_IDToolToCalibrate);
    TrackingToolToCalibrate->SetToolTipPosition(ToolTipTransformInToolCoordinates->GetPosition(), ToolTipTransformInToolCoordinates->GetOrientation());
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Error while trying to set the tool tip to the running tracking device. Aborting! (" << e.what() << ")";
  }
  MITK_INFO << message;
}

void QmitkIGTNavigationToolCalibration::ShowToolTipPreview(mitk::NavigationData::Pointer ToolTipInTrackingCoordinates)
{
  if(m_ToolTipPointPreview.IsNull())
  {
    m_ToolTipPointPreview = mitk::DataNode::New();
    m_ToolTipPointPreview->SetName("Modified Tool Tip Preview");
    mitk::Color blue;
    blue.SetBlue(1);
    m_ToolTipPointPreview->SetColor(blue);
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkSmartPointer<vtkSphereSource> vtkData = vtkSmartPointer<vtkSphereSource>::New();
    vtkData->SetRadius(3.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    m_ToolTipPointPreview->SetData(mySphere);

    this->GetDataStorage()->Add(m_ToolTipPointPreview);
  }
  m_ToolTipPointPreview->GetData()->GetGeometry()->SetIndexToWorldTransform(ToolTipInTrackingCoordinates->GetAffineTransform3D());
}

void QmitkIGTNavigationToolCalibration::RemoveToolTipPreview()
{
  this->GetDataStorage()->Remove(m_ToolTipPointPreview.GetPointer());
}
void QmitkIGTNavigationToolCalibration::UpdateManualToolTipCalibrationView()
{
  if (m_ToolToCalibrate.IsNull()) { return; }
  //parse human readable transformation data and display it
  std::stringstream translation;
  std::stringstream orientation;
  translation << m_ToolToCalibrate->GetToolTipPosition();
  orientation << "Quaternion: (" << m_ToolToCalibrate->GetToolAxisOrientation() << ")" << std::endl;
  orientation << std::endl;
  orientation << "Euler Angles [rad]: (" << m_ToolToCalibrate->GetToolAxisOrientation().rotation_euler_angles() << ")" << std::endl;
  orientation << std::endl;
  orientation << "Matrix:" << std::endl;
  vnl_matrix_fixed<double, 3, 3> rotMatrix = m_ToolToCalibrate->GetToolAxisOrientation().rotation_matrix_transpose();
  orientation << rotMatrix[0][0] << " " << rotMatrix[0][1] << " " << rotMatrix[0][2] << std::endl;
  orientation << rotMatrix[1][0] << " " << rotMatrix[1][1] << " " << rotMatrix[1][2] << std::endl;
  orientation << rotMatrix[2][0] << " " << rotMatrix[2][1] << " " << rotMatrix[2][2] << std::endl;
  m_Controls.m_ManualCurrentTranslation->setText(translation.str().c_str());
  m_Controls.m_ManualCurrentOrientation->setPlainText(orientation.str().c_str());
}

void QmitkIGTNavigationToolCalibration::OnStartManualToolTipCalibration()
{
  if (!CheckInitialization(false)) { return; }

  m_ToolTransformationWidget->SetToolToEdit(m_ToolToCalibrate);
  m_ToolTransformationWidget->SetDefaultOffset(m_ToolToCalibrate->GetToolTipPosition());
  m_ToolTransformationWidget->SetDefaultRotation(m_ToolToCalibrate->GetToolAxisOrientation());

  m_ToolTransformationWidget->open();
}

void QmitkIGTNavigationToolCalibration::OnManualEditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip)
{
  //This function is called, when the toolTipEdit view is closed.
  //if user pressed cancle, nullptr is returned. Do nothing. Else, set values.
  if (toolTip)
  {
    mitk::NavigationData::Pointer tempND = mitk::NavigationData::New(toolTip);//Convert to Navigation data for simple transversion to quaternion
    QString resultString = QString("Manual edited values are written to ") + m_ToolToCalibrate->GetToolName().c_str();
    ApplyToolTipTransform(tempND, resultString.toStdString());
    m_Controls.m_ResultText->setText(resultString);
    m_ComputedToolTipTransformation = tempND;
  }

  UpdateManualToolTipCalibrationView();
}

void QmitkIGTNavigationToolCalibration::OnGetPositions()
{
  if (!CheckInitialization(true)) { return; }

  //Navigation Data from Tool which should be calibrated
  if (!m_AxisCalibration_ToolToCalibrate)
    m_AxisCalibration_ToolToCalibrate = mitk::NavigationData::New();
  m_AxisCalibration_ToolToCalibrate->Graft(m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_IDToolToCalibrate));

  //Navigation Data from calibration pointer tool
  if (!m_AxisCalibration_NavDataCalibratingTool)
    m_AxisCalibration_NavDataCalibratingTool = mitk::NavigationData::New();
  m_AxisCalibration_NavDataCalibratingTool->Graft(m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_IDCalibrationPointer));

  MITK_DEBUG << "Positions for tool axis calibration:";
  MITK_DEBUG << "    ToolTip: " << m_AxisCalibration_ToolToCalibrate->GetPosition() << ",";
  MITK_DEBUG << "    Rotation: \n" << m_AxisCalibration_ToolToCalibrate->GetRotationMatrix();
  MITK_DEBUG << "    End of the tool: " << m_AxisCalibration_NavDataCalibratingTool->GetPosition();

  QString _label = "Position recorded: " + QString::number(m_AxisCalibration_NavDataCalibratingTool->GetPosition()[0], 'f', 1) + ", "
    + QString::number(m_AxisCalibration_NavDataCalibratingTool->GetPosition()[1], 'f', 1) + ", "
    + QString::number(m_AxisCalibration_NavDataCalibratingTool->GetPosition()[2], 'f', 1);
  m_Controls.m_ToolAxisPositionLabel->setText(_label);
}

void QmitkIGTNavigationToolCalibration::OnCalibrateToolAxis()
{
  if (!m_ComputedToolTipTransformation)
  {
    MITK_ERROR << "Please compute tool tip first.";
    QMessageBox::information(nullptr, "Error", "Please compute / specifiy tool tip first");
    return;
  }
  if (!m_AxisCalibration_ToolToCalibrate || !m_AxisCalibration_NavDataCalibratingTool)
  {
    MITK_ERROR << "Please record position first.";
    QMessageBox::information(nullptr, "Error", "Please record position first");
    return;
  }

  //Calculate the tool tip
  //here is an explanation, what is happening here:
  /*
  The axis is equal to the (tool tip) minus the (end of the tool) in tool coordinates of the tool which should be calibrated.
  The tool tip is defined as the origin of the tool coordinate system.
  The end of the tool is recorded by the calibration pointer's position and is transformed into the coordinate system of the tool to be calibrated
  Normalize it.
  */
  //m_CalibratedToolAxis = -m_AxisCalibration_ToolToCalibrate->TransformPoint(m_AxisCalibration_NavDataCalibratingTool->GetPosition()).GetVectorFromOrigin();
  m_CalibratedToolAxis = -m_AxisCalibration_ToolToCalibrate->GetInverse()->TransformPoint(m_AxisCalibration_NavDataCalibratingTool->GetPosition()).GetVectorFromOrigin();
  MITK_DEBUG << "Tool Endpoint in Tool coordinates: " << m_CalibratedToolAxis;
  m_CalibratedToolAxis.Normalize();
  MITK_DEBUG << "Tool Axis: " << m_CalibratedToolAxis;

  m_ToolToCalibrate->SetToolAxis(mitk::Point3D(m_CalibratedToolAxis));

  // Update TrackingTool
  m_ComputedToolTipTransformation->SetPosition(m_ToolToCalibrate->GetToolTipPosition());
  m_ComputedToolTipTransformation->SetOrientation(m_ToolToCalibrate->GetToolAxisOrientation());
  ApplyToolTipTransform(m_ComputedToolTipTransformation);

  //Update GUI
  QString calibratedToolAxisString = "Tool Axis: " + QString::number(m_CalibratedToolAxis.GetElement(0), 'f', 3) + ", " +
    QString::number(m_CalibratedToolAxis.GetElement(1), 'f', 3) + ", " + QString::number(m_CalibratedToolAxis.GetElement(2), 'f', 3);
  m_Controls.m_ToolAxisCalibrationLabel->setText(calibratedToolAxisString);

  //Block QT signals, we don't want to emit SpinboxChanged on the first value to overwrite the next ones
  m_Controls.m_ToolAxis_X->blockSignals(true); m_Controls.m_ToolAxis_Y->blockSignals(true); m_Controls.m_ToolAxis_Z->blockSignals(true);
  m_Controls.m_ToolAxis_X->setValue(m_CalibratedToolAxis[0]);
  m_Controls.m_ToolAxis_Y->setValue(m_CalibratedToolAxis[1]);
  m_Controls.m_ToolAxis_Z->setValue(m_CalibratedToolAxis[2]);
  m_Controls.m_ToolAxis_X->blockSignals(false); m_Controls.m_ToolAxis_Y->blockSignals(false); m_Controls.m_ToolAxis_Z->blockSignals(false);
}

void QmitkIGTNavigationToolCalibration::OnToolAxisSpinboxChanged()
{
  m_CalibratedToolAxis.SetElement(0, m_Controls.m_ToolAxis_X->value());
  m_CalibratedToolAxis.SetElement(1, m_Controls.m_ToolAxis_Y->value());
  m_CalibratedToolAxis.SetElement(2, m_Controls.m_ToolAxis_Z->value());
  m_ToolToCalibrate->SetToolAxis(mitk::Point3D(m_CalibratedToolAxis));
  // Update TrackingTool
  if (m_ComputedToolTipTransformation.IsNull())
  {
    m_ComputedToolTipTransformation = mitk::NavigationData::New();
  }
  m_ComputedToolTipTransformation->SetPosition(m_ToolToCalibrate->GetToolTipPosition());
  m_ComputedToolTipTransformation->SetOrientation(m_ToolToCalibrate->GetToolAxisOrientation());
  ApplyToolTipTransform(m_ComputedToolTipTransformation);

  MITK_INFO << "Tool axis changed to " << m_CalibratedToolAxis;
}

void QmitkIGTNavigationToolCalibration::SetToolToCalibrate()
{
  m_IDToolToCalibrate = m_Controls.m_SelectionWidget->GetSelectedToolID();
  if (m_IDToolToCalibrate == -1) //no valid tool to calibrate
  {
    m_Controls.m_CalToolLabel->setText("<none>");
    m_Controls.m_StatusWidgetToolToCalibrate->RemoveStatusLabels();
    m_TrackingTimer->stop();
  }
  else
  {
    m_ToolToCalibrate = m_Controls.m_SelectionWidget->GetSelectedNavigationTool();
    m_NavigationDataSourceOfToolToCalibrate = m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource();
    m_Controls.m_CalToolLabel->setText(m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate)->GetName());
    //initialize widget
    m_Controls.m_StatusWidgetToolToCalibrate->RemoveStatusLabels();
    m_Controls.m_StatusWidgetToolToCalibrate->SetShowPositions(true);
    m_Controls.m_StatusWidgetToolToCalibrate->SetTextAlignment(Qt::AlignLeft);
    m_Controls.m_StatusWidgetToolToCalibrate->AddNavigationData(m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate));
    m_Controls.m_StatusWidgetToolToCalibrate->ShowStatusLabels();
    //initialize manual tool tip calibration view
    UpdateManualToolTipCalibrationView();
    //save tool surface in tool coordinates for further editing
    mitk::Surface::Pointer ToolSurface = dynamic_cast<mitk::Surface*>(m_ToolToCalibrate->GetDataNode()->GetData())->Clone();
    m_ToolSurfaceInToolCoordinatesDataNode->SetData(ToolSurface);
    m_ToolSurfaceInToolCoordinatesDataNode->GetData()->GetGeometry()->SetIdentity();
    // update tool tip and rotation information for tool tip calibration tab
    UpdateManualToolTipCalibrationView();
    // update tool axis information for tool axis calibration tab
    mitk::Point3D toolAxis = m_ToolToCalibrate->GetToolAxis();
    m_CalibratedToolAxis[0] = toolAxis[0];
    m_CalibratedToolAxis[1] = toolAxis[1];
    m_CalibratedToolAxis[2] = toolAxis[2];
    m_Controls.m_ToolAxis_X->blockSignals(true); m_Controls.m_ToolAxis_Y->blockSignals(true); m_Controls.m_ToolAxis_Z->blockSignals(true);
    m_Controls.m_ToolAxis_X->setValue(m_CalibratedToolAxis[0]);
    m_Controls.m_ToolAxis_Y->setValue(m_CalibratedToolAxis[1]);
    m_Controls.m_ToolAxis_Z->setValue(m_CalibratedToolAxis[2]);
    m_Controls.m_ToolAxis_X->blockSignals(false); m_Controls.m_ToolAxis_Y->blockSignals(false); m_Controls.m_ToolAxis_Z->blockSignals(false);

    //start updating timer for status widgets, etc.
    if (!m_TrackingTimer->isActive()) m_TrackingTimer->start(100);
  }
}

void QmitkIGTNavigationToolCalibration::SetCalibrationPointer()
{
  m_IDCalibrationPointer = m_Controls.m_SelectionWidget->GetSelectedToolID();
  if (m_IDCalibrationPointer == -1)
  {
    m_Controls.m_PointerLabel->setText("<none>");
    m_Controls.m_StatusWidgetCalibrationPointer->RemoveStatusLabels();
    m_TrackingTimer->stop();
  }
  else
  {
    m_NavigationDataSourceOfCalibrationPointer = m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource();
    m_Controls.m_PointerLabel->setText(m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer)->GetName());
    //initialize widget
    m_Controls.m_StatusWidgetCalibrationPointer->RemoveStatusLabels();
    m_Controls.m_StatusWidgetCalibrationPointer->SetShowPositions(true);
    m_Controls.m_StatusWidgetCalibrationPointer->SetTextAlignment(Qt::AlignLeft);
    m_Controls.m_StatusWidgetCalibrationPointer->AddNavigationData(m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer));
    m_Controls.m_StatusWidgetCalibrationPointer->ShowStatusLabels();
    if (!m_TrackingTimer->isActive()) m_TrackingTimer->start(100);
  }
}

void QmitkIGTNavigationToolCalibration::UpdateOffsetCoordinates()
{
  if (m_NavigationDataSourceOfCalibrationPointer.IsNull() || m_NavigationDataSourceOfToolToCalibrate.IsNull())
  {
    return;
  }

  mitk::NavigationData::Pointer referenceToolND = m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer);
  mitk::NavigationData::Pointer toolToCalibrateND = m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate);

  if (referenceToolND->IsDataValid() && toolToCalibrateND->IsDataValid())
  {
    //computation: difference between both tools (in tool coordinates)
    //differenceND = toolToCalibrateND^-1 * referenceToolND
    mitk::NavigationData::Pointer differenceND = mitk::NavigationData::New();
    differenceND->Compose(referenceToolND);
    differenceND->Compose(toolToCalibrateND->GetInverse());

    //display this orientation in the UI
    m_Controls.m_OffsetCoordinates->setText(
      QString("x: ") + QString(QString::number(differenceND->GetPosition()[0], 103, 3)) +
      QString("; y: ") + (QString::number(differenceND->GetPosition()[1], 103, 3)) +
      QString("; z: ") + (QString::number(differenceND->GetPosition()[2], 103, 3)));

    m_Controls.m_OrientationOffsetCoordinates->setText(
      QString("qx: ") + QString(QString::number(differenceND->GetOrientation().x(), 103, 3)) +
      QString("; qy: ") + (QString::number(differenceND->GetOrientation().y(), 103, 3)) +
      QString("; qz: ") + (QString::number(differenceND->GetOrientation().z(), 103, 3)) +
      QString("; qr: ") + (QString::number(differenceND->GetOrientation().r(), 103, 3)));

    //also update preview if active
    if (m_ToolTipPointPreview.IsNotNull()) //NOT WORKING! TODO: fix or remove!
    {
      mitk::NavigationData::Pointer ToolTipTransform = mitk::NavigationData::New();
      ToolTipTransform->SetPosition(m_ResultOffsetVector);
      mitk::NavigationData::Pointer ToolTipInTrackingCoordinates = mitk::NavigationData::New(); //maybe store as for better peformance...
      ToolTipInTrackingCoordinates->Compose(m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate));
      ToolTipInTrackingCoordinates->Compose(ToolTipTransform);
      m_ToolTipPointPreview->GetData()->GetGeometry()->SetIndexToWorldTransform(ToolTipInTrackingCoordinates->GetAffineTransform3D());
    }
  }
}

void QmitkIGTNavigationToolCalibration::UpdateTrackingTimer()
{
  m_Controls.m_StatusWidgetToolToCalibrate->Refresh();
  m_Controls.m_StatusWidgetCalibrationPointer->Refresh();

  if (m_OnLoginSingleRefToolNavigationDataClicked) LoginSingleRefToolNavigationData();

  if (m_OnAddPivotPoseClicked) AddPivotPose();

  // 1 == Single Reference Calibration Method
  if (m_IndexCurrentCalibrationMethod == 1) UpdateOffsetCoordinates();

}

void QmitkIGTNavigationToolCalibration::AddLandmark()
{
  if (!CheckInitialization()) { return; }
  mitk::NavigationData::Pointer navDataTool = m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate);
  mitk::Point3D landmark = m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer)->GetPosition();

  //convert to itk transform
  itk::Vector<double, 3> translation;
  for (int k = 0; k < 3; k++) translation[k] = navDataTool->GetPosition()[k];
  itk::Matrix<double, 3, 3> rotation;
  for (int k = 0; k < 3; k++) for (int l = 0; l < 3; l++) rotation[k][l] = navDataTool->GetOrientation().rotation_matrix_transpose()[k][l];
  rotation = rotation.GetTranspose();
  itk::Vector<double> landmarkItk;
  landmarkItk[0] = landmark[0];
  landmarkItk[1] = landmark[1];
  landmarkItk[2] = landmark[2];

  //compute landmark in tool coordinates
  itk::Matrix<double, 3, 3> rotationInverse;
  for (int k = 0; k < 3; k++) for (int l = 0; l < 3; l++) rotationInverse[k][l] = rotation.GetInverse()[k][l];
  landmarkItk = rotationInverse * (landmarkItk - translation);

  //convert back and add landmark to pointset
  landmark[0] = landmarkItk[0];
  landmark[1] = landmarkItk[1];
  landmark[2] = landmarkItk[2];
  m_RegistrationLandmarks->InsertPoint(m_RegistrationLandmarks->GetSize(), landmark);
}

void QmitkIGTNavigationToolCalibration::SaveCalibratedTool()
{
  if (m_ToolToCalibrate.IsNotNull())
  {
    mitk::NavigationTool::Pointer calibratedTool = m_ToolToCalibrate;
    calibratedTool->SetToolControlPoints(this->m_CalibrationLandmarks);
    calibratedTool->SetToolLandmarks(this->m_RegistrationLandmarks);
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    QString filename = QFileDialog::getSaveFileName(nullptr,tr("Save Navigation Tool"), "/", "*.IGTTool");
    if (filename.isEmpty()) return;

    // ensure that file suffix is set
    QFileInfo file(filename);
    if (file.suffix().isEmpty())
    {
      filename += ".IGTTool";
    }

    if (myWriter->DoWrite(filename.toStdString(), calibratedTool)) MITK_INFO << "Saved calibrated tool to file " << filename;
    else MITK_WARN << "Can't write tool to file " << filename;
  }
  else
  {
    MITK_ERROR << "Did not find navigation tool storage of calibrated tool, aborting!";
  }
}

bool QmitkIGTNavigationToolCalibration::CheckInitialization(bool CalibrationPointerRequired)
{
  if ((m_IDToolToCalibrate == -1) ||
    ((CalibrationPointerRequired) &&
    (m_IDCalibrationPointer == -1)
    )
    )
  {
    QMessageBox msgBox;
    msgBox.setText("Tool to calibrate and/or calibration pointer not initialized, cannot proceed!");
    msgBox.exec();
    return false;
  }
  else { return true; }
}
