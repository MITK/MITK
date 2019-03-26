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

#include "mitkProperties.h"

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"

#include "itkImageRegionIterator.h"
// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

struct Params {
  bool invert;
  float zeroValue;
};
template<typename TPixel, unsigned int VImageDimension>
void
CreateXRay(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask1, std::string output, Params param)
{
  typedef itk::Image<TPixel, VImageDimension>         ImageType;
  typedef itk::Image<unsigned char, VImageDimension>  MaskType;
  typedef itk::Image<double, 2 >                      NewImageType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask1, itkMask);

  NewImageType::SpacingType newSpacing;
  typename ImageType::SpacingType spacing;
  spacing[0] = 0;
  spacing[1] = 0;
  spacing[2] = 0;
  spacing = itkImage->GetSpacing();

  NewImageType::RegionType region1,region2,region3,region1m,region2m,region3m;
  NewImageType::IndexType start;
  start[0] = 0; start[1] = 0;

  NewImageType::SizeType size1, size2, size3;
  size1[0] = mask1->GetDimensions()[0];
  size2[0] = mask1->GetDimensions()[0];
  size3[0] = mask1->GetDimensions()[1];
  size1[1] = mask1->GetDimensions()[1];
  size2[1] = mask1->GetDimensions()[2];
  size3[1] = mask1->GetDimensions()[2];

  region1.SetSize(size1);
  region1m.SetSize(size1);
  region2.SetSize(size2);
  region2m.SetSize(size2);
  region3.SetSize(size3);
  region3m.SetSize(size3);
  region1.SetIndex(start);
  region1m.SetIndex(start);
  region2.SetIndex(start);
  region2m.SetIndex(start);
  region3.SetIndex(start);
  region3m.SetIndex(start);

  NewImageType::Pointer image1 = NewImageType::New();
  image1->SetRegions(region1);
  image1->Allocate();
  image1->FillBuffer(0);
  newSpacing[0] = spacing[0]; newSpacing[1] = spacing[1];
  image1->SetSpacing(newSpacing);
  NewImageType::Pointer image2 = NewImageType::New();
  image2->SetRegions(region2);
  image2->Allocate();
  image2->FillBuffer(0);
  newSpacing[0] = spacing[0]; newSpacing[1] = spacing[2];
  image2->SetSpacing(newSpacing);
  NewImageType::Pointer image3 = NewImageType::New();
  image3->SetRegions(region3);
  image3->Allocate();
  image3->FillBuffer(0);
  newSpacing[0] = spacing[1]; newSpacing[1] = spacing[2];
  image3->SetSpacing(newSpacing);
  NewImageType::Pointer image1m = NewImageType::New();
  image1m->SetRegions(region1m);
  image1m->Allocate();
  image1m->FillBuffer(0);
  newSpacing[0] = spacing[0]; newSpacing[1] = spacing[1];
  image1m->SetSpacing(newSpacing);
  NewImageType::Pointer image2m = NewImageType::New();
  image2m->SetRegions(region2m);
  image2m->Allocate();
  image2m->FillBuffer(0);
  newSpacing[0] = spacing[0]; newSpacing[1] = spacing[2];
  image2m->SetSpacing(newSpacing);
  NewImageType::Pointer image3m = NewImageType::New();
  image3m->SetRegions(region3m);
  image3m->Allocate();
  image3m->FillBuffer(0);
  newSpacing[0] = spacing[1]; newSpacing[1] = spacing[2];
  image3m->SetSpacing(newSpacing);

  for (unsigned int x = 0; x < mask1->GetDimensions()[0]; ++x)
  {
    for (unsigned int y = 0; y < mask1->GetDimensions()[1]; ++y)
    {
      for (unsigned int z = 0; z < mask1->GetDimensions()[2]; ++z)
      {
        NewImageType::IndexType newIndex;
        typename ImageType::IndexType index;
        index[0] = x; index[1] = y; index[2] = z;
        double pixel = itkImage->GetPixel(index)+1024;
        pixel = pixel / 1000.0;
        pixel = (pixel < 0)? 0 : pixel;
        newIndex[0] = x; newIndex[1] = y;
        image1->SetPixel(newIndex, image1->GetPixel(newIndex) + pixel);
        newIndex[0] = x; newIndex[1] = z;
        image2->SetPixel(newIndex, image2->GetPixel(newIndex) + pixel);
        newIndex[0] = y; newIndex[1] = z;
        image3->SetPixel(newIndex, image3->GetPixel(newIndex) + pixel);
        if (itkMask->GetPixel(index) > 0 && !param.invert)
        {
          pixel = param.zeroValue + 1024;
          pixel = pixel / 1000.0;
        }
        if (itkMask->GetPixel(index) < 1 && param.invert)
        {
          pixel = param.zeroValue + 1024;
          pixel = pixel / 1000.0;
        }
        pixel = (pixel < 0)? 0 : pixel;
        newIndex[0] = x; newIndex[1] = y;
        image1m->SetPixel(newIndex, image1m->GetPixel(newIndex) + pixel);
        newIndex[0] = x; newIndex[1] = z;
        image2m->SetPixel(newIndex, image2m->GetPixel(newIndex) + pixel);
        newIndex[0] = y; newIndex[1] = z;
        image3m->SetPixel(newIndex, image3m->GetPixel(newIndex) + pixel);
      }
    }
  }


  mitk::Image::Pointer img = mitk::ImportItkImage(image1);
  mitk::IOUtil::Save(img, output + "1.nrrd");
  img = mitk::ImportItkImage(image2);
  mitk::IOUtil::Save(img, output + "2.nrrd");
  img = mitk::ImportItkImage(image3);
  mitk::IOUtil::Save(img, output + "3.nrrd");
  img = mitk::ImportItkImage(image1m);
  mitk::IOUtil::Save(img, output + "1m.nrrd");
  img = mitk::ImportItkImage(image2m);
  mitk::IOUtil::Save(img, output + "2m.nrrd");
  img = mitk::ImportItkImage(image3m);
  mitk::IOUtil::Save(img, output + "3m.nrrd");
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Dicom Loader");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("-","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::Directory, "Input image:", "Input folder", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::Directory, "Input mask:", "Input folder", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("invert", "invert", mitkCommandLineParser::Bool, "Input mask:", "Input folder", us::Any());
  parser.addArgument("zero_value", "zero", mitkCommandLineParser::Float, "Output file:", "Output file", us::Any());

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputImage = us::any_cast<std::string>(parsedArgs["input"]);
  MITK_INFO << inputImage;
  std::string inputMask = us::any_cast<std::string>(parsedArgs["mask"]);
  MITK_INFO << inputMask;

  Params param;
  param.invert = false;
  param.zeroValue = 0;
  if (parsedArgs.count("invert"))
  {
    param.invert = true;
  }
  if (parsedArgs.count("zero_value"))
  {
    param.zeroValue = us::any_cast<float>(parsedArgs["zero_value"]);
  }


  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(inputImage);
  mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(inputMask);

  AccessByItk_3(image, CreateXRay, mask, parsedArgs["output"].ToString(),param);

  //const mitk::Image::Pointer image = *imageIter;
  //mitk::IOUtil::SaveImage(image,outFileName);



  return EXIT_SUCCESS;
}
