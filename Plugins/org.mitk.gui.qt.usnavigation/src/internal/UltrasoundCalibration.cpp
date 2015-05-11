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
#include <QTextStream>

// MITK
#include <mitkVector.h>
//#include <mitkPointSetWriter.h>
#include <mitkImageGenerator.h>
#include <mitkSceneIO.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// us
#include <usServiceReference.h>

//VTK
#include <vtkPoints.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>

#include <vtkVertexGlyphFilter.h>

#include "internal/org_mbi_gui_qt_usnavigation_Activator.h"

const std::string UltrasoundCalibration::VIEW_ID = "org.mitk.views.ultrasoundcalibration";

UltrasoundCalibration::UltrasoundCalibration()
{
  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();

  if ( pluginContext )
  {
    // to be notified about service event of an USDevice
    pluginContext->connectServiceListener(this, "OnDeciveServiceEvent",
      QString::fromStdString("(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>() + ")"));
  }
}

UltrasoundCalibration::~UltrasoundCalibration()
{
  m_Timer->stop();
  // Sleep(500); //This might be problematic... seems like sometimes some ressources are still in use at calling time.

  this->OnStopCalibrationProcess();

  /*mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Tool Calibration Points");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);
  node = this->GetDataStorage()->GetNamedNode("Image Calibration Points");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);
  node = this->GetDataStorage()->GetNamedNode("US Image Stream");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);*/
  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Needle Path");
  if (node.IsNotNull())this->GetDataStorage()->Remove(node);

  delete m_Timer;
}

void UltrasoundCalibration::SetFocus()
{
  m_Controls.m_TabWidget->setFocus();
}

void UltrasoundCalibration::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_Controls.m_CombinedModalityManagerWidget->SetCalibrationLoadedNecessary(false);

  m_Timer = new QTimer(this);

  m_Controls.m_TabWidget->setTabEnabled(1, false);
  m_Controls.m_TabWidget->setTabEnabled(2, false);

  // Pointset for Calibration Points
  m_CalibPointsTool = mitk::PointSet::New();

  // Pointset for Worldpoints
  m_CalibPointsImage = mitk::PointSet::New();

  m_CalibPointsCount = 0;

  // Evaluation Pointsets (Non-Visualized)
  m_EvalPointsImage     = mitk::PointSet::New();
  m_EvalPointsTool      = mitk::PointSet::New();
  m_EvalPointsProjected = mitk::PointSet::New();

  // Neelde Projection Filter
  m_NeedleProjectionFilter = mitk::NeedleProjectionFilter::New();

  // Tracking Status Widgets
  m_Controls.m_CalibTrackingStatus->ShowStatusLabels();
  m_Controls.m_EvalTrackingStatus->ShowStatusLabels();

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

  //connect( m_Controls.m_CombinedModalityManagerWidget, SIGNAL(SignalCombinedModalitySelected(mitk::USCombinedModality::Pointer)),
  //  this, SLOT(OnSelectDevice(mitk::USCombinedModality::Pointer)) );
  connect( m_Controls.m_CombinedModalityManagerWidget, SIGNAL(SignalReadyForNextStep()),
           this, SLOT(OnDeviceSelected()) );
  connect( m_Controls.m_CombinedModalityManagerWidget, SIGNAL(SignalNoLongerReadyForNextStep()),
    this, SLOT(OnDeviceDeselected()) );

  connect( m_Controls.m_StartCalibrationButton, SIGNAL(clicked()), this, SLOT(OnStartCalibrationProcess()) );

  connect( m_Controls.m_CalibBtnRestartCalibration, SIGNAL(clicked()), this, SLOT(OnReset()) );
  connect( m_Controls.m_CalibBtnStopCalibration, SIGNAL(clicked()), this, SLOT(OnStopCalibrationProcess()) );
}

void UltrasoundCalibration::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                               const QList<mitk::DataNode::Pointer>& /*nodes*/ )
{
}

void UltrasoundCalibration::OnTabSwitch(int index)
{
  switch (index)
  {
  case 0:
    if ( m_Controls.m_TabWidget->isTabEnabled(1) || m_Controls.m_TabWidget->isTabEnabled(2) )
    {
      this->OnStopCalibrationProcess();
    }
    break;
  default:
    ;
  }
}

//void UltrasoundCalibration::OnSelectDevice(mitk::USCombinedModality::Pointer combinedModality)
void UltrasoundCalibration::OnDeviceSelected()
{
  mitk::USCombinedModality::Pointer combinedModality;

  combinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();

  if ( combinedModality.IsNotNull() )
  {
    //m_Tracker = m_CombinedModality->GetNavigationDataSource();

    // Construct Pipeline
    //this->m_NeedleProjectionFilter->SetInput(0, m_Tracker->GetOutput(0));

    m_Controls.m_StartCalibrationButton->setEnabled(true);
  }
}

