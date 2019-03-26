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

// std includes
#include <string>

// itk includes
#include "itksys/SystemTools.hxx"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkCastImageFilter.h"
#include "itkExtractImageFilter.h"

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

#include <mitkImageTimeSelector.h>
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"

mitkCommandLineParser::StringContainerType inFilenames;
std::string outFileName;
std::string maskFileName;
mitkCommandLineParser::StringContainerType captions;

using ImageVectorType = std::vector<mitk::Image::Pointer>;
ImageVectorType images;

mitk::Image::Pointer mask;

bool verbose(false);

typedef itk::Image<mitk::ScalarType, 3> InternalImageType;
typedef std::map<std::string, InternalImageType::Pointer> InternalImageMapType;
InternalImageMapType internalImages;

itk::ImageRegion<3> relevantRegion;
InternalImageType::PointType relevantOrigin;
InternalImageType::SpacingType relevantSpacing;
InternalImageType::DirectionType relevantDirection;

typedef itk::Index<3> DumpIndexType;
typedef std::vector<mitk::ScalarType> DumpedValuesType;

struct DumpIndexCompare
{
  bool operator() (const DumpIndexType& lhs, const DumpIndexType& rhs)
  {
    if (lhs[0] < rhs[0])
    {
      return true;
    }
    else if (lhs[0] > rhs[0])
    {
      return false;
    }

    if (lhs[1] < rhs[1])
    {
      return true;
    }
    else if (lhs[1] > rhs[1])
    {
      return false;
    }

    return lhs[2] < rhs[2];
  }
};

typedef std::map<DumpIndexType, DumpedValuesType, DumpIndexCompare> DumpPixelMapType;
DumpPixelMapType dumpedPixels;

void setupParser(mitkCommandLineParser& parser)
{
  // set general information about your MiniApp
  parser.setCategory("Generic Analysis Tools");
  parser.setTitle("Pixel Dumper");
  parser.setDescription("MiniApp that allows to dump the pixel values of all passed files into a csv. The region of dumping can defined by a mask. All images (and mask) must have the same geometrie.");
  parser.setContributor("DKFZ MIC");
  //! [create parser]

  //! [add arguments]
  // how should arguments be prefixed
  parser.setArgumentPrefix("--", "-");
  // add each argument, unless specified otherwise each argument is optional
  // see mitkCommandLineParser::addArgument for more information
  parser.beginGroup("Required I/O parameters");
  parser.addArgument(
    "inputs", "i", mitkCommandLineParser::StringList, "Input files", "list of the images that should be dumped.", us::Any(), false);
  parser.addArgument("output",
    "o",
    mitkCommandLineParser::File,
    "Output file",
    "where to save the csv.",
    us::Any(),
    false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "mask", "m", mitkCommandLineParser::File, "Mask file", "Mask that defines the spatial image region that should be dumped. Must have the same geometry as the input images!", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "captions", "c", mitkCommandLineParser::StringList, "Captions of image columns", "If provided the pixel columns of the csv will be named according to the passed values instead of using the image pathes. Number of images and names must be equal.", us::Any(), false);
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.endGroup();
  //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
  if (parsedArgs.size() == 0)
    return false;

  // parse, cast and set required arguments

  inFilenames = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["inputs"]);
  outFileName = us::any_cast<std::string>(parsedArgs["output"]);

  if (parsedArgs.count("mask"))
  {
    maskFileName = us::any_cast<std::string>(parsedArgs["mask"]);
  }

  captions = inFilenames;

  if (parsedArgs.count("captions"))
  {
    captions = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["captions"]);
  }

  return true;
}

template < typename TPixel, unsigned int VImageDimension >
void ExtractRelevantInformation(
  const itk::Image< TPixel, VImageDimension > *image)
{
  relevantRegion = image->GetLargestPossibleRegion();
  relevantOrigin = image->GetOrigin();
  relevantSpacing = image->GetSpacing();
  relevantDirection = image->GetDirection();
}

