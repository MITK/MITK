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
int mitkRenderWindowTest(int argc, char* argv[])
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

  std::cout << "Testing glrenderer->GetVtkRenderer()==NULL: ";
  if(glrenderer->GetVtkRenderer()!=NULL)
  {
    std::cout<<"[FAILED] - Not really a failure! Event better like that it is now != NULL! " << std::endl
      << "In former times  it was NULL without presence of a vtk-based Mapper." <<std::endl
      << "Please remove this part of the test and check, whether the rest of the test show reasonable results." << std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  vtkRenderer * ownvtkrenderer = vtkRenderer::New();
  renderwindow->GetVtkRenderWindow()->AddRenderer(ownvtkrenderer);
 // assign the actor to the rendering environment
  ownvtkrenderer->AddProp(coneActor);

  // interact with data
  mitk::NativeRenderWindowInteractor::Pointer iren = mitk::NativeRenderWindowInteractor::New();
    iren->SetMitkRenderWindow(renderwindow);
  renderwindow->Update();
//  iren->Start();

  cone->Delete();
  coneMapper->Delete();
  coneActor->Delete();
  delete renderwindow;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
