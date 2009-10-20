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


#include "mitkContour.h"
#include "mitkDataTreeNode.h"
#include "mitkContourMapper2D.h"
#include "mitkCommon.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkGlobalInteraction.h"

#include "mitkVtkPropRenderer.h"

#include <fstream>

int mitkContourMapper2DTest(int /*argc*/, char* /*argv*/[])
{
  mitk::Contour::Pointer contour;
  mitk::ContourMapper2D::Pointer contourMapper;
  mitk::DataTreeNode::Pointer node;

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  contourMapper = mitk::ContourMapper2D::New();
  node = mitk::DataTreeNode::New();
  std::cout << "Testing mitk::ContourMapper2D::New(): ";

  contour = mitk::Contour::New();
  node->SetData(contour);

  if (contour.IsNull()) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  }

  contourMapper->SetDataTreeNode( node );
  contourMapper->Update(NULL);
  mitk::Contour* testContour = (mitk::Contour*)contourMapper->GetInput();
  std::cout << testContour << std::endl;
  
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  ds->Add( node );

  mitk::BoundingBox::Pointer bounds = ds->ComputeBoundingBox();

  std::cout << "bounds: " << bounds << std::endl;

  bounds = ds->ComputeVisibleBoundingBox();
  std::cout << "visible bounds: " << bounds << std::endl;

  vtkRenderWindow* renWin = vtkRenderWindow::New();

  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New("ContourRenderer",renWin);

  std::cout<<"Testing mitk::BaseRenderer::SetData()"<<std::endl;

  renderer->SetDataStorage(ds);

  std::cout<<"[TEST DONE]"<<std::endl;

  renWin->Delete();

  return EXIT_SUCCESS;
}
