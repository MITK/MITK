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


#include "mitkImage.h"
#include "mitkImageDataItem.h"
#include <fstream>
#include <itkSmartPointerForwardReference.txx>


int mitkImageTest(int /*argc*/, char* /*argv*/[])
{
	//Create Image out of nowhere
	mitk::Image::Pointer imgMem;
	mitk::PixelType pt(typeid(int));
	unsigned int dim[]={100,100,20};

  std::cout << "Testing creation of Image: ";
	imgMem=mitk::Image::New();
  if(imgMem.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions): ";
	imgMem->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IsInitialized(): ";
  if(imgMem->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetData(): ";
  int *p = (int*)imgMem->GetData();
  if(p==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Filling image: ";
  unsigned int i;
  unsigned int size = dim[0]*dim[1]*dim[2];
  for(i=0; i<size; ++i, ++p)
    *p= (signed int)i;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Getting it again and compare with filled values: ";
  int *p2 = (int*)imgMem->GetData();
  if(p2==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  for(i=0; i<size; ++i, ++p2)
  {
    if(*p2!= (signed int)i )
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IsInitialized(): ";
  if(imgMem->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing GetSliceData() and compare with filled values: ";
  p2 = (int*)imgMem->GetSliceData(dim[2]/2)->GetData();
  unsigned int xy_size = dim[0]*dim[1];
  unsigned int start_mid_slice = (dim[2]/2)*xy_size;
  for(i=0; i<xy_size; ++i, ++p2)
  {
    if(*p2!=(signed int)(i+start_mid_slice))
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;

  //----
  ipPicDescriptor *pic_slice=ipPicClone(imgMem->GetSliceData(dim[2]/2)->GetPicDescriptor());
	imgMem=mitk::Image::New();

  std::cout << "Testing reinitializing via Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions): ";
	imgMem->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing slice-wise filling via SetPicSlice(): ";
  for(i=0;i<dim[2];++i)
  {
    imgMem->SetPicSlice(pic_slice, i, 0, 0);
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Getting it again and compare with filled values: ";
  p2 = (int*)imgMem->GetData();
  if(p2==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  for(i=0; i<size; ++i, ++p2)
  {
    if(*p2!= (signed int)((i%xy_size)+start_mid_slice))
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IsInitialized(): ";
  if(imgMem->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Setting a copy of the volume once again: ";
  imgMem->SetPicVolume(ipPicClone(imgMem->GetVolumeData(0)->GetPicDescriptor()),0);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set a slice with different content via SetPicSlice(): ";
  memset(pic_slice->data,0,xy_size*sizeof(int));
  imgMem->SetPicSlice(pic_slice, 1);

  std::cout << "Getting the volume again and compare the check the changed slice: ";
  p2 = (int*)imgMem->GetData();
  if(p2==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  p2+=xy_size;
  for(i=0; i<xy_size; ++i, ++p2)
  {
    if(*p2!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout << "Setting volume again: ";
  imgMem->SetVolume(imgMem->GetData());
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set a slice with different content via SetSlice(): ";
  memset(pic_slice->data,0,xy_size*sizeof(int));
  imgMem->SetSlice(pic_slice->data, 0);

  std::cout << "Getting the volume again and compare the check the changed slice: ";
  p2 = (int*)imgMem->GetData();
  if(p2==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  for(i=0; i<xy_size; ++i, ++p2)
  {
    if(*p2!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;



  //std::cout << "Testing SetVolume(): ";
  //imgMem->SetVolume(data);
  //std::cout<<"[PASSED]"<<std::endl;

  ipPicFree(pic_slice);

  //-----------------
  // geometry information for image
  mitk::Point3D origin;
  mitk::Vector3D right, bottom;
  mitk::Vector3D spacing;
  mitk::FillVector3D(origin, 17.0, 19.92, 7.83);
  mitk::FillVector3D(right, 1.0, 2.0, 3.0);
  mitk::FillVector3D(bottom, 0.0, -3.0, 2.0);
  mitk::FillVector3D(spacing, 0.78, 0.91, 2.23);

  std::cout << "Testing InitializeStandardPlane(rightVector, downVector, spacing): " << std::flush;
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(100, 100, right, bottom, &spacing);
  planegeometry->SetOrigin(origin);
  std::cout << "done" << std::endl;

  std::cout << "Testing Initialize(const mitk::PixelType& type, const mitk::Geometry3D& geometry, unsigned int slices) with PlaneGeometry and GetData(): ";
  imgMem->Initialize(mitk::PixelType(typeid(int)), *planegeometry);
  p = (int*)imgMem->GetData();
  if(p==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Initialize(const mitk::PixelType& type, int sDim, const mitk::PlaneGeometry& geometry) and GetData(): ";
  imgMem->Initialize(mitk::PixelType(typeid(int)), 40, *planegeometry);
  p = (int*)imgMem->GetData();
  if(p==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
