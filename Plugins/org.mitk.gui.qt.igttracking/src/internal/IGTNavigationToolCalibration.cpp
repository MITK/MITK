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

#include <numeric>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "IGTNavigationToolCalibration.h"

// mitk
#include <mitkNavigationToolWriter.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkTrackingDevice.h>
#include <mitkTrackingTool.h>
#include <mitkQuaternionAveraging.h>

// Qt
#include <QMessageBox>
#include <qfiledialog.h>

//vtk
#include <vtkSphereSource.h>


const std::string IGTNavigationToolCalibration::VIEW_ID = "org.mitk.views.igtnavigationtoolcalibration";

IGTNavigationToolCalibration::IGTNavigationToolCalibration()
{}

IGTNavigationToolCalibration::~IGTNavigationToolCalibration()
{
//The following code is required due to a bug in the point list widget.
//If this is removed, MITK crashes when closing the view:
m_Controls.m_RegistrationLandmarkWidget->SetPointSetNode(NULL);
m_Controls.m_CalibrationLandmarkWidget->SetPointSetNode(NULL);
}

void IGTNavigationToolCalibration::SetFocus()
{

}

void IGTNavigationToolCalibration::OnToolCalibrationMethodChanged(int index)
{
  UpdateManualToolTipCalibrationView();
  m_Controls.m_CalibrationMethodsWidget->setCurrentIndex(index);
  m_IndexCurrentCalibrationMethod = index;
}

void IGTNavigationToolCalibration::CreateQtPartControl( QWidget *parent )
{
  //initialize manual tool editing widget
  m_ManualToolTipEditWidget = new QmitkNavigationToolCreationAdvancedWidget(parent);
  m_ManualToolTipEditWidget->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
  m_ManualToolTipEditWidget->setWindowTitle("Edit Tool Tip Manually");
  m_ManualToolTipEditWidget->setModal(false);
  m_ManualToolTipEditWidget->SetDataStorage(this->GetDataStorage());

  m_TrackingTimer = new QTimer(this);

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.m_SetToolToCalibrate, SIGNAL(clicked()), this, SLOT(SetToolToCalibrate()) );
  connect( m_Controls.m_SetPointer, SIGNAL(clicked()), this, SLOT(SetCalibrationPointer()) );
  connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackingTimer()));
  connect( m_Controls.m_AddLandmark, SIGNAL(clicked()), this, SLOT(AddLandmark()));
  connect( m_Controls.m_SaveCalibratedTool, SIGNAL(clicked()), this, SLOT(SaveCalibratedTool()));
  connect( m_Controls.m_AddPivotPose, SIGNAL(clicked()), this, SLOT(OnAddPivotPose()));
  connect( m_Controls.m_ComputePivot, SIGNAL(clicked()), this, SLOT(OnComutePivot()));
  connect( m_Controls.m_UseComputedPivotPoint, SIGNAL(clicked()), this, SLOT(OnUseComutedPivotPoint()));
  connect( m_Controls.m_StartEditTooltipManually, SIGNAL(clicked()), this, SLOT(OnStartManualToolTipCalibration()));
  connect( (QObject*)(m_ManualToolTipEditWidget), SIGNAL(RetrieveDataForManualToolTipManipulation()), this, SLOT(OnRetrieveDataForManualTooltipManipulation()) );
  connect( (QObject*)(m_ManualToolTipEditWidget), SIGNAL(DialogCloseRequested()), this, SLOT(OnProcessManualTooltipEditDialogCloseRequest()) );
  connect( m_Controls.m_CalibrationMethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnToolCalibrationMethodChanged(int)));

  connect( (QObject*)(m_Controls.m_RunCalibrationButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnRunSingleRefToolCalibrationClicked()));
  connect( (QObject*)(m_Controls.m_CollectNavigationDataButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnLoginSingleRefToolNavigationDataClicked()));
  connect( (QObject*)(m_Controls.m_SetNewToolTipPosButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnSetNewToolTipPosButtonClicked()));

  m_IDToolToCalibrate = -1;
  m_IDCalibrationPointer = -1;
  m_IndexCurrentCalibrationMethod = -1;
  m_OnLoginSingleRefToolNavigationDataClicked = false;
  m_NumberOfNavigationDataCounter = 0;
  m_NumberOfNavigationData = -1;

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

  m_PivotPoses = std::vector<mitk::NavigationData::Pointer>();

  m_LoggedNavigationDataDifferences = std::vector< mitk::NavigationData::Pointer >();
}


