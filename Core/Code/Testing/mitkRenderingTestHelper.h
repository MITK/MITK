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

#ifndef mitkRenderingTestHelper_h
#define mitkRenderingTestHelper_h

#include <string>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <mitkRenderWindow.h>

class vtkRenderWindow;
class vtkRenderer;

namespace mitk
{
  class DataStorage;
}

class mitkRenderingTestHelper
{
  public:
    mitkRenderingTestHelper(int width, int height, mitk::DataStorage* ds);

    ~mitkRenderingTestHelper();

    vtkRenderer* GetVtkRenderer();
    vtkRenderWindow* GetVtkRenderWindow();
    void SaveAsPNG(std::string fileName);

  protected:
    mitk::RenderWindow::Pointer m_RenderWindow;

};

#endif

