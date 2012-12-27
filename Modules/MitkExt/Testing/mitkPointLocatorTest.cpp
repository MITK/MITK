/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include <mitkPointSet.h>
#include <mitkTestingMacros.h>
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
  MITK_TEST_BEGIN("mitkPointLocator");
  std::srand( 1 );
  unsigned int num_points = 10000;

  // Create PointSet with randomly defined point
  MITK_TEST_OUTPUT(<< "Creating random point set of 10000 points ");
  vtkPoints* points = vtkPoints::New();
  for (unsigned int i = 0; i < num_points ; ++i )
  {
    points->InsertPoint(i, GenerateRandomNumber(), GenerateRandomNumber(), GenerateRandomNumber());
  }
  vtkPolyData* pointSet = vtkPolyData::New();
  pointSet->SetPoints( points );
  points->Delete();

  MITK_TEST_CONDITION_REQUIRED((unsigned) pointSet->GetNumberOfPoints()== num_points,"Test number of points in vtkPointSet");

  // feed the point set into a vtk point locator
  MITK_TEST_OUTPUT(<< "Building vtkPointLocator ");
  vtkPointLocator* vtkPointLoc = vtkPointLocator::New();
  vtkPointLoc->SetDataSet( pointSet );
  vtkPointLoc->BuildLocator();
  MITK_TEST_OUTPUT(<< "[PASSED]");

  // feed the point set into the mitk point locator
  MITK_TEST_OUTPUT(<< "Building mitkPointLocator ");
  mitk::PointLocator::Pointer mitkPointLocatorInitializedByVtkPointSet = mitk::PointLocator::New();
  mitk::PointLocator::Pointer mitkPointLocatorInitializedByMITKPointSet = mitk::PointLocator::New();
  mitk::PointLocator::Pointer mitkPointLocatorInitializedByITKPointSet = mitk::PointLocator::New();
  MITK_TEST_CONDITION_REQUIRED(mitkPointLocatorInitializedByVtkPointSet.IsNotNull(), "Test whether mitkPointLocator is not null");

  mitk::PointSet::Pointer mitkPointSet = mitk::PointSet::New();
  mitk::PointLocator::ITKPointSet::Pointer itkPointSet = mitk::PointLocator::ITKPointSet::New();
  for ( int i=0; i<pointSet->GetNumberOfPoints();i++ )
  {
    mitk::Point3D pnt;
    pnt[0] = pointSet->GetPoint( i )[0];
    pnt[1] = pointSet->GetPoint( i )[1];
    pnt[2] = pointSet->GetPoint( i )[2];
    mitkPointSet->InsertPoint(i, pnt );
    mitk::PointLocator::ITKPointSet::PointType itkPoint;
    itkPoint[0] = pointSet->GetPoint( i )[0];
    itkPoint[1] = pointSet->GetPoint( i )[1];
    itkPoint[2] = pointSet->GetPoint( i )[2];
    itkPointSet->SetPoint(i,itkPoint);
  }

  MITK_TEST_OUTPUT(<< "Setting random point set ");
  mitkPointLocatorInitializedByVtkPointSet->SetPoints( pointSet );
  mitkPointLocatorInitializedByMITKPointSet->SetPoints( mitkPointSet );
  mitkPointLocatorInitializedByITKPointSet->SetPoints( itkPointSet );
  MITK_TEST_OUTPUT(<< "[PASSED]");

  MITK_TEST_OUTPUT(<< "Testing 1000 random points ");
  // generate N random points and calculate the closest
  // points with both the vtk and mitk pointlocator.
  // verify, that the point ids are the same.
  vtkFloatingPointType p[3], x, y, z;
  mitk::PointSet::PointType pointType;
  for ( unsigned int i = 0 ; i < 100 ; ++i )
  {
    GenerateRandomPoint( x, y, z );
    p[0] = x;
    p[1] = y;
    p[2] = z;
    pointType[0] = p[0];
    pointType[1] = p[1];
    pointType[2] = p[2];
    int closestPointReference = vtkPointLoc->FindClosestPoint(p); // ground truth vtkPointLocator
    int closestPointVTK1 = mitkPointLocatorInitializedByVtkPointSet->FindClosestPoint(p);
    int closestPointVTK2 = mitkPointLocatorInitializedByVtkPointSet->FindClosestPoint(x, y, z);
    int closestPointVTK3 = mitkPointLocatorInitializedByVtkPointSet->FindClosestPoint(pointType);
    int closestPointMITK1 = mitkPointLocatorInitializedByMITKPointSet->FindClosestPoint(p);
    int closestPointMITK2 = mitkPointLocatorInitializedByMITKPointSet->FindClosestPoint(x, y, z);
    int closestPointMITK3 = mitkPointLocatorInitializedByMITKPointSet->FindClosestPoint(pointType);
    int closestPointITK1 = mitkPointLocatorInitializedByITKPointSet->FindClosestPoint(p);
    int closestPointITK2 = mitkPointLocatorInitializedByITKPointSet->FindClosestPoint(x, y, z);
    int closestPointITK3 = mitkPointLocatorInitializedByITKPointSet->FindClosestPoint(pointType);
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointVTK1,"Test FindClosestPoint() using a point array with a PointLocator initialized with a vtkPointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointVTK2,"Test FindClosestPoint() using single coordinates with a PointLocator initialized with a vtkPointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointVTK3,"Test FindClosestPoint() using an mitk::PointSet with a PointLocator initialized with a vtkPointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointMITK1,"Test FindClosestPoint() using a point array with a PointLocator initialized with a mitk::PointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointMITK2,"Test FindClosestPoint() using single coordinates with a PointLocator initialized with a mitk::PointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointMITK3,"Test FindClosestPoint() using an mitk::PointSet with a PointLocator initialized with a mitk::PointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointITK1,"Test FindClosestPoint() using a point array with a PointLocator initialized with a itk::PointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointITK2,"Test FindClosestPoint() using single coordinates with a PointLocator initialized with a itk::PointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointITK3,"Test FindClosestPoint() using an mitk::PointSet with a PointLocator initialized with a itk::PointSet");
    //Test  GetMinimalDistance
    // Get closest point
    //double* closestPoint = vtkPointLoc->GetPoints()->GetPoint(closestPointReference);
    double* closestPoint = pointSet->GetPoint(closestPointReference);
    mitk::PointSet::PointType cP;
    cP[0] = closestPoint[0];
    cP[1] = closestPoint[1];
    cP[2] = closestPoint[2];
    mitk::PointLocator::DistanceType minimalDistanceReference = cP.SquaredEuclideanDistanceTo(pointType);

    //mitk::PointLocator::DistanceType minimalDistanceReference =
    //  (x-closestPoint[0])*(x-closestPoint[0])+(y-closestPoint[1])*(y-closestPoint[1])+(z-closestPoint[2])*(z-closestPoint[2]);
    mitk::PointLocator::DistanceType minimalDistanceVtk = mitkPointLocatorInitializedByVtkPointSet->GetMinimalDistance(pointType);
    mitk::PointLocator::DistanceType minimalDistanceItk = mitkPointLocatorInitializedByITKPointSet->GetMinimalDistance(pointType);
    mitk::PointLocator::DistanceType minimalDistanceMITK = mitkPointLocatorInitializedByMITKPointSet->GetMinimalDistance(pointType);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(minimalDistanceReference,minimalDistanceVtk), "Test GetMinimalDistance() using a PointLocator initialized with a vtkPointSet" );
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(minimalDistanceReference,minimalDistanceItk), "Test GetMinimalDistance() using a PointLocator initialized with a itkPointSet" );
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(minimalDistanceReference,minimalDistanceMITK), "Test GetMinimalDistance() using a PointLocator initialized with a MITKPointSet" );

    int closestPointCombinedVtk;
    mitk::PointLocator::DistanceType minimalDistanceCombinedVtk;
    mitkPointLocatorInitializedByVtkPointSet->FindClosestPointAndDistance(pointType,&closestPointCombinedVtk,&minimalDistanceCombinedVtk);
    int closestPointCombinedITK;
    mitk::PointLocator::DistanceType minimalDistanceCombinedITK;
    mitkPointLocatorInitializedByITKPointSet->FindClosestPointAndDistance(pointType,&closestPointCombinedITK,&minimalDistanceCombinedITK);
    int closestPointCombinedMITK;
    mitk::PointLocator::DistanceType minimalDistanceCombinedMITK;
    mitkPointLocatorInitializedByMITKPointSet->FindClosestPointAndDistance(pointType,&closestPointCombinedMITK,&minimalDistanceCombinedMITK);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(minimalDistanceReference,minimalDistanceCombinedVtk), "Test distance returned by FindClosestPointAndDistance() using a PointLocator initialized with a vtkPointSet" );
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(minimalDistanceReference,minimalDistanceCombinedITK), "Test distance returned by FindClosestPointAndDistance() using a PointLocator initialized with a itkPointSet" );
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(minimalDistanceReference,minimalDistanceCombinedMITK), "Test distance returned by FindClosestPointAndDistance() using a PointLocator initialized with a MITKPointSet" );
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointCombinedVtk,"Test closest point returned by FindClosestPointAndDistance() using a point array with a PointLocator initialized with a vtkPointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointCombinedITK,"Test closest point returned by FindClosestPointAndDistance() using a point array with a PointLocator initialized with a itkPointSet");
    MITK_TEST_CONDITION_REQUIRED(closestPointReference==closestPointCombinedMITK,"Test closest point returned by FindClosestPointAndDistance() using a point array with a PointLocator initialized with a MITKPointSet");
  }

  vtkPointLoc->Delete();
  pointSet->Delete();

  MITK_TEST_END();
}