template < typename TPixel, unsigned int VImageDimension >
void DoInternalImageConversion(
  const itk::Image< TPixel, VImageDimension > *image,
  InternalImageType::Pointer& internalImage)
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  //check if image fit to geometry

  // Make sure that spacing are the same
  typename ImageType::SpacingType imageSpacing = image->GetSpacing();
  typename ImageType::PointType zeroPoint; zeroPoint.Fill(0.0);
  if ((zeroPoint + imageSpacing).SquaredEuclideanDistanceTo((zeroPoint + relevantSpacing)) >
    1e-6) // for the dumper we are not as strict as mitk normally would be (mitk::eps)
  {
    mitkThrow() << "Images need to have same spacing! (Image spacing: " << imageSpacing
      << "; relevant spacing: " << relevantSpacing << ")";
  }

  // Make sure that orientation of mask and image are the same
  typename ImageType::DirectionType imageDirection = image->GetDirection();
  for (unsigned int i = 0; i < imageDirection.RowDimensions; ++i)
  {
    for (unsigned int j = 0; j < imageDirection.ColumnDimensions; ++j)
    {
      double differenceDirection = imageDirection[i][j] - relevantDirection[i][j];
      if (fabs(differenceDirection) > 1e-6) // SD: 1e6 wird hier zum zweiten mal als Magic Number benutzt -> Konstante
      {
        // for the dumper we are not as strict as mitk normally would be (mitk::eps)
        mitkThrow() << "Images need to have same direction! (Image direction: "
          << imageDirection << "; relevant direction: " << relevantDirection << ")";
      }
    }
  }

  // Make sure that origin of mask and image are the same
  typename ImageType::PointType imageOrigin = image->GetOrigin();
  if (imageOrigin.SquaredEuclideanDistanceTo(relevantOrigin) > 1e-6)
  {
    // for the dumper we are not as strict as mitk normally would be (mitk::eps)
    mitkThrow() << "Image need to have same spacing! (Image spacing: "
      << imageSpacing << "; relevant spacing: " << relevantOrigin << ")";
  }

  typename ImageType::RegionType imageRegion = image->GetLargestPossibleRegion();

  if (!imageRegion.IsInside(relevantRegion) && imageRegion != relevantRegion)
  {
    mitkThrow() << "Images need to have same region! (Image region: "
      << imageRegion << "; relevant region: " << relevantRegion << ")";
  }

  //convert to internal image
  typedef itk::ExtractImageFilter<ImageType, ImageType> ExtractFilterType;
  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  typedef itk::CastImageFilter<ImageType, InternalImageType> CastFilterType;
  typename CastFilterType::Pointer castFilter = CastFilterType::New();

  extractFilter->SetInput(image);
  extractFilter->SetExtractionRegion(relevantRegion);
  castFilter->SetInput(extractFilter->GetOutput());
  castFilter->Update();
  internalImage = castFilter->GetOutput();
}

template < typename TPixel, unsigned int VImageDimension >
void DoMaskedCollecting(
  const itk::Image< TPixel, VImageDimension > *image)
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  itk::ImageRegionConstIteratorWithIndex<ImageType> it(image, relevantRegion);

  it.GoToBegin();

  while (!it.IsAtEnd())
  {
    if (mask.IsNull() || it.Get() > 0)
    {
      DumpedValuesType values;

      const auto index = it.GetIndex();

      for (auto& imagePos : internalImages)
      {
        double value = imagePos.second->GetPixel(index);
        values.push_back(value);
      }

      dumpedPixels.insert(std::make_pair(index, values));
    }
    ++it;
  }
}

InternalImageMapType ConvertImageTimeSteps(mitk::Image* image)
{
  InternalImageMapType map;

  InternalImageType::Pointer internalImage;

  for (unsigned int i = 0; i < image->GetTimeSteps(); ++i)
  {
    mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(image);
    imageTimeSelector->SetTimeNr(i);
    imageTimeSelector->UpdateLargestPossibleRegion();

    mitk::Image::Pointer imageTimePoint = imageTimeSelector->GetOutput();

    AccessFixedDimensionByItk_1(imageTimePoint,
      DoInternalImageConversion,
      3,
      internalImage);

    std::stringstream stream;
    stream << "[" << i << "]";
    map.insert(std::make_pair(stream.str(), internalImage));
  }

  return map;
}

