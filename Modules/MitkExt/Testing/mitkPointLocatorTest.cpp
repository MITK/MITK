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


#include <mitkPointSet.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <mitkPointLocator.h>
#include <vtkPointLocator.h>
#include <cstdlib>

vtkFloatingPointType GenerateRandomNumber( const vtkFloatingPointType& min = 0.0, const vtkFloatingPointType& max = 100.0 )
{
  return ( ( ( vtkFloatingPointType ) ( std::rand( ) ) ) / ( ( vtkFloatingPointType ) ( RAND_MAX ) ) ) * ( max - min) + min;
}

void GenerateRandomPoint( vtkFloatingPointType& x, vtkFloatingPointType& y, vtkFloatingPointType& z, const vtkFloatingPointType& min = 0.0, const vtkFloatingPointType& max = 100.0 )
{
    x = GenerateRandomNumber(min, max);
    y = GenerateRandomNumber(min, max);
    z = GenerateRandomNumber(min, max);
}

int mitkPointLocatorTest(int /*argc*/, char* /*argv*/[])
{
  std::srand( 1 );
  unsigned int num_points = 10000;
  
  // Create PointSet with randomly defined point
  std::cout << "Creating random point set of 10000 points ";
  vtkPoints* points = vtkPoints::New();
  if ( points == NULL )
  {
    std::cout << "vtkPoints == NULL. [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  
  for (unsigned int i = 0; i < num_points ; ++i )
  {
    points->InsertPoint(i, GenerateRandomNumber(), GenerateRandomNumber(), GenerateRandomNumber());
  }
  
  vtkPolyData* pointSet = vtkPolyData::New();
  pointSet->SetPoints( points );

  points->Delete();
  
  if ( (unsigned) pointSet->GetNumberOfPoints() != num_points )
  {
    std::cout << "Number of points in the vtkPointSet != "<< num_points <<". [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;  
  
  // feed the point set into a vtk point locator
  std::cout << "Building vtkPointLocator ";
  vtkPointLocator* vtkPointLoc = vtkPointLocator::New();
  vtkPointLoc->SetDataSet( pointSet );
  vtkPointLoc->BuildLocator();
  std::cout << "[PASSED]" << std::endl;  
  
  // feed the point set into the mitk point locator
  std::cout << "Building mitkPointLocator ";
  mitk::PointLocator::Pointer mitkPointLocator = mitk::PointLocator::New();
  if ( mitkPointLocator.IsNull() )
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;  
  }
  
  std::cout << "Setting random point set ";
  mitkPointLocator->SetPoints( pointSet );
  std::cout << "[PASSED]" << std::endl;  
  
  std::cout << "Testing 1000 random points ";
  // generate N random points and calculate the closest
  // points with both the vtk and mitk pointlocator.
  // verify, that the point ids are the same.
  vtkFloatingPointType p[3], x, y, z;
  for ( unsigned int i = 0 ; i < 1000 ; ++i )
  {
    GenerateRandomPoint( x, y, z );
    p[0] = x;
    p[1] = y;
    p[2] = z;
    int closestPoint1 = vtkPointLoc->FindClosestPoint(p);
    int closestPoint2 = mitkPointLocator->FindClosestPoint(p);
    int closestPoint3 = mitkPointLocator->FindClosestPoint(x, y, z);
    
    if ( closestPoint1 != closestPoint2 )
    {
      std::cout << "Id returned by vtkPointLocator doesn't match the one returned by mitk::PointLocator. [FAILED]" << std::endl;  
      return EXIT_FAILURE;
    }    
    if ( closestPoint2 != closestPoint3 )
    {
      std::cout << "There is a mismatch between passing coords as array and as single x/y/z coords. [FAILED]" << std::endl;  
      return EXIT_FAILURE;
    }    
  }  

  vtkPointLoc->Delete();
  pointSet->Delete();

  std::cout << "[PASSED]" << std::endl;  
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
