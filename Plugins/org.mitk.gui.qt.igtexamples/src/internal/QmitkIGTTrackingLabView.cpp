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
#include "QmitkIGTTrackingLabView.h"
#include "QmitkStdMultiWidget.h"

#include <QmitkNDIConfigurationWidget.h>
#include <QmitkFiducialRegistrationWidget.h>
#include <QmitkUpdateTimerWidget.h>
#include <QmitkToolSelectionWidget.h>
#include <QmitkToolTrackingStatusWidget.h>


#include <mitkCone.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>

#include <vtkConeSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkLandmarkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>

// Qt
#include <QMessageBox>
#include <QIcon>


const std::string QmitkIGTTrackingLabView::VIEW_ID = "org.mitk.views.igttrackinglab";

QmitkIGTTrackingLabView::QmitkIGTTrackingLabView()
: QmitkFunctionality()
,m_Source(NULL)
,m_PermanentRegistrationFilter(NULL)
,m_Visualizer(NULL)
,m_VirtualView(NULL)
,m_PSRecordingPointSet(NULL)
,m_RegistrationTrackingFiducialsName("Tracking Fiducials")
,m_RegistrationImageFiducialsName("Image Fiducials")
,m_PointSetRecordingDataNodeName("Recorded Points")
,m_PointSetRecording(false)
,m_PermanentRegistration(false)
,m_CameraView(false)
,m_ImageFiducialsDataNode(NULL)
,m_TrackerFiducialsDataNode(NULL)
,m_PermanentRegistrationSourcePoints(NULL)
{

  //[-1;0;0] for WOLF_6D bronchoscope
  m_DirectionOfProjectionVector[0]=0;
  m_DirectionOfProjectionVector[1]=0;
  m_DirectionOfProjectionVector[2]=-1;}

QmitkIGTTrackingLabView::~QmitkIGTTrackingLabView()
{
  if (m_Timer->isActive()) m_Timer->stop();
}

void QmitkIGTTrackingLabView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_ToolBox = m_Controls.m_ToolBox;

  this->CreateBundleWidgets( parent );
  this->CreateConnections();
}


void QmitkIGTTrackingLabView::CreateBundleWidgets( QWidget* parent )
{
  //initialize registration widget
  m_RegistrationWidget = m_Controls.m_RegistrationWidget;
  m_RegistrationWidget->HideStaticRegistrationRadioButton(true);
  m_RegistrationWidget->HideContinousRegistrationRadioButton(true);
  m_RegistrationWidget->HideUseICPRegistrationCheckbox(true);
}


void QmitkIGTTrackingLabView::CreateConnections()
{
  m_Timer = new QTimer(this);
  connect(m_Timer, SIGNAL(timeout()), this, SLOT(UpdateTimer()));
  connect( m_ToolBox, SIGNAL(currentChanged(int)), this, SLOT(OnToolBoxCurrentChanged(int)) );
  connect( m_Controls.m_UsePermanentRegistrationToggle, SIGNAL(toggled(bool)), this, SLOT(OnPermanentRegistration(bool)) );
  connect( m_Controls.m_TrackingDeviceSelectionWidget, SIGNAL(NavigationDataSourceSelected(mitk::NavigationDataSource::Pointer)), this, SLOT(OnSetupNavigation()) );
  connect( m_Controls.m_UseAsPointerButton, SIGNAL(clicked()), this, SLOT(OnInstrumentSelected()) );
  connect( m_Controls.m_UseAsObjectmarkerButton, SIGNAL(clicked()), this, SLOT(OnObjectmarkerSelected()) );
  connect( m_RegistrationWidget, SIGNAL(AddedTrackingFiducial()), this, SLOT(OnAddRegistrationTrackingFiducial()) );
  connect( m_RegistrationWidget, SIGNAL(PerformFiducialRegistration()), this, SLOT(OnRegisterFiducials()) );
  connect( m_Controls.m_PointSetRecordCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnPointSetRecording(bool)) );
  connect( m_Controls.m_ActivateNeedleView, SIGNAL(toggled(bool)), this, SLOT(OnVirtualCamera(bool)) );

  //start timer
  m_Timer->start(30);

  //initialize Combo Boxes
  m_Controls.m_ObjectComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_ObjectComboBox->SetAutoSelectNewItems(false);
  m_Controls.m_ObjectComboBox->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  m_Controls.m_ImageComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_ImageComboBox->SetAutoSelectNewItems(false);
  m_Controls.m_ImageComboBox->SetPredicate(mitk::NodePredicateDataType::New("Image"));
}