void IGTNavigationToolCalibration::OnRunSingleRefToolCalibrationClicked()
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
        for (int i = 0; i < m_LoggedNavigationDataDifferences.size(); i++) { allOrientations.push_back(m_LoggedNavigationDataDifferences.at(i)->GetOrientation()); }
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
      MITK_INFO << "Quaternion: (" << ToolTipTransform->GetOrientation() <<")";
      MITK_INFO<<"Euler Angles [rad]: (" << ToolTipTransform->GetOrientation().rotation_euler_angles() <<")";
      MITK_INFO<<"Matrix:";
      vnl_matrix_fixed<double,3,3> rotMatrix =ToolTipTransform->GetOrientation().rotation_matrix_transpose();
      MITK_INFO<<rotMatrix[0][0]<<" "<<rotMatrix[0][1]<<" "<<rotMatrix[0][2]<<std::endl;
      MITK_INFO<<rotMatrix[1][0]<<" "<<rotMatrix[1][1]<<" "<<rotMatrix[1][2]<<std::endl;
      MITK_INFO<<rotMatrix[2][0]<<" "<<rotMatrix[2][1]<<" "<<rotMatrix[2][2]<<std::endl;

      //3: write everything into the final tool tip transform and save it as member (it will be written to the tool later on)
      mitk::NavigationData::Pointer ToolTipInTrackingCoordinates = mitk::NavigationData::New();
      ToolTipInTrackingCoordinates->Compose(ToolTipTransform);
      ToolTipInTrackingCoordinates->Compose(m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate));
      ShowToolTipPreview(ToolTipInTrackingCoordinates);
      m_Controls.m_SetNewToolTipPosButton->setEnabled(true);
      m_ComputedToolTipTransformation = ToolTipTransform;

}

void IGTNavigationToolCalibration::OnLoginSingleRefToolNavigationDataClicked()
{
  if (!CheckInitialization()) {return;}
  m_OnLoginSingleRefToolNavigationDataClicked = true;
  m_Controls.m_CollectNavigationDataButton->setEnabled(false);
  m_NumberOfNavigationData = m_Controls.m_NumberOfNavigationDataToCollect->value();
  MITK_INFO << "Collecting " << m_NumberOfNavigationData << " NavigationData ... "<<endl;
}

void IGTNavigationToolCalibration::LoginSingleRefToolNavigationData()
{
  if (!CheckInitialization()) {return;}

  if(m_NumberOfNavigationDataCounter < m_NumberOfNavigationData)
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

  if( m_NumberOfNavigationDataCounter == m_NumberOfNavigationData)
    {
      m_NumberOfNavigationDataCounter = 0;
      m_OnLoginSingleRefToolNavigationDataClicked = false;
      m_Controls.m_CollectNavigationDataButton->setEnabled(true);
      m_Controls.m_RunCalibrationButton->setEnabled(true);
      MITK_INFO << "Collecting " << m_NumberOfNavigationData << " NavigationData ... Finished"<<endl;
      QString labelText = "Collected " + QString::number(m_NumberOfNavigationData) + " data samples!";
      m_Controls.m_CollectionStatus->setText(labelText);
     }
}

void IGTNavigationToolCalibration::OnSetNewToolTipPosButtonClicked()
{
  ApplyToolTipTransform(m_ComputedToolTipTransformation);
  RemoveToolTipPreview();
}

