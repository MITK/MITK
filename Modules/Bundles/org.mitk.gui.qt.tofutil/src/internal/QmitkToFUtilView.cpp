/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Qmitk
#include "QmitkToFUtilView.h"
#include <QmitkStdMultiWidget.h>

// Qt
#include <QMessageBox>
#include <QString>

// MITK
#include <mitkBaseRenderer.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkToFCompositeFilter.h>
#include <mitkToFVisualizationFilter.h>
#include <mitkVtkRepresentationProperty.h>

// VTK
#include <vtkCamera.h>

const std::string QmitkToFUtilView::VIEW_ID = "org.mitk.views.tofutil";

QmitkToFUtilView::QmitkToFUtilView()
: QmitkFunctionality()
, m_Controls(NULL)
, m_MultiWidget( NULL )
, m_DistanceImageNode(NULL)
, m_AmplitudeImageNode(NULL)
, m_IntensityImageNode(NULL)
, m_SurfaceNode(NULL)
, m_QmitkVideoBackground(NULL)
, m_QmitkToFImageBackground1(NULL)
, m_QmitkToFImageBackground2(NULL)
, m_QmitkToFImageBackground3(NULL)
{
  //this->m_FrameRate = 0; 
  this->m_Frametimer = new QTimer(this);

  this->m_MitkDistanceImage = mitk::Image::New();
  this->m_MitkAmplitudeImage = mitk::Image::New();
  this->m_MitkIntensityImage = mitk::Image::New();
  this->m_MitkAllImage = mitk::Image::New();
  this->m_MitkRawImage = mitk::Image::New();
  this->m_SurfaceInputImage = mitk::Image::New();
  this->m_SurfaceTextureImage = mitk::Image::New();
  this->m_SurfaceIntensityImage = mitk::Image::New();
  this->m_Surface = mitk::Surface::New();

//  this->m_ToFSurfaceGenerationFilter = mitk::ToFSurfaceGenerationFilter::New();
  this->m_ToFDistanceImageToSurfaceFilter = mitk::ToFDistanceImageToSurfaceFilter::New();


  this->m_ToFImageGrabber = NULL;
  this->m_ToFImageRecorder = mitk::ToFImageRecorder::New();

  this->m_DataNodesInitilized = false;
  this->m_TransferFunctionInitialized = false;
  this->m_SurfaceInitialized = false;
  this->m_ImageSequence = 0;  

  //this->m_DistLut = vtkLookupTable::New();
  //this->m_AmplLut = vtkLookupTable::New();
  //this->m_IntenLut = vtkLookupTable::New();
  //PrepareColorLut(this->m_DistLut, 300.0, 1000.0);
  //PrepareBinaryLut(this->m_AmplLut, 0.0, 100.0);
  //PrepareBinaryLut(this->m_IntenLut, 0.0, 100.0);

  this->m_DataBuffer = NULL;
  this->m_DataBufferCurrentIndex = 0;
  this->m_DataBufferMaxSize = 0;
  this->m_IndexBuffer = NULL;
  this->m_VideoEnabled = false;

  this->m_ToFSurfaceVtkMapper3D = mitk::ToFSurfaceVtkMapper3D::New();

  this->m_ToFCompositeFilter = mitk::ToFCompositeFilter::New();
  this->m_ToFVisualizationFilter = mitk::ToFVisualizationFilter::New();
}

QmitkToFUtilView::~QmitkToFUtilView()
{
  // remove nodes
  RemoveNode("ToF_Distance", this->m_DistanceImageNode);
  RemoveNode("ToF_Amplitude", this->m_AmplitudeImageNode);
  RemoveNode("ToF_Intensity", this->m_IntensityImageNode);
  RemoveNode("ToF_Surface", this->m_SurfaceNode);
  //RemoveBackground();
}

void QmitkToFUtilView::RemoveNode(const char* nodename, mitk::DataNode::Pointer node)
{
  if(this->GetDataStorage()->GetNamedNode(nodename) != NULL)
  {
    this->GetDataStorage()->Remove(node);
  }
}

void QmitkToFUtilView::CreateNode(const char* nodename, mitk::DataNode::Pointer& node)
{
  node = mitk::DataNode::New();
  node->SetProperty( "name", mitk::StringProperty::New(nodename));
}


void QmitkToFUtilView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkToFUtilViewControls;
    m_Controls->setupUi( parent );

    connect(m_Frametimer, SIGNAL(timeout()), this, SLOT(OnUpdateCamera()));
    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(ToFCameraConnected()), this, SLOT(OnToFCameraConnected()) );
    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(ToFCameraDisconnected()), this, SLOT(OnToFCameraDisconnected()) );
    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(ToFCameraStop()), this, SLOT(OnToFCameraStop()) );
    connect( (QObject*)(m_Controls->m_ToFConnectionWidget), SIGNAL(ToFCameraSelected(const QString)), this, SLOT(OnToFCameraSelected(const QString)) );
    connect( (QObject*)(m_Controls->m_ToFRecorderWidget), SIGNAL(ToFCameraStarted()), this, SLOT(OnToFCameraStarted()) );
    connect( (QObject*)(m_Controls->m_ToFRecorderWidget), SIGNAL(ToFCameraStopped()), this, SLOT(OnToFCameraStopped()) );
    connect( (QObject*)(m_Controls->m_SurfaceCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnSurfaceCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_TextureCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnTextureCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_VideoTextureCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnVideoTextureCheckBoxChecked(bool)) );
  }
}


