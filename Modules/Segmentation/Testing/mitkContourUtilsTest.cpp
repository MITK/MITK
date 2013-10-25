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
#include "mitkTestingMacros.h"
#include "mitkContourUtils.h"
#include "mitkContourModel.h"

//file loading
#include <fstream>
#include "mitkDataNodeFactory.h"

#include "mitkIOUtil.h"
#include "mitkExtractSliceFilter.h"
#include "mitkGeometry2D.h"
#include "mitkGeometry3D.h"
#include "mitkImage.h"
#include "mitkVector.h"

  void TestContourModel2itkMesh()
  {
    mitk::ContourModel::Pointer contourModel = mitk::ContourModel::New();
    mitk::ContourUtils::Pointer utils = mitk::ContourUtils::New();

    mitk::Point3D p,p1,p2,p3;


    p[0] = 0;
    p[1] = 0;
    p[2] = 0;

    p1[0] = 10;
    p1[1] = 0;
    p1[2] = 0;

    p2[0] = 10;
    p2[1] = 10;
    p2[2] = 0;

    p3[0] = 0;
    p3[1] = 10;
    p3[2] = 0;

    contourModel->AddVertex(p);
    contourModel->AddVertex(p1);
    contourModel->AddVertex(p2);
    contourModel->AddVertex(p3);

    //Create Image out of nowhere
    mitk::Image::Pointer image = mitk::Image::New();
    mitk::PixelType pt = mitk::MakeScalarPixelType<int>();
    unsigned int dim[]={11,11,3};

    // Initialize image
    image->Initialize( pt, 3, dim);
    image->SetSpacing(1.0);
    mitk::Point3D m_origin;
    m_origin[0] = 0.0;
    m_origin[1] = 0.0;
    m_origin[2] = 0.0;
    image->SetOrigin(m_origin);

    int *pixel = (int*)image->GetData(); // pointer to pixel data
    int size = dim[0]*dim[1]*dim[2];
    for(int i=0; i<size; ++i, ++pixel) *pixel=0; // fill image


    //write out all interesting values
    MITK_INFO << "ContourUtils-EmptyImage/origin: " << image->GetGeometry()->GetOrigin();
    MITK_INFO << "ContourUtils-EmptyImage/spacing: " << image->GetGeometry()->GetSpacing();
    MITK_INFO << "ContourUtils-EmptyImage/Dimension: " << image->GetDimension();
    if(mitk::IOUtil::SaveImage(image, "/home/lars/TestingMethodeOutput/testClassEmptyImage.mhd"))
        MITK_INFO << "Saved Image";


    mitk::ExtractSliceFilter::Pointer sliceFilter = mitk::ExtractSliceFilter::New();
    sliceFilter->SetInput(image);

    sliceFilter->SetWorldGeometry( image->GetSlicedGeometry()->GetGeometry2D(0) );

    sliceFilter->Update();

    mitk::Image::Pointer sliceImage = mitk::Image::New();
    sliceImage = sliceFilter->GetOutput();

    MITK_INFO << "ContourUtils-SliceImage/origin: " << sliceImage->GetGeometry()->GetOrigin();
    MITK_INFO << "ContourUtils-SliceImage/spacing: " << sliceImage->GetGeometry()->GetSpacing();
    MITK_INFO << "ContourUtils-SliceImage/Dimension: " << sliceImage->GetDimension();
    if(mitk::IOUtil::SaveImage(sliceImage, "/home/lars/TestingMethodeOutput/testClassSliceImage.mhd"))
        MITK_INFO << "Saved Image";

    //test FillContourInSlice
    utils->FillContourInSlice( contourModel, 0, sliceImage, 1 );

    //MITK_INFO << utils->m_mesh;
    MITK_INFO << "should be done";

  }

  int mitkContourUtilsTest(int argc, char* argv[])
  {
    MITK_TEST_BEGIN("mitkContourUtilsTest")

    TestContourModel2itkMesh();

    MITK_TEST_END()
  }