void IGTNavigationToolCalibration::OnAddPivotPose()
{
  if (!CheckInitialization()) {return;}
  mitk::NavigationData::Pointer currentPose = mitk::NavigationData::New();
  currentPose->Graft(m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_IDToolToCalibrate));
  m_PivotPoses.push_back(currentPose);
  m_Controls.m_PoseNumber->setText(QString::number(m_PivotPoses.size()));
}

void IGTNavigationToolCalibration::OnComutePivot()
{
  MITK_WARN << "Not implemented yet!";
  /*
  mitk::PivotCalibration::Pointer myPivotCalibration = mitk::PivotCalibration::New();
  for (int i=0; i<this->m_PivotPoses.size(); i++)
  {
    myPivotCalibration->AddNavigationData(m_PivotPoses.at(i));
  }
  QString resultString;
  if (myPivotCalibration->ComputePivotResult())
  {
  //Get first marker transformation (TODO: compute for each pivot pose and average laster... this would be more accurate!)
  mitk::NavigationData::Pointer markerTransformationTrackingCoordinates = m_PivotPoses.at(0);

  //Get computed pivot transfromation in tool coordinates
  mitk::NavigationData::Pointer pivotPointToolCoordinates = mitk::NavigationData::New();
  pivotPointToolCoordinates->SetPosition(myPivotCalibration->GetResultPivotPoint());
  pivotPointToolCoordinates->SetOrientation(myPivotCalibration->GetResultPivotRotation());
  mitk::NavigationData::Pointer toolRotation = mitk::NavigationData::New();
  toolRotation->SetOrientation(markerTransformationTrackingCoordinates->GetOrientation());
  pivotPointToolCoordinates->Compose(toolRotation);

  //Compute pivot point in relation to marker transformation for preview
  mitk::NavigationData::Pointer pivotInTrackingCoordinates = mitk::NavigationData::New();
  pivotInTrackingCoordinates->Compose(pivotPointToolCoordinates);
  pivotInTrackingCoordinates->Compose(markerTransformationTrackingCoordinates);

  //add the preview node to the data storage
  ShowToolTipPreview(pivotInTrackingCoordinates);

  //parse result string
  resultString = QString("Pivot comutation succeeded!\n")
                 + QString("RMS Error: ") + QString::number(myPivotCalibration->GetResultRMSError()) + QString("\n")
                 + QString("Pivot Point: ") + QString::number(myPivotCalibration->GetResultPivotPoint()[0]) + ";" + QString::number(myPivotCalibration->GetResultPivotPoint()[1]) + ";" + QString::number(myPivotCalibration->GetResultPivotPoint()[2]) + QString("\n")
                 + QString("Pivot Rotation: ") + QString::number(myPivotCalibration->GetResultPivotRotation()[0]) + ";" + QString::number(myPivotCalibration->GetResultPivotRotation()[1]) + ";" + QString::number(myPivotCalibration->GetResultPivotRotation()[2]) + ";" + QString::number(myPivotCalibration->GetResultPivotRotation()[3]) + QString("\n");

  //finally: save results to member variable
  m_ComputedToolTipTransformation = pivotPointToolCoordinates;

  //enable button to use the computed point with the tool
  m_Controls.m_UseComputedPivotPoint->setEnabled(true);
  }
  else
  {
    resultString = "Pivot comutation failed!";
  }
  MITK_INFO << resultString.toStdString().c_str();
  m_Controls.m_ResultText->setText(resultString);
  */
}

void IGTNavigationToolCalibration::OnUseComutedPivotPoint()
{
  if (!CheckInitialization(false)) {return;}
  RemoveToolTipPreview();
  QString resultString = QString("Pivoted tool tip transformation was written to the tool ") + m_ToolToCalibrate->GetToolName().c_str();
  ApplyToolTipTransform(m_ComputedToolTipTransformation,resultString.toStdString());
  m_Controls.m_ResultText->setText(resultString);
}

