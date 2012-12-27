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

// mitk includes
#include <mitkImage.h>
#include <mitkImageDataItem.h>
#include <mitkImageCast.h>
#include "mitkItkImageFileReader.h"
#include <mitkTestingMacros.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkConvert2Dto3DImageFilter.h>
#include <mitkRotationOperation.h>
#include <mitkInteractionConst.h>
#include <mitkImageWriter.h>
#include <mitkPlaneOperation.h>
#include "mitkTestingConfig.h"
#include "mitkItkImageFileReader.h"

// itk includes
#include <itkImage.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

// stl includes
#include <fstream>

// vtk includes
#include <vtkImageData.h>

int mitkImageDimensionConverterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN(mitkImageDimensionConverterTest);

  // Define an epsilon which is the allowed error
  float eps = 0.00001;

  // Define helper variables
  float error = 0;
  bool matrixIsEqual = true;
  std::stringstream sstream;
  mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
  mitk::ItkImageFileReader::Pointer imageReader = mitk::ItkImageFileReader::New();
  mitk::Convert2Dto3DImageFilter::Pointer convertFilter = mitk::Convert2Dto3DImageFilter::New();

  ///////////////////////////////////////
  // Create 2D Image with a 3D rotation from scratch.
  typedef itk::Image<double,2> ImageType;
  ImageType::Pointer itkImage = ImageType::New();
  ImageType::RegionType myRegion;
  ImageType::SizeType mySize;
  ImageType::IndexType myIndex;
  ImageType::SpacingType mySpacing;
  mySpacing[0] = 1;
  mySpacing[1] = 1;
  myIndex[0] = 0;
  myIndex[1] = 0;
  mySize[0] = 50;
  mySize[1] = 50;
  myRegion.SetSize( mySize);
  myRegion.SetIndex( myIndex );
  itkImage->SetSpacing(mySpacing);
  itkImage->SetRegions( myRegion);
  itkImage->Allocate();
  itkImage->FillBuffer(50);

  mitk::Image::Pointer mitkImage2D;
  mitk::CastToMitkImage(itkImage,mitkImage2D);

  // rotate
  mitk::Point3D p;
  p[0] = 1;
  p[1] = 3;
  p[2] = 5;
  mitk::Vector3D v;
  v[0] = 0.3;
  v[1] = 1;
  v[2] = 0.1;
  mitk::RotationOperation op(mitk::OpROTATE, p, v, 35);
  mitkImage2D->GetGeometry()->ExecuteOperation(&op);


  // Save original Geometry infos
  mitk::Vector3D Original_col0, Original_col1, Original_col2;
  Original_col0.Set_vnl_vector(mitkImage2D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
  Original_col1.Set_vnl_vector(mitkImage2D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
  Original_col2.Set_vnl_vector(mitkImage2D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));
  MITK_INFO << "Rotated Matrix: " << Original_col0 << " " << Original_col1 << " " << Original_col2;

  mitk::Point3D Original_Origin = mitkImage2D->GetGeometry()->GetOrigin();
  mitk::Vector3D Original_Spacing = mitkImage2D->GetGeometry()->GetSpacing();


  MITK_TEST_CONDITION_REQUIRED( mitkImage2D->GetDimension() == 2   , "Created Image is Dimension 2");

  ///////////////////////////////////////
  // mitkImage2D is now a 2D image with 3D Geometry information.
  // Save it without conversion and load it again. It should have an identitiy matrix
  sstream.clear();
  sstream << MITK_TEST_OUTPUT_DIR << "" << "/rotatedImage2D";
  imageWriter->SetInput(mitkImage2D);
  imageWriter->SetFileName(sstream.str().c_str());
  imageWriter->SetExtension(".nrrd");
  imageWriter->Write();
  sstream << ".nrrd";

  imageReader->SetFileName(sstream.str().c_str());
  imageReader->Update();
  mitk::Image::Pointer imageLoaded2D = imageReader->GetOutput();

  // check if image can be loaded
  MITK_TEST_CONDITION_REQUIRED( imageLoaded2D.IsNotNull() , "Loading saved 2D Image");


  MITK_TEST_CONDITION_REQUIRED( imageLoaded2D->GetDimension() == 2   , "Loaded Image is Dimension 2");

  // check if spacing is ok
  mitk::Vector3D Loaded2D_Spacing = imageLoaded2D->GetGeometry()->GetSpacing();

  error = abs(Loaded2D_Spacing[0] - Original_Spacing[0]) +
     abs(Loaded2D_Spacing[1] - Original_Spacing[1]) +
     abs(Loaded2D_Spacing[2] - 1) ;

  MITK_TEST_CONDITION_REQUIRED( error < eps , "Compare Geometry: Spacing");

  // Check origin
  mitk::Point3D Loaded2D_Origin = imageLoaded2D->GetGeometry()->GetOrigin();

  error = abs(Loaded2D_Origin[0] - Original_Origin[0]) +
     abs(Loaded2D_Origin[1] - Original_Origin[1]) +
     abs(Loaded2D_Origin[2] - 0) ;

  MITK_TEST_CONDITION_REQUIRED( error < eps, "Compare Geometry: Origin");

  // Check matrix
  mitk::Vector3D Loaded2D_col0, Loaded2D_col1, Loaded2D_col2;
  Loaded2D_col0.Set_vnl_vector(imageLoaded2D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
  Loaded2D_col1.Set_vnl_vector(imageLoaded2D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
  Loaded2D_col2.Set_vnl_vector(imageLoaded2D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));

  if (
     (abs(1 - Loaded2D_col0[0]) > eps) ||
     (abs(0 - Loaded2D_col0[1]) > eps) ||
     (abs(0 - Loaded2D_col0[2]) > eps) ||
     (abs(0 - Loaded2D_col1[0]) > eps) ||
     (abs(1 - Loaded2D_col1[1]) > eps) ||
     (abs(0 - Loaded2D_col1[2]) > eps) ||
     (abs(0 - Loaded2D_col2[0]) > eps) ||
     (abs(0 - Loaded2D_col2[1]) > eps) ||
     (abs(1 - Loaded2D_col2[2]) > eps) )
  {
     matrixIsEqual = false;
  }
  else
     matrixIsEqual = true;

  MITK_TEST_CONDITION_REQUIRED( matrixIsEqual , "Compare Geometry: Matrix");


  ///////////////////////////////////////
  // mitkImage2D is a 2D image with 3D Geometry information.
  // Convert it with filter to a 3D image and check if everything went well
  convertFilter->SetInput(mitkImage2D);
  convertFilter->Update();
  mitk::Image::Pointer mitkImage3D = convertFilter->GetOutput();

  MITK_TEST_CONDITION_REQUIRED( mitkImage3D->GetDimension() == 3   , "Converted Image is Dimension 3");

  // check if geometry is still same
  mitk::Vector3D Converted_Spacing = mitkImage3D->GetGeometry()->GetSpacing();

  error = abs(Converted_Spacing[0] - Original_Spacing[0]) +
     abs(Converted_Spacing[1] - Original_Spacing[1]) +
     abs(Converted_Spacing[2] - Original_Spacing[2]) ;

  MITK_TEST_CONDITION_REQUIRED( error < eps , "Compare Geometry: Spacing");


  mitk::Point3D Converted_Origin = mitkImage3D->GetGeometry()->GetOrigin();

  error = abs(Converted_Origin[0] - Original_Origin[0]) +
     abs(Converted_Origin[1] - Original_Origin[1]) +
     abs(Converted_Origin[2] - Original_Origin[2]) ;

  MITK_INFO << Converted_Origin << " and " << Original_Origin;
  MITK_TEST_CONDITION_REQUIRED(  error < eps    , "Compare Geometry: Origin");

  mitk::Vector3D Converted_col0, Converted_col1, Converted_col2;
  Converted_col0.Set_vnl_vector(mitkImage3D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
  Converted_col1.Set_vnl_vector(mitkImage3D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
  Converted_col2.Set_vnl_vector(mitkImage3D->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));

     if (
        (abs(Original_col0[0] - Converted_col0[0]) > eps) ||
        (abs(Original_col0[1] - Converted_col0[1]) > eps) ||
        (abs(Original_col0[2] - Converted_col0[2]) > eps) ||
        (abs(Original_col1[0] - Converted_col1[0]) > eps) ||
        (abs(Original_col1[1] - Converted_col1[1]) > eps) ||
        (abs(Original_col1[2] - Converted_col1[2]) > eps) ||
        (abs(Original_col2[0] - Converted_col2[0]) > eps) ||
        (abs(Original_col2[1] - Converted_col2[1]) > eps) ||
        (abs(Original_col2[2] - Converted_col2[2]) > eps) )
     {
        MITK_INFO << "Oh No! Original Image Matrix and Converted Image Matrix are different!";
        MITK_INFO << "original Image:" << Original_col0 << " " << Original_col1 << " " << Original_col2;
        MITK_INFO << "converted Image:" << Converted_col0 << " " << Converted_col1 << " " << Converted_col2;
        matrixIsEqual = false;
     }
     else
        matrixIsEqual = true;


     MITK_TEST_CONDITION_REQUIRED( matrixIsEqual , "Compare Geometry: Matrix");


     ///////////////////////////////////////
     // So far it seems good! now try to save and load the file

     std::stringstream sstream2;
     sstream2 << MITK_TEST_OUTPUT_DIR << "" << "/rotatedImage";
     imageWriter->SetInput(mitkImage3D);
     imageWriter->SetFileName(sstream2.str().c_str());
     imageWriter->SetExtension(".nrrd");
     imageWriter->Write();
     sstream2 << ".nrrd";
     imageReader->SetFileName(sstream2.str().c_str());
     imageReader->Update();
     mitk::Image::Pointer imageLoaded = imageReader->GetOutput();

     // check if image can be loaded
     MITK_TEST_CONDITION_REQUIRED( imageLoaded.IsNotNull() , "Loading saved Image");

     // check if loaded image is still what it should be:
     MITK_TEST_CONDITION_REQUIRED( imageLoaded->GetDimension() == 3   , "Loaded Image is Dimension 3");

     // check if geometry is still same
     mitk::Vector3D Loaded_Spacing = imageLoaded->GetGeometry()->GetSpacing();
     error = abs(Loaded_Spacing[0] - Original_Spacing[0]) +
        abs(Loaded_Spacing[1] - Original_Spacing[1]) +
        abs(Loaded_Spacing[2] - Original_Spacing[2]) ;

     MITK_TEST_CONDITION_REQUIRED( error < eps    , "Compare Geometry: Spacing");

     mitk::Point3D Loaded_Origin = imageLoaded->GetGeometry()->GetOrigin();
     error = abs(Loaded_Origin[0] - Original_Origin[0]) +
        abs(Loaded_Origin[1] - Original_Origin[1]) +
        abs(Loaded_Origin[2] - Original_Origin[2]) ;
     MITK_TEST_CONDITION_REQUIRED( error < eps     , "Compare Geometry: Origin");

     mitk::Vector3D Loaded_col0, Loaded_col1, Loaded_col2;
     Loaded_col0.Set_vnl_vector(imageLoaded->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
     Loaded_col1.Set_vnl_vector(imageLoaded->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
     Loaded_col2.Set_vnl_vector(imageLoaded->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));

     if (
        (abs(Original_col0[0] - Loaded_col0[0]) > eps) ||
        (abs(Original_col0[1] - Loaded_col0[1]) > eps) ||
        (abs(Original_col0[2] - Loaded_col0[2]) > eps) ||
        (abs(Original_col1[0] - Loaded_col1[0]) > eps) ||
        (abs(Original_col1[1] - Loaded_col1[1]) > eps) ||
        (abs(Original_col1[2] - Loaded_col1[2]) > eps) ||
        (abs(Original_col2[0] - Loaded_col2[0]) > eps) ||
        (abs(Original_col2[1] - Loaded_col2[1]) > eps) ||
        (abs(Original_col2[2] - Loaded_col2[2]) > eps) )
     {
        MITK_INFO << "Oh No! Original Image Matrix and Converted Image Matrix are different!";
        MITK_INFO << "original Image:" << Original_col0 << " " << Original_col1 << " " << Original_col2;
        MITK_INFO << "converted Image:" << Loaded_col0 << " " << Loaded_col1 << " " << Loaded_col2;
        matrixIsEqual = false;
     }
     else
        matrixIsEqual = true;


     MITK_TEST_CONDITION_REQUIRED( matrixIsEqual , "Compare Geometry: Matrix");




   MITK_TEST_END();
}
