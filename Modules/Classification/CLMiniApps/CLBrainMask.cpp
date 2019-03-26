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
#ifndef mitkCLPolyToNrrd_cpp
#define mitkCLPolyToNrrd_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

#include "itkImageRegionIterator.h"
// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

template<typename TPixel, unsigned int VImageDimension>
void
  Normalize(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer im2, mitk::Image::Pointer mask1, std::string output)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> MaskType;

  typename ImageType::Pointer itkIm2 = ImageType::New();
  typename MaskType::Pointer itkMask1 = MaskType::New();
  mitk::CastToItkImage(im2, itkIm2);
  mitk::CastToItkImage(mask1, itkMask1);

  itk::ImageRegionIterator<ImageType> iterI1(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType> iterI2(itkIm2, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<MaskType> iter(itkMask1, itkImage->GetLargestPossibleRegion());
  while (! iter.IsAtEnd())
  {
    unsigned char maskV = 0;
    if (iterI1.Value() > 0.0001 && iterI2.Value() > 0.00001)
      maskV = 1;
    iter.Set(maskV);
    ++iter;
    ++iterI1;
    ++iterI2;
  }

  mitk::Image::Pointer img = mitk::ImportItkImage(itkMask1);
  mitk::IOUtil::Save(img, output);
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::Image, "Input Image", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("image2", "i2", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 0", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 1", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output Image", "Target file. The output statistic is appended to this file.", us::Any(), false, false, false, mitkCommandLineParser::Output);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("MR Normalization Tool");
  parser.setDescription("Normalizes a MR image. Sets the Median of the tissue covered by mask 0 to 0 and the median of the area covered by mask 1 to 1.");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["image"].ToString());
  mitk::Image::Pointer im2= mitk::IOUtil::Load<mitk::Image>(parsedArgs["image2"].ToString());
  mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(parsedArgs["mask"].ToString());

  AccessByItk_3(image, Normalize, im2, mask, parsedArgs["output"].ToString());

  return 0;
}

#endif