void QmitkToFUtilView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkToFUtilView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkToFUtilView::Activated()
{
  QmitkFunctionality::Activated();
  // add necessary nodes
  CreateNode("ToF_Distance", this->m_DistanceImageNode);
  CreateNode("ToF_Amplitude", this->m_AmplitudeImageNode);
  CreateNode("ToF_Intensity", this->m_IntensityImageNode);
  CreateNode("ToF_Surface", this->m_SurfaceNode);
  // configure views
  m_MultiWidget->SetWidgetPlanesVisibility(false);
  m_MultiWidget->mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Transversal);
  m_MultiWidget->mitkWidget1->GetSliceNavigationController()->SliceLockedOn();
  m_MultiWidget->mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Transversal);
  m_MultiWidget->mitkWidget2->GetSliceNavigationController()->SliceLockedOn();
  m_MultiWidget->mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Transversal);
  m_MultiWidget->mitkWidget3->GetSliceNavigationController()->SliceLockedOn();

  //m_Controls->m_ToFImageConverterWidget->Initialize(this->GetDefaultDataStorage(), m_MultiWidget);
  m_Controls->m_ToFVisualisationSettingsWidget->Initialize(this->GetDefaultDataStorage(), m_MultiWidget);
  m_Controls->m_ToFVisualisationSettingsWidget->SetParameter(this->m_ToFVisualizationFilter);

  m_Controls->m_ToFCompositeFilterWidget->SetToFCompositeFilter(this->m_ToFCompositeFilter);

  if (this->m_ToFImageGrabber == NULL)
  {
    m_Controls->m_ToFRecorderWidget->setEnabled(false);
    m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(false);
  }
}

void QmitkToFUtilView::Deactivated()
{
  m_MultiWidget->SetWidgetPlanesVisibility(true);
  m_MultiWidget->mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Transversal);
  m_MultiWidget->mitkWidget1->GetSliceNavigationController()->SliceLockedOff();
  m_MultiWidget->mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
  m_MultiWidget->mitkWidget2->GetSliceNavigationController()->SliceLockedOff();
  m_MultiWidget->mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
  m_MultiWidget->mitkWidget3->GetSliceNavigationController()->SliceLockedOff();

  mitk::RenderingManager::GetInstance()->InitializeViews();
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  RemoveBackground();
  QmitkFunctionality::Deactivated();
}

