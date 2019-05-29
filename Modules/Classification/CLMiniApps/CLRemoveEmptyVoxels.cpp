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

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"
#include "mitkImageCast.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include "mitkImageGenerator.h"

int main(int argc, char* argv[])
{
  typedef itk::Image<double, 3> ImageType;
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef ImageType::Pointer ImagePointerType;
  typedef MaskImageType::Pointer MaskImagePointerType;

  typedef itk::ImageRegionConstIterator<ImageType> ConstIteratorType;
  typedef itk::ImageRegionConstIterator<MaskImageType> ConstMaskIteratorType;
  typedef itk::ImageRegionIterator<ImageType> IteratorType;
  typedef itk::ImageRegionIterator<MaskImageType> MaskIteratorType;

  mitkCommandLineParser parser;

  parser.setTitle("Remove empty voxels Sampling");
  parser.setCategory("Classification Command Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("mask-input", "mi", mitkCommandLineParser::Directory, "Input file:", "Input file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask-output", "mo", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

  for (int i = 0; i < 100; ++i)
  {
    std::stringstream s1; s1 << i; std::string number = s1.str();
    parser.addArgument("input"+number, "i"+number, mitkCommandLineParser::File, "Input file", "input file", us::Any(), true, false, false, mitkCommandLineParser::Output);
    parser.addArgument("output" + number, "o" + number, mitkCommandLineParser::File, "Output File", "Output file", us::Any(), true, false, false, mitkCommandLineParser::Output);
  }

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  // Load Mask Image and count number of non-zero voxels
  mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(parsedArgs["mask-input"].ToString());
  MaskImagePointerType itkMask = MaskImageType::New();
  mitk::CastToItkImage(mask, itkMask);
  ConstMaskIteratorType maskIter(itkMask, itkMask->GetLargestPossibleRegion());
  std::size_t nonZero = 0;
  while (!maskIter.IsAtEnd())
  {
    if (maskIter.Value() > 0)
    {
      ++nonZero;
    }
    ++maskIter;
  }
  maskIter.GoToBegin();

  // Create new mask image
  auto mitkNewMask = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(nonZero, 1, 1, 1, 1, 1);
  MaskImagePointerType itkNewMask = MaskImageType::New();
  mitk::CastToItkImage(mitkNewMask, itkNewMask);
  MaskIteratorType newMaskIter(itkNewMask, itkNewMask->GetLargestPossibleRegion());

  // Read additional image
  std::vector<mitk::Image::Pointer> mitkImagesVector;
  std::vector<ImagePointerType> itkImageVector;
  std::vector<ImagePointerType> itkOutputImageVector;
  std::vector<ConstIteratorType> inputIteratorVector;
  std::vector<IteratorType> outputIteratorVector;
  for (int i = 0; i < 100; ++i)
  {
    std::stringstream s1; s1 << i; std::string number = s1.str();
    if (parsedArgs.count("input" + number) < 1)
      break;
    if (parsedArgs.count("output" + number) < 1)
      break;

    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["input"+number].ToString());
    mitkImagesVector.push_back(image);

    ImagePointerType itkImage = ImageType::New();
    mitk::CastToItkImage(image, itkImage);
    itkImageVector.push_back(itkImage);

    ConstIteratorType iter(itkImage, itkImage->GetLargestPossibleRegion());
    inputIteratorVector.push_back(iter);

    auto mitkNewImage = mitk::ImageGenerator::GenerateGradientImage<double>(nonZero, 1, 1, 1, 1, 1);
    ImagePointerType itkNewOutput = ImageType::New();
    mitk::CastToItkImage(mitkNewImage, itkNewOutput);
    IteratorType outputIter(itkNewOutput, itkNewOutput->GetLargestPossibleRegion());
    itkOutputImageVector.push_back(itkNewOutput);
    outputIteratorVector.push_back(outputIter);
  }

  // Convert non-zero voxels to the new images
  while (!maskIter.IsAtEnd())
  {
    if (maskIter.Value() > 0)
    {
      newMaskIter.Set(maskIter.Value());
      ++newMaskIter;
      for (std::size_t i = 0; i < outputIteratorVector.size(); ++i)
      {
        outputIteratorVector[i].Set(inputIteratorVector[i].Value());
        ++(outputIteratorVector[i]);
      }
    }
    ++maskIter;
    for (std::size_t i = 0; i < inputIteratorVector.size(); ++i)
    {
      ++(inputIteratorVector[i]);
    }

  }

  // Save the new images
  for (std::size_t i = 0; i < outputIteratorVector.size(); ++i)
  {
    std::stringstream s1; s1 << i; std::string number = s1.str();
    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitk::CastToMitkImage(itkOutputImageVector[i], mitkImage);
    mitk::IOUtil::Save(mitkImage, parsedArgs["output" + number].ToString());
  }
  // Save the new mask
  {
    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitk::CastToMitkImage(itkNewMask, mitkImage);
    mitk::IOUtil::Save(mitkImage, parsedArgs["mask-output"].ToString());
  }

  return EXIT_SUCCESS;
}
