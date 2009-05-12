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


NOT READY!!!

#include "mitkImage.h"
#include "mitkPicFileReader.h"
#include "mitkImageSliceSelector.h"

#include <fstream>
int mitkRequestedRegionTest(int argc, char* argv[])
{
	//Read pic-Image from file
	mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();
	  reader->SetFileName(argv[1]);

  //Take a slice
	mitk::ImageSliceSelector::Pointer slice = mitk::ImageSliceSelector::New();
	  slice->SetInput(reader->GetOutput());
	  slice->SetSliceNr(1);
	  slice->Update();

  std::cout << "Testing IsInitialized(): ";
  if(slice->GetOutput()->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  if(reader->GetOutput()->GetDimension(3) > 1)
  {
    int time=reader->GetOutput()->GetDimension(3)-1;

    std::cout << "Testing 3D+t: Setting time to " << time << ": ";
	  slice->SetTimeNr(time);
    if(slice->GetTimeNr()!=time)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing 3D+t: Updating slice: ";
    slice->Update();
    if(slice->GetOutput()->IsInitialized()==false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing 3D+t: Slice in reader available: ";
    if(reader->GetOutput()->IsSliceSet(1, time)==false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing 3D+t: Time index inside requested region in reader: ";
    if((reader->GetOutput()->GetRequestedRegion().GetIndex(3)>time) ||
       (reader->GetOutput()->GetRequestedRegion().GetIndex(3)+reader->GetOutput()->GetRequestedRegion().GetSize(3)<=time)) 
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing 3D+t: Time index of requested region in reader: ";
    if(reader->GetOutput()->GetRequestedRegion().GetIndex(3)!=time)
    {
      std::cout<<"[FAILED - non fatal]"<<std::endl;
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing 3D+t: Time size of requested region in reader: ";
    if(reader->GetOutput()->GetRequestedRegion().GetSize(3)!=1)
    {
      std::cout<<"[FAILED - non fatal]"<<std::endl;
    }
    std::cout<<"[PASSED]"<<std::endl;
  }

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
