/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2006-06-09 15:24:07 +0200 (Fr, 09 Jun 2006) $
Version:   $Revision: 7074 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

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

  mitk::TimeBounds timeBounds;
  timeBounds[0] = 1.3;
  timeBounds[1] = 17.4;

  std::cout << "Initializing a PlaneGeometry by InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Setting TimeBounds of PlaneGeometry by SetTimeBounds(): "<<std::endl;
  planegeometry->SetTimeBounds(timeBounds);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing PlaneGeometry::GetTimeBounds(): "<<std::endl;
  if(planegeometry->GetTimeBounds() != timeBounds)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



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
  timeSlicedGeometry->Initialize(numOfTimeSteps);
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



  --numOfTimeSteps;
  std::cout << "Testing TimeSlicedGeometry::InitializeEvenlyTimed(planegeometry, timesteps = " << numOfTimeSteps << "): " <<std::endl;
  timeSlicedGeometry->InitializeEvenlyTimed(planegeometry, numOfTimeSteps);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetTimeSteps()==" << numOfTimeSteps << ": " <<std::endl;
  if(timeSlicedGeometry->GetTimeSteps() != numOfTimeSteps)
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

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the TimeSlicedGeometry: ";
  mitk::PlaneGeometry* accessedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry->GetGeometry3D(0));
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
  if( timeBounds != accessedplanegeometry->GetTimeBounds() )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing availability and type (PlaneGeometry) of second geometry in the TimeSlicedGeometry: ";
  mitk::PlaneGeometry* secondplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry->GetGeometry3D(1));
  if(secondplanegeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing PlaneGeometry::GetTimeBounds(): "<<std::endl;
  const mitk::TimeBounds & secondtimebounds = secondplanegeometry->GetTimeBounds();
  if( (timeBounds[1] != secondtimebounds[0]) && (secondtimebounds[1] != secondtimebounds[0] + timeBounds[1]-timeBounds[0]) )
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
  timeSlicedGeometry = mitk::TimeSlicedGeometry::New();
  if(timeSlicedGeometry.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  numOfTimeSteps += 7;
  std::cout << "Testing TimeSlicedGeometry::InitializeEmpty(timesteps = " << numOfTimeSteps << "): " <<std::endl;
  timeSlicedGeometry->InitializeEmpty(numOfTimeSteps);

  std::cout << "Testing TimeSlicedGeometry::GetEvenlyTimed():" <<std::endl;
  if(timeSlicedGeometry->GetEvenlyTimed()!=false)
  {
    std::cout<<"[FAILED]"<<std::endl; ///\todo additionally test Initialize, default should be non-evenly-timed
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::SetEvenlyTimed(false):" <<std::endl;
  timeSlicedGeometry->SetEvenlyTimed(false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetEvenlyTimed()==false:" <<std::endl;
  if(timeSlicedGeometry->GetEvenlyTimed()!=false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::GetTimeSteps()==" << numOfTimeSteps << ": " <<std::endl;
  if(timeSlicedGeometry->GetTimeSteps() != numOfTimeSteps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability of first geometry in the TimeSlicedGeometry (should not exist): ";
  mitk::Geometry3D* accessedgeometry = timeSlicedGeometry->GetGeometry3D(0);
  if(accessedgeometry!=NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::SetGeometry3D(planegeometry, timesteps = 0): " <<std::endl;
  timeSlicedGeometry->SetGeometry3D(planegeometry, 0);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the TimeSlicedGeometry: ";
  accessedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry->GetGeometry3D(0));
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
  accessedgeometry = timeSlicedGeometry->GetGeometry3D(1);
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
  mitk::TimeBounds timeBounds2;
  timeBounds2[0] = timeBounds[1];
  timeBounds2[1] = timeBounds2[0]+13.2334;
  planegeometry2->SetTimeBounds(timeBounds2);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing TimeSlicedGeometry::SetGeometry3D(planegeometry2, timesteps = 1): " <<std::endl;
  timeSlicedGeometry->SetGeometry3D(planegeometry2, 1);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of second geometry in the TimeSlicedGeometry: ";
  accessedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(timeSlicedGeometry->GetGeometry3D(1));
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
  if( timeBounds2 != accessedplanegeometry->GetTimeBounds() )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
