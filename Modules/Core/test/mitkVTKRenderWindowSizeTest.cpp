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

#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include "mitkTestingMacros.h"

void CheckRWSize(int width, int height)
{
  MITK_TEST_OUTPUT(<< "Requesting VTK render window of size " << width << "x" << height);
  vtkSmartPointer<vtkRenderWindow> renderWindowVTK = vtkRenderWindow::New();
  renderWindowVTK->SetSize(width, height);
  renderWindowVTK->SetOffScreenRendering(1); // seems to be related to off-screen rendering (only?)

  int *renderWindowSize = renderWindowVTK->GetSize();

  MITK_TEST_CONDITION(renderWindowSize[0] >= width,
                      "  Window is at least " << width << "px wide (actually:" << renderWindowSize[0] << ")");
  MITK_TEST_CONDITION(renderWindowSize[1] >= height,
                      "  Window is at least " << height << "px high (actually:" << renderWindowSize[1] << ")");
}

/**
  Check if a patch to VTK is applied as in http://paraview.org/Bug/view.php?id=14122

  Bug description from there:

  "
  After moving over to Visual Studio 2012, we found a strange problem where vtkRenderWindow GetSize()
  returns something different than what was provided to SetSize().
  Specifically, width and height are 8 pixels less than expected.
  I found a problem report which appears to describe what we are seeing. The problem goes away if we change the desktop
  theme to Windows Classic.
  "

  the patch from this bug should be applied to MITK's default VTK version.

  Test can be removed with VTK 6.
*/
int mitkVTKRenderWindowSizeTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("mitkVTKRenderWindowSizeTest")

  // some power of two default sizes
  CheckRWSize(128, 128);
  CheckRWSize(256, 256);
  CheckRWSize(512, 512);

  // some other size
  CheckRWSize(300, 200);
  CheckRWSize(150, 243);

  MITK_TEST_END()
}
