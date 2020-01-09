/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkIOUtil.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>

#include <itksys/SystemTools.hxx>

#include <QApplication>

//##Documentation
//## @brief Change the type of display to 3D
//##
//## As in Step2, load one or more data sets (many image, surface
//## and other formats), but display it in a 3D view.
//## The QmitkRenderWindow is now used for displaying a 3D view, by
//## setting the used mapper-slot to Standard3D.
//## Since volume-rendering is a (rather) slow procedure, the default
//## is that images are not displayed in the 3D view. For this example,
//## we want volume-rendering, thus we switch it on by setting
//## the Boolean-property "volumerendering" to "true".
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

    //*********************************************************************
    // Part III: Put the data into the datastorage
    //*********************************************************************
    // Load datanode (eg. many image formats, surface formats, etc.)
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(argv[i], *ds);

    if (dataNodes->empty())
    {
      fprintf(stderr, "Could not open file %s \n\n", argv[i]);
      exit(2);
    }
    mitk::DataNode::Pointer node = dataNodes->at(0);

    // *********************************************************
    // ********************* START OF NEW PART 1 (Step 3a) *****
    // *********************************************************

    //*********************************************************************
    // Part IV: We want all images to be volume-rendered
    //*********************************************************************

    // Check if the data is an image by dynamic_cast-ing the data
    // contained in the node. Warning: dynamic_cast's are rather slow,
    // do not use it too often!
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());
    if (image.IsNotNull())
    {
      // Set the property "volumerendering" to the Boolean value "true"
      node->SetProperty("volumerendering", mitk::BoolProperty::New(true));

      // Create a transfer function to assign optical properties (color and opacity) to grey-values of the data
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      tf->InitializeByMitkImage(image);

      // Set the color transfer function AddRGBPoint(double x, double r, double g, double b)
      tf->GetColorTransferFunction()->AddRGBPoint(tf->GetColorTransferFunction()->GetRange()[0], 1.0, 0.0, 0.0);
      tf->GetColorTransferFunction()->AddRGBPoint(tf->GetColorTransferFunction()->GetRange()[1], 1.0, 1.0, 0.0);

      // Set the piecewise opacity transfer function AddPoint(double x, double y)
      tf->GetScalarOpacityFunction()->AddPoint(0, 0);
      tf->GetScalarOpacityFunction()->AddPoint(tf->GetColorTransferFunction()->GetRange()[1], 1);

      node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
    }

    // *********************************************************
    // ******************* END OF NEW PART 1 (Step 3a) *********
    // *********************************************************
  }

  //*************************************************************************
  // Part V: Create window and pass the tree to it
  //*************************************************************************

  // Create a renderwindow
  QmitkRenderWindow renderWindow;

  // Tell the renderwindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(ds);

  // *********************************************************
  // ****************** START OF NEW PART 2 (Step 3b) ********
  // *********************************************************
  // Use it as a 3D view!
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Reposition the camera to include all visible actors
  renderWindow.GetRenderer()->GetVtkRenderer()->ResetCamera();

  // *********************************************************
  // ******************* END OF NEW PART 2 (Step 3b) *********
  // *********************************************************

  //*************************************************************************
  // Part VI: Qt-specific initialization
  //*************************************************************************
  renderWindow.show();
  renderWindow.resize(256, 256);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return qtapplication.exec();
}

/**
\example Step3.cpp
*/
