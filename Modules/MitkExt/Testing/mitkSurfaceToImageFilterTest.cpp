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


#include "mitkSurfaceToImageFilter.h"
#include "mitkDataNodeFactory.h"
#include "mitkPicFileWriter.h"

#include <vtkPolyData.h>

#include <fstream>

int mitkSurfaceToImageFilterTest(int argc, char* argv[])
{
  mitk::SurfaceToImageFilter::Pointer s2iFilter;
  std::cout << "Testing mitk::Surface::New(): " << std::flush;
  s2iFilter = mitk::SurfaceToImageFilter::New();
  if (s2iFilter.IsNull()) 
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else 
  {
    std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Loading file: " << std::flush;
  if(argc==0)
  {
    std::cout<<"no file specified [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  mitk::Surface::Pointer surface = NULL;
  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
  try
  {
    std::cout<<argv[1]<<std::endl;
    factory->SetFileName( argv[1] );
    factory->Update();

    if(factory->GetNumberOfOutputs()<1)
    {
      std::cout<<"file could not be loaded [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    mitk::DataNode::Pointer node = factory->GetOutput( 0 );
    surface = dynamic_cast<mitk::Surface*>(node->GetData());
    if(surface.IsNull())
    {
      std::cout<<"file not a surface - test will not be applied [PASSED]"<<std::endl;
      std::cout<<"[TEST DONE]"<<std::endl;
      return EXIT_SUCCESS;
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    std::cout << "Exception: " << ex << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing number of points of surface: " << std::flush;
  if(surface->GetVtkPolyData()->GetNumberOfPoints() == 0)
  {
    std::cout<<"number of points is 0 - test will not be applied [PASSED]"<<std::endl;
    std::cout<<"[TEST DONE]"<<std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "Testing creation of mitk::Image with same Geometry as Surface: " << std::flush;
  mitk::Image::Pointer image = mitk::Image::New();
  //surface->UpdateOutputInformation(); //is not necessary anymore (bug #1536), should never be neccessary
  image->Initialize(typeid(unsigned char), *surface->GetGeometry());

  std::cout << "Testing mitk::SurfaceToImageFilter::MakeOutputBinaryOn(): " << std::flush;
  s2iFilter->MakeOutputBinaryOn();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::SurfaceToImageFilter::SetInput(): " << std::flush;
  s2iFilter->SetInput(surface);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::SurfaceToImageFilter::SetImage(): " << std::flush;
  s2iFilter->SetImage(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::SurfaceToImageFilter::Update(): " << std::flush;
  s2iFilter->Update();
  std::cout<<"[PASSED]"<<std::endl;

  //mitk::PicFileWriter::Pointer picWriter = mitk::PicFileWriter::New();
  //picWriter->SetInput(s2iFilter->GetOutput());
  //picWriter->SetFileName("SurfaceToImageFilterTestOutput.pic");
  //picWriter->Write();

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
