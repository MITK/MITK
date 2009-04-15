/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkVtkWidgetRendering.h"
#include <vtkRenderWindow.h>
#include <vtkScalarBarWidget.h>

#include "mitkTestingMacros.h"

#include <iostream>

/**
 *  Test for class mitk::VtkWidgetRendering (for rendering vtkWidgets on the
 *  screen)
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkVtkWidgetRenderingTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("ClassName")

  // Test: instantiation
  mitk::VtkWidgetRendering::Pointer widgetRendering = mitk::VtkWidgetRendering::New();
  MITK_TEST_CONDITION_REQUIRED(widgetRendering.IsNotNull(),"Testing instantiation") 

  // Test: Create and set vtkRenderWindow
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  //mitk::VtkPropRenderer::Pointer propRenderer = mitk::VtkPropRenderer::New( "the renderer", renderWindow );
  //propRenderer->SetMapperID(2);
  widgetRendering->SetRenderWindow( renderWindow );
  MITK_TEST_CONDITION_REQUIRED(widgetRendering->GetRenderWindow() == renderWindow,
    "Setting vtkRenderWindow...")

  // Test: Try to enable before widget has been set (should stay disabled)
  widgetRendering->Enable();
  MITK_TEST_CONDITION(!widgetRendering->IsEnabled(), 
    "Trying to enable widget rendering before setting widget")
  widgetRendering->Disable();

  // Test: Retrieve widget before it has been set (should return NULL)
  MITK_TEST_CONDITION(widgetRendering->GetVtkWidget() == NULL,
    "Trying to retrieve widget before it has been set")

  // Test: Create vtkWidget instance (vtkScalarWidget) and add it
  vtkScalarBarWidget *scalarBarWidget = vtkScalarBarWidget::New();
  widgetRendering->SetVtkWidget( scalarBarWidget );
  MITK_TEST_CONDITION(widgetRendering->GetVtkWidget() == scalarBarWidget,
    "Retrieving widget after it has been set")

  // Test: Try to enable widget rendering (should work now)
  widgetRendering->Enable();
  MITK_TEST_CONDITION(widgetRendering->IsEnabled(),
    "Enabling widget rendering")

  // Test: Try to disable widget rendering (should be disabled)
  widgetRendering->Disable();
  MITK_TEST_CONDITION(!widgetRendering->IsEnabled(),
    "Disabling widget rendering")




  // Clean up
  scalarBarWidget->Delete();
  renderWindow->Delete();
  

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}

