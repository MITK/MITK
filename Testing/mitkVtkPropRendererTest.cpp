/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13308 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"

#include <mitkPicFileReader.h>
#include <mitkImage.h>
#include <mitkDataTree.h>
//#include <mitkImageMapper2D.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>

#include <mitkVtkPropRenderer.h>
#include <vtkMitkRenderProp.h>
#include <mitkVtkLayerController.h>
#include <mitkNativeRenderWindowInteractor.h>

#include <fstream>
int mitkVtkPropRendererTest(int argc, char* argv[])
{
  //Read pic-Image from file
  std::cout << "Reading image: ";
	mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();
  reader->SetFileName(argv[1]);
  reader->Update();
  std::cout<<"[PASSED]"<<std::endl;

  mitk::Image::Pointer image = reader->GetOutput();

  std::cout << "Creating node: ";
  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
  node->SetData(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating tree: ";
  mitk::DataTree* tree;
  (tree=mitk::DataTree::New())->Register(); //@FIXME: da DataTreeIteratorClone keinen Smartpointer auf DataTree hält, wird tree sonst gelöscht.
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating iterator on tree: ";
  mitk::DataTreePreOrderIterator it(tree);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Adding node via iterator: ";
  it.Add(node);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Adding level-window property: ";
  mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( image );
  levWinProp->SetLevelWindow( levelwindow );
  node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating a vtk sphere: ";
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetRadius(1.0);
  sphere->SetThetaResolution(18);
  sphere->SetPhiResolution(18);

  vtkPolyDataMapper *map = vtkPolyDataMapper::New();
  map->SetInput(sphere->GetOutput());

  vtkActor *aSphere = vtkActor::New();
  aSphere->SetMapper(map);
  aSphere->GetProperty()->SetColor(0,0,1); // sphere color blue
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating a renderer for the sphere: ";
  vtkRenderer *sphereRenderer = vtkRenderer::New();
  sphereRenderer->AddActor(aSphere);
  //sphereRenderer->SetBackground(1,1,1); // Background color white
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating vtkRenderWindow and VtkPropRenderer: ";
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  mitk::VtkPropRenderer *propRenderer = new mitk::VtkPropRenderer( "the renderer", renderWindow );
  //propRenderer->SetMapperID(2);
  std::cout<<"[PASSED]"<<std::endl;

  //renderWindow->AddRenderer(sphereRenderer);
  //renderWindow->SetErase(0);

  std::cout << "BaseRenderer::SetData(iterator): ";
  propRenderer->SetData(&it);
  std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing if an mitk::ImageMapper2D was created: ";
  //if(dynamic_cast<mitk::ImageMapper2D*>(node->GetMapper(1))==NULL)
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating vtkMitkRenderProp and connecting it to the VtkPropRenderer: ";
  vtkMitkRenderProp* renderProp = vtkMitkRenderProp::New();
  renderProp->SetPropRenderer(propRenderer);
  propRenderer->GetVtkRenderer()->AddViewProp(renderProp);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Inserting the sphere into the foreground of the VtkLayerController: ";
  mitk::VtkLayerController::GetInstance(renderWindow)->InsertForegroundRenderer(sphereRenderer,true);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Do the rendering: ";
  renderWindow->Render();
  std::cout<<"[PASSED]"<<std::endl;

  // mouse interaction for debugging
  //vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  //iren->SetRenderWindow(renderWindow);
  //iren->Start();
  
  renderWindow->Delete();
  tree = NULL; // As the tree has been registered explicitely, destroy it again.

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