void QmitkIGTTrackingLabView::UpdateTimer()
  {
  if (m_PermanentRegistration && m_PermanentRegistrationFilter.IsNotNull()) {m_PermanentRegistrationFilter->Update();}

  if (m_CameraView && m_VirtualView.IsNotNull()) {m_VirtualView->Update();}

  if(m_PointSetRecording && m_PSRecordingPointSet.IsNotNull())
    {
      int size = m_PSRecordingPointSet->GetSize();
      mitk::NavigationData::Pointer nd = m_PointSetRecordingNavigationData;

      if(size > 0)
      {
        mitk::Point3D p = m_PSRecordingPointSet->GetPoint(size-1);
        if(p.EuclideanDistanceTo(nd->GetPosition()) > (double) m_Controls.m_PSRecordingSpinBox->value())
          m_PSRecordingPointSet->InsertPoint(size, nd->GetPosition());
      }
      else
        m_PSRecordingPointSet->InsertPoint(size, nd->GetPosition());
    }
  }


void QmitkIGTTrackingLabView::OnAddRegistrationTrackingFiducial()
{
  mitk::NavigationData::Pointer nd = m_InstrumentNavigationData;

  if( nd.IsNull() || !nd->IsDataValid())
  {
    QMessageBox::warning( 0, "Invalid tracking data", "Navigation data is not available or invalid!", QMessageBox::Ok );
    return;
  }

  if(m_TrackerFiducialsDataNode.IsNotNull() && m_TrackerFiducialsDataNode->GetData() != NULL)
  {
    mitk::PointSet::Pointer ps = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsDataNode->GetData());
    ps->InsertPoint(ps->GetSize(), nd->GetPosition());
  }
  else
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "Can not access Tracker Fiducials. Adding fiducial not possible!");
}

void QmitkIGTTrackingLabView::OnInstrumentSelected()
{
  if (m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationDataSource().IsNotNull())
    {
    m_InstrumentNavigationData = m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedToolID());
    }
  else
    {
    m_Controls.m_PointerNameLabel->setText("<not available>");
    return;
    }

  if (m_InstrumentNavigationData.IsNotNull())
    {
    m_Controls.m_PointerNameLabel->setText(m_InstrumentNavigationData->GetName());
    }
  else
    {
    m_Controls.m_PointerNameLabel->setText("<not available>");
    }
}

void QmitkIGTTrackingLabView::OnObjectmarkerSelected()
{

if (m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationDataSource().IsNotNull())
    {
    m_ObjectmarkerNavigationData = m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedToolID());
    }
  else
    {
    m_Controls.m_ObjectmarkerNameLabel->setText("<not available>");
    return;
    }

  if (m_ObjectmarkerNavigationData.IsNotNull())
    {
    m_Controls.m_ObjectmarkerNameLabel->setText(m_ObjectmarkerNavigationData->GetName());
    }
  else
    {
    m_Controls.m_ObjectmarkerNameLabel->setText("<not available>");
    }
}

void QmitkIGTTrackingLabView::OnSetupNavigation()
{
  MITK_INFO << "SetupNavigationCalled";
  if(m_Source.IsNotNull())
    if(m_Source->IsTracking())
      return;

  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if(ds == NULL)
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "can not access DataStorage. Navigation not possible");
    return;
  }

  // Building up the filter pipeline
  try
  {
    this->SetupIGTPipeline();

  }
  catch(std::exception& e)
  {
    QMessageBox::warning(NULL, QString("IGTSurfaceTracker: Error"), QString("Error while building the IGT-Pipeline: %1").arg(e.what()));
    this->DestroyIGTPipeline(); // destroy the pipeline if building is incomplete
    return;
  }
  catch(...)
  {
    QMessageBox::warning(NULL, QString("IGTSurfaceTracker: Error"), QString("Error while building the IGT-Pipeline"));
    this->DestroyIGTPipeline();
    return;
  }
}

