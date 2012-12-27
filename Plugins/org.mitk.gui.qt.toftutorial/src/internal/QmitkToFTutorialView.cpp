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
#include "QmitkToFTutorialView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

// mitk includes
#include <mitkCameraIntrinsics.h> // class holding the intrinsic parameters of the according camera
#include <mitkSurface.h>

// MITK-ToF related includes
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFConfig.h> // configuration file holding e.g. plugin paths or path to test file directory
#include <mitkToFDistanceImageToSurfaceFilter.h> // filter from module ToFProcessing that calculates a surface from the given range image
#include <mitkToFImageGrabber.h> // allows access to images provided by the ToF camera

const std::string QmitkToFTutorialView::VIEW_ID = "org.mitk.views.toftutorial";

QmitkToFTutorialView::QmitkToFTutorialView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

QmitkToFTutorialView::~QmitkToFTutorialView()
{
}

void QmitkToFTutorialView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkToFTutorialViewControls;
    m_Controls->setupUi( parent );

    connect( m_Controls->step1Button, SIGNAL(clicked()), this, SLOT(OnStep1()) );
    connect( m_Controls->step2Button, SIGNAL(clicked()), this, SLOT(OnStep2()) );
  }
}


void QmitkToFTutorialView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkToFTutorialView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkToFTutorialView::OnStep1()
{
  // clean up data storage
  RemoveAllNodesFromDataStorage();
  // Create an instance of ToFImageGrabber that holds a ToFCameraMITKPlayerDevice for playing ToF data
  mitk::ToFImageGrabber::Pointer tofImageGrabber = mitk::ToFImageGrabber::New();
  tofImageGrabber->SetCameraDevice(mitk::ToFCameraMITKPlayerDevice::New());
  // set paths to test data
  std::string distanceFileName = MITK_TOF_DATA_DIR;
  distanceFileName.append("/PMDCamCube2_MF0_IT0_20Images_DistanceImage.pic");
  std::string amplitudeFileName = MITK_TOF_DATA_DIR;
  amplitudeFileName.append("/PMDCamCube2_MF0_IT0_20Images_AmplitudeImage.pic");
  std::string intensityFileName = MITK_TOF_DATA_DIR;
  intensityFileName.append("/PMDCamCube2_MF0_IT0_20Images_IntensityImage.pic");
  // set file name property in image grabber. This will be propagated to the corresponding device and controller class
  tofImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(distanceFileName));
  tofImageGrabber->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(amplitudeFileName));
  tofImageGrabber->SetProperty("IntensityImageFileName",mitk::StringProperty::New(intensityFileName));
  // connect to device
  if (tofImageGrabber->ConnectCamera())
  {
    //// start camera (internally starts thread that continuously grabs images from the camera)
    tofImageGrabber->StartCamera();
    // update image grabber which itself represents the source of a MITK filter pipeline
    tofImageGrabber->Update();
    // grab distance image
    mitk::Image::Pointer distanceImage = tofImageGrabber->GetOutput(0);
    // grab amplitude image
    mitk::Image::Pointer amplitudeImage = tofImageGrabber->GetOutput(1);
    // grab intensity image
    mitk::Image::Pointer intensityImage = tofImageGrabber->GetOutput(2);
    //add distance image to data storage
    mitk::DataNode::Pointer distanceNode = mitk::DataNode::New();
    distanceNode->SetName("Distance Image");
    distanceNode->SetData(distanceImage);
    this->GetDefaultDataStorage()->Add(distanceNode);
    //add amplitude image to data storage
    mitk::DataNode::Pointer amplitudeNode = mitk::DataNode::New();
    amplitudeNode->SetName("Amplitude Image");
    amplitudeNode->SetData(amplitudeImage);
    this->GetDefaultDataStorage()->Add(amplitudeNode);
    //add intensity image to data storage
    mitk::DataNode::Pointer intensityNode = mitk::DataNode::New();
    intensityNode->SetName("Intensity Image");
    intensityNode->SetData(intensityImage);
    this->GetDefaultDataStorage()->Add(intensityNode);
    // stop camera (terminate internally used thread)
    tofImageGrabber->StopCamera();
    //// disconnect from camera
    tofImageGrabber->DisconnectCamera();
    // adjust views to new data in DataStorage
    mitk::RenderingManager::GetInstance()->InitializeViews(distanceImage->GetGeometry());
  }
  else
  {
    MITK_ERROR<<"Connection to ToF camera could not be established";
  }
}

void QmitkToFTutorialView::OnStep2()
{
  // Check if distance image is available
  mitk::DataNode::Pointer distanceNode = this->GetDefaultDataStorage()->GetNamedNode("Distance Image");
  if (distanceNode.IsNotNull())
  {
    // get distance image from node and check if node contains image
    mitk::Image::Pointer distanceImage = dynamic_cast<mitk::Image*>(distanceNode->GetData());
    if (distanceImage.IsNotNull())
    {
      // create object of CameraIntrinsics that holds intrinsic parameters of the ToF camera
      mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
      // set focal length in pixel
      cameraIntrinsics->SetFocalLength(295.8,296.1);
      // set principal point in pixel
      cameraIntrinsics->SetPrincipalPoint(113.2,97.1);
      // set up filter for surface calculation
      mitk::ToFDistanceImageToSurfaceFilter::Pointer surfaceFilter = mitk::ToFDistanceImageToSurfaceFilter::New();
      // apply intrinsic parameters to filter
      surfaceFilter->SetCameraIntrinsics(cameraIntrinsics);
      // set distance between pixels on chip in mm (in this example squared pixel)
      mitk::ToFProcessingCommon::ToFPoint2D interPixelDistance;
      interPixelDistance[0] = 0.045;
      interPixelDistance[1] = 0.045;
      surfaceFilter->SetInterPixelDistance(interPixelDistance);
      // set distance image as input
      surfaceFilter->SetInput(distanceImage);
      // update the filter
      surfaceFilter->Update();
      // get surface from filter
      mitk::Surface::Pointer surface = surfaceFilter->GetOutput();
      // add surface to data storage
      mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
      surfaceNode->SetName("ToF surface");
      surfaceNode->SetData(surface);
      this->GetDefaultDataStorage()->Add(surfaceNode);
      // adjust views to new data in DataStorage
      mitk::RenderingManager::GetInstance()->InitializeViews(surface->GetGeometry());
      mitk::RenderingManager::GetInstance()->InitializeViews(surface->GetGeometry());
    }
    else
    {
      QMessageBox::warning(NULL,"ToF Tutorial","Node 'Distance Image' contains no image");
    }
  }
  else
  {
    QMessageBox::warning(NULL,"ToF Tutorial","Perform Step 1 first to acquire a distance image");
  }
}

void QmitkToFTutorialView::RemoveAllNodesFromDataStorage()
{
  mitk::DataStorage::SetOfObjects::ConstPointer allNodes = this->GetDefaultDataStorage()->GetAll();
  this->GetDefaultDataStorage()->Remove(allNodes);
}





