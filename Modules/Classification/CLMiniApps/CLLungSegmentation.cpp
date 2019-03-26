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

#include <mitkOtsuSegmentationFilter.h>
#include "mitkLabelSetImage.h"

#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"

#include <mitkMorphologicalOperations.h>

#include <itkConnectedThresholdImageFilter.h>
#include <itkImageRegionConstIterator.h>

template<typename TPixel, unsigned int VImageDimension>
void GetMinimum(itk::Image<TPixel, VImageDimension>* itkImage, double &minimum)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;

  minimum = std::numeric_limits<double>::max();
  itk::ImageRegionConstIterator<InputImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());

  while (!iter.IsAtEnd())
  {
    minimum = std::min<double>(minimum, iter.Get());
    ++iter;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void StartRegionGrowing(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer &result)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  // convert world coordinates to image indices
  IndexType startIndex;
  IndexType seedIndex;
  IndexType bestSeedIndex;
  startIndex[0] = itkImage->GetLargestPossibleRegion().GetSize()[0]/2;
  startIndex[1] = itkImage->GetLargestPossibleRegion().GetSize()[1]/2;
  startIndex[2] = itkImage->GetLargestPossibleRegion().GetSize()[2]/2;
  auto region = itkImage->GetLargestPossibleRegion();
  auto spacing = itkImage->GetSpacing();
  spacing[0] = itkImage->GetSpacing()[0];
  spacing[1] = itkImage->GetSpacing()[1];
  spacing[2] = itkImage->GetSpacing()[2];

  int minimumDistance = 50 * 50 * (spacing[0] + spacing[1] + spacing[2]);

  for (int x = -50; x < 50; ++x)
  {
    for (int y = -50; y < 50; ++y)
    {
      for (int z = -20; z < 20; ++z)
      {
        seedIndex[0] = startIndex[0] + x;
        seedIndex[1] = startIndex[1] + y;
        seedIndex[2] = startIndex[2] + z;
        if (region.IsInside(seedIndex))
        {
          if (itkImage->GetPixel(seedIndex) > 0)
          {
            int newDistance = x*x*spacing[0] + y*y*spacing[1] + z*z*spacing[2];
            if (newDistance < minimumDistance)
            {
              bestSeedIndex = seedIndex;
              minimumDistance = newDistance;
            }
          }
        }
      }
    }
  }
  seedIndex = bestSeedIndex;

  MITK_INFO << "Seedpoint: " << seedIndex;
  //perform region growing in desired segmented region
  regionGrower->SetInput(itkImage);
  regionGrower->AddSeed(seedIndex);

  regionGrower->SetLower(1);
  regionGrower->SetUpper(255);

  try
  {
    regionGrower->Update();
  }
  catch (const itk::ExceptionObject&)
  {
    return; // can't work
  }
  catch (...)
  {
    return;
  }

  //Store result and preview
  mitk::CastToMitkImage(regionGrower->GetOutput(), result);
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Dicom Loader");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::Directory, "Input folder:", "Input folder", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file",us::Any(),false, false, false, mitkCommandLineParser::Output);


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFile = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["output"]);

  MITK_INFO << "Start Image Loading";

  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(inputFile);

  MITK_INFO << "Loaded Image";
  double minimum = 0;
  AccessByItk_1(image, GetMinimum, minimum);

  unsigned int offset = 0;
  if (minimum < -3000)
  {
    offset = 1;
  }
  MITK_INFO << "With Minimum at " << minimum<< " Offset is set to: " << offset;

  mitk::OtsuSegmentationFilter::Pointer otsuFilter = mitk::OtsuSegmentationFilter::New();
  otsuFilter->SetNumberOfThresholds(1+offset);
  otsuFilter->SetValleyEmphasis(false);
  otsuFilter->SetNumberOfBins(128);
  otsuFilter->SetInput(image);
  try
  {
    otsuFilter->Update();
  }
  catch (...)
  {
    mitkThrow() << "itkOtsuFilter error (image dimension must be in {2, 3} and image must not be RGB)";
  }

  MITK_INFO << "Calculated Otsu";

  mitk::LabelSetImage::Pointer resultImage = mitk::LabelSetImage::New();
  resultImage->InitializeByLabeledImage(otsuFilter->GetOutput());
  mitk::Image::Pointer rawMask = resultImage->CreateLabelMask(offset);
  mitk::Image::Pointer pickedMask;

  AccessByItk_1(rawMask, StartRegionGrowing, pickedMask);

  mitk::MorphologicalOperations::FillHoles(pickedMask);
  mitk::MorphologicalOperations::Closing(pickedMask, 5, mitk::MorphologicalOperations::StructuralElementType::Ball);
  mitk::MorphologicalOperations::FillHoles(pickedMask);


  mitk::IOUtil::Save(pickedMask, outFileName);

  return EXIT_SUCCESS;
}
