/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

#include "mitkDataNodeFactory.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkGlobalInteraction.h"
#include "mitkPointSet.h"
#include "mitkPointSetInteractor.h"

#include <itksys/SystemTools.hxx>
#include <QApplication>
#include <QHBoxLayout>

//##Documentation
//## @brief Interactively add points
//##
//## As in Step4, load one or more data sets (many image,
//## surface and other formats) and create 3 views on the data.
//## Additionally, we want to interactively add points. A node containing
//## a PointSet as data is added to the data tree and a PointSetInteractor
//## is associated with the node, which handles the interaction. The
//## @em interaction @em pattern is defined in a state-machine, stored in an
//## external XML file. Thus, we need to load a state-machine
//## The interaction patterns defines the @em events,
//## on which the interactor reacts (e.g., which mouse buttons are used to
//## set a point), the @em transition to the next state (e.g., the initial
//## may be "empty point set") and associated @a actions (e.g., add a point
//## at the position where the mouse-click occured).
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
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
  for(i=1; i<argc; ++i)
  {
    // For testing
    if(strcmp(argv[i], "-testing")==0) continue;

    // Create a DataNodeFactory to read a data format supported
    // by the DataNodeFactory (many image formats, surface formats, etc.)
    mitk::DataNodeFactory::Pointer nodeReader=mitk::DataNodeFactory::New();
    const char * filename = argv[i];
    try
    {
      nodeReader->SetFileName(filename);
      nodeReader->Update();
      //*********************************************************************
      // Part III: Put the data into the datastorage
      //*********************************************************************

      // Since the DataNodeFactory directly creates a node,
      // use the iterator to add the read node to the tree
      mitk::DataNode::Pointer node = nodeReader->GetOutput();
      ds->Add(node);
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
  }

  // *******************************************************
  // ****************** START OF NEW PART ******************
  // *******************************************************

  //*************************************************************************
  // Part VI: For allowing to interactively add points ...
  //*************************************************************************

  // Create PointSet and a node for it
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  pointSetNode->SetData(pointSet);

  // Add the node to the tree
  ds->Add(pointSetNode);

  // Create PointSetInteractor, associate to pointSetNode and add as
  // interactor to GlobalInteraction
  mitk::GlobalInteraction::GetInstance()->AddInteractor(
    mitk::PointSetInteractor::New("pointsetinteractor", pointSetNode)
  );

  // *******************************************************
  // ******************* END OF NEW PART *******************
  // *******************************************************

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

  //*************************************************************************
  // Part Vb: 2D view for slicing transversally
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view2(&toplevelWidget);
  layout.addWidget(&view2);

  // Tell the QmitkSliceWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2.SetDataStorage(ds);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetAll();
  view2.SetData(rs->Begin(), mitk::SliceNavigationController::Transversal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  ds->Add(view2.GetRenderer()->GetCurrentWorldGeometry2DNode());

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
  ds->Add(view3.GetRenderer()->GetCurrentWorldGeometry2DNode());

  //*************************************************************************
  //Part VII: Qt-specific initialization
  //*************************************************************************
  toplevelWidget.show();

  // For testing
  #include "QtTesting.h"
  if(strcmp(argv[argc-1], "-testing")!=0)
    return qtapplication.exec();
  else
    return QtTesting();
}
/**
\example Step5.cpp
*/