void QmitkToFUtilView::OnToFCameraConnected()
{
  RemoveNode("ToF_Distance", this->m_DistanceImageNode);
  RemoveNode("ToF_Amplitude", this->m_AmplitudeImageNode);
  RemoveNode("ToF_Intensity", this->m_IntensityImageNode);
  RemoveNode("ToF_Surface", this->m_SurfaceNode);

  CreateNode("ToF_Distance", this->m_DistanceImageNode);
  CreateNode("ToF_Amplitude", this->m_AmplitudeImageNode);
  CreateNode("ToF_Intensity", this->m_IntensityImageNode);
  CreateNode("ToF_Surface", this->m_SurfaceNode);

  this->m_DistanceImageNode->SetData( NULL );
  this->m_AmplitudeImageNode->SetData( NULL );
  this->m_IntensityImageNode->SetData( NULL );
  this->m_SurfaceNode->SetData( NULL );

  this->m_DataNodesInitilized = false;
  this->m_TransferFunctionInitialized = false;
  this->m_SurfaceInitialized = false;
  this->m_ImageSequence = 0;

  //this->m_ToFImageGrabber = dynamic_cast<mitk::ToFImageGrabber*>(m_Controls->m_ToFConnectionWidget->GetToFImageGrabber());
  this->m_ToFImageGrabber = m_Controls->m_ToFConnectionWidget->GetToFImageGrabber();
  this->m_ToFCaptureWidth = this->m_ToFImageGrabber->GetCaptureWidth();
  this->m_ToFCaptureHeight = this->m_ToFImageGrabber->GetCaptureHeight();

  this->m_ToFImageRecorder->SetCameraDevice(this->m_ToFImageGrabber->GetCameraDevice());

  //this->m_MitkDistanceImage->ReleaseData();
  //this->m_MitkAmplitudeImage->ReleaseData();
  //this->m_MitkIntensityImage->ReleaseData();
  //this->m_MitkAllImage->ReleaseData();
  this->m_MitkDistanceImage = mitk::Image::New();
  this->m_MitkAmplitudeImage = mitk::Image::New();
  this->m_MitkIntensityImage = mitk::Image::New();
  this->m_MitkAllImage = mitk::Image::New();

  InitImage(this->m_MitkDistanceImage, 1);
  InitImage(this->m_MitkAmplitudeImage, 1);
  InitImage(this->m_MitkIntensityImage, 1);
  InitImage(this->m_MitkAllImage, 3);

  InitImage(this->m_SurfaceInputImage, 1);
  InitImage(this->m_SurfaceIntensityImage, 1);

  this->m_SurfaceDisplayCount = 0;
  this->m_2DDisplayCount = 0;

  this->m_IplDistanceImage = cvCreateImageHeader(cvSize(this->m_ToFCaptureWidth, this->m_ToFCaptureHeight), IPL_DEPTH_32F, 1); // no allocation
  this->m_IplIntensityImage = cvCreateImageHeader(cvSize(this->m_ToFCaptureWidth, this->m_ToFCaptureHeight), IPL_DEPTH_32F, 1); // no allocation
  this->m_OutputIplImage = cvCreateImage(cvSize(this->m_ToFCaptureWidth, this->m_ToFCaptureHeight), IPL_DEPTH_32F, 1);

  this->m_ItkInputImage = ItkImageType2D::New();
  ItkImageType2D::IndexType startIndex;
  startIndex[0] =   0;  // first index on X
  startIndex[1] =   0;  // first index on Y
  ItkImageType2D::SizeType  size;
  size[0]  = this->m_ToFCaptureWidth;  // size along X
  size[1]  = this->m_ToFCaptureHeight;  // size along Y
  ItkImageType2D::RegionType region;
  region.SetSize( size );
  region.SetIndex( startIndex );
  this->m_ItkInputImage->SetRegions( region );
  this->m_ItkInputImage->Allocate();

  if ( this->m_SurfaceNode.IsNotNull() )
  {
    this->m_SurfaceNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    this->m_SurfaceNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget1->GetRenderWindow() ) );
    this->m_SurfaceNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget2->GetRenderWindow() ) );
    this->m_SurfaceNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget3->GetRenderWindow() ) );
    this->m_SurfaceNode->SetProperty( "inRenWin4" , mitk::BoolProperty::New( true ));
    this->m_SurfaceNode->SetProperty( "material.representation", mitk::VtkRepresentationProperty::New("Surface"));
    this->m_SurfaceNode->SetIntProperty("layer", 10);
    this->m_SurfaceNode->SetBoolProperty("scalar visibility", false);
    this->m_SurfaceNode->SetFloatProperty("ScalarsRangeMaximum", 20000);
    this->m_SurfaceNode->SetFloatProperty("color_coefficient", 0);
    this->m_SurfaceNode->SetBoolProperty("color mode", true);

    this->m_SurfaceNode->SetData(mitk::Surface::New());
    this->m_SurfaceNode->SetMapper(mitk::BaseRenderer::Standard3D, this->m_ToFSurfaceVtkMapper3D);
    this->GetDefaultDataStorage()->Add( this->m_SurfaceNode );
  }
  if(this->GetDataStorage()->GetNamedNode("ToF_Distance") == NULL)
  {
    this->m_DistanceImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    this->m_DistanceImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget1->GetRenderWindow() ) );
    this->m_DistanceImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget3->GetRenderWindow() ) );
    this->m_DistanceImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow() ) );
    this->m_DistanceImageNode->SetProperty( "inRenWin2" , mitk::BoolProperty::New( true ));
    //m_DistanceImageNode->SetProperty( "use color", mitk::BoolProperty::New( false ));
    this->m_DistanceImageNode->SetProperty( "use color", mitk::BoolProperty::New( true ));
    this->m_DistanceImageNode->SetProperty( "binary", mitk::BoolProperty::New( false ));
    //m_DistanceImageNode->SetIntProperty( "layer", 10);
    //this->m_DistanceImageNode->SetData( this->m_MitkDistanceImage );

    mitk::Image::Pointer myImage = mitk::Image::New();
    unsigned int dimensions[2];
    dimensions[0] = this->m_ToFImageGrabber->GetCaptureWidth();
    dimensions[1] = this->m_ToFImageGrabber->GetCaptureHeight();       
    myImage->Initialize(mitk::PixelType(mitkIpPicUInt, 24, 3), 2, dimensions); //unsigned char RGB

    this->m_DistanceImageNode->SetData( myImage );
    this->GetDataStorage()->Add( this->m_DistanceImageNode );
  }

  m_Controls->m_ToFRecorderWidget->SetParameter(dynamic_cast<mitk::ToFImageGrabber*>(this->m_ToFImageGrabber), this->m_ToFImageRecorder);


  //TODO
  this->m_RealTimeClock = mitk::RealTimeClock::New();
  this->m_StepsForFramerate = 100; 
  this->m_TimeBefore = this->m_RealTimeClock->GetCurrentStamp();
  this->m_2DTimeBefore = this->m_RealTimeClock->GetCurrentStamp();
  this->m_SurfaceTimeBefore = this->m_RealTimeClock->GetCurrentStamp();

  try
  {
    this->m_VideoSource = mitk::OpenCVVideoSource::New();

    this->m_VideoSource->SetVideoCameraInput(0, false);
    this->m_VideoSource->StartCapturing();
    if(!this->m_VideoSource->IsCapturingEnabled())
    {
      MITK_INFO << "unable to initialize video grabbing/playback";
      this->m_VideoEnabled = false;
      m_Controls->m_VideoTextureCheckBox->setEnabled(false);
    }
    else
    {
      this->m_VideoEnabled = true;
      m_Controls->m_VideoTextureCheckBox->setEnabled(true);
    }

    if (this->m_VideoEnabled)
    {

      this->m_VideoSource->FetchFrame();
      this->m_VideoCaptureHeight = this->m_VideoSource->GetImageHeight();
      this->m_VideoCaptureWidth = this->m_VideoSource->GetImageWidth();
      int videoTexSize = this->m_VideoCaptureWidth * this->m_VideoCaptureHeight * 3; // for each pixel three values for rgb are needed!!
      this->m_VideoTexture = this->m_VideoSource->GetVideoTexture();

      unsigned int dimensions[2];
      dimensions[0] = this->m_VideoCaptureWidth;
      dimensions[1] = this->m_VideoCaptureHeight;       
      this->m_SurfaceTextureImage->Initialize(mitk::PixelType(typeid(unsigned char), 3), 2, dimensions, 1);

      //this->m_SurfaceWorkerThread.SetTextureImageWidth(this->m_VideoCaptureWidth);
      //this->m_SurfaceWorkerThread.SetTextureImageHeight(this->m_VideoCaptureHeight);

      this->m_ToFDistanceImageToSurfaceFilter->SetTextureImageWidth(this->m_VideoCaptureWidth);
      this->m_ToFDistanceImageToSurfaceFilter->SetTextureImageHeight(this->m_VideoCaptureHeight);


      this->m_ToFSurfaceVtkMapper3D->SetTextureWidth(this->m_VideoCaptureWidth);
      this->m_ToFSurfaceVtkMapper3D->SetTextureHeight(this->m_VideoCaptureHeight);
    }
    m_MultiWidget->DisableGradientBackground();

    this->AddBackground();
  }
  catch (std::logic_error& e)
  {
    QMessageBox::warning(NULL, "Warning", QString(e.what()));
    MITK_ERROR << e.what();
    return;
  }

  //this->m_SurfaceWorkerThread.SetAbort(false);
  //this->m_SurfaceWorkerThread.SetFilterThread(&(this->m_FilterWorkerThread));
  //this->m_FilterWorkerThread.SetAbort(false);


  m_Controls->m_ToFRecorderWidget->setEnabled(true);
  m_Controls->m_ToFRecorderWidget->ResetGUIToInitial();
  m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(true);
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

}

