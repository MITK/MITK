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

#include "QmitkToFSurfaceGenerationWidget.h"

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>

//QT headers
#include <QString>
#include <mitkSmartPointerProperty.h>
#include <mitkRenderingManager.h>
#include <vtkCamera.h>

const std::string QmitkToFSurfaceGenerationWidget::VIEW_ID = "org.mitk.views.qmitktofsurfacegenerationwidget";

QmitkToFSurfaceGenerationWidget::QmitkToFSurfaceGenerationWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
, m_Controls(NULL),
  m_ToFDistanceImageToSurfaceFilter(NULL),
  m_ToFImageGrabber(NULL),
  m_CameraIntrinsics(NULL),
  m_Active(false)
{
  CreateQtPartControl(this);
}

QmitkToFSurfaceGenerationWidget::~QmitkToFSurfaceGenerationWidget()
{
}

void QmitkToFSurfaceGenerationWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFSurfaceGenerationWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
    this->OnShowAdvancedOptionsCheckboxChecked(false);
  }
}

void QmitkToFSurfaceGenerationWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_Compute3DDataCheckbox), SIGNAL(toggled(bool)), this, SLOT(OnCompute3DDataCheckboxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_DistanceColorMapCheckbox), SIGNAL(toggled(bool)), this, SLOT(OnDistanceColorMapCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_RGBTextureCheckbox), SIGNAL(toggled(bool)), this, SLOT(OnRGBTextureCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_TriangulationThresholdSpinbox), SIGNAL(valueChanged(double)), this, SLOT(OnTriangulationThresholdSpinBoxChanged()) );
    connect( (QObject*)(m_Controls->m_ShowAdvancedOptionsCheckbox), SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedOptionsCheckboxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_RepresentationCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnRepresentationChanged(int)) );
    connect( (QObject*)(m_Controls->m_ReconstructionCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnReconstructionChanged(int)) );
  }
}

mitk::ToFDistanceImageToSurfaceFilter::Pointer QmitkToFSurfaceGenerationWidget::GetToFDistanceImageToSurfaceFilter()
{
  return m_ToFDistanceImageToSurfaceFilter;
}

void QmitkToFSurfaceGenerationWidget::OnShowAdvancedOptionsCheckboxChecked(bool checked)
{
  this->m_Controls->m_TextureGroupBox->setVisible(checked);
  this->m_Controls->m_TriangulationThresholdSpinbox->setVisible(checked);
  this->m_Controls->m_ReconstructionCombobox->setVisible(checked);
  this->m_Controls->m_RepresentationCombobox->setVisible(checked);
  this->m_Controls->label->setVisible(checked);
  this->m_Controls->label_2->setVisible(checked);
  this->m_Controls->label_3->setVisible(checked);
}

void QmitkToFSurfaceGenerationWidget::Initialize(mitk::ToFDistanceImageToSurfaceFilter::Pointer filter,
                                                 mitk::ToFImageGrabber::Pointer grabber,
                                                 mitk::CameraIntrinsics::Pointer intrinsics,
                                                 mitk::DataNode::Pointer surface,
                                                 vtkSmartPointer<vtkCamera> camera,
                                                 bool generateSurface,
                                                 bool showAdvancedOptions)
{
  m_ToFDistanceImageToSurfaceFilter = filter;
  m_ToFImageGrabber = grabber;
  m_CameraIntrinsics = intrinsics;
  m_Active = true;
  m_Camera3d = camera;

  bool hasSurface = false;
  m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasSurface", hasSurface);
  if(hasSurface)
  {
    this->m_Surface = mitk::Surface::New();
  }
  else
  {
    this->m_Surface = this->m_ToFDistanceImageToSurfaceFilter->GetOutput(0);
  }

  m_SurfaceNode = surface;
  m_SurfaceNode->SetData(m_Surface);

  this->FindReconstructionModeProperty();
  m_Controls->m_ShowAdvancedOptionsCheckbox->setChecked(showAdvancedOptions);
  this->OnShowAdvancedOptionsCheckboxChecked(showAdvancedOptions);
  m_Controls->m_Compute3DDataCheckbox->setChecked(generateSurface);
}

bool QmitkToFSurfaceGenerationWidget::IsActive()
{
  if(!m_Active)
  {
    MITK_ERROR << "QmitkToFSurfaceGenerationWidget is not active - please call QmitkToFSurfaceGenerationWidget::Initialize() first";
  }
  return m_Active;
}

void QmitkToFSurfaceGenerationWidget::OnTriangulationThresholdSpinBoxChanged()
{
  if(IsActive())
  {
      this->m_ToFDistanceImageToSurfaceFilter->SetTriangulationThreshold( this->m_Controls->m_TriangulationThresholdSpinbox->value() );
      this->m_ToFImageGrabber->GetCameraDevice()->SetFloatProperty("TriangulationThreshold", this->m_Controls->m_TriangulationThresholdSpinbox->value());
  }
}

void QmitkToFSurfaceGenerationWidget::OnReconstructionChanged(int index)
{
  if(IsActive())
  {
    mitk::ToFDistanceImageToSurfaceFilter::ReconstructionModeType type = mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance;
    switch (index)
    {
    case 0:
    {
      type = mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance;
      break;
    }
    case 1:
    {
      type = mitk::ToFDistanceImageToSurfaceFilter::WithOutInterPixelDistance;
      break;
    }
    case 2:
    {
      type = mitk::ToFDistanceImageToSurfaceFilter::Kinect;
      break;
    }
    default:
    {
      MITK_ERROR << "ReconstructionModeType does not exist or is not known in QmitkToFSurfaceGenerationWidget.";
      break;
    }
    }
    this->m_ToFDistanceImageToSurfaceFilter->SetReconstructionMode( type );
  }
}