void QmitkIGTTrackingLabView::SetupIGTPipeline()
{
  this->InitializeRegistration(); //initializes the registration widget
}

void QmitkIGTTrackingLabView::InitializeFilters()
{

}

void QmitkIGTTrackingLabView::OnRegisterFiducials()
{
  //Check for initialization
  if (!CheckRegistrationInitialization()) return;

  /* retrieve fiducials from data storage */
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  mitk::PointSet::Pointer imageFiducials = dynamic_cast<mitk::PointSet*>(m_ImageFiducialsDataNode->GetData());
  mitk::PointSet::Pointer trackerFiducials = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsDataNode->GetData());

  //convert point sets to vtk poly data
  vtkSmartPointer<vtkPoints> sourcePoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPoints> targetPoints = vtkSmartPointer<vtkPoints>::New();
  for (int i=0; i<imageFiducials->GetSize(); i++)
    {
    double point[3] = {imageFiducials->GetPoint(i)[0],imageFiducials->GetPoint(i)[1],imageFiducials->GetPoint(i)[2]};
    sourcePoints->InsertNextPoint(point);
    double point_targets[3] = {trackerFiducials->GetPoint(i)[0],trackerFiducials->GetPoint(i)[1],trackerFiducials->GetPoint(i)[2]};
    targetPoints->InsertNextPoint(point_targets);
    }

  //compute transform
  vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();
  transform->SetSourceLandmarks(sourcePoints);
  transform->SetTargetLandmarks(targetPoints);
  transform->Modified();
  transform->Update();

  //compute FRE
  double FRE = 0;
  for(unsigned int i = 0; i < imageFiducials->GetSize(); i++)
    {
    itk::Point<double> transformed = transform->TransformPoint(imageFiducials->GetPoint(i)[0],imageFiducials->GetPoint(i)[1],imageFiducials->GetPoint(i)[2]);
    double cur_error_squared = transformed.SquaredEuclideanDistanceTo(trackerFiducials->GetPoint(i));
    FRE += cur_error_squared;
    }

  FRE = sqrt(FRE/ (double) imageFiducials->GetSize());
  m_Controls.m_RegistrationWidget->SetQualityDisplayText("FRE: " + QString::number(FRE) + " mm");

  //convert from vtk to itk data types
  itk::Matrix<float,3,3> rotationFloat = itk::Matrix<float,3,3>();
  itk::Vector<float,3> translationFloat = itk::Vector<float,3>();

  vtkSmartPointer<vtkMatrix4x4> m = transform->GetMatrix();
  for(int k=0; k<3; k++) for(int l=0; l<3; l++)
  {
    rotationFloat[k][l] = m->GetElement(k,l);

  }
  for(int k=0; k<3; k++)
  {
    translationFloat[k] = m->GetElement(k,3);
  }

  //transform surface
  mitk::AffineTransform3D::Pointer newTransform = mitk::AffineTransform3D::New();
  newTransform->SetMatrix(rotationFloat);
  newTransform->SetOffset(translationFloat);
  m_Controls.m_ObjectComboBox->GetSelectedNode()->GetData()->GetGeometry()->SetIndexToWorldTransform(newTransform);

  //transform ct image
  mitk::AffineTransform3D::Pointer imageTransform = m_Controls.m_ImageComboBox->GetSelectedNode()->GetData()->GetGeometry()->GetIndexToWorldTransform();
  imageTransform->Compose(newTransform);
  mitk::AffineTransform3D::Pointer newImageTransform = mitk::AffineTransform3D::New(); //create new image transform... setting the composed leads to an error
  itk::Matrix<float,3,3> rotationFloatNew = imageTransform->GetMatrix();
  itk::Vector<float,3> translationFloatNew = imageTransform->GetOffset();
  newImageTransform->SetMatrix(rotationFloatNew);
  newImageTransform->SetOffset(translationFloatNew);
  m_Controls.m_ImageComboBox->GetSelectedNode()->GetData()->GetGeometry()->SetIndexToWorldTransform(newImageTransform);

}


void QmitkIGTTrackingLabView::OnTrackerDisconnected()
{
  this->DestroyInstrumentVisualization(this->GetDefaultDataStorage(), m_NDIConfigWidget->GetTracker());
}


