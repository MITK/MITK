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

// MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"
#include "mitkNodePredicateDataType.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkPlanarFigure.h"
#include "mitkSurface.h"

// ITK
#include <itkVectorContainer.h>

// VTK
#include <vtkRegressionTestImage.h>

// stdlib
#include <stdlib.h>

std::string gen_random_filename() {
  const int len = 8;
  std::string result;
  result.insert(0,len,'x');
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
    result[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return result;
}

int mitkViewportRenderingTest(int argc, char* argv[]) {
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkViewportRenderingTest")

  // enough parameters?
  if ( argc < 2 )
  {
    MITK_TEST_OUTPUT( << "Usage: " << std::string(*argv) << " [file1 file2 ...] outputfile" )
    MITK_TEST_OUTPUT( << "Will render a central axial slice of all given files into outputfile" )
    exit( EXIT_FAILURE );
  }

  double renderWindowWidth = atof(argv[1]);
  double renderWindowHeight = atof(argv[2]);
  double left =   atof(argv[3]);
  double bottom = atof(argv[4]);
  double right =  atof(argv[5]);
  double top =    atof(argv[6]);
  argv += 6; // DO NOT attempt to read these as files, this just makes no sense
  argc -= 6; // DO NOT attempt to read these as files, this just makes no sense

  MITK_INFO << "Testing viewport ("
            << left << ", "
            << bottom << ") to ("
            << right << ", "
            << top << ") "
            << "in render window of size "
            << renderWindowWidth << "x"
            << renderWindowHeight << "px";

  mitkRenderingTestHelper renderingHelper(renderWindowWidth, renderWindowHeight, argc, argv); // non-power-of-2

  /*
  //chose a level window: here we randomly chosen the blood preset.
  mitk::LevelWindowPreset* levelWindowPreset = mitk::LevelWindowPreset::New();
  bool loadedPreset = levelWindowPreset->LoadPreset();
  MITK_TEST_CONDITION_REQUIRED(loadedPreset == true, "Testing if level window preset could be loaded");
  double level = levelWindowPreset->getLevel("Blood");
  double window = levelWindowPreset->getWindow("Blood");
  //apply level window to all images
  renderingHelper.SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(level, window)) );
  */

  //for now this test renders Sagittal
  //renderingHelper.SetViewDirection(mitk::SliceNavigationController::Axial);
  renderingHelper.SetViewDirection(mitk::SliceNavigationController::Transversal);

  typedef mitk::DataStorage::SetOfObjects ObjectsSet;

  ObjectsSet::ConstPointer figures = renderingHelper.GetDataStorage()->GetSubset( mitk::TNodePredicateDataType<mitk::PlanarFigure>::New());
  for (ObjectsSet::const_iterator iterFigures = figures->begin();
       iterFigures != figures->end();
       ++iterFigures)
  {
    (*iterFigures)->SetProperty("planarfigure.default.line.color", mitk::ColorProperty::New( 1.0, 0.0, 0.0 ) ); // red
    (*iterFigures)->SetProperty("planarfigure.drawcontrolpoints", mitk::BoolProperty::New( false ) );
    (*iterFigures)->SetProperty("planarfigure.drawname", mitk::BoolProperty::New( false ) );
  }

  ObjectsSet::ConstPointer surfaces = renderingHelper.GetDataStorage()->GetSubset( mitk::TNodePredicateDataType<mitk::Surface>::New());
  for (ObjectsSet::const_iterator iterSurfaces = surfaces->begin();
       iterSurfaces != surfaces->end();
       ++iterSurfaces)
  {
    (*iterSurfaces)->SetProperty("color", mitk::ColorProperty::New( 0.0, 1.0, 0.0 ) ); // green
  }

  ObjectsSet::ConstPointer images = renderingHelper.GetDataStorage()->GetSubset( mitk::TNodePredicateDataType<mitk::Image>::New());
  for (ObjectsSet::const_iterator iterImages = images->begin();
       iterImages != images->end();
       ++iterImages)
  {
    (*iterImages)->SetProperty("levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(128.0, 256.0) ) ); // green
  }

  mitk::RenderingManager::GetInstance()->InitializeViews( renderingHelper.GetDataStorage()->ComputeBoundingGeometry3D( images ) );

  double vLeft = left / renderWindowWidth;
  double vBottom = bottom / renderWindowWidth;
  double vRight = right / renderWindowWidth;
  double vTop = top / renderWindowWidth;
  MITK_INFO << "VTK viewport ("
          << vLeft << ", "
          << vBottom << ") to ("
          << vRight << ", "
          << vTop << ")";

  renderingHelper.GetVtkRenderer()->SetViewport( vLeft, vBottom, vRight, vTop );

  renderingHelper.Render();

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = true;
  if(generateReferenceScreenshot)
  {
    std::string tmpFilename = std::string("/tmp/viewportrendering_") + gen_random_filename() + ".png";
    renderingHelper.SaveAsPNG( tmpFilename );
  }

  //### Usage of vtkRegressionTestImage:
  //vtkRegressionTestImage( vtkRenderWindow )
  //Set a vtkRenderWindow containing the desired scene.
  //vtkRegressionTestImage automatically searches in argc and argv[]
  //for a path a valid image with -V. If the test failed with the
  //first image (foo.png) check if there are images of the form
  //foo_N.png (where N=1,2,3...) and compare against them.
  int retVal = vtkRegressionTestImage( renderingHelper.GetVtkRenderWindow() );

  //retVal meanings: (see VTK/Rendering/vtkTesting.h)
  //0 = test failed
  //1 = test passed
  //2 = test not run
  //3 = something with vtkInteraction
  MITK_TEST_CONDITION( retVal == 1, "VTK rendering result matches expectation" );

  MITK_TEST_END();
}

