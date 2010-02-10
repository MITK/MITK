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
#include "vtkUnsignedCharArray.h"

#include <mitkPicFileReader.h>
#include <mitkImage.h>

#include <mitkDataStorage.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>

#include <mitkVtkPropRenderer.h>
#include <vtkMitkRenderProp.h>
#include <mitkVtkLayerController.h>
#include <mitkNativeRenderWindowInteractor.h>  // Needs MitkExt!

#include <itksys/SystemTools.hxx>

#include <fstream>
int mitkVtkPropRendererTest(int argc, char* argv[])
{
  //independently read header of pic file
  mitkIpPicDescriptor *picheader = NULL;
  if (argc >= 1)
  {
    if(itksys::SystemTools::LowerCase(itksys::SystemTools::GetFilenameExtension(argv[1])).find(".pic")!=std::string::npos)
      picheader = mitkIpPicGetHeader(argv[1], NULL);
  }
  if( picheader==NULL)
  {
    std::cout<<"file not found/not a pic-file - test not applied [PASSED]"<<std::endl;
    std::cout<<"[TEST DONE]"<<std::endl;
    return EXIT_SUCCESS;
  }

  static long int sum_orig_Pic3D_pic_gz = 14685408;

  std::string argv_str(argv[1]);

  mitkIpPicGetTags(argv[1], picheader);

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

  std::cout << "Creating DataStorage: ";
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Adding node via DataStorage: ";
  ds->Add(node);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Adding level-window property: ";
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
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
  sphere->Delete();

  vtkActor *aSphere = vtkActor::New();
  aSphere->SetMapper(map);
  map->Delete();
  aSphere->GetProperty()->SetColor(0,0,1); // sphere color blue
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating a renderer for the sphere: ";
  vtkRenderer *sphereRenderer = vtkRenderer::New();
  sphereRenderer->AddActor(aSphere);
  aSphere->Delete();
  //sphereRenderer->SetBackground(1,1,1); // Background color white
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating vtkRenderWindow and VtkPropRenderer: ";
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer propRenderer = mitk::VtkPropRenderer::New( "the renderer", renderWindow );
  //propRenderer->SetMapperID(2);
  std::cout<<"[PASSED]"<<std::endl;

  //renderWindow->AddRenderer(sphereRenderer);
  //renderWindow->SetErase(0);

  std::cout << "BaseRenderer::SetData(iterator): ";
  propRenderer->SetDataStorage(ds);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating vtkMitkRenderProp and connecting it to the VtkPropRenderer: ";
  vtkMitkRenderProp* renderProp = vtkMitkRenderProp::New();
  renderProp->SetPropRenderer(propRenderer);
  propRenderer->GetVtkRenderer()->AddViewProp(renderProp);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Inserting the sphere into the foreground of the VtkLayerController: ";
  mitk::VtkLayerController::GetInstance(renderWindow)->InsertForegroundRenderer(sphereRenderer,true);
  std::cout<<"[PASSED]"<<std::endl;

  // mouse interaction for debugging
  //vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  //iren->SetRenderWindow(renderWindow);

  std::cout << "Setting and getting size of renderWindow: ";
  renderWindow->SetSize(400, 400);
  int *size = renderWindow->GetSize();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Do the rendering: ";
  renderWindow->Render();
  std::cout<<"[PASSED]"<<std::endl;

  //iren->Start();

  std::cout << "Testing to pick a world position: ";
  mitk::Point2D p;
  mitk::Point3D p_mm;
  p[0] = 10;
  p[1] = 10;
  propRenderer->PickWorldPoint(p, p_mm);
  std::cout << "returned world position: " << p_mm << "\n";

  std::cout << "Creating vtkUnsignedCharArray: ";
  vtkUnsignedCharArray *vtkImage = vtkUnsignedCharArray::New();
  std::cout<<"[PASSED]"<<std::endl;

  cout << "Reading image from renderWindow" << std::endl;
  renderWindow->GetRGBACharPixelData(0, 0, size[0]-1, size[1]-1, 0, vtkImage);
  cout << "Read " << size[0]*size[1] << " data points\n";

  cout << "Computing sum of all RGBA values..\n";
  long int sum_now = 0;
  for(int i=0;i<size[0]*size[1];i++) 
    sum_now += vtkImage->GetValue(i);
  std::cout << "Sum of all RGBA values: " << sum_now << "\n";
  std::cout << "Sum should be: " << sum_orig_Pic3D_pic_gz << "\n";

  //std::string Pic3d_pic_gz_str("Pic3D.pic.gz");
  //std::cout << "pic3d " << Pic3d_pic_gz_str << "\n";
  //std::cout << "argv " << argv_str << "\n";
  //std::cout << "find " << (int) argv_str.find("Pic3D.pic.gz") << "\n";
  //std::cout << "size " << argv_str.size() << "\n";

  //if(argv_str.size() - ((int) argv_str.find("Pic3D.pic.gz")) == 12)
  //{
  //  std::cout << "Input image is Pic3D.pic.gz\n";    
  //  std::cout << "Sum should be: " << sum_orig_Pic3D_pic_gz << "\n";
  //  if(sum_orig_Pic3D_pic_gz!=sum_now)
  //  {
  //    std::cout<<"[FAILED]"<<std::endl;
  //    return EXIT_FAILURE;
  //  }
  //  std::cout<<"[PASSED]"<<std::endl;
  //}
  //else
  //{
  //  std::cout<<"Unknown image, comparison test skipped"<<std::endl;
  //}


  propRenderer->GetVtkRenderer()->RemoveViewProp(renderProp);
  renderProp->Delete();
  propRenderer = NULL;
  sphereRenderer->Delete();

  renderWindow->Delete();

  vtkImage->Delete();
  ds = NULL;
  
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}


