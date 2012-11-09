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

#include <mitkSTLFileReader.h>
#include <mitkSurface.h>
#include <mitkStandaloneDataStorage.h>

#include <itksys/SystemTools.hxx>
#include <QApplication>

//for the interaction
#include <mitkMoveSurfaceInteractor.h>
#include "mitkGlobalInteraction.h"

//##Documentation
//## @brief Load two or more surfaces (stl format, see e.g. Core/Code/Testing/data directory for binary.stl) and display it in a 3D view.
//## The MoveSurfaceInteractor explained in tutorial Step10.dox is used to move the surfaces in 3D by arrow keys in combination
//## with and without Shift key. Use two surfaces to see that the objects and not the camera are moving.
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
  // Part II: Create surface data by reading an stl file
  //*************************************************************************

  for(int i=1; i<argc; ++i)
  {
    // For testing
    if(strcmp(argv[i], "-testing")==0) continue;

    // Create a STLFileReader to read a .stl-file
    mitk::STLFileReader::Pointer reader = mitk::STLFileReader::New();
    const char * filename = argv[i];
    try
    {
      reader->SetFileName(filename);
      reader->Update();
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }

    //*************************************************************************
    // Part III: Put the data into the datastorage
    //*************************************************************************

    // Create a node and add the Image (which is read from the file) to it
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(reader->GetOutput());

    // *******************************************************
    // ****************** START OF NEW PART ******************
    // *******************************************************

    // create interactor
    // use it with up, down (->z direction), left and right (x-direction) arrow keys. Also hold Shift to translate in y direction.
    // see state machine pattern SelectAndMoveObjectWithArrowKeys in file StateMachine.xml for definition of interaction or use the StatemachineEditor.
    mitk::MoveSurfaceInteractor::Pointer surfaceInteractor =
      mitk::MoveSurfaceInteractor::New("SelectAndMoveObjectWithArrowKeys",node);

    //activate interactor at interaction controller:
    mitk::GlobalInteraction::GetInstance()->AddInteractor(surfaceInteractor);

    // *******************************************************
    // ******************* END OF NEW PART *******************
    // *******************************************************

    // Add the node to the DataStorage
    ds->Add(node);

    //doesn't have to be done, but nicer! Is destroyed when leaving the sccope anyway
    reader = NULL;
    surfaceInteractor = NULL;
  }

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

  MITK_INFO<<"Select an object with a mouse click. Use arrow keys (also with shift-key) to move the surface.\n";
  MITK_INFO<<"Deselecting and selecting an other surface by clicking onto it. Selected surfaces turn yellow, deselected blue.\n";

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
