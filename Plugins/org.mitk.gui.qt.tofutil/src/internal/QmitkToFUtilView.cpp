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
#include <mitkIRenderingManager.h>
#include <mitkIRenderWindowPart.h>
#include <mitkILinkedRenderWindowPart.h>

// Qmitk
#include "QmitkToFUtilView.h"
#include <QmitkStdMultiWidget.h>

// Qt
#include <QMessageBox>
#include <QString>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

// MITK
#include <mitkBaseRenderer.h>
#include <mitkGlobalInteraction.h>
#include <mitkToFDistanceImageToPointSetFilter.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkToFDeviceFactoryManager.h>
#include <mitkToFCameraDevice.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkSmartPointerProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkVtkScalarModeProperty.h>

// VTK
#include <vtkCamera.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// ITK
#include <itkCommand.h>
#include <itksys/SystemTools.hxx>

const std::string QmitkToFUtilView::VIEW_ID = "org.mitk.views.tofutil";

//Constructor
QmitkToFUtilView::QmitkToFUtilView()
  : QmitkAbstractView()
  , m_Controls(NULL), m_MultiWidget( NULL )
  , m_MitkDistanceImage(NULL), m_MitkAmplitudeImage(NULL), m_MitkIntensityImage(NULL), m_Surface(NULL)
  , m_DistanceImageNode(NULL), m_AmplitudeImageNode(NULL), m_IntensityImageNode(NULL), m_RGBImageNode(NULL), m_SurfaceNode(NULL)
  , m_ToFImageRecorder(NULL), m_ToFImageGrabber(NULL), m_ToFDistanceImageToSurfaceFilter(NULL), m_ToFCompositeFilter(NULL)
  , m_2DDisplayCount(0)
  , m_RealTimeClock(NULL)
  , m_StepsForFramerate(100)
  , m_2DTimeBefore(0.0)
  , m_2DTimeAfter(0.0)
  , m_CameraIntrinsics(NULL)
{
  this->m_Frametimer = new QTimer(this);

  this->m_ToFDistanceImageToSurfaceFilter = mitk::ToFDistanceImageToSurfaceFilter::New();
  this->m_ToFCompositeFilter = mitk::ToFCompositeFilter::New();
  this->m_ToFImageRecorder = mitk::ToFImageRecorder::New();
}

//Destructor, specifically calling OnToFCameraStopped() and OnToFCammeraDiconnected()
QmitkToFUtilView::~QmitkToFUtilView()
{
  OnToFCameraStopped();
  OnToFCameraDisconnected();
}

//Createing the PartControl Signal-Slot principal
void QmitkToFUtilView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkToFUtilViewControls;
    m_Controls->setupUi( parent );

    //Looking for Input and Defining reaction
    connect(m_Frametimer, SIGNAL(timeout()), this, SLOT(OnUpdateCamera()));

    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(KinectAcquisitionModeChanged()), this, SLOT(OnKinectAcquisitionModeChanged()) ); // Todo in Widget2
    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(ToFCameraConnected()), this, SLOT(OnToFCameraConnected()) );
    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(ToFCameraDisconnected()), this, SLOT(OnToFCameraDisconnected()) );
    connect( (QObject*)(m_Controls->m_ToFRecorderWidget), SIGNAL(ToFCameraStarted()), this, SLOT(OnToFCameraStarted()) );
    connect( (QObject*)(m_Controls->m_ToFRecorderWidget), SIGNAL(ToFCameraStopped()), this, SLOT(OnToFCameraStopped()) );
    connect( (QObject*)(m_Controls->m_ToFRecorderWidget), SIGNAL(RecordingStarted()), this, SLOT(OnToFCameraStopped()) );
    connect( (QObject*)(m_Controls->m_ToFRecorderWidget), SIGNAL(RecordingStopped()), this, SLOT(OnToFCameraStarted()) );
}
}

//SetFocus-Method -> actually seting Focus to the Recorder
void QmitkToFUtilView::SetFocus()
{
  m_Controls->m_ToFRecorderWidget->setFocus();
}