void QmitkToFUtilView::OnToFCameraDisconnected()
{
  //this->m_SurfaceWorkerThread.SetAbort(true);
  //this->m_SurfaceWorkerThread.quit();
  //this->m_FilterWorkerThread.SetAbort(true);

  m_Controls->m_ToFRecorderWidget->setEnabled(false);
  m_Controls->m_ToFVisualisationSettingsWidget->setEnabled(false);
  if(this->m_VideoSource)
  {
    this->m_VideoSource->StopCapturing();
    this->m_VideoSource = NULL;
  }
  RemoveBackground();
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

  cvReleaseImage(&(this->m_IplDistanceImage));
  cvReleaseImage(&(this->m_IplIntensityImage));
  cvReleaseImage(&(this->m_OutputIplImage));

  delete[] this->m_Widget1Texture;
  delete[] this->m_Widget2Texture;
  delete[] this->m_Widget3Texture;

  //mitk::ToFImageGrabber* aToFImageGrabber = dynamic_cast<mitk::ToFImageGrabber*>(m_ToFImageGrabber);
  //aToFImageGrabber->Delete();
}

void QmitkToFUtilView::OnToFCameraStarted()
{
  //this->m_FilterWorkerThread.SetAbort(false);
  //this->m_FilterWorkerThread.StartProcessing(this->m_ToFImageGrabber, this->m_MitkAllImage);
  //this->m_SurfaceWorkerThread.SetAbort(false);
  
  // initial update of image grabber
  this->m_ToFImageGrabber->Update();

  this->m_ToFCompositeFilter->SetInput(0,this->m_ToFImageGrabber->GetOutput(0));
  this->m_ToFCompositeFilter->SetInput(1,this->m_ToFImageGrabber->GetOutput(1));
  this->m_ToFCompositeFilter->SetInput(2,this->m_ToFImageGrabber->GetOutput(2));
  // initial update of composite filter
  this->m_ToFCompositeFilter->Update();
  this->m_ToFVisualizationFilter->SetInput(0,this->m_ToFCompositeFilter->GetOutput(0));
  this->m_ToFVisualizationFilter->SetInput(1,this->m_ToFCompositeFilter->GetOutput(1));
  this->m_ToFVisualizationFilter->SetInput(2,this->m_ToFCompositeFilter->GetOutput(2));
  // initial update of visualization filter
  this->m_ToFVisualizationFilter->Update();
  //this->m_MitkDistanceImage = m_ToFVisualizationFilter->GetOutput(0);
  //this->m_MitkAmplitudeImage = m_ToFVisualizationFilter->GetOutput(1);
  //this->m_MitkIntensityImage = m_ToFVisualizationFilter->GetOutput(2);
  this->m_MitkDistanceImage = m_ToFCompositeFilter->GetOutput(0);
  this->m_MitkAmplitudeImage = m_ToFCompositeFilter->GetOutput(1);
  this->m_MitkIntensityImage = m_ToFCompositeFilter->GetOutput(2);
  this->m_ToFDistanceImageToSurfaceFilter->SetInput(0,this->m_ToFCompositeFilter->GetOutput(0));
  this->m_ToFDistanceImageToSurfaceFilter->SetInput(1,this->m_ToFCompositeFilter->GetOutput(1));
  this->m_ToFDistanceImageToSurfaceFilter->SetInput(2,this->m_ToFCompositeFilter->GetOutput(2));
  this->m_Surface = this->m_ToFDistanceImageToSurfaceFilter->GetOutput(0);

  this->m_Frametimer->start(0);

  //if (m_Controls->m_AveragingFilterCheckBox->isChecked())
  //{
  //  OnAveragingFilterCheckBoxChecked(true);
  //}
  //if (m_Controls->m_ThresholdFilterCheckBox->isChecked())
  //{
  //  OnThresholdFilterCheckBoxChecked(true);
  //}
  //if (m_Controls->m_BilateralFilterCheckBox->isChecked())
  //{
  //  OnBilateralFilterCheckBoxChecked(true);
  //}

  m_Controls->m_ToFCompositeFilterWidget->UpdateFilterParameter();

  if (m_Controls->m_SurfaceCheckBox->isChecked())
  {
    OnSurfaceCheckBoxChecked(true);
  }
  if (m_Controls->m_TextureCheckBox->isChecked())
  {
    OnTextureCheckBoxChecked(true);
  }
  if (m_Controls->m_VideoTextureCheckBox->isChecked())
  {
    OnVideoTextureCheckBoxChecked(true);
  }

}

void QmitkToFUtilView::OnToFCameraStop()
{
	
  //this->m_Frametimer->stop();

  //this->m_SurfaceWorkerThread.SetAbort(true);
  //this->m_FilterWorkerThread.SetAbort(true);
  m_Controls->m_ToFRecorderWidget->OnStop();
}

