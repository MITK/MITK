/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 16:11:33 +0200 (Do, 18 Jun 2009) $
Version:   $Revision: 17789 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSurface.h"
#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkCommon.h"
#include "mitkVector.h"
#include "mitkTimeSlicedGeometry.h"

#include "vtkPolyData.h"
#include "vtkSphereSource.h"

#include <fstream>

int mitkSurfaceToSurfaceFilterTest(int /*argc*/, char* /*argv*/[])
{
  mitk::Surface::Pointer surface;
  surface = mitk::Surface::New();
  vtkSphereSource* sphereSource = vtkSphereSource::New();
  sphereSource->SetCenter(0,0,0);
  sphereSource->SetRadius(5.0);
  sphereSource->SetThetaResolution(10);
  sphereSource->SetPhiResolution(10);
  sphereSource->Update();

  vtkPolyData* polys = sphereSource->GetOutput();
  surface->SetVtkPolyData( polys );
  sphereSource->Delete();
  

  mitk::SurfaceToSurfaceFilter::Pointer filter = mitk::SurfaceToSurfaceFilter::New();
  std::cout << "Testing mitk::SurfaceToSurfaceFilter::SetInput() and ::GetNumberOfInputs() : " ;
  filter->SetInput( surface );
  if ( filter->GetNumberOfInputs() < 1 )
  {
    std::cout<<"[FAILED] : zero inputs set "<<std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "Testing if GetInput returns the right Input : " << std::endl;
  if ( filter->GetInput() != surface )
  {
    std::cout<<"[FAILED] : GetInput does not return correct input. "<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[SUCCESS] : input correct" << std::endl;


  if ( filter->GetInput(5) != NULL )
  {
    std::cout<<"[FAILED] : GetInput returns inputs that were not set. "<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[SUCCESS] : Input nr.5 was not set -> is NULL" << std::endl;


  std::cout << "Testing whether Output is created correctly : " << std::endl;
  if ( filter->GetNumberOfOutputs() != filter->GetNumberOfInputs() )
  {
    std::cout <<"[FAILED] : number of outputs != number of inputs" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[SUCCESS] : number of inputs == number of outputs." << std::endl;


  mitk::Surface::Pointer outputSurface = filter->GetOutput(0);
  if ( outputSurface->GetVtkPolyData()->GetNumberOfPolys() != surface->GetVtkPolyData()->GetNumberOfPolys() )
  {
    std::cout << "[FAILED] : number of Polys in PolyData of output != number of Polys in PolyData of input" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[SUCCESS] : number of Polys in PolyData of input and output are identical." << std::endl;



  filter->Update();
  outputSurface = filter->GetOutput(0);
  if ( outputSurface->GetSizeOfPolyDataSeries() != surface->GetSizeOfPolyDataSeries() )
  {
    std::cout << "[FAILED] : number of PolyDatas in PolyDataSeries of output != number of PolyDatas of input" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[SUCCESS] : Size of PolyDataSeries of input and output are identical." << std::endl;


  std::cout << "Testing removeInputs() : " << std::endl;
  unsigned int numOfInputs = filter->GetNumberOfInputs();
  filter->RemoveInputs( mitk::Surface::New() );
  if ( filter->GetNumberOfInputs() != numOfInputs )
  {
    std::cout << "[FAILED] : input was removed that was not set." << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[SUCCESS] : no iput was removed that was not set." << std::endl;
  filter->RemoveInputs( surface );
  if ( filter->GetNumberOfInputs() != 0 )
  {
    std::cout << "[FAILED] : existing input was not removed correctly." << std::endl;
    return EXIT_FAILURE; 
  }
  std::cout << "[SUCCESS] : existing input was removed correctly." << std::endl;


  
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
