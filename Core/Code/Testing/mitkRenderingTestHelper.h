/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 17:34:48 +0200 (Wed, 31 Mar 2010) $
Version:   $Revision: 21985 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

class MITK_CORE_EXPORT mitkRenderingTestHelper
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