void QmitkToFUtilView::OnToFCameraStopped()
{
  this->m_Frametimer->stop();
  //this->m_SurfaceWorkerThread.SetAbort(true);
  //this->m_FilterWorkerThread.SetAbort(true);
}

void QmitkToFUtilView::OnToFCameraSelected(const QString selected)
{
  if ((selected=="PMD CamBoard")||(selected=="PMD O3D"))
  {
    MITK_INFO<<"Surface representation currently not available for CamBoard and O3. Intrinsic parameters missing.";
    this->m_Controls->m_SurfaceCheckBox->setEnabled(false);
    this->m_Controls->m_TextureCheckBox->setEnabled(false);
    this->m_Controls->m_VideoTextureCheckBox->setEnabled(false);
    this->m_Controls->m_SurfaceCheckBox->setChecked(false);
    this->m_Controls->m_TextureCheckBox->setChecked(false);
    this->m_Controls->m_VideoTextureCheckBox->setChecked(false);
  }
  else
  {
    this->m_Controls->m_SurfaceCheckBox->setEnabled(true);
    this->m_Controls->m_TextureCheckBox->setEnabled(true); // TODO enable when bug 8106 is solved
    this->m_Controls->m_VideoTextureCheckBox->setEnabled(true);
  }
}

void QmitkToFUtilView::InitImage(mitk::Image::Pointer image, int numOfChannel)
{
  unsigned int dimensions[4];
  dimensions[0] = this->m_ToFImageGrabber->GetCaptureWidth();
  dimensions[1] = this->m_ToFImageGrabber->GetCaptureHeight();       
  dimensions[2] = 1;
  dimensions[3] = 1;
  image->Initialize(mitk::PixelType(typeid(float)), 2, dimensions, numOfChannel);
}

void QmitkToFUtilView::InitTexture(unsigned char* &image, int width, int height)
{
  int texSize = width * height * 3;
  image = new unsigned char[ texSize ];
  for(int i=0; i<texSize; i++)
  {
    image[i] = 0;
  }
}

void QmitkToFUtilView::AddNodesToDataStorage()
{

  if(this->GetDataStorage()->GetNamedNode("ToF_Distance") == NULL)
  {
    //m_DistanceImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    //m_DistanceImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget1->GetRenderWindow() ) );
    //m_DistanceImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget3->GetRenderWindow() ) );
    //m_DistanceImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow() ) );
    //m_DistanceImageNode->SetProperty( "inRenWin2" , mitk::BoolProperty::New( true ));
    ////m_DistanceImageNode->SetProperty( "use color", mitk::BoolProperty::New( false ));
    //m_DistanceImageNode->SetProperty( "use color", mitk::BoolProperty::New( true ));
    //m_DistanceImageNode->SetProperty( "binary", mitk::BoolProperty::New( false ));
    //m_DistanceImageNode->SetIntProperty( "layer", 10);
    //this->GetDataStorage()->Add( this->m_DistanceImageNode );
  }
  if(this->GetDataStorage()->GetNamedNode("ToF_Amplitude") == NULL)
  {
    //m_AmplitudeImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    //m_AmplitudeImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget2->GetRenderWindow() ) );
    //m_AmplitudeImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget3->GetRenderWindow() ) );
    //m_AmplitudeImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow() ) );
    //m_AmplitudeImageNode->SetProperty( "inRenWin1" , mitk::BoolProperty::New( true ));
    //m_AmplitudeImageNode->SetProperty( "use color", mitk::BoolProperty::New( true ));
    //m_AmplitudeImageNode->SetProperty( "binary", mitk::BoolProperty::New( false ));
    //m_AmplitudeImageNode->SetIntProperty( "layer", 10);
    //this->GetDataStorage()->Add( this->m_AmplitudeImageNode );
  }
  if(this->GetDataStorage()->GetNamedNode("ToF_Intensity") == NULL)
  {
    //m_IntensityImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    //m_IntensityImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget1->GetRenderWindow() ) );
    //m_IntensityImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget2->GetRenderWindow() ) );
    //m_IntensityImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow() ) );
    //m_IntensityImageNode->SetProperty( "inRenWin3" , mitk::BoolProperty::New( true ));
    //m_IntensityImageNode->SetProperty( "use color", mitk::BoolProperty::New( true ));
    //m_IntensityImageNode->SetProperty( "binary", mitk::BoolProperty::New( false ));
    //m_IntensityImageNode->SetIntProperty( "layer", 10);
    //this->GetDataStorage()->Add( this->m_IntensityImageNode );
  }  

  if(this->GetDataStorage()->GetNamedNode("ToF_All") == NULL)
  {
    //m_AllImageNode->SetProperty( "visible" , mitk::BoolProperty::New( true ));
    /*
    m_IntensityImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget1->GetRenderWindow() ) );
    m_IntensityImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget2->GetRenderWindow() ) );
    m_IntensityImageNode->SetVisibility( false, mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow() ) );
    m_IntensityImageNode->SetProperty( "inRenWin3" , mitk::BoolProperty::New( true ));
    m_IntensityImageNode->SetProperty( "use color", mitk::BoolProperty::New( true ));
    m_IntensityImageNode->SetProperty( "binary", mitk::BoolProperty::New( false ));
    m_IntensityImageNode->SetIntProperty( "layer", 10);
    */

    //mitk::Mapper::Pointer aMapper = mitk::ToFImageMapper::New();
    //m_AllImageNode->SetMapper(mitk::BaseRenderer::Standard2D, aMapper);
    //this->GetDataStorage()->Add( this->m_AllImageNode );
  }

}

