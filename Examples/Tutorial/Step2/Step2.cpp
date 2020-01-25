/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkStandaloneDataStorage.h>

#include <QApplication>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>

//##Documentation
//## @brief Load one or more data sets (many image, surface
//## and other formats) and display it in a 2D view
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

  // Create a data storage object. We will use it as a singleton
  mitk::StandaloneDataStorage::Pointer storage = mitk::StandaloneDataStorage::New();

  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for (i = 1; i < argc; ++i)
  {
    // For testing
    if (strcmp(argv[i], "-testing") == 0)
      continue;

    //*********************************************************************
    // Part III: Put the data into the datastorage
    //*********************************************************************
    // Add the node to the DataStorage
    mitk::IOUtil::Load(argv[i], *storage);
  }

  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(storage);

  // Initialize the RenderWindow
  auto geo = storage->ComputeBoundingGeometry3D(storage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  // Select a slice
  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  if (sliceNaviController)
    sliceNaviController->GetSlice()->SetPos(2);

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  renderWindow.show();
  renderWindow.resize(256, 256);

  return qtapplication.exec();
}

/**
\example Step2.cpp
*/
