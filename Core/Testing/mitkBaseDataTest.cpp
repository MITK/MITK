/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17230 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkBaseData.h"

#include <fstream>

int mitkBaseDataTest(int /*argc*/, char* /*argv*/[])
{
  mitk::BaseData::Pointer bd;
  std::cout << "Testing mitk::BaseData::New(): ";
  bd = mitk::BaseData::New();
  if (bd.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Testing time sliced geometry after New(): ";
  mitk::TimeSlicedGeometry::Pointer tsg = bd->GetTimeSlicedGeometry();
  if (tsg.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  unsigned int ts = tsg->GetTimeSteps();
  if(ts != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Testing with initialized time sliced geometry: ";
  bg->InitializeTimeSlicedGeometry(2);
  tsg = bd->GetUpdatedTimeSlicedGeometry();
  if (tsg.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  unsigned int ts = tsg->GetTimeSteps();
  if(ts != 2)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Testing Set and Get methods for property lists: ";
  mitk::PropertyList::Pointer pl = mitk::PropertyList::New();
  pl->SetBoolProperty("Test", true);
  bd->SetPropertyList(pl);
  pl = bd->GetPropertyList();
  if( pl.IsNull() || !(pl->GetProperty("Test")) || (pl->GetBoolProperty("Test")!=true) ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Testing CopyInformation method: ";
  mitk::BaseData::Pointer nbd = mitk::BaseData::New();
  nbd->CopyInformation(bd);

  std::cout << "Testing copied time sliced geometry...  ";
  tsg = nbd->GetTimeSlicedGeometry();
  if (tsg.IsNull() || (tsg->GetTimeSteps() != 2) )  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing copied property list...  ";
  pl = nbd->GetPropertyList();
  if( pl.IsNull() || !(pl->GetProperty("Test")) || (pl->GetBoolProperty("Test")!=true) ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
