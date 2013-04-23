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
#include <itkImage.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itksys/SystemTools.hxx>

#include <mitkPicFileReader.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

#include <vtkImageViewer.h>
#include <vtkRenderWindowInteractor.h>

///
/// Small application for demonstrating the interaction between MITK,
/// ITK and VTK (not necessarily useful).
///
int main( int /*argc*/, char ** argv )
{
  // MITK: Read a .pic.gz file, e.g. Core/Code/Testing/Data/Pic3D.pic.gz from
  // disk
  mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();
  const char * filename = argv[1];
  try
  {
    reader->SetFileName(filename);
    reader->Update();
  }
  catch(...)
  {
    fprintf( stderr, "Could not open file %s \n\n", filename );
    return EXIT_FAILURE;
  }
  mitk::Image::Pointer mitkImage = reader->GetOutput();

  // ITK: Image smoothing
  // Create ITK image, cast from MITK image
  typedef itk::Image< short , 3 > ImageType;
  ImageType::Pointer itkImage = ImageType::New();
  mitk::CastToItkImage( mitkImage, itkImage );

  typedef itk::DiscreteGaussianImageFilter<ImageType, ImageType > FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkImage );
  filter->SetVariance( 2 );
  filter->SetMaximumKernelWidth( 5 );

  filter->Update(); // run filter

  // reimport filtered image data
  mitk::CastToMitkImage( filter->GetOutput(), mitkImage );

  // VTK: Show result in renderwindow
  vtkImageViewer *viewer=vtkImageViewer::New();
  vtkRenderWindowInteractor* renderWindowInteractor
      =vtkRenderWindowInteractor ::New();
  viewer->SetupInteractor(renderWindowInteractor);
  viewer->SetInput(mitkImage->GetVtkImageData());
  viewer->Render();
  viewer->SetColorWindow(255);
  viewer->SetColorLevel(128);
  renderWindowInteractor->Start();

  renderWindowInteractor->Delete();
  viewer->Delete();

  return EXIT_SUCCESS;
}