//Activated-Method->Generating RenderWindow
void QmitkToFUtilView::Activated()
{
  //get the current RenderWindowPart or open a new one if there is none
  if(this->GetRenderWindowPart(OPEN))
  {
    mitk::ILinkedRenderWindowPart* linkedRenderWindowPart = dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart());
    if(linkedRenderWindowPart == 0)
    {
      MITK_ERROR << "No linked StdMultiWidget avaiable!!!";
    }
    else
    {
      linkedRenderWindowPart->EnableSlicingPlanes(false);
    }
    GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
    GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SliceLockedOn();
    GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
    GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController()->SliceLockedOn();
    GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
    GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetSliceNavigationController()->SliceLockedOn();

    this->GetRenderWindowPart()->GetRenderingManager()->InitializeViews();

    this->UseToFVisibilitySettings(true);

    if (this->m_ToFCompositeFilter)
    {
      m_Controls->m_ToFCompositeFilterWidget->SetToFCompositeFilter(this->m_ToFCompositeFilter);
    }
    if (this->GetDataStorage())
    {
      m_Controls->m_ToFCompositeFilterWidget->SetDataStorage(this->GetDataStorage());
    }

    if (this->m_ToFImageGrabber.IsNull())
    {
      m_Controls->m_ToFRecorderWidget->setEnabled(false);
      m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(false);
      m_Controls->m_ToFCompositeFilterWidget->setEnabled(false);
      m_Controls->m_ToFMeasurementWidget->setEnabled(false);
      m_Controls->m_ToFSurfaceGenerationWidget->setEnabled(false);
    }
  }
}

//ZomnnieView-Method -> Resetting GUI to default. Why not just QmitkToFUtilView()?!
void QmitkToFUtilView::ActivatedZombieView(berry::IWorkbenchPartReference::Pointer /*zombieView*/)
{
  ResetGUIToDefault();
}

void QmitkToFUtilView::Deactivated()
{
}

void QmitkToFUtilView::Visible()
{
}

//Reset of the ToFUtilView
void QmitkToFUtilView::Hidden()
{
  ResetGUIToDefault();
}

void QmitkToFUtilView::OnToFCameraConnected()
{
  MITK_DEBUG <<"OnToFCameraConnected";
  this->m_2DDisplayCount = 0;

  this->m_ToFImageGrabber = m_Controls->m_ToFConnectionWidget->GetToFImageGrabber();

  // initialize surface generation
  this->m_ToFDistanceImageToSurfaceFilter = mitk::ToFDistanceImageToSurfaceFilter::New();

  // initialize ToFImageRecorder and ToFRecorderWidget
  this->m_ToFImageRecorder = mitk::ToFImageRecorder::New();
  this->m_ToFImageRecorder->SetCameraDevice(this->m_ToFImageGrabber->GetCameraDevice());
  m_Controls->m_ToFRecorderWidget->SetParameter(this->m_ToFImageGrabber, this->m_ToFImageRecorder);
  m_Controls->m_ToFRecorderWidget->setEnabled(true);
  m_Controls->m_ToFRecorderWidget->ResetGUIToInitial();
  m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(false);

  // initialize ToFCompositeFilterWidget
  this->m_ToFCompositeFilter = mitk::ToFCompositeFilter::New();
  if (this->m_ToFCompositeFilter)
  {
    m_Controls->m_ToFCompositeFilterWidget->SetToFCompositeFilter(this->m_ToFCompositeFilter);
  }
  if (this->GetDataStorage())
  {
    m_Controls->m_ToFCompositeFilterWidget->SetDataStorage(this->GetDataStorage());
  }

  if ( this->GetRenderWindowPart() )
    // initialize measurement widget
    m_Controls->m_ToFMeasurementWidget->InitializeWidget(this->GetRenderWindowPart()->GetQmitkRenderWindows(),this->GetDataStorage(), this->m_ToFDistanceImageToSurfaceFilter->GetCameraIntrinsics());
  else
    MITK_WARN << "No StdMultiWidget available!!! MeasurementWidget will not work.";

  this->m_RealTimeClock = mitk::RealTimeClock::New();
  this->m_2DTimeBefore = this->m_RealTimeClock->GetCurrentStamp();

  this->RequestRenderWindowUpdate();
}

