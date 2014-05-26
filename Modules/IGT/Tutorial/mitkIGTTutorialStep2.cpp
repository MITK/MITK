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

#include <mitkVirtualTrackingDevice.h>
#include <mitkNavigationData.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>

#include <mitkStandaloneDataStorage.h>
#include <mitkDataNode.h>
#include <mitkCone.h>
#include <mitkCylinder.h>
#include <mitkRenderWindow.h>
#include <mitkGlobalInteraction.h>
#include <itksys/SystemTools.hxx>
/**
  * \brief This tutorial shows how to compose navigation datas. Therefore we render two objects.
  * The first object is a cone that is tracked. The second object is a cylinder at a fixed position
  * relative to the cone. At the end of the tracking, the cylinder is moved to its new relative position
  * according to the last output of the tracking device.
  * In addition to IGT tutorial step 1, the objects are added to a datastorage. Furthermore, a renderwindow
  * is used for visual output.
  */
int main(int argc, char* argv[])
{
  // Before rendering, global interaction must be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  //General code rendering the data in a renderwindow. See MITK Tutorial Step1 for more details.
  mitk::StandaloneDataStorage::Pointer dataStorage = mitk::StandaloneDataStorage::New();
  mitk::RenderWindow::Pointer renderWindow = mitk::RenderWindow::New();
  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

  //Here, we want a 3D renderwindow
  renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
  renderWindow->GetVtkRenderWindow()->SetSize( 500, 500 );
  renderWindow->GetRenderer()->Resize( 500, 500);
  //Connect datastorage and renderwindow
  renderWindow->GetRenderer()->SetDataStorage(dataStorage);

  // --------------begin of moving object code -------------------------- //

  //Virtual tracking device to generate random positions
  mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New();
  //Bounds (within the random numbers are generated) must be set before the tools are added
  double bound = 10.0;
  mitk::ScalarType bounds[] = {-bound, bound, -bound, bound, -bound, bound};
  tracker->SetBounds(bounds);
  tracker->AddTool("tool1");

  //Tracking device source to get the data
  mitk::TrackingDeviceSource::Pointer source = mitk::TrackingDeviceSource::New();
  source->SetTrackingDevice(tracker);
  source->Connect();

  //Cone representation for rendering of the moving object
  mitk::Cone::Pointer cone = mitk::Cone::New();
  dataNode->SetData(cone);
  dataNode->SetName("My tracked object");
  dataNode->SetColor(0.0, 1.0, 1.0);
  dataStorage->Add(dataNode);

  //Filter for rendering the cone at correct postion and orientation
  mitk::NavigationDataObjectVisualizationFilter::Pointer visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
  visualizer->SetInput(0, source->GetOutput());
  visualizer->SetRepresentationObject(0, cone);
  source->StartTracking();

  // --------------end of moving object code -------------------------- //

  // --------------begin of fixed object code ------------------------- //

  //Cylinder representation for rendering of the fixed object
  mitk::DataNode::Pointer cylinderNode = mitk::DataNode::New();
  mitk::Cylinder::Pointer cylinder = mitk::Cylinder::New();
  cylinderNode->SetData(cylinder);
  cylinderNode->SetName("My fixed object");
  cylinderNode->SetColor(1.0, 0.0, 0.0);
  dataStorage->Add(cylinderNode);

  //Define a rotation and a translation for the fixed object
  mitk::Matrix3D rotationMatrix;
  rotationMatrix.SetIdentity();
  double alpha = 0.3;
  rotationMatrix[1][1] = cos(alpha);
  rotationMatrix[1][2] = -sin(alpha);
  rotationMatrix[2][1] = sin(alpha);
  rotationMatrix[2][2] = cos(alpha);

  mitk::Vector3D offset;
  offset.Fill(5.0);
  //Add rotation and translation to affine transform
  mitk::AffineTransform3D::Pointer affineTransform3D = mitk::AffineTransform3D::New();
  affineTransform3D->SetOffset(offset);
  affineTransform3D->SetMatrix(rotationMatrix);

  //apply rotation and translation
  mitk::NavigationData::Pointer fixedNavigationData = mitk::NavigationData::New(affineTransform3D);
  cylinder->GetGeometry()->SetIndexToWorldTransform(fixedNavigationData->GetAffineTransform3D());

  // --------------end of fixed object code ------------------------- //

  // Global reinit with the bounds of the virtual tracking device
  mitk::TimeGeometry::Pointer timeGeometry = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
  mitk::BaseGeometry::Pointer geometry = timeGeometry->GetGeometryForTimeStep(0);
  geometry->SetBounds(bounds);

  mitk::RenderingManager::GetInstance()->InitializeViews( geometry );

  //Generate and render 75 time steps to move the tracked object
  for(int i=0; i < 75; ++i)
  {
    //Update the cone position
    visualizer->Update();

    //Update rendering
    renderWindow->GetVtkRenderWindow()->Render();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    MITK_INFO << "Position " << source->GetOutput()->GetPosition();
    //Slight delay for the random numbers
    itksys::SystemTools::Delay(100);
  }
  //Stop the tracking device and disconnect it
  //The tracking is done, now we want to move the fixed object to its correct relative position regarding the tracked object.
  source->StopTracking();
  source->Disconnect();

  //Now the tracking is finished and we can use the transformation to move
  //the fixed object to its correct position relative to the new position
  //of the moving/tracked object. Therefore, we compose the navigation datas.
  fixedNavigationData->Compose(source->GetOutput(), false);

  //Update the transformation matrix of the cylinder
  cylinder->GetGeometry()->SetIndexToWorldTransform(fixedNavigationData->GetAffineTransform3D());

  //Update the rendering
  renderWindow->GetVtkRenderWindow()->Render();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //Wait a little before closing the renderwindow
  itksys::SystemTools::Delay(2000);
}
