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


#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkGeometry2D.h"
#include "mitkTestingMacros.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>


void mitkTimeSlicedGeometry_ChangeImageGeometryConsideringOriginOffset_Test()
{
  // additional tests to check the function ChangeImageGeometryConsideringOriginOffset(..)

  //first create a new timeslicedgeometry
  mitk::TimeSlicedGeometry::Pointer geoTime = mitk::TimeSlicedGeometry::New();
  mitk::Geometry3D::Pointer geo3d = mitk::Geometry3D::New();
  geo3d->Initialize();
  int numOfTimeSteps = 5;
  geoTime->InitializeEvenlyTimed(geo3d, numOfTimeSteps);

  for (int i=0; i < numOfTimeSteps; i++)
  {
    mitk::Geometry3D::Pointer geo3d_sub = mitk::Geometry3D::New();
    geo3d_sub->Initialize();
    geoTime->SetGeometry3D(geo3d_sub, i);
  }

  MITK_TEST_OUTPUT( << "Testing whether geoTime->GetImageGeometry() is false by default");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetImageGeometry()==false, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the geoTime have GetImageGeometry()==false by default");
  mitk::Geometry3D* subSliceGeo3D_first = geoTime->GetGeometry3D(0);
  mitk::Geometry3D* subSliceGeo3D_last  = geoTime->GetGeometry3D(numOfTimeSteps-1);
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetImageGeometry()==false, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetImageGeometry()==false, "");

  // Save some Origins and cornerpoints
  mitk::Point3D OriginTimeGeo( geoTime->GetOrigin() );
  mitk::Point3D OriginFirstGeo( subSliceGeo3D_first->GetOrigin() );
  mitk::Point3D OriginLastGeo( subSliceGeo3D_last->GetOrigin() );
  mitk::Point3D CornerPoint0TimeGeo(geoTime->GetCornerPoint(0));
  mitk::Point3D CornerPoint1FirstGeo(subSliceGeo3D_first->GetCornerPoint(1));
  mitk::Point3D CornerPoint2LastGeo(subSliceGeo3D_last->GetCornerPoint(2));

  //std::cout << "vorher Origin: " << subSliceGeo3D_first->GetOrigin() << std::endl;
  //std::cout << "vorher Corner: " << subSliceGeo3D_first->GetCornerPoint(0) << std::endl;
  MITK_TEST_OUTPUT( << "Calling geoTime->ChangeImageGeometryConsideringOriginOffset(true)");
  geoTime->ChangeImageGeometryConsideringOriginOffset(true);
  //std::cout << "nachher Origin: " << subSliceGeo3D_first->GetOrigin() << std::endl;
  //std::cout << "nachher Corner: " << subSliceGeo3D_first->GetCornerPoint(0) << std::endl;

  MITK_TEST_OUTPUT( << "Testing whether geoTime->GetImageGeometry() is now true");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetImageGeometry()==true, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==true now");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetImageGeometry()==true, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetImageGeometry()==true, "");

  MITK_TEST_OUTPUT( << "Testing wether offset has been added to origins");
  // Manually adding Offset.
  OriginTimeGeo[0] += (geoTime->GetSpacing()[0]) / 2;
  OriginTimeGeo[1] += (geoTime->GetSpacing()[1]) / 2;
  OriginTimeGeo[2] += (geoTime->GetSpacing()[2]) / 2;
  OriginFirstGeo[0] += (subSliceGeo3D_first->GetSpacing()[0]) / 2;
  OriginFirstGeo[1] += (subSliceGeo3D_first->GetSpacing()[1]) / 2;
  OriginFirstGeo[2] += (subSliceGeo3D_first->GetSpacing()[2]) / 2;
  OriginLastGeo[0] += (subSliceGeo3D_last->GetSpacing()[0]) / 2;
  OriginLastGeo[1] += (subSliceGeo3D_last->GetSpacing()[1]) / 2;
  OriginLastGeo[2] += (subSliceGeo3D_last->GetSpacing()[2]) / 2;
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetCornerPoint(1)==CornerPoint1FirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetCornerPoint(2)==CornerPoint2LastGeo, "");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetCornerPoint(0)==CornerPoint0TimeGeo, "");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetOrigin()==OriginTimeGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetOrigin()==OriginFirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetOrigin()==OriginLastGeo, "");


  MITK_TEST_OUTPUT( << "Calling geoTime->ChangeImageGeometryConsideringOriginOffset(false)");
  geoTime->ChangeImageGeometryConsideringOriginOffset(false);
  MITK_TEST_OUTPUT( << "Testing whether geoTime->GetImageGeometry() is now false");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetImageGeometry()==false, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the geoTime have GetImageGeometry()==false now");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetImageGeometry()==false, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetImageGeometry()==false, "");

  MITK_TEST_OUTPUT( << "Testing wether offset has been added to origins");
  // Manually substracting Offset.
  OriginTimeGeo[0] -= (geoTime->GetSpacing()[0]) / 2;
  OriginTimeGeo[1] -= (geoTime->GetSpacing()[1]) / 2;
  OriginTimeGeo[2] -= (geoTime->GetSpacing()[2]) / 2;
  OriginFirstGeo[0] -= (subSliceGeo3D_first->GetSpacing()[0]) / 2;
  OriginFirstGeo[1] -= (subSliceGeo3D_first->GetSpacing()[1]) / 2;
  OriginFirstGeo[2] -= (subSliceGeo3D_first->GetSpacing()[2]) / 2;
  OriginLastGeo[0] -= (subSliceGeo3D_last->GetSpacing()[0]) / 2;
  OriginLastGeo[1] -= (subSliceGeo3D_last->GetSpacing()[1]) / 2;
  OriginLastGeo[2] -= (subSliceGeo3D_last->GetSpacing()[2]) / 2;
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetCornerPoint(1)==CornerPoint1FirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetCornerPoint(2)==CornerPoint2LastGeo, "");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetCornerPoint(0)==CornerPoint0TimeGeo, "");
  MITK_TEST_CONDITION_REQUIRED( geoTime->GetOrigin()==OriginTimeGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_first->GetOrigin()==OriginFirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo3D_last->GetOrigin()==OriginLastGeo, "");

}


