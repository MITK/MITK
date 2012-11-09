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

#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkGlobalInteraction.h"
#include "mitkVtkPropRenderer.h"
#include "mitkStandaloneDataStorage.h"
#include "vtkRenderWindow.h"

#include "mitkTestingMacros.h"

#include <vtkCubeSource.h>
#include "mitkSurface.h"


//Propertylist Test



/**
 *  Simple example for a test for the class "RenderingManager".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
class mitkRenderingManagerTestClass
{

public:

static void TestPropertyList(mitk::RenderingManager::Pointer renderingManager)
{

  mitk::PropertyList::Pointer propertyList =  renderingManager->GetPropertyList();

  MITK_TEST_CONDITION(renderingManager->GetPropertyList().IsNotNull(), "Testing if the constructor set the propertylist" )

  //check if the default properties are set
  renderingManager->SetProperty("booltest", mitk::BoolProperty::New(true));

  mitk::BoolProperty* prop = dynamic_cast<mitk::BoolProperty*>( renderingManager->GetProperty("booltest") );

  MITK_TEST_CONDITION(prop->GetValue(), "Testing if getting the bool property" )

  MITK_TEST_CONDITION(propertyList == renderingManager->GetPropertyList(), "Testing if the propertylist has changed during the last tests" )
}

static void TestSurfaceLoading( mitk::RenderingManager::Pointer renderingManager )
{
  // create and render two dimensional surface
  vtkCubeSource* plane = vtkCubeSource::New();

  double planeBounds[] = { -1.0, 1.0, -1.0, 1.0, 0.0, 0.0 };
  double cubeBounds[] = { -0.5, 0.5, -0.5, 0.5, -0.5, 0.5 };
  plane->SetBounds( planeBounds );
  plane->SetCenter( 0.0, 0.0, 0.0 );

  vtkPolyData* polys = plane->GetOutput();
  mitk::Surface::Pointer mitkPlane = mitk::Surface::New();
  mitkPlane->SetVtkPolyData( polys );
  plane->Delete();

  mitk::DataNode::Pointer planeNode = mitk::DataNode::New();
  planeNode->SetData( mitkPlane );

  renderingManager->GetDataStorage()->Add( planeNode );

  mitk::Geometry3D::Pointer planeGeometry = mitk::Geometry3D::New();
  planeGeometry->SetFloatBounds( planeBounds );

  MITK_TEST_CONDITION( renderingManager->InitializeViews( planeGeometry ), "Testing if two dimensional Geometry3Ds can be displayed" )

  //clear rendering
  renderingManager->GetDataStorage()->Remove( planeNode );

  renderingManager->InitializeViews();

  // create and render three dimensional surface
  vtkCubeSource* cube = vtkCubeSource::New();
  cube->SetBounds( cubeBounds );
  cube->SetCenter( 0.0, 0.0, 0.0 );

  vtkPolyData* polyCube = cube->GetOutput();
  mitk::Surface::Pointer mitkCube = mitk::Surface::New();
  mitkCube->SetVtkPolyData( polyCube );
  cube->Delete();

  mitk::DataNode::Pointer cubeNode = mitk::DataNode::New();
  cubeNode->SetData( mitkCube );

  renderingManager->GetDataStorage()->Add( cubeNode );

  mitk::Geometry3D::Pointer cubeGeometry = mitk::Geometry3D::New();
  cubeGeometry->SetFloatBounds( cubeBounds );

  MITK_TEST_CONDITION( renderingManager->InitializeViews( cubeGeometry ), "Testing if three dimensional Geometry3Ds can be displayed" )

    //clear rendering
  renderingManager->GetDataStorage()->Remove( cubeNode );

  renderingManager->InitializeViews();

}

static void TestAddRemoveRenderWindow()
{
  mitk::RenderingManager::Pointer myRenderingManager = mitk::RenderingManager::New();


  //mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  //myRenderingManager->SetDataStorage(ds);

  {
    vtkRenderWindow* vtkRenWin = vtkRenderWindow::New();

    MITK_TEST_CONDITION_REQUIRED(myRenderingManager->GetAllRegisteredRenderWindows().size() == 0, "Render window list must be empty")

    // Add Render Window
    myRenderingManager->AddRenderWindow(vtkRenWin);

    MITK_TEST_CONDITION_REQUIRED(myRenderingManager->GetAllRegisteredRenderWindows().size() == 1, "Render window list must contain one item")

    // Remove Render Window
    myRenderingManager->RemoveRenderWindow(vtkRenWin);

    MITK_TEST_CONDITION_REQUIRED(myRenderingManager->GetAllRegisteredRenderWindows().size() == 0, "Render window list must be empty")

    // Test if the Render Window was removed properly. This should not do anything
    MITK_TEST_OUTPUT(<< "Call RequestUpdate on removed render window")
    myRenderingManager->RequestUpdate(vtkRenWin);
    MITK_TEST_OUTPUT(<< "Call ForceImmediateUpdate on removed render window")
    myRenderingManager->ForceImmediateUpdate(vtkRenWin);

    MITK_TEST_CONDITION_REQUIRED(myRenderingManager->GetAllRegisteredRenderWindows().size() == 0, "Render window list must be empty")

    // Delete vtk variable correctly
    vtkRenWin->Delete();
  }

  // Check that the previous calls to RequestUpdate and ForceImmediateUpdate
  // did not modify the internal vtkRenderWindow list. This should not crash.
  MITK_TEST_OUTPUT(<< "Call RequestUpdateAll after deleting render window")
  myRenderingManager->RequestUpdateAll();
  MITK_TEST_OUTPUT(<< "Call ForceImmediateUpdateAll after deleting render window")
  myRenderingManager->ForceImmediateUpdateAll();
}

}; //mitkDataNodeTestClass
int mitkRenderingManagerTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("RenderingManager")

  mitkRenderingManagerTestClass::TestAddRemoveRenderWindow();

  mitk::RenderingManager::Pointer globalRenderingManager = mitk::RenderingManager::GetInstance();

  MITK_TEST_CONDITION_REQUIRED(globalRenderingManager.IsNotNull(),"Testing instantiation of global static instance")


  mitk::RenderingManager::Pointer myRenderingManager = mitk::RenderingManager::New();

  MITK_TEST_CONDITION_REQUIRED(myRenderingManager.IsNotNull(),"Testing instantiation of second 'local' instance")

  MITK_TEST_CONDITION_REQUIRED(myRenderingManager != globalRenderingManager ,"Testing whether global instance equals new local instance (must not be!)")


  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  mitk::StandaloneDataStorage::Pointer ds2 = mitk::StandaloneDataStorage::New();
  mitk::GlobalInteraction::Pointer gi = mitk::GlobalInteraction::New();
  gi->Initialize("global");

  myRenderingManager->SetDataStorage(ds);
  myRenderingManager->SetGlobalInteraction(gi);

  vtkRenderWindow* vtkRenWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer br = mitk::VtkPropRenderer::New("testingBR", vtkRenWin, myRenderingManager);

  mitk::BaseRenderer::AddInstance(vtkRenWin,br);
  myRenderingManager->AddRenderWindow(vtkRenWin);

  MITK_TEST_CONDITION_REQUIRED(myRenderingManager->GetDataStorage() == ds, "Testing the setter and getter for internal DataStorage")
  MITK_TEST_CONDITION_REQUIRED(myRenderingManager->GetGlobalInteraction() ==gi, "Testing the setter and getter for internal GlobalInteraction")

  MITK_TEST_CONDITION_REQUIRED(br->GetDataStorage() == ds,"Testing if internal DataStorage has been set correctly for registered BaseRenderer")
  myRenderingManager->SetDataStorage(ds2);
  MITK_TEST_CONDITION_REQUIRED(br->GetDataStorage() == ds2,"Testing if change of internal DataStorage has been forwarded correctly to registered BaseRenderer")

  mitkRenderingManagerTestClass::TestPropertyList(myRenderingManager);

  mitkRenderingManagerTestClass::TestSurfaceLoading( myRenderingManager );

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!

  //Remove Render Window
  myRenderingManager->RemoveRenderWindow(vtkRenWin);

  //Delete vtk variable correctly
  vtkRenWin->Delete();

  // always end with this!
  MITK_TEST_END()
}
