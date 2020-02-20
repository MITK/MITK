/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"
#include <mitkImageCast.h>

#include <itkSTAPLEImageFilter.h>

int main(int argc, char* argv[])
{
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef itk::Image<float, 3> ImageType;
  typedef itk::STAPLEImageFilter<MaskImageType, ImageType> FilterType;

  FilterType::Pointer filter = FilterType::New();
  filter->SetForegroundValue(2);
  for (int i = 2; i < argc; ++i)
  {
    MITK_INFO << argv[i];
    MaskImageType::Pointer itkImg = MaskImageType::New();
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(argv[i]);
    mitk::CastToItkImage(img,itkImg);
    filter->SetInput(i-2, itkImg);
  }
  filter->Update();
  auto out = filter->GetOutput();
  mitk::Image::Pointer outImg = mitk::Image::New();
  mitk::CastToMitkImage(out, outImg);
  mitk::IOUtil::Save(outImg, argv[1]);

  return EXIT_SUCCESS;
}
