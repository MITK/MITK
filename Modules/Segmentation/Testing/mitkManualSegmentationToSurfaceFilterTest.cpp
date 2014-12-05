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


  std::cout<<"Input file: "<<fileIn<<std::endl;
  std::cout<<"Output file: "<<fileOut<<std::endl;
  mitk::BaseData::Pointer baseData;
  mitk::Image::Pointer image;

  try
  {
    std::cout << "Loading file: " << std::flush;
    baseData = mitk::IOUtil::LoadBaseData(fileIn);

    if(baseData.IsNull())
    {
      std::cout<<"file could not be loaded [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }

    image = dynamic_cast<mitk::Image*>(baseData.GetPointer());
    if(image.IsNull())
    {
      std::cout<<"file not an image - test will not be applied [PASSED]"<<std::endl;
      std::cout<<"[TEST DONE]"<<std::endl;
      return EXIT_SUCCESS;
    }
  }
  catch ( const mitk::Exception& e )
  {
    std::cout << "Exception: " << e << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }


  mitk::ManualSegmentationToSurfaceFilter::Pointer filter;
  std::cout << "Instantiat mitk::ManualSegmentationToSurfaceFilter - implicit: mitk::ImageToSurface: ";
  filter = mitk::ManualSegmentationToSurfaceFilter::New();

  if (filter.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  }


  //Wirter instance
  if (filter.IsNull())
  {
    std::cout<<"Instantiat SurfaceVtkWirter: [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Create surface with default settings: ";
  if (image->GetDimension()==3)
  {
    filter->SetInput(image);
    filter->Update();

    try
    {
      mitk::IOUtil::SaveSurface(filter->GetOutput(), fileOut);
    }
    catch(const mitk::Exception&)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }

    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Create surface all optimised settings: ";
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
      mitk::IOUtil::SaveSurface(filter->GetOutput(), fileOut);
    }
    catch (const mitk::Exception& e)
    {
      std::cout<<"caught exception: "<<e<<std::endl;
    }
  }
  else
  {
    std::cout<<"Image not suitable for filter: Dimension!=3"<<std::endl;
  }

  std::cout<<"[TEST DONE]"<<std::endl;

  return EXIT_SUCCESS;
}



