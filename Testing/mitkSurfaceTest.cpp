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


#include "mitkSurface.h"
#include "mitkCommon.h"
//#include "mitkBoundingBox.h"

#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkSphereSource.h"

#include <fstream>

int mitkSurfaceTest(int argc, char* argv[])
{
  mitk::Surface::Pointer surface;
  std::cout << "Testing mitk::Surface::New(): ";
  surface = mitk::Surface::New();
  if (surface.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 

  vtkSphereSource* sphereSource = vtkSphereSource::New();
  sphereSource->SetCenter(0,0,0);
  sphereSource->SetRadius(5.0);
  sphereSource->SetThetaResolution(10);
  sphereSource->SetPhiResolution(10);
  sphereSource->Update();

  vtkPolyData* polys = sphereSource->GetOutput();
  surface->SetVtkPolyData( polys );
  std::cout << "Testing mitk::Surface::SetVtkPolyData(): ";
  if (surface->GetVtkPolyData() ==  NULL ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 

  float bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  polys->ComputeBounds();
  polys->GetBounds( bounds );

  std::cout << "Testing GetBoundingBox() ";
  surface->UpdateOutputInformation();
  surface->SetRequestedRegionToLargestPossibleRegion();
//  mitk::BoundingBox bb = const_cast<mitk::BoundingBox*>(
  mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(surface->GetGeometry()->GetBoundingBox());
  mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

  if ( bounds[0] != surfBounds[0] 
      || bounds[1] != surfBounds[1] 
      || bounds[2] != surfBounds[2] 
      || bounds[3] != surfBounds[3] 
      || bounds[4] != surfBounds[4] 
      || bounds[5] != surfBounds[5] 
      ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Testing mitk::Surface::Testing 4D surface data creation: ";

  surface = mitk::Surface::New();
  mitk::Geometry3D::Pointer geometry = surface->GetGeometry();
  geometry->GetVtkTransform()->Identity();
  geometry->GetVtkTransform()->Translate(10,10,10);
  geometry->TransferVtkToITKTransform();
  mitk::TimeSlicedGeometry* timeSlicedGeometry = surface->GetTimeSlicedGeometry();
  timeSlicedGeometry->InitializeEvenlyTimed(geometry, 5);

  if (surface.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 

  //std::cout << "Testing mitk::Surface::*TESTED_METHOD_DESCRIPTION: ";
  //// METHOD_TEST_CODE
  //if (surface.IsNull()) {
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //else {
  //std::cout<<"[PASSED]"<<std::endl;
  //} 

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