void QmitkToFUtilView::ResetGUIToDefault()
{
  if(this->GetRenderWindowPart())
  {
    mitk::ILinkedRenderWindowPart* linkedRenderWindowPart = dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart());
    if(linkedRenderWindowPart == 0)
    {
      MITK_ERROR << "No linked StdMultiWidget avaiable!!!";
    }
    else
    {
      linkedRenderWindowPart->EnableSlicingPlanes(true);
    }
    GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
    GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SliceLockedOff();
    GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
    GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController()->SliceLockedOff();
    GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
    GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetSliceNavigationController()->SliceLockedOff();

    this->UseToFVisibilitySettings(false);

    //global reinit
    this->GetRenderWindowPart()->GetRenderingManager()->InitializeViews();
    this->RequestRenderWindowUpdate();
  }
}

void QmitkToFUtilView::OnToFCameraDisconnected()
{
  this->GetDataStorage()->Remove(m_DistanceImageNode);
  if(m_RGBImageNode)
    this->GetDataStorage()->Remove(m_RGBImageNode);
  if(m_AmplitudeImageNode)
    this->GetDataStorage()->Remove(m_AmplitudeImageNode);
  if(m_IntensityImageNode)
    this->GetDataStorage()->Remove(m_IntensityImageNode);
  if(m_SurfaceNode)
    this->GetDataStorage()->Remove(m_SurfaceNode);

  m_Controls->m_ToFRecorderWidget->OnStop();
  m_Controls->m_ToFRecorderWidget->setEnabled(false);
  m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(false);
  m_Controls->m_ToFMeasurementWidget->setEnabled(false);
  m_Controls->m_ToFSurfaceGenerationWidget->setEnabled(false);
  //clean up measurement widget
  m_Controls->m_ToFMeasurementWidget->CleanUpWidget();
}

void QmitkToFUtilView::OnKinectAcquisitionModeChanged()
{
  if (m_ToFCompositeFilter.IsNotNull()&&m_ToFImageGrabber.IsNotNull())
  {
    if (m_SelectedCamera.contains("Kinect"))
    {
      if (m_ToFImageGrabber->GetBoolProperty("RGB"))
      {
        this->m_RGBImageNode = ReplaceNodeData("RGB image",this->m_ToFImageGrabber->GetOutput(3));
        this->m_ToFDistanceImageToSurfaceFilter->SetInput(3,this->m_ToFImageGrabber->GetOutput(3));
      }
      else if (m_ToFImageGrabber->GetBoolProperty("IR"))
      {
        this->m_MitkAmplitudeImage = m_ToFCompositeFilter->GetOutput(1);
        this->m_AmplitudeImageNode = ReplaceNodeData("Amplitude image",m_MitkAmplitudeImage);
      }
    }
    this->UseToFVisibilitySettings(true);
  }
}

