/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <mitkRenderWindow.h>
#include <mitkImageMapper2D.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>

#include <mitkNativeRenderWindowInteractor.h>


#include "itkTextOutput.h"

#include <fstream>
int mitkDataTreeTest(int argc, char* argv[])
{
  itk::OutputWindow::SetInstance(
       itk::TextOutput::New().GetPointer() );

  //Create Image out of nowhere
	mitk::Image::Pointer image;
	mitk::PixelType pt(typeid(int));
	unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
	image=mitk::Image::New();
  image->DebugOn();
	image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  int *p = (int*)image->GetData();

  int size = dim[0]*dim[1]*dim[2];
  int i;
  for(i=0; i<size; ++i, ++p)
    *p=i;
  std::cout<<"[PASSED]"<<std::endl;

  {
    std::cout << "Creating node: ";
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData(image);
    std::cout<<"[PASSED]"<<std::endl;
  }

  std::cout << "Creating node: ";
  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
  node->SetData(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating tree: ";
  mitk::DataTree::Pointer tree;
  tree=mitk::DataTree::New(); //@FIXME: da DataTreeIteratorClone keinen Smartpointer auf DataTree hält, wird tree sonst gelöscht.
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

  std::cout << "Remove node after incrementing iterator: ";
  it++;
  if (it.Remove()) std::cout<<"[PASSED]"<<std::endl;
  else std::cout<<"[FAILED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
