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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "UltrasoundCalibration.h"
#include <QTimer>

// Qt
#include <QMessageBox>
#include <QmitkServiceListWidget.h>
#include <QFileDialog>

// MITK
#include <mitkVector.h>
//#include <mitkPointSetModifier.h>
#include <mitkPointSetWriter.h>
//#include <mitkTransformationFileWriter.h>

// us
#include <usServiceReference.h>

//VTK
#include <vtkPoints.h>
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>


const std::string UltrasoundCalibration::VIEW_ID = "org.mitk.views.ultrasoundcalibration";

UltrasoundCalibration::~UltrasoundCalibration()
{
  m_Timer->stop();
  // Sleep(500); //This might be problematic... seems like sometimes some ressources are still in use at calling time.
  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Tool Calibration Points");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);
  node = this->GetDataStorage()->GetNamedNode("Image Calibration Points");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);
  node = this->GetDataStorage()->GetNamedNode("US Image Stream");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);
  node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);

  delete m_Timer;
}

void UltrasoundCalibration::SetFocus()
{
  m_Controls.m_TabWidget->setFocus();
}

void UltrasoundCalibration::CreateQtPartControl( QWidget *parent )
{
  m_Timer = new QTimer(this);

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  // General & Device Selection
  connect( m_Timer, SIGNAL(timeout()), this, SLOT(Update()));
  connect( m_Controls.m_TabWidget,        SIGNAL(currentChanged ( int )), this, SLOT(OnTabSwitch( int )) );
  // Calibration
  connect( m_Controls.m_CalibBtnFreeze,    SIGNAL(clicked()), this, SLOT(SwitchFreeze()) );       // Freeze
  connect( m_Controls.m_CalibBtnAddPoint,  SIGNAL(clicked()), this, SLOT(OnAddCalibPoint()) );    // Tracking & Image Points (Calibration)
  connect( m_Controls.m_CalibBtnCalibrate, SIGNAL(clicked()), this, SLOT(OnCalibration()) );      // Perform Calibration
  // Evaluation
  connect( m_Controls.m_EvalBtnStep1, SIGNAL(clicked()), this, SLOT(OnAddEvalProjectedPoint()) ); // Needle Projection
  connect( m_Controls.m_EvalBtnStep2, SIGNAL(clicked()), this, SLOT(SwitchFreeze()) );            // Freeze
  connect( m_Controls.m_EvalBtnStep3, SIGNAL(clicked()), this, SLOT(OnAddEvalTargetPoint()) );    // Tracking & Image Points (Evaluation)
  connect( m_Controls.m_EvalBtnSave,  SIGNAL(clicked()), this, SLOT(OnSaveEvaluation()) );        // Save Evaluation Results
  connect( m_Controls.m_CalibBtnSaveCalibration,  SIGNAL(clicked()), this, SLOT(OnSaveCalibration()) );         // Save Evaluation Results
  connect( m_Controls.m_BtnReset,     SIGNAL(clicked()), this, SLOT(OnReset()) );                 // Reset Pointsets

  connect( m_Controls.t1devices, SIGNAL(SignalCombinedModalitySelected(mitk::USCombinedModality::Pointer)),
           this, SLOT(OnSelectDevice(mitk::USCombinedModality::Pointer)));

  m_Controls.m_TabWidget->setTabEnabled(1, false);
  m_Controls.m_TabWidget->setTabEnabled(2, false);

   // Pointset for Calibration Points
  m_CalibPointsTool = mitk::PointSet::New();
  m_Node = mitk::DataNode::New();
  m_Node->SetName("Tool Calibration Points");
  m_Node->SetData(this->m_CalibPointsImage);
  this->GetDataStorage()->Add(m_Node);

  // Pointset for Worldpoints
  this->m_CalibPointsImage = mitk::PointSet::New();
  m_Node = mitk::DataNode::New();
  m_Node->SetName("Image Calibration Points");
  m_Node->SetData(this->m_CalibPointsTool);
  this->GetDataStorage()->Add(m_Node);

  m_CalibPointsCount = 0;

  // Evaluation Pointsets (Non-Visualized)
  m_EvalPointsImage     = mitk::PointSet::New();
  m_EvalPointsTool      = mitk::PointSet::New();
  m_EvalPointsProjected = mitk::PointSet::New();

  // Neelde Projection Filter
  m_NeedleProjectionFilter = mitk::NeedleProjectionFilter::New();

  // US Image Stream
  m_Node = mitk::DataNode::New();
  m_Node->SetName("US Image Stream");
  this->GetDataStorage()->Add(m_Node);

  // Tracking Status Widgets
  m_Controls.m_CalibTrackingStatus->ShowStatusLabels();
  m_Controls.m_EvalTrackingStatus->ShowStatusLabels();
}

