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

//! [includes]
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkDataNodeFactory.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>

#include <itksys/SystemTools.hxx>
#include <QApplication>
#include <mitkOverlay2DLayouter.h>
#include <mitkOverlayManager.h>
#include <mitkTextOverlay2D.h>
#include <mitkTextOverlay3D.h>
//! [includes]

//##Documentation
//## @brief Load image (nrrd format) and display it in a 2D view
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if (argc < 2)
  {
    fprintf( stderr, "Usage:   %s [filename] \n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  // Create a DataStorage
  // The DataStorage manages all data objects. It is used by the
  // rendering mechanism to render all data objects
  // We use the standard implementation mitk::StandaloneDataStorage.
  mitk::StandaloneDataStorage::Pointer dataStorage = mitk::StandaloneDataStorage::New();


  //*************************************************************************
  // Part II: Create some data by reading a file
  //*************************************************************************

  // Create a DataNodeFactory to read a data format supported
  // by the DataNodeFactory (many image formats, surface formats, etc.)
  mitk::DataNodeFactory::Pointer reader=mitk::DataNodeFactory::New();
  const char * filename = argv[1];
  try
  {
    reader->SetFileName(filename);
    reader->Update();
    //*************************************************************************
    // Part III: Put the data into the datastorage
    //*************************************************************************

    // Add the node to the DataStorage
    dataStorage->Add(reader->GetOutput());
  }
  catch(...)
  {
    fprintf( stderr, "Could not open file %s \n\n", filename );
    exit(2);
  }

  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(dataStorage);

  // Initialize the RenderWindow
  mitk::TimeGeometry::Pointer geo = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );
  //mitk::RenderingManager::GetInstance()->InitializeViews();

  // Select a slice
  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  if (sliceNaviController)
    sliceNaviController->GetSlice()->SetPos( 0 );

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************

  //! [CreateOverlayManager]
  mitk::OverlayManager::Pointer OverlayManagerInstance = mitk::OverlayManager::New();
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderWindow.GetVtkRenderWindow());
  renderer->SetOverlayManager(OverlayManagerInstance);
  //! [CreateOverlayManager]


  //! [GetOverlayManagerInstance]
  //The id that is passed identifies the correct mitk::OverlayManager and is '0' by default.
  mitk::BaseRenderer* renderer2D = mitk::BaseRenderer::GetInstance(renderWindow.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer overlayManager = renderer2D->GetOverlayManager();
  //! [GetOverlayManagerInstance]


  //! [AddLayouter]
  //This creates a 2DLayouter that is only active for the recently fetched axialRenderer and positione
  mitk::Overlay2DLayouter::Pointer topleftLayouter = mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(), renderer2D);

  //Now, the created Layouter is added to the OverlayManager and can be referred to by its identification string.
  overlayManager->AddLayouter(topleftLayouter.GetPointer());

  //Several other Layouters can be added to the overlayManager
  mitk::Overlay2DLayouter::Pointer bottomLayouter = mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM(), renderer2D);
  overlayManager->AddLayouter(bottomLayouter.GetPointer());
  //! [AddLayouter]


  //! [TextOverlay2D]
  //Create a textOverlay2D
  mitk::TextOverlay2D::Pointer textOverlay = mitk::TextOverlay2D::New();

  textOverlay->SetText("Test!"); //set UTF-8 encoded text to render
  textOverlay->SetFontSize(40);
  textOverlay->SetColor(1,0,0); //Set text color to red
  textOverlay->SetOpacity(1);

  //The position of the Overlay can be set to a fixed coordinate on the display.
  mitk::Point2D pos;
  pos[0] = 10,pos[1] = 20;
  textOverlay->SetPosition2D(pos);

  //Add the overlay to the overlayManager. It is added to all registered renderers automaticly
  overlayManager->AddOverlay(textOverlay.GetPointer());

  //Alternatively, a layouter can be used to manage the position of the overlay. If a layouter is set, the absolute position of the overlay is not used anymore
  //The Standard TopLeft Layouter has to be registered to the OverlayManager first
  overlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(), renderer2D).GetPointer());
  //! [TextOverlay2D]

  //! [SetLayouterToOverlay]
  //Because a Layouter is specified by the identification string AND the Renderer, both have to be passed to the call.
  overlayManager->SetLayouter(textOverlay.GetPointer(),mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(),renderer2D);
  //! [SetLayouterToOverlay]


  //! [TextOverlay3D]
  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  // This vector is used to define an offset for the annotations, in order to show them with a margin to the actual coordinate.
  mitk::Point3D offset;
  offset[0] = .5;
  offset[1] = .5;
  offset[2] = .5;

  //Just a loop to create some points
  for(int i=0 ; i < 10 ; i++){
    //To each point, a TextOverlay3D is created
    mitk::TextOverlay3D::Pointer textOverlay3D = mitk::TextOverlay3D::New();
    mitk::Point3D point;
    point[0] = i*20;
    point[1] = i*30;
    point[2] = -i*50;
    pointset->InsertPoint(i, point);
    textOverlay3D->SetText("A Point");

    // The Position is set to the point coordinate to create an annotation to the point in the PointSet.
    textOverlay3D->SetPosition3D(point);

    // move the annotation away from the actual point
    textOverlay3D->SetOffsetVector(offset);

    overlayManager->AddOverlay(textOverlay3D.GetPointer());
  }

  // also show the created pointset
  mitk::DataNode::Pointer datanode = mitk::DataNode::New();
  datanode->SetData(pointset);
  datanode->SetName("pointSet");
  dataStorage->Add(datanode);
  //! [TextOverlay3D]
  renderWindow.show();
  renderWindow.resize( 256, 256 );

  return qtapplication.exec();

  // cleanup: Remove References to DataStorage. This will delete the object
  dataStorage = NULL;
}
/**
\example Step1.cpp
*/