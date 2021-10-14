/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImage.h>
#include <itksys/SystemTools.hxx>

#include <mitkIOUtil.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkStandaloneDataStorage.h>

#include <vtkImageViewer.h>
#include <vtkRenderWindowInteractor.h>

///
/// Small application for demonstrating the interaction between MITK,
/// ITK and VTK (not necessarily useful).
///
int main(int /*argc*/, char **argv)
{
  // MITK: Read a .nrrd file, e.g. Core/Code/Testing/Data/Pic3D.nrrd from
  // disk
  const char *filename = argv[1];

  mitk::Image::Pointer mitkImage;
  try
  {
    mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
  }
  catch (const std::exception &e)
  {
    MITK_WARN << "Could not open file " << filename;
    return EXIT_FAILURE;
  }

  // ITK: Image smoothing
  // Create ITK image, cast from MITK image
  typedef itk::Image<short, 3> ImageType;
  ImageType::Pointer itkImage = ImageType::New();
  mitk::CastToItkImage(mitkImage, itkImage);

  typedef itk::DiscreteGaussianImageFilter<ImageType, ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(itkImage);
  filter->SetVariance(2);
  filter->SetMaximumKernelWidth(5);

  filter->Update(); // run filter

  // reimport filtered image data
  mitk::CastToMitkImage(filter->GetOutput(), mitkImage);

  // VTK: Show result in renderwindow
  vtkImageViewer *viewer = vtkImageViewer::New();
  vtkRenderWindowInteractor *renderWindowInteractor = vtkRenderWindowInteractor::New();
  viewer->SetupInteractor(renderWindowInteractor);
  viewer->SetInputData(mitkImage->GetVtkImageData());
  viewer->Render();
  viewer->SetColorWindow(255);
  viewer->SetColorLevel(128);
  renderWindowInteractor->Start();

  renderWindowInteractor->Delete();
  viewer->Delete();

  return EXIT_SUCCESS;
}
