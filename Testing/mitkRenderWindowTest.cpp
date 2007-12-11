/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <mitkRenderWindow.h>
#include <mitkVtkRenderWindow.h>
#include <mitkNativeRenderWindowInteractor.h>

#include <mitkOpenGLRenderer.h>

#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>


#include <fstream>
int mitkRenderWindowTest(int /*argc*/, char* /*argv*/[])
{
  //Create a (native) mitk::RenderWindow
  std::cout << "Testing creation of RenderWindow: ";
  mitk::RenderWindow* renderwindow = new mitk::RenderWindow("the render window");
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing InitRenderer(): ";
  renderwindow->InitRenderer();
  std::cout<<"[PASSED]"<<std::endl;

  // use a source to create a vtkPolyData with a cone …
  vtkConeSource * cone = vtkConeSource::New();

  // creating the visualization objects (mapper, actor) …
  vtkPolyDataMapper * coneMapper = vtkPolyDataMapper::New();
	  coneMapper->SetInput(cone->GetOutput());
  vtkActor * coneActor = vtkActor::New();
	  coneActor->SetMapper(coneMapper);
 
  std::cout << "Testing whether GetRenderer() returns an mitk::OpenGLRenderer: ";
  mitk::OpenGLRenderer* glrenderer = dynamic_cast<mitk::OpenGLRenderer*>(renderwindow->GetRenderer());
  if(glrenderer==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  vtkRenderer * ownvtkrenderer = vtkRenderer::New();
  renderwindow->GetVtkRenderWindow()->AddRenderer(ownvtkrenderer);
 // assign the actor to the rendering environment
  
#if (VTK_MAJOR_VERSION < 5)
  ownvtkrenderer->AddProp(coneActor);
#else
  ownvtkrenderer->AddViewProp(coneActor);
#endif

  // interact with data
  mitk::NativeRenderWindowInteractor::Pointer iren = mitk::NativeRenderWindowInteractor::New();
    iren->SetMitkRenderWindow(renderwindow);
  renderwindow->RequestUpdate();
//  iren->Start();

  cone->Delete();
  coneMapper->Delete();
  coneActor->Delete();
  ownvtkrenderer->Delete();
  delete renderwindow;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