void doDumping()
{
  if (mask.IsNotNull() && mask->GetTimeSteps() > 1)
  {
    std::cout <<
      "Pixel Dumper: Selected mask has multiple timesteps. Only use first timestep to mask the pixel dumping." << std::endl;

    mitk::ImageTimeSelector::Pointer maskTimeSelector = mitk::ImageTimeSelector::New();
    maskTimeSelector->SetInput(mask);
    maskTimeSelector->SetTimeNr(0);
    maskTimeSelector->UpdateLargestPossibleRegion();
    mask = maskTimeSelector->GetOutput();
  }

  try
  {
    if (mask.IsNotNull())
    { // if mask exist, we use the mask because it could be a sub region.
      AccessFixedDimensionByItk(mask, ExtractRelevantInformation, 3);
    }
    else
    {
      AccessFixedDimensionByItk(images.front(), ExtractRelevantInformation, 3);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error extracting image geometry. Error text: " << e.what();
    throw;
  }


  for (unsigned int index = 0; index < images.size(); ++index)
  {
    try
    {
      InternalImageMapType conversionMap = ConvertImageTimeSteps(images[index]);

      if (conversionMap.size() == 1)
      {
        internalImages.insert(std::make_pair(captions[index], conversionMap.begin()->second));
      }
      else if (conversionMap.size() > 1)
      {
        for (auto& pos : conversionMap)
        {
          std::stringstream namestream;
          namestream << captions[index] << " " << pos.first;
          internalImages.insert(std::make_pair(namestream.str(), pos.second));
        }
      }
    }
    catch (const std::exception& e)
    {
      std::stringstream errorStr;
      errorStr << "Inconsistent image \"" << captions[index] << "\" will be excluded from the collection. Error: " << e.what();
      std::cerr << errorStr.str() << std::endl;
    }

  }

  if (mask.IsNotNull())
  { // if mask exist, we use the mask because it could be a sub region.
    AccessFixedDimensionByItk(mask, DoMaskedCollecting, 3);
  }
  else
  {
    AccessFixedDimensionByItk(images.front(), DoMaskedCollecting, 3);
  }

}

void storeCSV()
{
  std::ofstream resultfile;
  resultfile.open(outFileName.c_str());
  resultfile << "x,y,z";
  for (auto aImage : internalImages)
  {
    resultfile << "," << aImage.first;
  }

  resultfile << std::endl;

  for (auto dumpPos : dumpedPixels)
  {
    resultfile << dumpPos.first[0] << "," << dumpPos.first[1] << "," << dumpPos.first[2];

    for(auto d : dumpPos.second)
    {
      resultfile << "," << d;
    }

    resultfile << std::endl;
  }
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  setupParser(parser);
  const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);

  mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (classic config)" }, { "MITK DICOM Reader" });

  if (!configureApplicationSettings(parsedArgs))
  {
    return EXIT_FAILURE;
  };

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  if (!captions.empty() && inFilenames.size() != captions.size())
  {
    std::cerr << "Cannot dump images. Number of given captions does not equal number of given images.";
    return EXIT_FAILURE;
  };

  //! [do processing]
  try
  {
    std::cout << "Load images:" << std::endl;
    for (auto path : inFilenames)
    {
      std::cout << "Input: " << path << std::endl;
      auto image = mitk::IOUtil::Load<mitk::Image>(path, &readerFilterFunctor);
      images.push_back(image);

    }

    if (!maskFileName.empty())
    {
      mask = mitk::IOUtil::Load<mitk::Image>(maskFileName, &readerFilterFunctor);
      std::cout << "Mask:  " << maskFileName << std::endl;
    }
    else
    {
      std::cout << "Mask:  none" << std::endl;
    }

    doDumping();
    storeCSV();

    std::cout << "Processing finished." << std::endl;

    return EXIT_SUCCESS;
  }
  catch (itk::ExceptionObject e)
  {
    MITK_ERROR << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    MITK_ERROR << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_ERROR << "Unexpected error encountered.";
    return EXIT_FAILURE;
  }
}