mitk::DataNode::Pointer QmitkIGTTrackingLabView::CreateInstrumentVisualization(mitk::DataStorage* ds, const char* toolName)
{
    //const char* toolName = tracker->GetTool(i)->GetToolName();
    mitk::DataNode::Pointer toolRepresentationNode;
    toolRepresentationNode = ds->GetNamedNode(toolName); // check if node with same name already exists

    if(toolRepresentationNode.IsNotNull())
      ds->Remove(toolRepresentationNode); // remove old node with same name

    toolRepresentationNode = this->CreateConeRepresentation( toolName );

    ds->Add(toolRepresentationNode); // adds node to data storage

    return toolRepresentationNode;
}


mitk::DataNode::Pointer QmitkIGTTrackingLabView::CreateConeRepresentation( const char* label )
{

  //new data
  mitk::Cone::Pointer activeToolData = mitk::Cone::New();
  vtkConeSource* vtkData = vtkConeSource::New();

  vtkData->SetRadius(7.5);
  vtkData->SetHeight(15.0);
  vtkData->SetDirection(m_DirectionOfProjectionVector[0],m_DirectionOfProjectionVector[1],m_DirectionOfProjectionVector[2]);
  vtkData->SetCenter(0.0, 0.0, 7.5);
  vtkData->SetResolution(20);
  vtkData->CappingOn();
  vtkData->Update();
  activeToolData->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  //new node
  mitk::DataNode::Pointer coneNode = mitk::DataNode::New();
  coneNode->SetData(activeToolData);
  coneNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New( label ));
  coneNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(0));
  coneNode->GetPropertyList()->SetProperty("visible", mitk::BoolProperty::New(true));
  coneNode->SetColor(1.0,0.0,0.0);
  coneNode->SetOpacity(0.85);
  coneNode->Modified();

  return coneNode;
}

void QmitkIGTTrackingLabView::DestroyIGTPipeline()
{
  if(m_Source.IsNotNull())
  {
    m_Source->StopTracking();
    m_Source->Disconnect();
    m_Source = NULL;
  }
  m_PermanentRegistrationFilter = NULL;
  m_Visualizer = NULL;
  m_VirtualView = NULL;
}

void QmitkIGTTrackingLabView::InitializeRegistration()
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if( ds == NULL )
    return;


  m_RegistrationWidget->SetMultiWidget(this->GetActiveStdMultiWidget()); // passing multiwidget to pointsetwidget

  if(m_ImageFiducialsDataNode.IsNull())
  {
    m_ImageFiducialsDataNode = mitk::DataNode::New();
    mitk::PointSet::Pointer ifPS = mitk::PointSet::New();

    m_ImageFiducialsDataNode->SetData(ifPS);

    mitk::Color color;
    color.Set(1.0f, 0.0f, 0.0f);
    m_ImageFiducialsDataNode->SetName(m_RegistrationImageFiducialsName);
    m_ImageFiducialsDataNode->SetColor(color);
    m_ImageFiducialsDataNode->SetBoolProperty( "updateDataOnRender", false );

    ds->Add(m_ImageFiducialsDataNode);
  }
  m_RegistrationWidget->SetMultiWidget(this->GetActiveStdMultiWidget());
  m_RegistrationWidget->SetImageFiducialsNode(m_ImageFiducialsDataNode);

  if(m_TrackerFiducialsDataNode.IsNull())
  {
    m_TrackerFiducialsDataNode = mitk::DataNode::New();
    mitk::PointSet::Pointer tfPS = mitk::PointSet::New();
    m_TrackerFiducialsDataNode->SetData(tfPS);

    mitk::Color color;
    color.Set(0.0f, 1.0f, 0.0f);
    m_TrackerFiducialsDataNode->SetName(m_RegistrationTrackingFiducialsName);
    m_TrackerFiducialsDataNode->SetColor(color);
    m_TrackerFiducialsDataNode->SetBoolProperty( "updateDataOnRender", false );

    ds->Add(m_TrackerFiducialsDataNode);
  }

  m_RegistrationWidget->SetTrackerFiducialsNode(m_TrackerFiducialsDataNode);
}


void QmitkIGTTrackingLabView::OnToolBoxCurrentChanged(const int index)
{
  switch (index)
  {
  case RegistrationWidget:
    this->InitializeRegistration();
    break;

  default:
    break;
  }
}


