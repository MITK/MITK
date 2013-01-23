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

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkStandaloneDataStorage.h>
#include <mitkIOUtil.h>

#include <itksys/SystemTools.hxx>
#include <QApplication>

//for the interaction
#include <mitkMoveBaseDataInteractor.h>
#include "mitkGlobalInteraction.h"

//##Documentation
//## @brief Load two or more surfaces (or objects) (e.g. ../mitk-superbuild-release/CMakeExternals/Source/MITK-Data/ball.stl) and display it in a 3D view.
//## The MoveBaseDataInteractor explained in tutorial Step10.dox is used to move the surfaces in 3D by arrow keys in combination
//## with and without Shift key. Use two surfaces to see that the objects and not the camera are moving.
//## @Warning: In principle, the MoveBaseDataInteractor can move any object
//## and IOUtil can be used to load any object, however, there are no helper
//## planes in this example added to scene. Thus, no image is displayed in
//## the 3D renderwindow (and nothing can be moved/visualized/picked). The
//## data repository contains some pointsets with a single point. For unknown
//## reasons, the picking does not work on pointsets with just one point.
//## You have to use pointsets like:
//## ../mitk-superbuild-release/CMakeExternals/Source/MITK-Data/RenderingTestData/openMeAlone.mps
//## in order to perform the picking/moving.
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if (argc < 2)
  {
    fprintf( stderr, "Usage:   %s [filename_1] ... [filename_n] \n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
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

  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  //*************************************************************************
  // Part II: Create objects by reading files
  //*************************************************************************

  for(int i=1; i<argc; ++i)
  {
    //For testing
    if(strcmp(argv[i], "-testing")==0) continue;

    //create a container node for the data
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    //try to read a file
    std::string filename = argv[i];
    try
    {
        //try to read the file with IOUtil which offers lots of I/O methods
        node = mitk::IOUtil::LoadDataNode(filename);
        // Add the node to the DataStorage
        ds->Add(node);
    }
    catch(mitk::Exception &e)
    {
        MITK_ERROR << "An exception occured! Message: " << e.what();
        exit(2);
    }
    // create interactor
    // use it with up, down (->z direction), left and right (x-direction) arrow keys. Also hold Shift to translate in y direction.
    // see state machine pattern SelectAndMoveObjectWithArrowKeys in file StateMachine.xml for definition of interaction or use the StatemachineEditor.
    mitk::MoveBaseDataInteractor::Pointer moveBaseDataInteractor =
      mitk::MoveBaseDataInteractor::New("SelectAndMoveObjectWithArrowKeys",node);

    //activate interactor at interaction controller:
    mitk::GlobalInteraction::GetInstance()->AddInteractor(moveBaseDataInteractor);

    //doesn't have to be done, but nicer! Is destroyed when leaving the sccope anyway
    moveBaseDataInteractor = NULL;
  }
  // *******************************************************
  // ******************* END OF NEW PART *******************
  // *******************************************************

  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(ds);

  // Use it as a 3D view
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Initialize the RenderWindow
  mitk::TimeSlicedGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  renderWindow.show();
  renderWindow.resize( 256, 256 );

  MITK_INFO<<"Select an object with a mouse click. Use arrow keys (also with shift-key) to move the object.\n";
  MITK_INFO<<"Deselecting and selecting an other object by clicking onto it. Selected objects turn green, deselected blue.\n";

  // for testing
  #include "QtTesting.h"
  if (strcmp(argv[argc-1], "-testing") != 0)
    return qtapplication.exec();
  else
    return QtTesting();

  // cleanup: Remove References
  ds = NULL;
}
/**
\example Step10.cpp
*/