int mitkTimeSlicedGeometryTest(int /*argc*/, char* /*argv*/[])
{
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

  width  = 100;    widthInMM  = width*0.5;
  height = 200;    heightInMM = height*1.2;
  thicknessInMM = 1.5;
  mitk::FillVector3D(origin,       2.5,       -3.3, 17.2);
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  mitk::FillVector3D(normal,         0,          0, thicknessInMM);


  std::cout << "Creating TimeSlicedGeometry" <<std::endl;
  mitk::TimeSlicedGeometry::Pointer timeSlicedGeometry = mitk::TimeSlicedGeometry::New();
  if(timeSlicedGeometry.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  unsigned int numOfTimeSteps = 5;
  std::cout << "Testing TimeSlicedGeometry::Initialize(timesteps = " << numOfTimeSteps << "): " <<std::endl;
  timeSlicedGeometry->InitializeEvenlyTimed(numOfTimeSteps);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetTimeSteps()==" << numOfTimeSteps << ": " <<std::endl;
  if(timeSlicedGeometry->GetTimeSteps()!=numOfTimeSteps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetEvenlyTimed(): " <<std::endl;
  if(timeSlicedGeometry->GetEvenlyTimed()!=true)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  mitk::TimeBounds timeBounds1;
  timeBounds1[0] = 1.3;
  timeBounds1[1] = 2.4;

  std::cout << "Initializing a PlaneGeometry by InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Setting TimeBounds of PlaneGeometry by SetTimeBounds(): "<<std::endl;
  planegeometry->SetTimeBounds(timeBounds1);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing PlaneGeometry::GetTimeBounds(): "<<std::endl;
  if(planegeometry->GetTimeBounds() != timeBounds1)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  --numOfTimeSteps;
  std::cout << "Testing TimeSlicedGeometry::InitializeEvenlyTimed(planegeometry, timesteps = " << numOfTimeSteps << "): " <<std::endl;
  mitk::TimeSlicedGeometry::Pointer timeSlicedGeometry2 = mitk::TimeSlicedGeometry::New();
  timeSlicedGeometry2->InitializeEvenlyTimed(planegeometry, numOfTimeSteps);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetTimeSteps()==" << numOfTimeSteps << ": " <<std::endl;
  if(timeSlicedGeometry2->GetTimeSteps() != numOfTimeSteps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetEvenlyTimed(): " <<std::endl;
  if(timeSlicedGeometry2->GetEvenlyTimed()!=true)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::TimeStepToMS(): " << std::endl;
  if(fabs(timeSlicedGeometry2->TimeStepToMS( 2 ) - 3.5) > mitk::eps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::MSToTimeStep(): " << std::endl;
  if(timeSlicedGeometry2->MSToTimeStep( 3.6 ) != 2)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout << "Testing TimeSlicedGeometry::TimeStepToTimeStep(): " << std::endl;

  // Re-use timeSlicedGeometry with new time bounds
  mitk::TimeBounds timeBounds;
  timeBounds[0] = 0.0;
  timeBounds[1] = 1.0;
  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  geometry->Initialize();
  geometry->SetTimeBounds( timeBounds );
  timeSlicedGeometry->InitializeEvenlyTimed( geometry, numOfTimeSteps+1 );

  if(timeSlicedGeometry2->TimeStepToTimeStep( timeSlicedGeometry, 4 ) != 2)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the TimeSlicedGeometry: ";
  mitk::PlaneGeometry* accessedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry2->GetGeometry3D(0));
  if(accessedplanegeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing identity of first geometry to the planegeometry in the TimeSlicedGeometry (should not be cloned): ";
  if(accessedplanegeometry != planegeometry.GetPointer())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the spatial part of the first geometry in the TimeSlicedGeometry is identical to planegeometry by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry->GetAxisVector(0), planegeometry->GetAxisVector(0))==false) ||
     (mitk::Equal(accessedplanegeometry->GetAxisVector(1), planegeometry->GetAxisVector(1))==false) ||
     (mitk::Equal(accessedplanegeometry->GetAxisVector(2), planegeometry->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry->GetOrigin(),      planegeometry->GetOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing timebounds of first geometry: "<<std::endl;
  if( timeBounds1 != accessedplanegeometry->GetTimeBounds() )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing availability and type (PlaneGeometry) of second geometry in the TimeSlicedGeometry: ";
  mitk::PlaneGeometry* secondplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry2->GetGeometry3D(1));
  if(secondplanegeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing PlaneGeometry::GetTimeBounds(): "<<std::endl;
  const mitk::TimeBounds & secondtimebounds = secondplanegeometry->GetTimeBounds();
  if( (timeBounds1[1] != secondtimebounds[0]) || (secondtimebounds[1] != secondtimebounds[0] + timeBounds1[1]-timeBounds1[0]) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout << "Testing whether the spatial part of the second geometry in the TimeSlicedGeometry is identical to planegeometry by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(secondplanegeometry->GetAxisVector(0), planegeometry->GetAxisVector(0))==false) ||
     (mitk::Equal(secondplanegeometry->GetAxisVector(1), planegeometry->GetAxisVector(1))==false) ||
     (mitk::Equal(secondplanegeometry->GetAxisVector(2), planegeometry->GetAxisVector(2))==false) ||
     (mitk::Equal(secondplanegeometry->GetOrigin(),      planegeometry->GetOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  // non-evenly-timed
  std::cout << "Creating (new) TimeSlicedGeometry" <<std::endl;
  timeSlicedGeometry2 = mitk::TimeSlicedGeometry::New();
  if(timeSlicedGeometry2.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  numOfTimeSteps += 7;
  std::cout << "Testing TimeSlicedGeometry::InitializeEmpty(timesteps = " << numOfTimeSteps << "): " <<std::endl;
  timeSlicedGeometry2->InitializeEmpty(numOfTimeSteps);

  std::cout << "Testing TimeSlicedGeometry::GetEvenlyTimed():" <<std::endl;
  if(timeSlicedGeometry2->GetEvenlyTimed()!=false)
  {
    std::cout<<"[FAILED]"<<std::endl; ///\todo additionally test Initialize, default should be non-evenly-timed
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::SetEvenlyTimed(false):" <<std::endl;
  timeSlicedGeometry2->SetEvenlyTimed(false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetEvenlyTimed()==false:" <<std::endl;
  if(timeSlicedGeometry2->GetEvenlyTimed()!=false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetTimeSteps()==" << numOfTimeSteps << ": " <<std::endl;
  if(timeSlicedGeometry2->GetTimeSteps() != numOfTimeSteps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability of first geometry in the TimeSlicedGeometry (should not exist): ";
  mitk::Geometry3D* accessedgeometry = timeSlicedGeometry2->GetGeometry3D(0);
  if(accessedgeometry!=NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::SetGeometry3D(planegeometry, timesteps = 0): " <<std::endl;
  timeSlicedGeometry2->SetGeometry3D(planegeometry, 0);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the TimeSlicedGeometry: ";
  accessedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry2->GetGeometry3D(0));
  if(accessedplanegeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing identity of first geometry to the planegeometry in the TimeSlicedGeometry: ";
  if(accessedplanegeometry != planegeometry.GetPointer())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing availability of second geometry in the TimeSlicedGeometry (should not exist): ";
  accessedgeometry = timeSlicedGeometry2->GetGeometry3D(1);
  if(accessedgeometry!=NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout << "Setting planegeometry2 to a cloned version of planegeometry: "<<std::endl;
  mitk::PlaneGeometry::Pointer planegeometry2;
  planegeometry2 = dynamic_cast<mitk::PlaneGeometry*>(planegeometry->Clone().GetPointer());;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Changing timebounds of planegeometry2: "<<std::endl;
  mitk::TimeBounds timeBounds3;
  timeBounds3[0] = timeBounds[1];
  timeBounds3[1] = timeBounds3[0]+13.2334;
  planegeometry2->SetTimeBounds(timeBounds3);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::SetGeometry3D(planegeometry2, timesteps = 1): " <<std::endl;
  timeSlicedGeometry2->SetGeometry3D(planegeometry2, 1);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of second geometry in the TimeSlicedGeometry: ";
  accessedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry2->GetGeometry3D(1));
  if(accessedplanegeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing identity of second geometry to the planegeometry2 in the TimeSlicedGeometry: ";
  if(accessedplanegeometry != planegeometry2.GetPointer())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing timebounds of second geometry: "<<std::endl;
  if( timeBounds3 != accessedplanegeometry->GetTimeBounds() )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  // run additional tests to check the function ChangeImageGeometryConsideringOriginOffset(..)
  mitkTimeSlicedGeometry_ChangeImageGeometryConsideringOriginOffset_Test();


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
