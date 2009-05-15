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


#include <mitkImage.h>
#include <mitkDataTree.h>
//#include <mitkRenderWindow.h>
#include <mitkImageMapper2D.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>
#include <mitkVtkPropRenderer.h>
#include <mitkVolumeDataVtkMapper3D.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunction.h>

#include <mitkNativeRenderWindowInteractor.h>

#include "mitkReferenceCountWatcher.h"

#include <fstream>
int mitkImageMapper2DTest(int /*argc*/, char* /*argv*/[])
{
  //Create Image out of nowhere
  mitk::Image::Pointer image;
  mitk::PixelType pt(typeid(int));
  unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
  image=mitk::Image::New();
  image->Initialize(pt, 3, dim);
  int *p = (int*)image->GetData();
  int size = dim[0]*dim[1]*dim[2];
  int i;
  for(i=0; i<size; ++i, ++p)
    *p=i;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating node: ";
  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
  node->SetData(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating DataStorage: ";
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of tree: ";
  mitk::ReferenceCountWatcher::Pointer dsWatcher = new mitk::ReferenceCountWatcher(ds, "DataStorage");
  if(dsWatcher->GetReferenceCount() != 1)
  {
    std::cout << dsWatcher->GetReferenceCount()<<"!=1 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of node: ";
  mitk::ReferenceCountWatcher::Pointer nodeWatcher = new mitk::ReferenceCountWatcher(node, "node");
  if(nodeWatcher->GetReferenceCount()!=1)
  {
    std::cout<<nodeWatcher->GetReferenceCount()<<"!=1 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
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

  std::cout << "Creating VtkPropRenderer: ";
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer propRenderer = mitk::VtkPropRenderer::New( "the renderer", renderWindow );
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "BaseRenderer::SetData(iterator): ";
  propRenderer->SetDataStorage(ds);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of node: ";
  if(nodeWatcher->GetReferenceCount()!=2)
  {
    std::cout<<nodeWatcher->GetReferenceCount()<<"!=2 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing if an mitk::ImageMapper2D was created: ";
  if(dynamic_cast<mitk::ImageMapper2D*>(node->GetMapper(mitk::BaseRenderer::Standard2D))==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing if an mitk::VolumeDataVtkMapper3D was created: ";
  if(dynamic_cast<mitk::VolumeDataVtkMapper3D*>(node->GetMapper(mitk::BaseRenderer::Standard3D))==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing if an mitk::TransferFunctionProperty was created: ";
  mitk::TransferFunctionProperty::Pointer transferFctProperty;
  if(node->GetProperty<mitk::TransferFunctionProperty>(transferFctProperty, "TransferFunction") == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing if an mitk::TransferFunctionProperty contains an mitk::TransferFunction: ";
  mitk::TransferFunction::Pointer transferFct = transferFctProperty->GetValue();
  if(transferFct.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of mitk::TransferFunctionProperty: ";
  mitk::ReferenceCountWatcher::Pointer transferFctPropertyWatcher = new mitk::ReferenceCountWatcher(transferFctProperty, "transferFctProperty");
  transferFctProperty = NULL;
  if(transferFctPropertyWatcher->GetReferenceCount()!=1)
  {
    std::cout<<transferFctPropertyWatcher->GetReferenceCount()<<"!=1 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of mitk::TransferFunctionProperty: ";
  mitk::ReferenceCountWatcher::Pointer transferFctWatcher = new mitk::ReferenceCountWatcher(transferFct, "transferFct");
  transferFct = NULL;
  if(transferFctWatcher->GetReferenceCount()!=1)
  {
    std::cout<<transferFctWatcher->GetReferenceCount()<<"!=1 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Deleting renderwindow, node and tree: ";
  renderWindow->Delete();
  node = NULL;
  ds = NULL;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of tree: ";
  if(dsWatcher->GetReferenceCount() != 0)
  {
    std::cout << ds->GetReferenceCount()<<"!=0 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of node: ";
  if(nodeWatcher->GetReferenceCount()!=0)
  {
    std::cout<<nodeWatcher->GetReferenceCount()<<"!=0 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of mitk::TransferFunctionProperty: ";
  if(transferFctPropertyWatcher->GetReferenceCount()!=0)
  {
    std::cout<<transferFctPropertyWatcher->GetReferenceCount()<<"!=0 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing reference count of mitk::TransferFunctionProperty: ";
  if(transferFctWatcher->GetReferenceCount()!=0)
  {
    std::cout<<transferFctWatcher->GetReferenceCount()<<"!=0 [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
