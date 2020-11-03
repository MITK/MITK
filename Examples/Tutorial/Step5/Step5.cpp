/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkPointSet.h"
// NEW INCLUDE
#include "mitkPointSetDataInteractor.h"

#include <QApplication>
#include <QHBoxLayout>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>

//##Documentation
//## @brief Interactively add points
//##
//## As in Step4, load one or more data sets (many image,
//## surface and other formats) and create 3 views on the data.
//## Additionally, we want to interactively add points. A node containing
//## a PointSet as data is added to the data tree and a PointSetDataInteractor
//## is associated with the node, which handles the interaction. The
//## @em interaction @em pattern is defined in a state-machine, stored in an
//## external XML file. Thus, we need to load a state-machine
//## The interaction patterns defines the @em events,
//## on which the interactor reacts (e.g., which mouse buttons are used to
//## set a point), the @em transition to the next state (e.g., the initial
//## may be "empty point set") and associated @a actions (e.g., add a point
//## at the position where the mouse-click occured).
int main(int argc, char *argv[])
{
  QApplication qtapplication(argc, argv);

  if (argc < 2)
  {
    fprintf(
      stderr, "Usage:   %s [filename1] [filename2] ...\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str());
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  // Create a DataStorage
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for (i = 1; i < argc; ++i)
  {
    // For testing
    if (strcmp(argv[i], "-testing") == 0)
      continue;

    // Load datanode (eg. many image formats, surface formats, etc.)
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(argv[i], *ds);

    //*********************************************************************
    // Part III: Put the data into the datastorage
    //*********************************************************************
    // Add the node to the DataStorage
    if (dataNodes->empty())
    {
      fprintf(stderr, "Could not open file %s \n\n", argv[i]);
      exit(2);
    }
  }

  //*************************************************************************
  // Part V: Create windows and pass the tree to it
  //*************************************************************************

  // Create toplevel widget with horizontal layout
  QWidget toplevelWidget;
  QHBoxLayout layout;
  layout.setSpacing(2);
  layout.setMargin(0);
  toplevelWidget.setLayout(&layout);

  //*************************************************************************
  // Part Va: 3D view
  //*************************************************************************

  // Create a renderwindow
  QmitkRenderWindow renderWindow(&toplevelWidget);
  layout.addWidget(&renderWindow);

  // Tell the renderwindow which (part of) the tree to render

  renderWindow.GetRenderer()->SetDataStorage(ds);

  // Use it as a 3D view
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Reposition the camera to include all visible actors
  renderWindow.GetRenderer()->GetVtkRenderer()->ResetCamera();

  //*************************************************************************
  // Part Vb: 2D view for slicing axially
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view2(&toplevelWidget);
  layout.addWidget(&view2);

  // Tell the QmitkSliceWidget which (part of) the tree to render.
  // By default, it slices the data axially
  view2.SetDataStorage(ds);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetSubset(mitk::TNodePredicateDataType<mitk::Image>::New());
  view2.SetData(rs->Begin(), mitk::SliceNavigationController::Axial);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  ds->Add(view2.GetRenderer()->GetCurrentWorldPlaneGeometryNode());

  //*************************************************************************
  // Part Vc: 2D view for slicing sagitally
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view3(&toplevelWidget);
  layout.addWidget(&view3);

  // Tell the QmitkSliceWidget which (part of) the tree to render
  // and to slice sagitall
  view3.SetDataStorage(ds);
  view3.SetData(rs->Begin(), mitk::SliceNavigationController::Sagittal);

  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  ds->Add(view3.GetRenderer()->GetCurrentWorldPlaneGeometryNode());

  // *******************************************************
  // ****************** START OF NEW PART ******************
  // *******************************************************

  //*************************************************************************
  // Part VI: For allowing to interactively add points ...
  //*************************************************************************

  // ATTENTION: It is very important that the renderer already know their DataStorage,
  // because registerig DataInteractors with the render windows is done automatically
  // and only works if the BaseRenderer and the DataStorage know each other.

  // Create PointSet and a node for it
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  // Store the point set in the DataNode
  pointSetNode->SetData(pointSet);

  // Add the node to the tree
  ds->Add(pointSetNode);

  // Create PointSetDataInteractor
  mitk::PointSetDataInteractor::Pointer interactor = mitk::PointSetDataInteractor::New();
  // Set the StateMachine pattern that describes the flow of the interactions
  interactor->LoadStateMachine("PointSet.xml");
  // Set the configuration file, which describes the user interactions that trigger actions
  // in this file SHIFT + LeftClick triggers add Point, but by modifying this file,
  // it could as well be changes to any other user interaction.
  interactor->SetEventConfig("PointSetConfig.xml");

  // Assign the pointSetNode to the interactor,
  // alternatively one could also add the DataInteractor to the pointSetNode using the SetDataInteractor() method.
  interactor->SetDataNode(pointSetNode);

  // *******************************************************
  // ******************* END OF NEW PART *******************
  // *******************************************************

  //*************************************************************************
  // Part VII: Qt-specific initialization
  //*************************************************************************
  toplevelWidget.show();

  return qtapplication.exec();
}
/**
\example Step5.cpp
*/