mitk::DataNode::Pointer QmitkIGTTrackingLabView::CreateRegistrationFiducialsNode( const std::string& label, const mitk::Color& color)
{
  mitk::DataNode::Pointer fiducialsNode = mitk::DataNode::New();
  mitk::PointSet::Pointer fiducialsPointSet = mitk::PointSet::New();

  fiducialsNode->SetData(fiducialsPointSet);
  fiducialsNode->SetName( label );
  fiducialsNode->SetColor( color );
  fiducialsNode->SetBoolProperty( "updateDataOnRender", false );

  return fiducialsNode;
}


void QmitkIGTTrackingLabView::ChangeToolRepresentation( int toolID , mitk::Surface::Pointer surface )
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if(ds == NULL)
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "Can not access DataStorage. Changing tool representation not possible!");
    return;
  }

  mitk::TrackingDevice::Pointer tracker = m_NDIConfigWidget->GetTracker();
  if(tracker.IsNull())
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "Can not access Tracker. Changing tool representation not possible!");
    return;
  }

  try
  {
    const char* name = tracker->GetTool(toolID)->GetToolName();   // get tool name by id

    mitk::DataNode::Pointer toolNode = ds->GetNamedNode(name);

    if(toolNode.IsNull())
      return;

    toolNode->SetData(surface); // change surface representation of node
    toolNode->SetColor(0.45,0.70,0.85); //light blue like old 5D sensors
    toolNode->Modified();

    m_Visualizer->SetRepresentationObject( toolID, toolNode->GetData()); // updating node with changed surface back in visualizer

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  catch(std::exception& e)
  {
    QMessageBox::warning(NULL, QString("IGTSurfaceTracker: Error"), QString("Can not change tool representation!").arg(e.what()));
    return;
  }
}

void QmitkIGTTrackingLabView::OnPointSetRecording(bool record)
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();

  if(record)
  {
    if (m_Controls.m_PointSetRecordingToolSelectionWidget->GetSelectedToolID() == -1)
      {
      QMessageBox::warning(NULL, "Error", "No tool selected for point set recording!");
      m_Controls.m_PointSetRecordCheckBox->setChecked(false);
      return;
      }
    m_PointSetRecordingNavigationData = m_Controls.m_PointSetRecordingToolSelectionWidget->GetSelectedNavigationDataSource()->GetOutput(m_Controls.m_PointSetRecordingToolSelectionWidget->GetSelectedToolID());

    //initialize point set
    mitk::DataNode::Pointer psRecND = ds->GetNamedNode(m_PointSetRecordingDataNodeName);
    if(m_PSRecordingPointSet.IsNull() || psRecND.IsNull())
    {
      m_PSRecordingPointSet = NULL;
      m_PSRecordingPointSet = mitk::PointSet::New();
      mitk::DataNode::Pointer dn = mitk::DataNode::New();
      dn->SetName(m_PointSetRecordingDataNodeName);
      dn->SetColor(0.,1.,0.);
      dn->SetData(m_PSRecordingPointSet);
      ds->Add(dn);
    }
    else
    {
      m_PSRecordingPointSet->Clear();
    }
    m_PointSetRecording = true;
  }

  else
  {
    m_PointSetRecording = false;
  }
}

void QmitkIGTTrackingLabView::DestroyInstrumentVisualization(mitk::DataStorage* ds, mitk::TrackingDevice::Pointer tracker)
{
  if(ds == NULL || tracker.IsNull())
    return;

  for(int i=0; i < tracker->GetToolCount(); ++i)
  {
    mitk::DataNode::Pointer dn = ds->GetNamedNode(tracker->GetTool(i)->GetToolName());

    if(dn.IsNotNull())
      ds->Remove(dn);
  }
}


void QmitkIGTTrackingLabView::GlobalReinit()
{
  // request global reiinit
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);

  // calculate bounding geometry of these nodes
  mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);

  //global reinit end
}