void UltrasoundCalibration::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{

}

void UltrasoundCalibration::OnTabSwitch(int index){

}

void UltrasoundCalibration::OnSelectDevice(mitk::USCombinedModality::Pointer combinedModality)
{
  m_CombinedModality = combinedModality;

  if (combinedModality.IsNotNull())
  {
    m_Tracker = m_CombinedModality->GetNavigationDataSource();

    // Construct Pipeline
    this->m_NeedleProjectionFilter->SetInput(0, m_Tracker->GetOutput(0));

    this->SwitchFreeze();

    // Todo: Maybe display this elsewhere
    this->ShowNeedlePath();

    // Switch active tab to Calibration page
    m_Controls.m_TabWidget->setTabEnabled(1, true);
    m_Controls.m_TabWidget->setCurrentIndex(1);
  }
  else
  {
    m_Controls.m_TabWidget->setCurrentIndex(0);
    m_Controls.m_TabWidget->setTabEnabled(1, false);
    m_Controls.m_TabWidget->setTabEnabled(2, false);
  }
}

void UltrasoundCalibration::OnAddCalibPoint()
{
  mitk::Point3D world = this->GetRenderWindowPart()->GetSelectedPosition();

  this->m_CalibPointsImage->InsertPoint(m_CalibPointsCount, world);
  this->m_CalibPointsTool->InsertPoint(m_CalibPointsCount, this->m_FreezePoint);

  QString text = text.number(m_CalibPointsCount + 1);
  text = "Point " + text;
  this->m_Controls.m_CalibPointList->addItem(text);

  m_CalibPointsCount ++;
  SwitchFreeze();
}

void UltrasoundCalibration::OnCalibration()
{
  // Compute transformation
  vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();
  // TODO: must be activated again, when ConvertPointSetTVtkPolyData() is possible in MITK
  //mitk::PointSetModifier::Pointer myModifier = mitk::PointSetModifier::New();
  //transform->SetSourceLandmarks(myModifier->ConvertPointSetToVtkPolyData(m_CalibPointsImage)->GetPoints());
  //transform->SetTargetLandmarks(myModifier->ConvertPointSetToVtkPolyData(m_CalibPointsTool)->GetPoints());
  //transform->Modified();
  //transform->Update();

  // Convert from vtk to itk data types
  itk::Matrix<float,3,3> rotationFloat = itk::Matrix<float,3,3>();
  itk::Vector<float,3> translationFloat = itk::Vector<float,3>();
  vtkSmartPointer<vtkMatrix4x4> m = transform->GetMatrix();
  rotationFloat[0][0] = m->GetElement(0,0);
  rotationFloat[0][1] = m->GetElement(0,1);
  rotationFloat[0][2] = m->GetElement(0,2);
  rotationFloat[1][0] = m->GetElement(1,0);
  rotationFloat[1][1] = m->GetElement(1,1);
  rotationFloat[1][2] = m->GetElement(1,2);
  rotationFloat[2][0] = m->GetElement(2,0);
  rotationFloat[2][1] = m->GetElement(2,1);
  rotationFloat[2][2] = m->GetElement(2,2);
  translationFloat[0] = m->GetElement(0,3);
  translationFloat[1] = m->GetElement(1,3);
  translationFloat[2] = m->GetElement(2,3);

  // Set output variable
  m_Transformation = mitk::AffineTransform3D::New();
  m_Transformation->SetTranslation(translationFloat);
  m_Transformation->SetMatrix(rotationFloat);

  mitk::SlicedGeometry3D::Pointer sliced3d  = dynamic_cast< mitk::SlicedGeometry3D* > (m_Node->GetData()->GetGeometry());

  mitk::PlaneGeometry::Pointer plane = dynamic_cast< mitk::PlaneGeometry* > (sliced3d->GetGeometry2D(0));

  plane->SetIndexToWorldTransform(m_Transformation);

  // Save to US-Device
  m_CombinedModality->SetCalibration(m_Transformation);
  m_Controls.m_TabWidget->setTabEnabled(2, true);

  // Save to NeedleProjectionFilter
  m_NeedleProjectionFilter->SetTargetPlane(m_Transformation);

  // Update Calibration FRE
  m_CalibrationStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
  mitk::PointSet::Pointer p1 = this->m_CalibPointsTool->Clone(); // We use clones to calculate statistics to avoid concurrency Problems
  mitk::PointSet::Pointer p2 = this->m_CalibPointsImage->Clone();
  p2->GetGeometry()->SetIndexToWorldTransform(m_Transformation); // Image Points need to be transformed for calculation of FRE
  m_CalibrationStatistics->SetPointSets(p1, p2);
  QString text = text.number(m_CalibrationStatistics->GetRMS());
  m_Controls.m_EvalLblCalibrationFRE->setText(text);

  // Enable Button to save Calibration
  m_Controls.m_CalibBtnSaveCalibration->setEnabled(true);
}