void UltrasoundCalibration::OnDeviceDeselected()
{
  m_Controls.m_StartCalibrationButton->setEnabled(false);
  m_Controls.m_TabWidget->setCurrentIndex(0);
  m_Controls.m_TabWidget->setTabEnabled(1, false);
  m_Controls.m_TabWidget->setTabEnabled(2, false);
}

void UltrasoundCalibration::OnStartCalibrationProcess()
{
  // US Image Stream
  m_Node = mitk::DataNode::New();
  m_Node->SetName("US Calibration Viewing Stream");
  //create a dummy image (gray values 0..255) for correct initialization of level window, etc.
  mitk::Image::Pointer dummyImage = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 1, 1, 1, 1, 1, 255,0);
  m_Node->SetData(dummyImage);
  this->GetDataStorage()->Add(m_Node);

  // data node for calibration point set
  m_CalibNode = mitk::DataNode::New();
  m_CalibNode->SetName("Tool Calibration Points");
  m_CalibNode->SetData(this->m_CalibPointsImage);
  this->GetDataStorage()->Add(m_CalibNode);

  // data node for world point set
  m_WorldNode = mitk::DataNode::New();
  m_WorldNode->SetName("Image Calibration Points");
  m_WorldNode->SetData(this->m_CalibPointsTool);
  this->GetDataStorage()->Add(m_WorldNode);

  m_CombinedModality = m_Controls.m_CombinedModalityManagerWidget->GetSelectedCombinedModality();
  if ( m_CombinedModality.IsNull() ) { return; }

  m_Tracker = m_CombinedModality->GetNavigationDataSource();

  //QString curDepth = service.getProperty(QString::fromStdString(mitk::USDevice::US_PROPKEY_BMODE_DEPTH)).toString();

  // Construct Pipeline
  this->m_NeedleProjectionFilter->SetInput(0, m_Tracker->GetOutput(0));

  QApplication::setOverrideCursor(Qt::WaitCursor);
  // make sure that the combined modality is in connected state before using it
  if ( m_CombinedModality->GetDeviceState() < mitk::USDevice::State_Connected ) { m_CombinedModality->Connect(); }
  if ( m_CombinedModality->GetDeviceState() < mitk::USDevice::State_Activated ) { m_CombinedModality->Activate(); }
  QApplication::restoreOverrideCursor();

  this->SwitchFreeze();

  // Todo: Maybe display this elsewhere
  this->ShowNeedlePath();

  // Switch active tab to Calibration page
  m_Controls.m_TabWidget->setTabEnabled(1, true);
  m_Controls.m_TabWidget->setCurrentIndex(1);
}

void UltrasoundCalibration::OnStopCalibrationProcess()
{
  this->ClearTemporaryMembers();

  m_Timer->stop();

  this->GetDataStorage()->Remove(m_Node);
  m_Node = 0;

  this->GetDataStorage()->Remove(m_CalibNode);
  m_CalibNode = 0;

  this->GetDataStorage()->Remove(m_WorldNode);
  m_WorldNode = 0;

  m_Controls.m_TabWidget->setCurrentIndex(0);
  m_Controls.m_TabWidget->setTabEnabled(1, false);
  m_Controls.m_TabWidget->setTabEnabled(2, false);
}