void QmitkToFUtilView::OnToFCameraStarted()
{
  if (m_ToFImageGrabber.IsNotNull())
  {
    // initialize camera intrinsics
    if (this->m_ToFImageGrabber->GetProperty("CameraIntrinsics"))
    {
      m_CameraIntrinsics = dynamic_cast<mitk::CameraIntrinsicsProperty*>(this->m_ToFImageGrabber->GetProperty("CameraIntrinsics"))->GetValue();
      MITK_INFO << m_CameraIntrinsics->ToString();
    }
    else
    {
      m_CameraIntrinsics = NULL;
      MITK_ERROR << "No camera intrinsics were found!";
    }

    // set camera intrinsics
    if ( m_CameraIntrinsics.IsNotNull() )
    {
      this->m_ToFDistanceImageToSurfaceFilter->SetCameraIntrinsics(m_CameraIntrinsics);
    }

    // initial update of image grabber
    this->m_ToFImageGrabber->Update();

    bool hasRGBImage = false;
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasRGBImage",hasRGBImage);

    bool hasIntensityImage = false;
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasIntensityImage",hasIntensityImage);

    bool hasAmplitudeImage = false;
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasAmplitudeImage",hasAmplitudeImage);

    this->m_ToFCompositeFilter->SetInput(0,this->m_ToFImageGrabber->GetOutput(0));
    if(hasAmplitudeImage)
      this->m_ToFCompositeFilter->SetInput(1,this->m_ToFImageGrabber->GetOutput(1));
    if(hasIntensityImage)
      this->m_ToFCompositeFilter->SetInput(2,this->m_ToFImageGrabber->GetOutput(2));

    // initial update of composite filter
    this->m_ToFCompositeFilter->Update();
    this->m_MitkDistanceImage = m_ToFCompositeFilter->GetOutput();
    this->m_DistanceImageNode = ReplaceNodeData("Distance image",m_MitkDistanceImage);

    std::string rgbFileName;
    m_ToFImageGrabber->GetCameraDevice()->GetStringProperty("RGBImageFileName",rgbFileName);

    if(hasRGBImage || (rgbFileName!=""))
    {
      if(m_ToFImageGrabber->GetBoolProperty("IR"))
      {
        this->m_MitkAmplitudeImage = m_ToFCompositeFilter->GetOutput(1);
      }
      else
      {
        this->m_RGBImageNode = ReplaceNodeData("RGB image",this->m_ToFImageGrabber->GetOutput(3));
      }
    }
    else
    {
      this->m_RGBImageNode = NULL;
    }

    if(hasAmplitudeImage)
    {
      this->m_MitkAmplitudeImage = m_ToFCompositeFilter->GetOutput(1);
      this->m_AmplitudeImageNode = ReplaceNodeData("Amplitude image",m_MitkAmplitudeImage);
    }

    if(hasIntensityImage)
    {
      this->m_MitkIntensityImage = m_ToFCompositeFilter->GetOutput(2);
      this->m_IntensityImageNode = ReplaceNodeData("Intensity image",m_MitkIntensityImage);
    }

    this->m_ToFDistanceImageToSurfaceFilter->SetInput(0,m_MitkDistanceImage);
    this->m_ToFDistanceImageToSurfaceFilter->SetInput(1,m_MitkAmplitudeImage);
    this->m_ToFDistanceImageToSurfaceFilter->SetInput(2,m_MitkIntensityImage);

    this->UseToFVisibilitySettings(true);

    this->m_SurfaceNode = ReplaceNodeData("Surface", NULL);
    m_Controls->m_ToFCompositeFilterWidget->UpdateFilterParameter();
    // initialize visualization widget
    m_Controls->m_ToFVisualisationSettingsWidget->Initialize(this->m_DistanceImageNode,
                                                             this->m_AmplitudeImageNode,
                                                             this->m_IntensityImageNode,
                                                             this->m_SurfaceNode);
    m_Controls->m_ToFSurfaceGenerationWidget->Initialize(m_ToFDistanceImageToSurfaceFilter,
                                                         m_ToFImageGrabber,
                                                         m_CameraIntrinsics,
                                                         m_SurfaceNode,
                                                         GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderer()->GetVtkRenderer()->GetActiveCamera());
    // set distance image to measurement widget
    m_Controls->m_ToFMeasurementWidget->SetDistanceImage(m_MitkDistanceImage);

    this->m_Frametimer->start(0);

    m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(true);
    m_Controls->m_ToFCompositeFilterWidget->setEnabled(true);
    m_Controls->m_ToFMeasurementWidget->setEnabled(true);
    m_Controls->m_ToFSurfaceGenerationWidget->setEnabled(true);
  }
}

void QmitkToFUtilView::OnToFCameraStopped()
{
  m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(false);
  m_Controls->m_ToFCompositeFilterWidget->setEnabled(false);

  this->m_Frametimer->stop();
}

void QmitkToFUtilView::OnUpdateCamera()
{
  if(!m_Controls->m_ToFSurfaceGenerationWidget->UpdateSurface())
  {
    // update pipeline
    this->m_MitkDistanceImage->Update();
  }

  this->RequestRenderWindowUpdate();

  this->m_2DDisplayCount++;
  if ((this->m_2DDisplayCount % this->m_StepsForFramerate) == 0)
  {
    this->m_2DTimeAfter = this->m_RealTimeClock->GetCurrentStamp() - this->m_2DTimeBefore;
    MITK_INFO << " 2D-Display-framerate (fps): " << this->m_StepsForFramerate / (this->m_2DTimeAfter/1000);
    this->m_2DTimeBefore = this->m_RealTimeClock->GetCurrentStamp();
  }
}