void* QmitkToFUtilView::GetDataFromImage(std::string imageType)
{
  void* data;
  if (imageType.compare("Distance")==0)
  {
    data = this->m_MitkDistanceImage->GetData();
    //data = this->m_MitkAllImage->GetSliceData(0, 0, 0)->GetData();
  }
  else if (imageType.compare("Amplitude")==0)
  {
    data = this->m_MitkAmplitudeImage->GetData();
    //data = this->m_MitkAllImage->GetSliceData(0, 0, 1)->GetData();
  }
  if (imageType.compare("Intensity")==0)
  {
    data = this->m_MitkIntensityImage->GetData();
    //data = this->m_MitkAllImage->GetSliceData(0, 0, 2)->GetData();
  }
  return data;
}

void QmitkToFUtilView::OnUpdateCamera()
{
  int currentImageSequence = 0;

  // update pipeline
  this->m_MitkDistanceImage->Update();
  // update distance image node
  this->m_DistanceImageNode->SetData(this->m_MitkDistanceImage);

  if (!this->m_TransferFunctionInitialized)
  {
    m_Controls->m_ToFVisualisationSettingsWidget->InitializeTransferFunction(this->m_MitkDistanceImage, this->m_MitkAmplitudeImage, this->m_MitkIntensityImage);
    this->m_TransferFunctionInitialized = true;
  }

  if (m_Controls->m_VideoTextureCheckBox->isChecked() && this->m_VideoEnabled && this->m_VideoSource)
  {
    this->m_VideoTexture = this->m_VideoSource->GetVideoTexture();
    ProcessVideoTransform();
  }

  vtkColorTransferFunction* colorTransferFunction;
  std::string imageType;

  colorTransferFunction = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget1ColorTransferFunction();
  imageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget1ImageType();
  RenderWidget(m_MultiWidget->mitkWidget1, this->m_QmitkToFImageBackground1, this->m_Widget1ImageType, imageType, 
    colorTransferFunction, this->m_VideoTexture, this->m_Widget1Texture );

  colorTransferFunction = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget2ColorTransferFunction();
  imageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget2ImageType();
  RenderWidget(m_MultiWidget->mitkWidget2, this->m_QmitkToFImageBackground2, this->m_Widget2ImageType, imageType,
    colorTransferFunction, this->m_VideoTexture, this->m_Widget2Texture );

  colorTransferFunction = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget3ColorTransferFunction();
  imageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget3ImageType();
  RenderWidget(m_MultiWidget->mitkWidget3, this->m_QmitkToFImageBackground3, this->m_Widget3ImageType, imageType, 
    colorTransferFunction, this->m_VideoTexture, this->m_Widget3Texture );

  if (m_Controls->m_SurfaceCheckBox->isChecked())
  {
    // update surface
    this->m_Surface->Update();

    colorTransferFunction = m_Controls->m_ToFVisualisationSettingsWidget->GetColorTransferFunctionByImageType("Intensity");
 
    this->m_ToFSurfaceVtkMapper3D->SetVtkScalarsToColors(colorTransferFunction);

    if (this->m_SurfaceDisplayCount < 2)
    {
      this->m_SurfaceNode->SetData(this->m_Surface);
      this->m_SurfaceNode->SetMapper(mitk::BaseRenderer::Standard3D, m_ToFSurfaceVtkMapper3D);

      mitk::RenderingManager::GetInstance()->InitializeViews(
        this->m_Surface->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS, true);

      mitk::Point3D surfaceCenter= this->m_Surface->GetGeometry()->GetCenter();
      m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetPosition(0,0,-50);
      m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetViewUp(0,-1,0);
      m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(0,0,surfaceCenter[2]);
      m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetViewAngle(40);
      m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetClippingRange(1, 10000);
    }
    this->m_SurfaceDisplayCount++;

  }



  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

  this->m_2DDisplayCount++;
  if ((this->m_2DDisplayCount % this->m_StepsForFramerate) == 0)
  {
    this->m_2DTimeAfter = this->m_RealTimeClock->GetCurrentStamp() - this->m_2DTimeBefore;
    MITK_INFO << " 2D-Display-framerate (fps): " << this->m_StepsForFramerate / (this->m_2DTimeAfter/1000) << " Sequence: " << this->m_ImageSequence;
    this->m_2DTimeBefore = this->m_RealTimeClock->GetCurrentStamp();
  }
}