void UltrasoundCalibration::OnDeciveServiceEvent(const ctkServiceEvent event)
{
  if ( m_CombinedModality.IsNull() || event.getType() != ctkServiceEvent::MODIFIED ) { return; }

  ctkServiceReference service = event.getServiceReference();

  QString curDepth = service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH)).toString();
  if ( m_CurrentDepth != curDepth )
  {
    m_CurrentDepth = curDepth;
    this->OnReset();
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

  transform->SetSourceLandmarks(this->ConvertPointSetToVtkPolyData(m_CalibPointsImage)->GetPoints());
  transform->SetTargetLandmarks(this->ConvertPointSetToVtkPolyData(m_CalibPointsTool)->GetPoints());
  if(m_Controls.m_ScaleTransform->isChecked())
    {transform->SetModeToSimilarity();} //use affine transform
  else
    {transform->SetModeToRigidBody();} //use similarity transform: scaling is not touched
  transform->Modified();
  transform->Update();

  // Convert from vtk to itk data types
  itk::Matrix<mitk::ScalarType,3,3> rotationFloat = itk::Matrix<mitk::ScalarType,3,3>();
  itk::Vector<mitk::ScalarType,3> translationFloat = itk::Vector<mitk::ScalarType,3>();
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

  mitk::DataNode::Pointer CalibPointsImage = mitk::DataNode::New();
  CalibPointsImage->SetName("Calibration Points Image");
  CalibPointsImage->SetData(m_CalibPointsImage);
  this->GetDataStorage()->Add(CalibPointsImage);

  mitk::DataNode::Pointer CalibPointsTracking = mitk::DataNode::New();
  CalibPointsTracking->SetName("Calibration Points Tracking");
  CalibPointsTracking->SetData(m_CalibPointsTool);
  this->GetDataStorage()->Add(CalibPointsTracking);

  mitk::PointSet::Pointer ImagePointsTransformed = m_CalibPointsImage->Clone();
  this->ApplyTransformToPointSet(ImagePointsTransformed,transform);
  mitk::DataNode::Pointer CalibPointsImageTransformed = mitk::DataNode::New();
  CalibPointsImageTransformed->SetName("Calibration Points Image (Transformed)");
  CalibPointsImageTransformed->SetData(ImagePointsTransformed);
  this->GetDataStorage()->Add(CalibPointsImageTransformed);

  // Set output variable

  mitk::AffineTransform3D::Pointer oldUSImageTransform = m_Image->GetGeometry()->GetIndexToWorldTransform(); //including spacing!

  MITK_INFO << "Old US Image transform: " << oldUSImageTransform;

  mitk::AffineTransform3D::Pointer calibTransform = mitk::AffineTransform3D::New();
  calibTransform->SetTranslation(translationFloat);
  calibTransform->SetMatrix(rotationFloat);

  MITK_INFO << "Calibration transform: " << calibTransform;

  m_Transformation = mitk::AffineTransform3D::New();
  if(!m_Controls.m_ScaleTransform->isChecked()) {m_Transformation->Compose(oldUSImageTransform);}
  m_Transformation->Compose(calibTransform);

  MITK_INFO << "New combined transform: " << m_Transformation;


  mitk::SlicedGeometry3D::Pointer sliced3d  = dynamic_cast< mitk::SlicedGeometry3D* > (m_Node->GetData()->GetGeometry());

  mitk::PlaneGeometry::Pointer plane = const_cast< mitk::PlaneGeometry* > (sliced3d->GetGeometry2D(0));

  plane->SetIndexToWorldTransform(m_Transformation);

  // Save to US-Device
  m_CombinedModality->SetCalibration(m_Transformation);
  m_Controls.m_TabWidget->setTabEnabled(2, true);

  // Save to NeedleProjectionFilter
  m_NeedleProjectionFilter->SetTargetPlane(m_Transformation);

  // Update Calibration FRE
  m_CalibrationStatistics = mitk::PointSetDifferenceStatisticsCalculator::New();
  mitk::PointSet::Pointer p1 = this->m_CalibPointsTool->Clone(); // We use clones to calculate statistics to avoid concurrency Problems

  // Create point set with transformed image calibration points for
  // calculating the difference of image calibration and tool
  // calibration points in one geometry space
  mitk::PointSet::Pointer p2 = mitk::PointSet::New();
  int n = 0;
  for ( mitk::PointSet::PointsConstIterator it = m_CalibPointsImage->Begin();
    it != m_CalibPointsImage->End(); ++it, ++n)
  {
    p2->InsertPoint(n, m_Transformation->TransformPoint(it->Value()));
  }

  m_CalibrationStatistics->SetPointSets(p1, p2);
  //QString text = text.number(m_CalibrationStatistics->GetRMS());
  QString text = QString::number(ComputeFRE(m_CalibPointsImage,m_CalibPointsTool,transform));
  MITK_INFO << "Calibration FRE: "  << text.toStdString().c_str();
  m_Controls.m_EvalLblCalibrationFRE->setText(text);

  m_Node->SetStringProperty("Calibration FRE",text.toStdString().c_str());
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

void UltrasoundCalibration::OnSaveCalibration()
{
  QString filename = QFileDialog::getSaveFileName( QApplication::activeWindow(),
    "Save Calibration",
    "",
    "Calibration files *.cal" );

  QFile file(filename);
  if ( ! file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) )
  {
    MITK_WARN << "Cannot open file '" << filename.toStdString() << "' for writing.";
    return;
  }

  std::string calibrationSerialization = m_CombinedModality->SerializeCalibration();

  QTextStream outStream(&file);
  outStream << QString::fromStdString(calibrationSerialization);

  //save additional information
  if (m_Controls.m_saveAdditionalCalibrationLog->isChecked())
    {
    mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
    QString filenameScene = filename + "_mitkScene.mitk";
    mitk::NodePredicateNot::Pointer isNotHelperObject =
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = this->GetDataStorage()->GetSubset(isNotHelperObject);
    mySceneIO->SaveScene(nodesToBeSaved,this->GetDataStorage(),filenameScene.toStdString().c_str());
    }
}

