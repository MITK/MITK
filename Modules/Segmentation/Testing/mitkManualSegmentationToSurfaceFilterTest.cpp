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


#include "mitkManualSegmentationToSurfaceFilter.h"
#include <itksys/SystemTools.hxx>
#include <vtkSTLWriter.h>
#include <mitkIOUtil.h>

#include <fstream>
/**
* Test class for ManualSegmentationToSurfaceFilter and ImageToSurface
* 1. Read an image
* 2. Create a surface
* 3. Create a Surface with all image processing facillities
*/
int mitkManualSegmentationToSurfaceFilterTest(int argc, char* argv[])
{

  if(argc==0)
  {
    std::cout<<"no file specified [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::string path = argv[1];
  itksys::SystemTools::ConvertToUnixSlashes(path);
  std::string fileIn = path;
  std::string fileOut = "BallBinary30x30x30.stl";


  MITK_INFO<<"Eingabe Datei: "<<fileIn;
  MITK_INFO<<"Ausgabe Datei: "<<fileOut;
  mitk::Image::Pointer image = NULL;
  try
  {
    image = mitk::IOUtil::LoadImage( fileIn.c_str() );
    if(image.IsNull())
    {
      MITK_INFO << "file not an image - test will not be applied";
      return EXIT_FAILURE;
    }
  }
  catch ( mitk::Exception &e )
  {
    MITK_INFO << "Exception: " << e.what();
    return EXIT_FAILURE;
  }

  mitk::ManualSegmentationToSurfaceFilter::Pointer filter;
  MITK_INFO << "Instantiat mitk::ManualSegmentationToSurfaceFilter - implicit: mitk::ImageToSurface: ";
  filter = mitk::ManualSegmentationToSurfaceFilter::New();

  MITK_INFO << "Create surface with default settings: ";
  if (image->GetDimension()==3)
  {

    filter->SetInput(image);
    filter->Update();

    mitk::IOUtil::SaveSurface(filter->GetOutput(), fileOut.c_str());

    MITK_INFO << "Create surface all optimised settings: ";
    //configure ImageToSurfaceFilter
    filter->MedianFilter3DOn();
    filter->SetGaussianStandardDeviation(1.5);
    filter->InterpolationOn();
    filter->UseGaussianImageSmoothOn();
    filter->SetThreshold( 1 ); //if( Gauss ) --> TH manipulated for vtkMarchingCube
    filter->SetDecimate( mitk::ImageToSurfaceFilter::DecimatePro );
    filter->SetTargetReduction(0.05f);
    filter->SmoothOn();

    try
    {
      filter->Update();
    }
    catch( itk::ExceptionObject & err )
    {
      MITK_INFO << " ERROR!" << std::endl;
      MITK_ERROR << "ExceptionObject caught!" << std::endl;
      MITK_ERROR << err << std::endl;
      return EXIT_FAILURE;
    }

    try
    {
      mitk::IOUtil::SaveSurface(filter->GetOutput(), fileOut.c_str());
    }
    catch( mitk::Exception &e)
    {
      MITK_INFO <<"Caught exception: "<<e.what();
      return EXIT_FAILURE;
    }
  }
  else
  {
    MITK_INFO<<"Image not suitable for filter: Dimension!=3";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}