void UltrasoundCalibration::OnAddEvalTargetPoint()
{
  mitk::Point3D world = this->GetRenderWindowPart()->GetSelectedPosition();

  this->m_EvalPointsImage->InsertPoint(m_EvalPointsImage->GetSize(), world);
  this->m_EvalPointsTool ->InsertPoint(m_EvalPointsTool->GetSize(),  this->m_FreezePoint);

  QString text = text.number(this->m_EvalPointsTool->GetSize());
  this->m_Controls.m_EvalLblNumTargetPoints->setText(text);

  // Update FREs
  // Update Evaluation FRE, but only if it contains more than one point (will crash otherwise)
  if ( (m_EvalPointsProjected->GetSize() > 1) && (m_EvalPointsTool->GetSize() > 1) ) {
    m_EvaluationStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
    m_ProjectionStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
    mitk::PointSet::Pointer p1 = this->m_EvalPointsTool->Clone(); // We use clones to calculate statistics to avoid concurrency Problems
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

void UltrasoundCalibration::OnAddEvalProjectedPoint()
{
  MITK_WARN << "Projection Evaluation may currently be inaccurate.";
  // TODO: Verify correct Evaluation. Is the Point that is added really current?
  mitk::Point3D projection = this->m_NeedleProjectionFilter->GetProjection()->GetPoint(1);
  m_EvalPointsProjected->InsertPoint(m_EvalPointsProjected->GetSize(), projection);
  QString text = text.number(this->m_EvalPointsProjected->GetSize());
  this->m_Controls.m_EvalLblNumProjectionPoints->setText(text);
}

void UltrasoundCalibration::OnSaveEvaluation()
{
  //Filename without suffix
  QString filename = m_Controls.m_EvalFilePath->text() + "//" + m_Controls.m_EvalFilePrefix->text();

  mitk::PointSetWriter::Pointer psWriter = mitk::PointSetWriter::New();
  psWriter->SetInput(0, m_CalibPointsImage);
  psWriter->SetInput(1, m_CalibPointsTool);
  psWriter->SetInput(2, m_EvalPointsImage);
  psWriter->SetInput(3, m_EvalPointsTool);
  psWriter->SetInput(4, m_EvalPointsProjected);
  psWriter->SetFileName(filename.toStdString() + ".xml");
  psWriter->Write();


  // TODO: New writer for transformations must be implemented.
  /*mitk::TransformationFileWriter::Pointer tWriter = mitk::TransformationFileWriter::New();
  tWriter->SetOutputFilename(filename.toStdString() + ".txt");
  tWriter->DoWrite(this->m_Transformation);*/
}

void UltrasoundCalibration::OnSaveCalibration()
{
  QString filename = QFileDialog::getSaveFileName( QApplication::activeWindow(),
                                                   "Save Calibration",
                                                   "",
                                                   "Calibration files *.cal" );

  // TODO: New writer for transformations must be implemented.
  //mitk::TransformationFileWriter::Pointer tWriter = mitk::TransformationFileWriter::New();
  //tWriter->SetOutputFilename(filename.toStdString());
  //tWriter->DoWrite(this->m_Transformation);
}

void UltrasoundCalibration::OnReset()
{
  m_CalibPointsTool->Clear();
  m_CalibPointsImage->Clear();
  m_CalibPointsCount = 0;

  m_EvalPointsImage->Clear();
  m_EvalPointsTool->Clear();
  m_EvalPointsProjected->Clear();

  this->m_Controls.m_CalibPointList->clear();

  m_Transformation->SetIdentity();

  m_CombinedModality->SetCalibration(m_Transformation);

  mitk::SlicedGeometry3D::Pointer sliced3d  = dynamic_cast< mitk::SlicedGeometry3D* > (m_Node->GetData()->GetGeometry());
  mitk::PlaneGeometry::Pointer plane = dynamic_cast< mitk::PlaneGeometry* > (sliced3d->GetGeometry2D(0));
  plane->SetIndexToWorldTransform(m_Transformation);

  QString text1 = text1.number(this->m_EvalPointsTool->GetSize());
  this->m_Controls.m_EvalLblNumTargetPoints->setText(text1);
  QString text2 = text2.number(this->m_EvalPointsProjected->GetSize());
  this->m_Controls.m_EvalLblNumProjectionPoints->setText(text2);
}

void UltrasoundCalibration::Update()
{
  //QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  // if (nodes.empty()) return;

  // Update Tracking Data
  std::vector<mitk::NavigationData::Pointer> datas;
  datas.push_back(m_Tracker->GetOutput());
  m_Controls.m_CalibTrackingStatus->SetNavigationDatas(&datas);
  m_Controls.m_CalibTrackingStatus->Refresh();
  m_Controls.m_EvalTrackingStatus->SetNavigationDatas(&datas);
  m_Controls.m_EvalTrackingStatus->Refresh();

  // Update US Image
  m_CombinedModality->UpdateOutputData(0);
  mitk::USImage::Pointer image = m_CombinedModality->GetOutput();
  m_Node->SetData(image);
  //m_Image->Update();

  // Update Needle Projection
  m_NeedleProjectionFilter->Update();

  this->RequestRenderWindowUpdate();
}

void UltrasoundCalibration::SwitchFreeze()
{
  m_Controls.m_CalibBtnAddPoint->setEnabled(false); // generally deactivate
  // We use the activity state of the timer to determine whether we are currently viewing images
  if ( ! m_Timer->isActive() ) // Activate Imaging
  {
    // if (m_Node) m_Node->ReleaseData();
    if (m_CombinedModality.IsNull()){
      m_Timer->stop();
      return;
    }
    m_CombinedModality->UpdateOutputData(0);
    m_Image = m_CombinedModality->GetOutput(0);
    m_Node->SetData(m_CombinedModality->GetOutput(0));

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
  }
  else if (this->m_Tracker->GetOutput(0)->IsDataValid())
  {
    //deactivate Imaging
    m_Timer->stop();
    // Remember last tracking coordinates
    m_FreezePoint = this->m_Tracker->GetOutput(0)->GetPosition();
    m_Controls.m_CalibBtnAddPoint->setEnabled(true); // activate only, if valid point is set
  }
}


void UltrasoundCalibration::ShowNeedlePath()
{
  // Init Filter
  this->m_NeedleProjectionFilter->SelectInput(0);

  // Create Node for Pointset
  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if ( node.IsNull() )
  {
    node = mitk::DataNode::New();
    node->SetName("Needle Path");
    node->SetData(m_NeedleProjectionFilter->GetProjection());
    node->SetBoolProperty("show contour", true);
    this->GetDataStorage()->Add(node);
  }
}
