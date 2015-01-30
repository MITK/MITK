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

// Load image (nrrd format) and display it in a 2D view
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
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();


  //*************************************************************************
  // Part II: Create some data by reading a file
  //*************************************************************************

  // Load a DataNode using the mitkIOUtil
  // (supports many image formats, surface formats, etc.)
  mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(argv[1],*ds);

  if(dataNodes->empty())
  {
    fprintf( stderr, "Could not open file %s \n\n", argv[1] );
    exit(2);
  }

  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(ds);

  // Initialize the RenderWindow
  mitk::TimeGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );
  //mitk::RenderingManager::GetInstance()->InitializeViews();

  // Select a slice
  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  if (sliceNaviController)
    sliceNaviController->GetSlice()->SetPos( 0 );

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  renderWindow.show();
  renderWindow.resize( 256, 256 );

  qtapplication.exec();

  // cleanup: Remove References to DataStorage. This will delete the object
  ds = NULL;
}