void QmitkToFSurfaceGenerationWidget::OnRepresentationChanged(int index)
{
  if(IsActive())
  {
    bool generateTriangularMesh = false; //PointCloud case
    if( index == 0) //Surface case
    {
      generateTriangularMesh = true;
    }
    this->m_ToFDistanceImageToSurfaceFilter->SetGenerateTriangularMesh(generateTriangularMesh);
    this->m_ToFImageGrabber->GetCameraDevice()->SetBoolProperty("GenerateTriangularMesh", generateTriangularMesh);

    this->m_ToFDistanceImageToSurfaceFilter->SetTriangulationThreshold( this->m_Controls->m_TriangulationThresholdSpinbox->value() );
    this->m_ToFImageGrabber->GetCameraDevice()->SetFloatProperty("TriangulationThreshold", this->m_Controls->m_TriangulationThresholdSpinbox->value());
    this->m_Controls->m_TriangulationThresholdSpinbox->setEnabled(generateTriangularMesh);
  }
}

void QmitkToFSurfaceGenerationWidget::OnRGBTextureCheckBoxChecked(bool checked)
{
  if(IsActive())
  {
    if(m_ToFImageGrabber->GetBoolProperty("HasRGBImage"))
    {
      if (checked)
      {
        // enable texture
        this->m_SurfaceNode->SetProperty("Surface.Texture",mitk::SmartPointerProperty::New(this->m_ToFImageGrabber->GetOutput(3)));
      } else {
        // disable texture
        this->m_SurfaceNode->GetPropertyList()->DeleteProperty("Surface.Texture");
      }
    }
  }
}

void QmitkToFSurfaceGenerationWidget::OnDistanceColorMapCheckBoxChecked(bool checked)
{
  if(m_SurfaceNode.IsNotNull())
  {
    this->m_SurfaceNode->SetBoolProperty("scalar visibility", checked);
  }
}

bool QmitkToFSurfaceGenerationWidget::UpdateSurface()
{
  if(IsActive())
  {
    //##### Code for surface #####
    if (m_Controls->m_Compute3DDataCheckbox->isChecked())
    {
      bool hasSurface = false;
      this->m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasSurface", hasSurface);
      if(hasSurface)
      {
        mitk::SmartPointerProperty::Pointer surfaceProp = dynamic_cast< mitk::SmartPointerProperty * >(this->m_ToFImageGrabber->GetCameraDevice()->GetProperty("ToFSurface"));
        this->m_Surface->SetVtkPolyData( dynamic_cast< mitk::Surface* >( surfaceProp->GetSmartPointer().GetPointer() )->GetVtkPolyData() );
      }
      else
      {
        this->m_Surface = m_ToFDistanceImageToSurfaceFilter->GetOutput(0);
      }

      //update pipeline
      this->m_Surface->Update();
      return true;
    }
    //##### End code for surface #####
  }
  return false;
}

void QmitkToFSurfaceGenerationWidget::OnCompute3DDataCheckboxChecked(bool checked)
{
  if(checked)
  {
    //initialize the surface once
    MITK_DEBUG << "OnSurfaceCheckboxChecked true";
    this->m_SurfaceNode->SetData(this->m_Surface);

    this->OnRepresentationChanged(m_Controls->m_RepresentationCombobox->currentIndex());

    //we need to initialize (reinit) the surface, to make it fit into the renderwindow
    mitk::RenderingManager::GetInstance()->InitializeViews(
      this->m_Surface->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS, true);

    // correctly place the vtk camera for appropriate surface rendering
    //1m distance to camera should be a nice default value for most cameras
    m_Camera3d->SetPosition(0,0,0);
    m_Camera3d->SetViewUp(0,-1,0);
    m_Camera3d->SetFocalPoint(0,0,1);
    if (this->m_CameraIntrinsics.IsNotNull())
    {
      // compute view angle from camera intrinsics
      m_Camera3d->SetViewAngle(mitk::ToFProcessingCommon::CalculateViewAngle(m_CameraIntrinsics,m_ToFImageGrabber->GetCaptureWidth()));
    }
    else
    {
      m_Camera3d->SetViewAngle(45);
    }
    m_Camera3d->SetClippingRange(1, 10000);
  }
}

void QmitkToFSurfaceGenerationWidget::FindReconstructionModeProperty()
{
  bool KinectReconstructionMode = false;
  m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("KinectReconstructionMode",KinectReconstructionMode);
  if(KinectReconstructionMode)
  {
    //set the reconstruction mode for kinect
    this->m_ToFDistanceImageToSurfaceFilter->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::Kinect);
    m_Controls->m_ReconstructionCombobox->setDisabled(true);
    m_Controls->m_ReconstructionCombobox->setCurrentIndex(2);
  }
  else
  {
    m_Controls->m_ReconstructionCombobox->setEnabled(true);
  }
}

mitk::Surface::Pointer QmitkToFSurfaceGenerationWidget::GetSurface()
{
  return m_Surface;
}