void QmitkToFUtilView::ProcessVideoTransform()
{
  /*
  this->m_VideoTexture
  this->m_IplIntensityImage->imageData = (char*)intensityFloatData;
  this->m_IplIntensityImage = cvCreateImageHeader(cvSize(this->m_ToFCaptureWidth, this->m_ToFCaptureHeight), IPL_DEPTH_32F, 1); // no allocation
  this->m_OutputIplImage = cvCreateImage(cvSize(this->m_ToFCaptureWidth, this->m_ToFCaptureHeight), IPL_DEPTH_32F, 1);
*/


  IplImage *src, *dst;
  src = cvCreateImageHeader(cvSize(this->m_VideoCaptureWidth, this->m_VideoCaptureHeight), IPL_DEPTH_8U, 3);
  src->imageData = (char*)this->m_VideoTexture;

  CvPoint2D32f srcTri[3], dstTri[3];
	CvMat* rot_mat = cvCreateMat(2,3,CV_32FC1);
	CvMat* warp_mat = cvCreateMat(2,3,CV_32FC1);
  dst = cvCloneImage(src);
	dst->origin = src->origin;
	cvZero( dst );

	// Create angle and scale
	double angle = 0.0;
	double scale = 1.0;

  int xOffset = 0;//m_Controls->m_XOffsetSpinBox->value();
  int yOffset = 0;//m_Controls->m_YOffsetSpinBox->value();
  int zoom = 0;//m_Controls->m_ZoomSpinBox->value();

	// Compute warp matrix
	srcTri[0].x = 0 + zoom;
	srcTri[0].y = 0 + zoom;
	srcTri[1].x = src->width - 1 - zoom;
	srcTri[1].y = 0 + zoom;
	srcTri[2].x = 0 + zoom;
	srcTri[2].y = src->height - 1 - zoom;

	dstTri[0].x = 0;
	dstTri[0].y = 0;
	dstTri[1].x = src->width - 1;
	dstTri[1].y = 0;
	dstTri[2].x = 0;
	dstTri[2].y = src->height - 1;

	cvGetAffineTransform( srcTri, dstTri, warp_mat );
	cvWarpAffine( src, dst, warp_mat );
	cvCopy ( dst, src );


	// Compute warp matrix
	srcTri[0].x = 0;
	srcTri[0].y = 0;
	srcTri[1].x = src->width - 1;
	srcTri[1].y = 0;
	srcTri[2].x = 0;
	srcTri[2].y = src->height - 1;

	dstTri[0].x = srcTri[0].x + xOffset;
	dstTri[0].y = srcTri[0].y + yOffset;
	dstTri[1].x = srcTri[1].x + xOffset;
	dstTri[1].y = srcTri[1].y + yOffset;
	dstTri[2].x = srcTri[2].x + xOffset;
	dstTri[2].y = srcTri[2].y + yOffset;

	cvGetAffineTransform( srcTri, dstTri, warp_mat );
	cvWarpAffine( src, dst, warp_mat );
	cvCopy ( dst, src );

  src->imageData = NULL;
	cvReleaseImage( &src );
	cvReleaseImage( &dst );
	cvReleaseMat( &rot_mat );
	cvReleaseMat( &warp_mat );

}

void QmitkToFUtilView::RenderWidget(QmitkRenderWindow* mitkWidget, QmitkToFImageBackground* imageBackground, 
                                    std::string& oldImageType, std::string newImageType,
                                    vtkColorTransferFunction* colorTransferFunction, unsigned char* videoTexture, unsigned char* tofTexture )
{
  if (newImageType.compare("Video") == 0)
  {
    if (this->m_VideoEnabled)
    {
      if (oldImageType.compare(newImageType)!=0)
      {
        imageBackground->AddRenderWindow(mitkWidget->GetRenderWindow(), this->m_VideoCaptureWidth, this->m_VideoCaptureHeight);
        oldImageType = newImageType;
      }
      imageBackground->UpdateBackground(videoTexture);
    }
  }
  else
  {
    if (oldImageType.compare(newImageType)!=0)
    {
      imageBackground->AddRenderWindow(mitkWidget->GetRenderWindow(), this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);
      oldImageType = newImageType;
    }
    void* data = GetDataFromImage(newImageType);
    PrepareImageForBackground(colorTransferFunction, (float*)data, tofTexture);
    imageBackground->UpdateBackground(tofTexture);
  }
}

//void QmitkToFUtilView::CreateSurface()
//{
//  static int count = 0;
//  //this->m_ToFSurfaceGenerationFilter = mitk::ToFSurfaceGenerationFilter::New();

//  this->m_ToFSurfaceGenerationFilter->SetInput(this->m_MitkDistanceImage);
//  this->m_ToFSurfaceGenerationFilter->Modified();
//  this->m_ToFSurfaceGenerationFilter->Update();
//  this->m_Surface->ReleaseData();
//  vtkPolyData* polydata = this->m_ToFSurfaceGenerationFilter->GetOutput()->GetVtkPolyData();
//  //this->m_Surface->GetVtkPolyData()->ReleaseData();
//  this->m_Surface->SetVtkPolyData(polydata);
//  //this->m_Surface = this->m_ToFSurfaceGenerationFilter->GetOutput();

//  this->m_SurfaceNode->SetData(this->m_Surface);
//  //if(!this->m_SurfaceInitialized)
//  if (count < 1)
//  {
//    //mitk::RenderingManager::GetInstance()->InitializeViews(
//    //  this->m_Surface->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS, true);

//    m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->ResetCamera();
//    m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetPosition(-500,-120,-6000);
//    m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetViewUp(0,-1,0);
//    m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(-130,0,2300);
//    m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetViewAngle(40);
//    //this->m_SurfaceInitialized = true;
//    count++;
//  }
//}

void QmitkToFUtilView::OnTextureCheckBoxChecked(bool checked)
{
  if (checked)
  {
    this->m_SurfaceNode->SetBoolProperty("scalar visibility", true);
  }
  else
  {
    this->m_SurfaceNode->SetBoolProperty("scalar visibility", false);
  }
}

void QmitkToFUtilView::OnVideoTextureCheckBoxChecked(bool checked)
{
  if (checked)
  {
    if (this->m_VideoEnabled)
    {
      this->m_SurfaceTextureImage->SetSlice(this->m_VideoTexture, 0, 0, 0);
      this->m_ToFSurfaceVtkMapper3D->SetTexture(this->m_VideoTexture);
    }
    else
    {
      this->m_ToFSurfaceVtkMapper3D->SetTexture(NULL);
    }
  }
  else
  {
    this->m_ToFSurfaceVtkMapper3D->SetTexture(NULL);
  }
}

