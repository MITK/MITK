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
#include <mitkInternalTrackingTool.h>

#include <mitkNavigationData.h>
#include <mitkTrackingDeviceSource.h>
#include "mitkNavigationDataDisplacementFilter.h"
#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataPlayer.h>

#include <mitkStandaloneDataStorage.h>
#include <mitkDataNode.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkCone.h>
#include <mitkRenderWindow.h>
#include <mitkGlobalInteraction.h>
#include <itksys/SystemTools.hxx>

int main(int argc, char* argv[])
{
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

  //Virtual tracking device to generate random positions
  mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New();
  tracker->AddTool("tool1");
  mitk::ScalarType bounds[] = {0.0, 200.0, 0.0, 200.0, 0.0, 200.0};
  tracker->SetBounds(bounds);

  //Tracking device source to get the data
  mitk::TrackingDeviceSource::Pointer source = mitk::TrackingDeviceSource::New();
  source->SetTrackingDevice(tracker);
  source->Connect();

  //Cone representation for rendering
  mitk::Cone::Pointer cone = mitk::Cone::New();
  double scale[] = {10.0, 10.0, 10.0};
  cone->GetGeometry()->SetSpacing(scale);
  dataNode->SetData(cone);
  dataNode->SetName("My tracked object");
  dataNode->SetColor(0.0, 1.0, 1.0); //sandy wants cyan
  dataStorage->Add(dataNode);

  //Filter for rendering the cone at correct postion and orientation
  mitk::NavigationDataObjectVisualizationFilter::Pointer visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
  visualizer->SetInput(0, source->GetOutput());
  visualizer->SetRepresentationObject(0, cone);
  source->StartTracking();

  //Generate and render 100 time steps
  for(int i=0; i < 100; ++i)
  {
    //Update the cone position
    visualizer->Update();

    //Update rendering & Global reinit
    renderWindow->GetVtkRenderWindow()->Render();
    mitk::TimeSlicedGeometry::Pointer geo = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
    mitk::RenderingManager::GetInstance()->InitializeViews( geo );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    MITK_INFO << "Position " << source->GetOutput()->GetPosition();
    //Slight delay for the random numbers
    itksys::SystemTools::Delay(100);
  }
  //Stop the tracking device and disconnect it
  source->StopTracking();
  source->Disconnect();
}
