/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkCameraVisualization.h"
#include "mitkContour.h"
#include "mitkNavigationData.h"
#include "mitkTestingMacros.h"
#include "mitkVtkPropRenderer.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <time.h>

/**Documentation
 *  test for the class "NavigationDataVisualizationByBaseDataTransformFilter".
 */
int mitkCameraVisualizationTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("CameraVisualization")

  // let's create an object of our class  
  //mitk::Contour::Pointer contour = mitk::Contour::New();
  mitk::CameraVisualization::Pointer myFilter = mitk::CameraVisualization::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  srand(time(NULL));
  // position of navigation data
  mitk::NavigationData::PositionType position;
  position[0] = rand()%1000;
  position[1] = rand()%1000;
  position[2] = rand()%1000;

  // orientation of navigation data
  mitk::NavigationData::OrientationType orientation;
  orientation[0] = rand()%2;
  orientation[1] = rand()%2;
  orientation[2] = rand()%2;
  orientation[3] = rand()%2;

  // error of navigation data
  mitk::NavigationData::ErrorType error = rand()%10;

  // data valid flag of navigation data
  int val = rand()%2;
  bool valid;
  if (val==0)
  {
    valid=false;
  }
  else if (val==1)
  {
    valid=true;
  }
  
  // set parameters of navigation data
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(position);
  nd1->SetOrientation(orientation);
  nd1->SetError(error);
  nd1->SetDataValid(valid);

  // create renderer
  vtkRenderWindow* renderWindow = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New("TestRenderer",renderWindow);
  //vtkRenderer* vtkRenderer = vtkRenderer::New();
  //renderer->SetVtkRenderer(vtkRenderer);

  myFilter->SetInput(nd1);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "Testing Set-/GetInput() input 1");

  // test for exception if renderer not set
  MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
    myFilter->Update();
  MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

  // set renderer
  myFilter->SetRenderer(renderer);
  //Update filter
  myFilter->Update();

    // always end with this!
  MITK_TEST_END();

}