void QmitkIGTTrackingLabView::OnVirtualCamera(bool on)
{
if(on)
  {
  if (m_Controls.m_CameraViewSelection->GetSelectedToolID() == -1)
    {
    m_Controls.m_ActivateNeedleView->setChecked(false);
    QMessageBox::warning(NULL, "Error", "No tool selected for camera view!");
    return;
    }
  m_VirtualView = mitk::CameraVisualization::New();
  m_VirtualView->SetInput(m_Controls.m_CameraViewSelection->GetSelectedNavigationDataSource()->GetOutput(m_Controls.m_CameraViewSelection->GetSelectedToolID()));

  mitk::Vector3D viewDirection;
  viewDirection[0] = (int)(m_Controls.m_NeedleViewX->isChecked());
  viewDirection[1] = (int)(m_Controls.m_NeedleViewY->isChecked());
  viewDirection[2] = (int)(m_Controls.m_NeedleViewZ->isChecked());
  if (m_Controls.m_NeedleViewInvert->isChecked()) viewDirection *= -1;
  m_VirtualView->SetDirectionOfProjectionInToolCoordinates(viewDirection);

  mitk::Vector3D viewUpVector;
  viewUpVector[0] = (int)(m_Controls.m_NeedleUpX->isChecked());
  viewUpVector[1] = (int)(m_Controls.m_NeedleUpY->isChecked());
  viewUpVector[2] = (int)(m_Controls.m_NeedleUpZ->isChecked());
  if (m_Controls.m_NeedleUpInvert->isChecked()) viewUpVector *= -1;
  m_VirtualView->SetViewUpInToolCoordinates(viewUpVector);

  m_VirtualView->SetRenderer(this->GetActiveStdMultiWidget()->GetRenderWindow4()->GetRenderer());
  //next line: better code when this plugin is migrated to mitk::abstractview
  //m_VirtualView->SetRenderer(mitk::BaseRenderer::GetInstance(this->GetRenderWindowPart()->GetRenderWindow("3d")->GetRenderWindow()));
  m_CameraView = true;

  //make pointer itself invisible
  m_Controls.m_CameraViewSelection->GetSelectedNavigationTool()->GetDataNode()->SetBoolProperty("visible",false);

  //disable UI elements
  m_Controls.m_ViewDirectionBox->setEnabled(false);
  m_Controls.m_ViewUpBox->setEnabled(false);
  }
else
  {
  m_VirtualView = NULL;
  m_CameraView = false;
  m_Controls.m_CameraViewSelection->GetSelectedNavigationTool()->GetDataNode()->SetBoolProperty("visible",true);

  m_Controls.m_ViewDirectionBox->setEnabled(true);
  m_Controls.m_ViewUpBox->setEnabled(true);
  }

}

bool QmitkIGTTrackingLabView::CheckRegistrationInitialization()
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  mitk::PointSet::Pointer imageFiducials = dynamic_cast<mitk::PointSet*>(m_ImageFiducialsDataNode->GetData());
  mitk::PointSet::Pointer trackerFiducials = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsDataNode->GetData());

  if (m_Controls.m_ObjectComboBox->GetSelectedNode().IsNull())
  {
    QMessageBox::warning(NULL, "Registration not possible", "No object selected for registration.\nRegistration is not possible");
    return false;
  }

  if (imageFiducials.IsNull() || trackerFiducials.IsNull())
  {
    QMessageBox::warning(NULL, "Registration not possible", "Fiducial data objects not found. \n"
      "Please set 3 or more fiducials in the image and with the tracking system.\n\n"
      "Registration is not possible");
    return false;
  }

  unsigned int minFiducialCount = 3; // \Todo: move to view option
  if ((imageFiducials->GetSize() < minFiducialCount) || (trackerFiducials->GetSize() < minFiducialCount) || (imageFiducials->GetSize() != trackerFiducials->GetSize()))
  {
    QMessageBox::warning(NULL, "Registration not possible", QString("Not enough fiducial pairs found. At least %1 fiducial must "
      "exist for the image and the tracking system respectively.\n"
      "Currently, %2 fiducials exist for the image, %3 fiducials exist for the tracking system").arg(minFiducialCount).arg(imageFiducials->GetSize()).arg(trackerFiducials->GetSize()));
    return false;
  }

  return true;
}


