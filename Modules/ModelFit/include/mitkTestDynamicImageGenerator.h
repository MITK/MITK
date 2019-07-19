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

#ifndef __TEST_ARTIFACT_GENERATOR_H
#define __TEST_ARTIFACT_GENERATOR_H

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  typedef itk::Image<int> TestImageType;
  typedef itk::Image<unsigned char> TestMaskType;

  TestImageType::Pointer MITKMODELFIT_EXPORT GenerateTestImage(int factor = 1);

  TestMaskType::Pointer MITKMODELFIT_EXPORT GenerateTestMask();

  Image::Pointer MITKMODELFIT_EXPORT GenerateTestFrame(double timePoint);

  Image::Pointer MITKMODELFIT_EXPORT GenerateTestMaskMITK();

  Image::Pointer MITKMODELFIT_EXPORT GenerateDynamicTestImageMITK();

}

#endif
