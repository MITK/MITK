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


#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>

#include "mitkSurfaceToImageFilter.h"

#include <vtkPolyData.h>

int mitkSurfaceToImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN( "mitkSurfaceToImageFilterTest");
  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

  mitk::Surface::Pointer inputSurface = mitk::IOUtil::LoadSurface( std::string(argv[1]) );

  //todo I don't know if this image is always needed. There is no documentation of the filter. Use git blame and ask the author.
  mitk::Image::Pointer additionalInputImage = mitk::Image::New();
  additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(), *inputSurface->GetGeometry());

  //Arrange the filter
  //The docu does not really tell if this is always needed. Could we skip SetImage in any case?
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->SetInput(inputSurface);
  surfaceToImageFilter->SetImage(additionalInputImage);
  surfaceToImageFilter->Update();

  MITK_TEST_CONDITION_REQUIRED( surfaceToImageFilter->GetOutput()->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR, "SurfaceToImageFilter_AnyInputImageAndModeSetToBinary_ResultIsImageWithUCHARPixelType");

  MITK_TEST_END();
}