void IGTNavigationToolCalibration::ApplyToolTipTransform(mitk::NavigationData::Pointer ToolTipTransformInToolCoordinates, std::string message)
{
  if (!CheckInitialization(false)) {return;}

  //Update tool in tool storage
  m_ToolToCalibrate->SetToolTipPosition(ToolTipTransformInToolCoordinates->GetPosition());
  m_ToolToCalibrate->SetToolTipOrientation(ToolTipTransformInToolCoordinates->GetOrientation());

  //And also update tracking device, so the transform is directly used
  mitk::TrackingDeviceSource::Pointer trackingDeviceSource;
  try
    {
    trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_NavigationDataSourceOfToolToCalibrate.GetPointer());
    mitk::TrackingTool::Pointer TrackingToolToCalibrate = trackingDeviceSource->GetTrackingDevice()->GetTool(m_IDToolToCalibrate);
    TrackingToolToCalibrate->SetToolTip(ToolTipTransformInToolCoordinates->GetPosition(),ToolTipTransformInToolCoordinates->GetOrientation());
    }
  catch (std::exception& e)
    {
    MITK_ERROR << "Error while trying to set the tool tip to the running tracking device. Aborting! (" << e.what() << ")";
    }
  MITK_INFO << message;
}

void IGTNavigationToolCalibration::ShowToolTipPreview(mitk::NavigationData::Pointer ToolTipInTrackingCoordinates)
{
  mitk::DataNode::Pointer m_ToolTipPointPreview = mitk::DataNode::New();
  m_ToolTipPointPreview->SetName("Modified Tool Tip Preview");
  mitk::Color red;
  red.SetRed(1);
  m_ToolTipPointPreview->SetColor(red);
  mitk::Surface::Pointer mySphere = mitk::Surface::New();
  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius(3.0f);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->Update();
  mySphere->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();
  m_ToolTipPointPreview->SetData(mySphere);
  m_ToolTipPointPreview->GetData()->GetGeometry()->SetIndexToWorldTransform(ToolTipInTrackingCoordinates->GetAffineTransform3D());
  this->GetDataStorage()->Add(m_ToolTipPointPreview);
}

void IGTNavigationToolCalibration::RemoveToolTipPreview()
{
  this->GetDataStorage()->Remove(m_ToolTipPointPreview.GetPointer());
}
void IGTNavigationToolCalibration::UpdateManualToolTipCalibrationView()
{
  if (m_ToolToCalibrate.IsNull()) {return;}
  //parse human readable transformation data and display it
  std::stringstream translation;
  std::stringstream orientation;
  translation<<m_ToolToCalibrate->GetToolTipPosition();
  orientation<<"Quaternion: (" << m_ToolToCalibrate->GetToolTipOrientation() <<")"<<std::endl;
  orientation<<std::endl;
  orientation<<"Euler Angles [rad]: (" << m_ToolToCalibrate->GetToolTipOrientation().rotation_euler_angles() <<")"<<std::endl;
  orientation<<std::endl;
  orientation<<"Matrix:"<<std::endl;
  vnl_matrix_fixed<double,3,3> rotMatrix =m_ToolToCalibrate->GetToolTipOrientation().rotation_matrix_transpose();
  orientation<<rotMatrix[0][0]<<" "<<rotMatrix[0][1]<<" "<<rotMatrix[0][2]<<std::endl;
  orientation<<rotMatrix[1][0]<<" "<<rotMatrix[1][1]<<" "<<rotMatrix[1][2]<<std::endl;
  orientation<<rotMatrix[2][0]<<" "<<rotMatrix[2][1]<<" "<<rotMatrix[2][2]<<std::endl;
  m_Controls.m_ManualCurrentTranslation->setText(translation.str().c_str());
  m_Controls.m_ManualCurrentOrientation->setPlainText(orientation.str().c_str());
}