void QmitkToFUtilView::OnSurfaceCheckBoxChecked(bool checked)
{
  //if (checked)
  //{
  //  void* surfaceInputdata = this->m_MitkAllImage->GetSliceData(0, 0, 0)->GetData();

  //  this->m_SurfaceInputImage->SetSlice(surfaceInputdata, 0, 0, 0);

  //  void* surfaceIntensitydata = this->m_MitkAllImage->GetSliceData(0, 0, 2)->GetData();
  //  this->m_SurfaceIntensityImage->SetSlice(surfaceIntensitydata, 0, 0, 0);
  //  if (m_Controls->m_VideoTextureCheckBox->isChecked())
  //  {
  //    OnVideoTextureCheckBoxChecked(true);
  //  }
  //  else
  //  {
  //    OnVideoTextureCheckBoxChecked(false);
  //  }
  //  if (m_Controls->m_TextureCheckBox->isChecked())
  //  {
  //    OnTextureCheckBoxChecked(true);
  //  }
  //  else
  //  {
  //    OnTextureCheckBoxChecked(false);
  //  }
  //  //this->m_SurfaceWorkerThread.SetAbort(false);
  //  //this->m_SurfaceDisplayCount = 0;
  //  //this->m_SurfaceWorkerThread.CreateSurface(this->m_SurfaceInputImage, this->m_SurfaceIntensityImage, this->m_SurfaceTextureImage, this->m_Surface);
  //}
  //else
  //{
  //  //this->m_SurfaceWorkerThread.SetAbort(true);
  //}
}

void QmitkToFUtilView::PrepareImageForBackground(vtkLookupTable* lut, float* floatData, unsigned char* image)
{
  vtkSmartPointer<vtkFloatArray> floatArrayDist = vtkFloatArray::New();
  floatArrayDist->Initialize();
  int numOfPixel = this->m_ToFCaptureWidth * this->m_ToFCaptureHeight;
  float* flippedFloatData = new float[numOfPixel];

  for (int i=0; i<this->m_ToFCaptureHeight; i++)
  {
    for (int j=0; j<this->m_ToFCaptureWidth; j++)
    {
      flippedFloatData[i*this->m_ToFCaptureWidth+j] = floatData[((this->m_ToFCaptureHeight-1-i)*this->m_ToFCaptureWidth)+j];
    }
  }

  floatArrayDist->SetArray(flippedFloatData, numOfPixel, 0);
  lut->MapScalarsThroughTable(floatArrayDist, image, VTK_RGB);

  delete[] flippedFloatData;
}

void QmitkToFUtilView::PrepareImageForBackground(vtkColorTransferFunction* colorTransferFunction, float* floatData, unsigned char* image)
{
  vtkSmartPointer<vtkFloatArray> floatArrayDist = vtkFloatArray::New();
  floatArrayDist->Initialize();
  int numOfPixel = this->m_ToFCaptureWidth * this->m_ToFCaptureHeight;
  float* flippedFloatData = new float[numOfPixel];

  for (int i=0; i<this->m_ToFCaptureHeight; i++)
  {
    for (int j=0; j<this->m_ToFCaptureWidth; j++)
    {
      flippedFloatData[i*this->m_ToFCaptureWidth+j] = floatData[((this->m_ToFCaptureHeight-1-i)*this->m_ToFCaptureWidth)+j];
    }
  }

  floatArrayDist->SetArray(flippedFloatData, numOfPixel, 0);
  colorTransferFunction->MapScalarsThroughTable(floatArrayDist, image, VTK_RGB);

  delete[] flippedFloatData;
}

void QmitkToFUtilView::RemoveBackground()
{
  if(this->m_QmitkToFImageBackground1)
  {
    this->m_QmitkToFImageBackground1->RemoveRenderWindow(m_MultiWidget->mitkWidget1->GetRenderWindow());
  }
  if(this->m_QmitkToFImageBackground2)
  {
    this->m_QmitkToFImageBackground2->RemoveRenderWindow(m_MultiWidget->mitkWidget2->GetRenderWindow());
  }
  if(this->m_QmitkToFImageBackground3)
  {
    this->m_QmitkToFImageBackground3->RemoveRenderWindow(m_MultiWidget->mitkWidget3->GetRenderWindow());
  }
}

void QmitkToFUtilView::AddBackground()
{
  InitTexture(this->m_Widget1Texture, this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);
  InitTexture(this->m_Widget2Texture, this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);
  InitTexture(this->m_Widget3Texture, this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);

  this->m_QmitkToFImageBackground1 = new QmitkToFImageBackground();
  this->m_QmitkToFImageBackground1->AddRenderWindow(m_MultiWidget->mitkWidget1->GetRenderWindow(), this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);
  this->m_QmitkToFImageBackground1->UpdateBackground(this->m_Widget1Texture);
  this->m_Widget1ImageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget1ImageType();

  this->m_QmitkToFImageBackground2 = new QmitkToFImageBackground();
  this->m_QmitkToFImageBackground2->AddRenderWindow(m_MultiWidget->mitkWidget2->GetRenderWindow(), this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);
  this->m_QmitkToFImageBackground2->UpdateBackground(this->m_Widget2Texture);
  this->m_Widget2ImageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget2ImageType();

  this->m_QmitkToFImageBackground3 = new QmitkToFImageBackground();
  this->m_QmitkToFImageBackground3->AddRenderWindow(m_MultiWidget->mitkWidget3->GetRenderWindow(), this->m_ToFCaptureWidth, this->m_ToFCaptureHeight);
  this->m_QmitkToFImageBackground3->UpdateBackground(this->m_Widget3Texture);
  this->m_Widget3ImageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget3ImageType();
}