void UltrasoundCalibration::OnReset()
{
  this->ClearTemporaryMembers();

  if ( m_Transformation.IsNull() ) { m_Transformation = mitk::AffineTransform3D::New(); }
  m_Transformation->SetIdentity();

  if ( m_Node.IsNotNull() && (m_Node->GetData() != NULL) && (m_Node->GetData()->GetGeometry() != NULL) )
  {
    mitk::SlicedGeometry3D::Pointer sliced3d  = dynamic_cast< mitk::SlicedGeometry3D* > (m_Node->GetData()->GetGeometry());
    mitk::PlaneGeometry::Pointer plane = const_cast< mitk::PlaneGeometry* > (sliced3d->GetGeometry2D(0));
    plane->SetIndexToWorldTransform(m_Transformation);
  }

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
  std::vector<mitk::NavigationData::Pointer>* datas = new std::vector<mitk::NavigationData::Pointer>();
  datas->push_back(m_Tracker->GetOutput());
  m_Controls.m_CalibTrackingStatus->SetNavigationDatas(datas);
  m_Controls.m_CalibTrackingStatus->Refresh();
  m_Controls.m_EvalTrackingStatus->SetNavigationDatas(datas);
  m_Controls.m_EvalTrackingStatus->Refresh();

  // Update US Image
  m_CombinedModality->Modified();
  m_CombinedModality->Update();
  mitk::Image::Pointer m_Image = m_CombinedModality->GetOutput();
  if (m_Image.IsNotNull() && m_Image->IsInitialized())
  {
    m_Node->SetData(m_Image);
  }

  // Update Needle Projection
  m_NeedleProjectionFilter->Update();

  //only update 2d window because it is faster
  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
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
    //deactivate Imaging
    m_Timer->stop();
    // Remember last tracking coordinates
    m_FreezePoint = this->m_Tracker->GetOutput(0)->GetPosition();
    m_Controls.m_CalibBtnAddPoint->setEnabled(true); // activate only, if valid point is set

    m_CombinedModality->SetIsFreezed(true);
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

void UltrasoundCalibration::ClearTemporaryMembers()
{
  m_CalibPointsTool->Clear();
  m_CalibPointsImage->Clear();
  m_CalibPointsCount = 0;

  m_EvalPointsImage->Clear();
  m_EvalPointsTool->Clear();
  m_EvalPointsProjected->Clear();

  this->m_Controls.m_CalibPointList->clear();
}

vtkSmartPointer<vtkPolyData> UltrasoundCalibration::ConvertPointSetToVtkPolyData(mitk::PointSet::Pointer PointSet)
{
  vtkSmartPointer<vtkPolyData> returnValue = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for(int i=0; i<PointSet->GetSize(); i++)
  {
    double point[3] = {PointSet->GetPoint(i)[0],PointSet->GetPoint(i)[1],PointSet->GetPoint(i)[2]};
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

double UltrasoundCalibration::ComputeFRE(mitk::PointSet::Pointer imageFiducials, mitk::PointSet::Pointer realWorldFiducials, vtkSmartPointer<vtkLandmarkTransform> transform)
{
  if(imageFiducials->GetSize() != realWorldFiducials->GetSize()) return -1;
  double FRE = 0;
  for(unsigned int i = 0; i < imageFiducials->GetSize(); i++)
    {
    itk::Point<double> current_image_fiducial_point = imageFiducials->GetPoint(i);
    if (transform != NULL)
      {
      current_image_fiducial_point = transform->TransformPoint(imageFiducials->GetPoint(i)[0],imageFiducials->GetPoint(i)[1],imageFiducials->GetPoint(i)[2]);
      }
    double cur_error_squared = current_image_fiducial_point.SquaredEuclideanDistanceTo(realWorldFiducials->GetPoint(i));
    FRE += cur_error_squared;
    }

  FRE = sqrt(FRE/ (double) imageFiducials->GetSize());

  return FRE;
}

void UltrasoundCalibration::ApplyTransformToPointSet(mitk::PointSet::Pointer pointSet, vtkSmartPointer<vtkLandmarkTransform> transform)
{
  for(unsigned int i = 0; i < pointSet->GetSize(); i++)
    {
    itk::Point<double> current_point_transformed = itk::Point<double>();
    current_point_transformed = transform->TransformPoint(pointSet->GetPoint(i)[0],pointSet->GetPoint(i)[1],pointSet->GetPoint(i)[2]);
    pointSet->SetPoint(i,current_point_transformed);
    }
}