void QmitkIGTTrackingLabView::OnPermanentRegistration(bool on)
{
  MITK_INFO << "Permanent registration" << on;
  if(on)
    {
    if(!CheckRegistrationInitialization())
    {
      m_Controls.m_UsePermanentRegistrationToggle->setChecked(false);
      return;
    }
    m_PermanentRegistrationFilter = mitk::NavigationDataObjectVisualizationFilter::New();

    //connect filter to source
    m_PermanentRegistrationFilter->SetInput(this->m_ObjectmarkerNavigationData);

    //TODO: add image when bug is fixed
    mitk::AffineTransform3D::Pointer initialTransform = this->m_Controls.m_ObjectComboBox->GetSelectedNode()->GetData()->GetGeometry()->GetIndexToWorldTransform();

    m_PermanentRegistrationFilter->SetRepresentationObject(0,this->m_Controls.m_ObjectComboBox->GetSelectedNode()->GetData());

    itk::Matrix<float,3,3> OffsetR = itk::Matrix<float,3,3>(this->m_ObjectmarkerNavigationData->GetOrientation().rotation_matrix_transpose());
    itk::Vector<float,3> OffsetT = itk::Vector<float,3>();
    OffsetT[0] = this->m_ObjectmarkerNavigationData->GetPosition()[0];
    OffsetT[1] = this->m_ObjectmarkerNavigationData->GetPosition()[1];
    OffsetT[2] = this->m_ObjectmarkerNavigationData->GetPosition()[2];

    MITK_INFO << "Offset R:" << OffsetR;
    MITK_INFO << "Orientation Quat:" << m_ObjectmarkerNavigationData->GetOrientation();

    OffsetR = OffsetR.GetInverse();
    OffsetT = (OffsetR * OffsetT) * -1.;

    mitk::AffineTransform3D::Pointer offset = mitk::AffineTransform3D::New();
    offset->SetMatrix(OffsetR);
    offset->SetOffset(OffsetT);

    mitk::AffineTransform3D::Pointer overall_transform = mitk::AffineTransform3D::New();
    overall_transform->SetIdentity();
    overall_transform->Compose(initialTransform);
    overall_transform->Compose(offset);

    m_PermanentRegistrationFilter->SetOffset(0,overall_transform);

    m_PermanentRegistration = true;
    }
  else
    {
    //stop permanent registration
    m_PermanentRegistration = false;

    //delete filter
    m_PermanentRegistrationFilter = NULL;
    }


}

mitk::PointSet::Pointer QmitkIGTTrackingLabView::GetVirtualPointSetFromPosition(mitk::NavigationData::Pointer navigationData)
{
  typedef itk::QuaternionRigidTransform<double> QuaternionTransformType;

  mitk::NavigationData::PositionType pointA;
  mitk::NavigationData::PositionType pointB;
  mitk::NavigationData::PositionType pointC;

  //initializing three points with position(0|0|0)
  pointA.Fill(0);
  pointB.Fill(0);
  pointC.Fill(0);

  // changing position off all points in order to make them orthogonal
  pointA[0] = 1;
  pointB[1] = 1;
  pointC[2] = 1;

  QuaternionTransformType::Pointer quatTransform = QuaternionTransformType::New();

  // orientation of NavigationData from parameter
  mitk::NavigationData::OrientationType quatIn = navigationData->GetOrientation();

  // set orientation to quaternion transform
  vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());
  quatTransform->SetRotation(vnlQuatIn);

  // transform each point
  pointA = quatTransform->TransformPoint(pointA);
  pointB = quatTransform->TransformPoint(pointB);
  pointC = quatTransform->TransformPoint(pointC);

  // add position data from NavigationData parameter to each point
  pointA[0] += navigationData->GetPosition()[0];
  pointA[1] += navigationData->GetPosition()[1];
  pointA[2] += navigationData->GetPosition()[2];

  pointB[0] += navigationData->GetPosition()[0];
  pointB[1] += navigationData->GetPosition()[1];
  pointB[2] += navigationData->GetPosition()[2];

  pointC[0] += navigationData->GetPosition()[0];
  pointC[1] += navigationData->GetPosition()[1];
  pointC[2] += navigationData->GetPosition()[2];

  // insert points in source points pointset for the permanent registration landmark transform
  m_PermanentRegistrationSourcePoints->InsertPoint(0,pointA);
  m_PermanentRegistrationSourcePoints->InsertPoint(1,pointB);
  m_PermanentRegistrationSourcePoints->InsertPoint(2,pointC);


  return m_PermanentRegistrationSourcePoints;
}