void IGTNavigationToolCalibration::OnStartManualToolTipCalibration()
{
  if (!CheckInitialization(false)) {return;}
  m_ManualToolTipEditWidget->SetToolTipSurface(false,m_ToolToCalibrate->GetDataNode());
  m_ManualToolTipEditWidget->show();
  m_ManualToolTipEditWidget->SetDefaultTooltip(m_ToolToCalibrate->GetToolTipTransform());
  m_ManualToolTipEditWidget->ReInitialize();
}

void IGTNavigationToolCalibration::OnRetrieveDataForManualTooltipManipulation()
{
  this->GetDataStorage()->Add(m_ToolSurfaceInToolCoordinatesDataNode);
  m_ManualToolTipEditWidget->SetToolTipSurface(false,m_ToolSurfaceInToolCoordinatesDataNode);
}

void IGTNavigationToolCalibration::OnProcessManualTooltipEditDialogCloseRequest()
{
  mitk::NavigationData::Pointer tempND = mitk::NavigationData::New(m_ManualToolTipEditWidget->GetManipulatedToolTip());
  this->ApplyToolTipTransform(tempND);
  UpdateManualToolTipCalibrationView();
  m_ManualToolTipEditWidget->hide();
  this->GetDataStorage()->Remove(m_ToolSurfaceInToolCoordinatesDataNode);
}

void IGTNavigationToolCalibration::SetToolToCalibrate()
{
  m_IDToolToCalibrate = m_Controls.m_SelectionWidget->GetSelectedToolID();
  m_ToolToCalibrate = m_Controls.m_SelectionWidget->GetSelectedNavigationTool();
  if (m_IDToolToCalibrate==-1) //no valid tool to calibrate
  {
    m_Controls.m_CalToolLabel->setText("<none>");
    m_Controls.m_StatusWidgetToolToCalibrate->RemoveStatusLabels();
    m_TrackingTimer->stop();
  }
  else
  {
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
    //start updating timer for status widgets, etc.
    if (!m_TrackingTimer->isActive()) m_TrackingTimer->start(100);
  }
}

