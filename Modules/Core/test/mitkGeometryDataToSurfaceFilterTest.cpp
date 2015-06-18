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

#include "mitkPlaneGeometryDataToSurfaceFilter.h"

#include "mitkSurface.h"
#include "mitkPlaneGeometry.h"
#include "mitkPlaneGeometryData.h"

#include "vtkPolyData.h"

#include <fstream>

template <typename TScalarType>
int testExpectedIndexBoundingBox(mitk::BaseGeometry* geometry, TScalarType expectedIndexBounds[6])
{
  mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox());
  mitk::BoundingBox::BoundsArrayType bounds = bb->GetBounds();

  int i;
  for(i=0;i<6;++i)
  {
    if( mitk::Equal(bounds[i], expectedIndexBounds[i]) == false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;
  return EXIT_SUCCESS;
}

template <typename TScalarType>
int testExpectedAxisParallelBoundingBox(mitk::BaseGeometry* geometry, TScalarType expectedAxisParallelBounds[6])
{
  mitk::BoundingBox::Pointer bb = geometry->CalculateBoundingBoxRelativeToTransform(nullptr);
  mitk::BoundingBox::BoundsArrayType bounds = bb->GetBounds();

  int i;
  for(i=0;i<6;++i)
  {
    if( mitk::Equal(bounds[i], expectedAxisParallelBounds[i]) == false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;
  return EXIT_SUCCESS;
}

int testSurfaceBoundingBoxConsistency(mitk::Surface* surface, bool expectIdentityTransform)
{
  int result;
  std::cout << "  Testing surface contents: ";
  if ((surface ==  nullptr )
    || (surface->GetVtkPolyData() == nullptr)
    || (surface->GetVtkPolyData()->GetNumberOfPoints() == 0 )) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }

  double bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  vtkPolyData* polys = surface->GetVtkPolyData();
  polys->ComputeBounds();
  polys->GetBounds( bounds );

  int i;
  if(expectIdentityTransform == false)
  {
    mitk::PlaneGeometry::Pointer geometry = mitk::PlaneGeometry::New();
    geometry->SetFloatBounds(bounds);
    geometry->SetIndexToWorldTransform(surface->GetGeometry()->GetIndexToWorldTransform());
    mitk::BoundingBox::BoundsArrayType bb = const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetBounds();
    for(i=0;i<6;++i)
      bounds[i]=bb[i];
  }

  std::cout << "  Testing GetBoundingBox() ";
  if((result=testExpectedIndexBoundingBox(surface->GetGeometry(), bounds)) != EXIT_SUCCESS)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}

int testGeometryDataToSurfaceFilter(mitk::PlaneGeometryDataToSurfaceFilter* geometryToSurfaceFilter, mitk::ScalarType expectedIndexBounds[6], mitk::ScalarType expectedAxisParallelBounds[6], bool expectIdentityTransform)
{
  int result;

  std::cout << "Testing SetRequestedRegionToLargestPossibleRegion(): ";
  geometryToSurfaceFilter->GetOutput()->SetRequestedRegionToLargestPossibleRegion();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing UpdateOutputInformation(): ";
  geometryToSurfaceFilter->UpdateOutputInformation();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing correctness of bounding-box after UpdateOutputInformation(): ";
  if((result=testExpectedIndexBoundingBox(geometryToSurfaceFilter->GetOutput()->GetGeometry(), expectedIndexBounds)) != EXIT_SUCCESS) {
    return result;
  }

  std::cout << "Testing correctness of axis-parallel bounding-box after UpdateOutputInformation(): ";
  if((result=testExpectedAxisParallelBoundingBox(geometryToSurfaceFilter->GetOutput()->GetGeometry(), expectedAxisParallelBounds)) != EXIT_SUCCESS) {
    return result;
  }


  std::cout << "Testing Update(): ";
  geometryToSurfaceFilter->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing correctness of bounding-box after Update(): ";
  if((result=testExpectedIndexBoundingBox(geometryToSurfaceFilter->GetOutput()->GetGeometry(), expectedIndexBounds)) != EXIT_SUCCESS) {
    return result;
  }

  std::cout << "Testing correctness of axis-parallel bounding-box after UpdateOutputInformation(): ";
  if((result=testExpectedAxisParallelBoundingBox(geometryToSurfaceFilter->GetOutput()->GetGeometry(), expectedAxisParallelBounds)) != EXIT_SUCCESS) {
    return result;
  }

  std::cout << "Testing bounding-box consistency: "<<std::endl;
  if((result=testSurfaceBoundingBoxConsistency(geometryToSurfaceFilter->GetOutput(), expectIdentityTransform)) != EXIT_SUCCESS) {
    std::cout<<"[FAILED]"<<std::endl;
    return result;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }
  return EXIT_SUCCESS;
}

int mitkGeometryDataToSurfaceFilterTest(int /*argc*/, char* /*argv*/[])
{
  int result;

  std::cout << "Testing mitk::PlaneGeometryDataToSurfaceFilter: " << std::endl;

  mitk::PlaneGeometryDataToSurfaceFilter::Pointer geometryToSurfaceFilter;
  std::cout << "Testing PlaneGeometryDataToSurfaceFilter::New(): ";
  geometryToSurfaceFilter = mitk::PlaneGeometryDataToSurfaceFilter::New();
  if (geometryToSurfaceFilter.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }

  mitk::Point3D origin;
  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
  plane->InitializeStandardPlane(50, 100);
  mitk::FillVector3D(origin, 1.0, 2.0, 3.0);
  plane->SetOrigin(origin);

  mitk::PlaneGeometryData::Pointer geometryData = mitk::PlaneGeometryData::New();
  geometryData->SetPlaneGeometry(plane);

  std::cout << "Testing SetInput(): ";
  geometryToSurfaceFilter->SetInput(geometryData);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetInput(): ";
  if (geometryToSurfaceFilter->GetInput() !=  geometryData ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }


  std::cout << "Testing default of PlaneGeometryDataToSurfaceFilter::m_PlaceByGeometry (expected is false): ";
  if (geometryToSurfaceFilter->GetPlaceByGeometry() !=  false ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }

  // test with m_PlaceByGeometry==false
  mitk::ScalarType expectedBoundsFinal[6] = {1.0, 51.0, 2.0, 102.0, 3.0, 3.0};
  if((result=testGeometryDataToSurfaceFilter(geometryToSurfaceFilter, expectedBoundsFinal, expectedBoundsFinal, true)) != EXIT_SUCCESS) {
    return result;
  }

  std::cout << "Testing PlaneGeometryDataToSurfaceFilter::SetPlaceByGeometry(true): ";
  geometryToSurfaceFilter->SetPlaceByGeometry(true);
  if (geometryToSurfaceFilter->GetPlaceByGeometry() !=  true ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }

  // test with m_PlaceByGeometry==true
  mitk::ScalarType expectedIndexBounds[6] = {0.0, 50.0, 0.0, 100.0, 0.0, 0.0};
  mitk::ScalarType expectedAxisParallelBounds[6] = {1.0, 51.0, 2.0, 102.0, 3.0, 3.0};
  if((result=testGeometryDataToSurfaceFilter(geometryToSurfaceFilter, expectedIndexBounds, expectedAxisParallelBounds, true)) != EXIT_SUCCESS) {
    return result;
  }



  // test with specified BoundingBox (m_PlaceByGeometry is irrelevant for this test)
  mitk::BoundingBox::Pointer boundingBox = mitk::BoundingBox::New();
  mitk::Point3D bbMin, bbMax;
  mitk::FillVector3D( bbMin, 10.0, 10.0, -6.0 );
  mitk::FillVector3D( bbMax, 40.0, 90.0, 6.0 );

  mitk::BoundingBox::PointsContainer::Pointer pointsContainer = mitk::BoundingBox::PointsContainer::New();
  pointsContainer->InsertElement( 0, bbMin );
  pointsContainer->InsertElement( 1, bbMax );
  boundingBox->SetPoints( pointsContainer );
  boundingBox->ComputeBoundingBox();


  geometryToSurfaceFilter->SetPlaceByGeometry( true );
  geometryToSurfaceFilter->SetBoundingBox( boundingBox );
  mitk::ScalarType expectedIndexBoundsWithBB[6] = {9.0, 39.0, 8.0, 88.0, 0.0, 0.0};
  mitk::ScalarType expectedAxisParallelBoundsWithBB[6] = {10.0, 40.0, 10.0, 90.0, 3.0, 3.0};
  if((result=testGeometryDataToSurfaceFilter(geometryToSurfaceFilter, expectedIndexBoundsWithBB, expectedAxisParallelBoundsWithBB, true)) != EXIT_SUCCESS) {
    return result;
  }


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