void QmitkToFUtilView::OnChangeCoronalWindowOutput(int index)
{
  this->OnToFCameraStopped();
  if(index == 0)
  {
    if(this->m_IntensityImageNode.IsNotNull())
      this->m_IntensityImageNode->SetVisibility(false);
    if(this->m_RGBImageNode.IsNotNull())
      this->m_RGBImageNode->SetVisibility(true);
  }
  else if(index == 1)
  {
    if(this->m_IntensityImageNode.IsNotNull())
      this->m_IntensityImageNode->SetVisibility(true);
    if(this->m_RGBImageNode.IsNotNull())
      this->m_RGBImageNode->SetVisibility(false);
  }
  this->RequestRenderWindowUpdate();
  this->OnToFCameraStarted();
}

mitk::DataNode::Pointer QmitkToFUtilView::ReplaceNodeData( std::string nodeName, mitk::BaseData* data )
{
  mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode(nodeName);
  if (node.IsNull())
  {
    node = mitk::DataNode::New();
    node->SetData(data);
    node->SetName(nodeName);
    node->SetBoolProperty("binary",false);
    this->GetDataStorage()->Add(node);
  }
  else
  {
    node->SetData(data);
  }
  return node;
}

void QmitkToFUtilView::UseToFVisibilitySettings(bool useToF)
{
  //We need this property for every node.
  mitk::RenderingModeProperty::Pointer renderingModePropertyForTransferFunction = mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);

  // set node properties
  if (m_DistanceImageNode.IsNotNull())
  {
    this->m_DistanceImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    this->m_DistanceImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderWindow() ) );
    this->m_DistanceImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetRenderWindow() ) );
    this->m_DistanceImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() ) );
    this->m_DistanceImageNode->SetProperty("Image Rendering.Mode", renderingModePropertyForTransferFunction);
  }
  if (m_AmplitudeImageNode.IsNotNull())
  {
    this->m_AmplitudeImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderWindow() ) );
    this->m_AmplitudeImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetRenderWindow() ) );
    this->m_AmplitudeImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() ) );
    this->m_AmplitudeImageNode->SetProperty("Image Rendering.Mode", renderingModePropertyForTransferFunction);
  }
  if (m_IntensityImageNode.IsNotNull())
  {
      this->m_IntensityImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
      this->m_IntensityImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderWindow() ) );
      this->m_IntensityImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderWindow() ) );
      this->m_IntensityImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() ) );
      this->m_IntensityImageNode->SetProperty("Image Rendering.Mode", renderingModePropertyForTransferFunction);
  }
  if ((m_RGBImageNode.IsNotNull()))
  {
      this->m_RGBImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
      this->m_RGBImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderWindow() ) );
      this->m_RGBImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderWindow() ) );
      this->m_RGBImageNode->SetVisibility( !useToF, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() ) );
  }
  // initialize images
  if (m_MitkDistanceImage.IsNotNull())
  {
    this->GetRenderWindowPart()->GetRenderingManager()->InitializeViews(
          this->m_MitkDistanceImage->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS, true);
  }
  if(this->m_SurfaceNode.IsNotNull())
  {
    QHash<QString, QmitkRenderWindow*> renderWindowHashMap = this->GetRenderWindowPart()->GetQmitkRenderWindows();
    QHashIterator<QString, QmitkRenderWindow*> i(renderWindowHashMap);
    while (i.hasNext()){
      i.next();
      this->m_SurfaceNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(i.value()->GetRenderWindow()) );
    }
    this->m_SurfaceNode->SetVisibility( true, mitk::BaseRenderer::GetInstance(GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderWindow() ) );
  }
  //disable/enable gradient background
  this->GetRenderWindowPart()->EnableDecorations(!useToF, QStringList(QString("background")));

  if((this->m_RGBImageNode.IsNotNull()))
  {
    bool RGBImageHasDifferentResolution = false;
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("RGBImageHasDifferentResolution",RGBImageHasDifferentResolution);
    if(RGBImageHasDifferentResolution)
    {
      //update the display geometry by using the RBG image node. Only for renderwindow coronal
      mitk::RenderingManager::GetInstance()->InitializeView( GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetRenderWindow(), this->m_RGBImageNode->GetData()->GetGeometry() );
    }
  }
}