void IGTNavigationToolCalibration::SetCalibrationPointer()
{
  m_IDCalibrationPointer = m_Controls.m_SelectionWidget->GetSelectedToolID();
  m_NavigationDataSourceOfCalibrationPointer = m_Controls.m_SelectionWidget->GetSelectedNavigationDataSource();
  if (m_IDCalibrationPointer==-1)
  {
    m_Controls.m_PointerLabel->setText("<none>");
    m_Controls.m_StatusWidgetCalibrationPointer->RemoveStatusLabels();
    m_TrackingTimer->stop();
  }
  else
  {
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

void IGTNavigationToolCalibration::UpdateOffsetCoordinates()
{
  mitk::NavigationData::Pointer referenceToolND = m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer);
  mitk::NavigationData::Pointer toolToCalibrateND = m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate);

  if(referenceToolND->IsDataValid() && toolToCalibrateND->IsDataValid())
  {
    //computation: difference between both tools (in tool coordinates)
    //differenceND = toolToCalibrateND^-1 * referenceToolND
    mitk::NavigationData::Pointer differenceND = mitk::NavigationData::New();
    differenceND->Compose(referenceToolND);
    differenceND->Compose(toolToCalibrateND->GetInverse());

    //display this orientation in the UI
    m_Controls.m_OffsetCoordinates->setText(
       QString("x: ") + QString(QString::number(differenceND->GetPosition()[0],103,3)) +
       QString("; y: ") + (QString::number(differenceND->GetPosition()[1],103,3)) +
       QString("; z: ") + (QString::number(differenceND->GetPosition()[2],103,3)));

    m_Controls.m_OrientationOffsetCoordinates->setText(
      QString("qx: ") + QString(QString::number(differenceND->GetOrientation().x(),103,3)) +
      QString("; qy: ") + (QString::number(differenceND->GetOrientation().y(),103,3)) +
      QString("; qz: ") + (QString::number(differenceND->GetOrientation().z(),103,3)) +
      QString("; qr: ") + (QString::number(differenceND->GetOrientation().r(),103,3)));

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

void IGTNavigationToolCalibration::UpdateTrackingTimer()
{
  m_Controls.m_StatusWidgetToolToCalibrate->Refresh();
  m_Controls.m_StatusWidgetCalibrationPointer->Refresh();

  if(m_OnLoginSingleRefToolNavigationDataClicked) LoginSingleRefToolNavigationData();

  // 1 == Single Reference Calibration Method
  if(m_IndexCurrentCalibrationMethod == 1 ) UpdateOffsetCoordinates();

}

void IGTNavigationToolCalibration::AddLandmark()
{
  if (!CheckInitialization()) {return;}
  mitk::NavigationData::Pointer navDataTool = m_NavigationDataSourceOfToolToCalibrate->GetOutput(m_IDToolToCalibrate);
  mitk::Point3D landmark = m_NavigationDataSourceOfCalibrationPointer->GetOutput(m_IDCalibrationPointer)->GetPosition();

  //convert to itk transform
  itk::Vector<double,3> translation;
  for(int k=0; k<3;k++) translation[k] = navDataTool->GetPosition()[k];
  itk::Matrix<double,3,3> rotation;
  for(int k=0; k<3; k++) for(int l=0; l<3; l++) rotation[k][l] = navDataTool->GetOrientation().rotation_matrix_transpose()[k][l];
  rotation = rotation.GetTranspose();
  itk::Vector<double> landmarkItk;
  landmarkItk[0] = landmark[0];
  landmarkItk[1] = landmark[1];
  landmarkItk[2] = landmark[2];

  //compute landmark in tool coordinates
  itk::Matrix<double,3,3> rotationInverse;
  for(int k=0; k<3; k++) for(int l=0; l<3; l++) rotationInverse[k][l] = rotation.GetInverse()[k][l];
  landmarkItk = rotationInverse * (landmarkItk - translation) ;

  //convert back and add landmark to pointset
  landmark[0] = landmarkItk[0];
  landmark[1] = landmarkItk[1];
  landmark[2] = landmarkItk[2];
  m_RegistrationLandmarks->InsertPoint(m_RegistrationLandmarks->GetSize(),landmark);
}

void IGTNavigationToolCalibration::SaveCalibratedTool()
{
  if (m_ToolToCalibrate.IsNotNull())
  {
    mitk::NavigationTool::Pointer calibratedTool = m_ToolToCalibrate;
    calibratedTool->SetToolCalibrationLandmarks(this->m_CalibrationLandmarks);
    calibratedTool->SetToolRegistrationLandmarks(this->m_RegistrationLandmarks);
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = QFileDialog::getSaveFileName(NULL,tr("Save Navigation Tool"), "/", "*.IGTTool").toUtf8().data();
    filename.append(".IGTTool");
    if (filename == "") return;
    if (myWriter->DoWrite(filename,calibratedTool)) MITK_INFO << "Saved calibrated tool to file " << filename;
    else MITK_WARN << "Can't write tool to file " << filename;
  }
  else
  {
    MITK_ERROR << "Did not find navigation tool storage of calibrated tool, aborting!";
  }
}

bool IGTNavigationToolCalibration::CheckInitialization(bool CalibrationPointerRequired)
{
if (  (m_IDToolToCalibrate == -1) ||
      ( (CalibrationPointerRequired) &&
        (m_IDCalibrationPointer == -1)
      )
   )
  {
  QMessageBox msgBox;
  msgBox.setText("Tool to calibrate and/or calibration pointer not initialized, cannot proceed!");
  msgBox.exec();
  return false;
  }
else {return true;}
}
