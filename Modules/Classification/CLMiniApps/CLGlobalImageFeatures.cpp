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

#include <mitkGIFCooccurenceMatrix2.h>
#include <mitkGIFGrayLevelRunLength.h>
#include <mitkGIFFirstOrderStatistics.h>
#include <mitkGIFVolumetricStatistics.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <itkImageDuplicator.h>
#include <itkImageRegionIterator.h>


#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"



typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

static std::vector<double> splitDouble(std::string str, char delimiter) {
  std::vector<double> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  double val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

template<typename TPixel, unsigned int VImageDimension>
void
ResampleImage(itk::Image<TPixel, VImageDimension>* itkImage, float resolution, mitk::Image::Pointer& newImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleFilterType;

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  auto spacing = itkImage->GetSpacing();
  auto size = itkImage->GetLargestPossibleRegion().GetSize();

  for (int i = 0; i < VImageDimension; ++i)
  {
    size[i] = size[i] / (1.0*resolution)*(1.0*spacing[i])+1.0;
  }
  spacing.Fill(resolution);

  resampler->SetInput(itkImage);
  resampler->SetSize(size);
  resampler->SetOutputSpacing(spacing);
  resampler->SetOutputOrigin(itkImage->GetOrigin());
  resampler->SetOutputDirection(itkImage->GetDirection());
  resampler->Update();

  newImage->InitializeByItk(resampler->GetOutput());
  mitk::GrabItkImageMemory(resampler->GetOutput(), newImage);
}



static void
CreateNoNaNMask(mitk::Image::Pointer mask, mitk::Image::Pointer ref, mitk::Image::Pointer& newMask)
{
  MaskImageType::Pointer itkMask = MaskImageType::New();
  FloatImageType::Pointer itkValue = FloatImageType::New();
  mitk::CastToItkImage(mask, itkMask);
  mitk::CastToItkImage(ref, itkValue);

  typedef itk::ImageDuplicator< MaskImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(itkMask);
  duplicator->Update();

  auto tmpMask = duplicator->GetOutput();

  itk::ImageRegionIterator<MaskImageType> mask1Iter(itkMask, itkMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<MaskImageType> mask2Iter(tmpMask, tmpMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatImageType> imageIter(itkValue, itkValue->GetLargestPossibleRegion());
  while (!mask1Iter.IsAtEnd())
  {
    mask2Iter.Set(0);
    if (mask1Iter.Value() > 0)
    {
      // Is not NaN
      if (imageIter.Value() == imageIter.Value())
      {
        mask2Iter.Set(1);
      }
    }
    ++mask1Iter;
    ++mask2Iter;
    ++imageIter;
  }

  newMask->InitializeByItk(tmpMask);
  mitk::GrabItkImageMemory(tmpMask, newMask);
}

static void
ResampleMask(mitk::Image::Pointer mask, mitk::Image::Pointer ref, mitk::Image::Pointer& newMask)
{
  typedef itk::NearestNeighborInterpolateImageFunction< MaskImageType> NearestNeighborInterpolateImageFunctionType;
  typedef itk::ResampleImageFilter<MaskImageType,MaskImageType> ResampleFilterType;

  NearestNeighborInterpolateImageFunctionType::Pointer nn_interpolator = NearestNeighborInterpolateImageFunctionType::New();
  MaskImageType::Pointer itkMoving = MaskImageType::New();
  MaskImageType::Pointer itkRef = MaskImageType::New();
  mitk::CastToItkImage(mask, itkMoving);
  mitk::CastToItkImage(ref, itkRef);


  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(itkMoving);
  resampler->SetReferenceImage(itkRef);
  resampler->UseReferenceImageOn();
  resampler->SetInterpolator(nn_interpolator);
  resampler->Update();

  newMask->InitializeByItk(resampler->GetOutput());
  mitk::GrabItkImageMemory(resampler->GetOutput(), newMask);
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input VTK polydata", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputImage, "Input Mask", "Mask Image that specifies the area over for the statistic, (Values = 1)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false);

  parser.addArgument("cooccurence","cooc",mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());
  parser.addArgument("volume","vol",mitkCommandLineParser::String, "Use Volume-Statistic", "calculates volume based features",us::Any());
  parser.addArgument("run-length","rl",mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());
  parser.addArgument("first-order","fo",mitkCommandLineParser::String, "Use First Order Features", "calculates First order based features",us::Any());
  parser.addArgument("header","head",mitkCommandLineParser::String,"Add Header (Labels) to output","",us::Any());
  parser.addArgument("description","d",mitkCommandLineParser::String,"Text","Description that is added to the output",us::Any());
  parser.addArgument("same-space", "sp", mitkCommandLineParser::String, "Bool", "Set the spacing of all images to equal. Otherwise an error will be thrown. ", us::Any());
  parser.addArgument("resample-mask", "rm", mitkCommandLineParser::Bool, "Bool", "Resamples the mask to the resolution of the input image ", us::Any());
  parser.addArgument("save-resample-mask", "srm", mitkCommandLineParser::String, "String", "If specified the resampled mask is saved to this path (if -rm is 1)", us::Any());
  parser.addArgument("fixed-isotropic", "fi", mitkCommandLineParser::Float, "Float", "Input image resampled to fixed isotropic resolution given in mm. Should be used with resample-mask ", us::Any());
  parser.addArgument("direction", "dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... Without dimension 0,1,2... ", us::Any());

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Global Image Feature calculator");
  parser.setDescription("Calculates different global statistics for a given segmentation / image combination");
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

  MITK_INFO << "Version: "<< 1.7;

  //bool useCooc = parsedArgs.count("cooccurence");

  bool resampleMask = false;
  if (parsedArgs.count("resample-mask"))
  {
    resampleMask = us::any_cast<bool>(parsedArgs["resample-mask"]);
  }

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  mitk::Image::Pointer mask = mitk::IOUtil::LoadImage(parsedArgs["mask"].ToString());


  if (parsedArgs.count("fixed-isotropic"))
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    float resolution = us::any_cast<float>(parsedArgs["fixed-isotropic"]);
    AccessByItk_2(image, ResampleImage, resolution, newImage);
    image = newImage;
  }

  if (resampleMask)
  {
    mitk::Image::Pointer newMaskImage = mitk::Image::New();
    ResampleMask(mask, image, newMaskImage);
    mask = newMaskImage;
    if (parsedArgs.count("save-resample-mask"))
    {
      mitk::IOUtil::SaveImage(mask, parsedArgs["save-resample-mask"].ToString());
    }
  }


  bool fixDifferentSpaces = parsedArgs.count("same-space");
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetOrigin(), image->GetGeometry(0)->GetOrigin()))
  {
    MITK_INFO << "Not equal Origins";
    if (fixDifferentSpaces)
    {
      image->GetGeometry(0)->SetOrigin(mask->GetGeometry(0)->GetOrigin());
    } else
    {
      return -1;
    }
  }
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetSpacing(), image->GetGeometry(0)->GetSpacing()))
  {
    MITK_INFO << "Not equal Sapcings";
    if (fixDifferentSpaces)
    {
      image->GetGeometry(0)->SetSpacing(mask->GetGeometry(0)->GetSpacing());
    } else
    {
      return -1;
    }
  }

  int direction = 0;
  if (parsedArgs.count("direction"))
  {
    direction = splitDouble(parsedArgs["direction"].ToString(), ';')[0];
  }

  mitk::Image::Pointer maskNoNaN = mitk::Image::New();
  CreateNoNaNMask(mask, image, maskNoNaN);

  mitk::AbstractGlobalImageFeature::FeatureListType stats;
  ////////////////////////////////////////////////////////////////
  // Calculate First Order Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("first-order"))
  {
    MITK_INFO << "Start calculating first order statistics....";
    mitk::GIFFirstOrderStatistics::Pointer firstOrderCalculator = mitk::GIFFirstOrderStatistics::New();
    auto localResults = firstOrderCalculator->CalculateFeatures(image, maskNoNaN);
    stats.insert(stats.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating first order statistics....";
  }

  ////////////////////////////////////////////////////////////////
  // Calculate Volume based Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("volume"))
  {
    MITK_INFO << "Start calculating volumetric ....";
    mitk::GIFVolumetricStatistics::Pointer volCalculator = mitk::GIFVolumetricStatistics::New();
    auto localResults = volCalculator->CalculateFeatures(image, mask);
    stats.insert(stats.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating volumetric....";
  }

  ////////////////////////////////////////////////////////////////
  // Calculate Co-occurence Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("cooccurence"))
  {
    auto ranges = splitDouble(parsedArgs["cooccurence"].ToString(),';');

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating coocurence with range " << ranges[i] << "....";
      mitk::GIFCooccurenceMatrix2::Pointer coocCalculator = mitk::GIFCooccurenceMatrix2::New();
      coocCalculator->SetRange(ranges[i]);
      coocCalculator->SetDirection(direction);
      auto localResults = coocCalculator->CalculateFeatures(image, maskNoNaN);
      stats.insert(stats.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating coocurence with range " << ranges[i] << "....";
    }
  }

  ////////////////////////////////////////////////////////////////
  // Calculate Run-Length Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("run-length"))
  {
    auto ranges = splitDouble(parsedArgs["run-length"].ToString(),';');

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating run-length with number of bins " << ranges[i] << "....";
      mitk::GIFGrayLevelRunLength::Pointer calculator = mitk::GIFGrayLevelRunLength::New();
      calculator->SetRange(ranges[i]);

      auto localResults = calculator->CalculateFeatures(image, mask);
      stats.insert(stats.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating run-length with number of bins " << ranges[i] << "....";
    }
  }
  for (std::size_t i = 0; i < stats.size(); ++i)
  {
    std::cout << stats[i].first << " - " << stats[i].second <<std::endl;
  }

  std::ofstream output(parsedArgs["output"].ToString(),std::ios::app);
  if ( parsedArgs.count("header") )
  {
    if ( parsedArgs.count("description") )
    {
      output << "Description" << ";";
    }
    for (std::size_t i = 0; i < stats.size(); ++i)
    {
      output << stats[i].first << ";";
    }
    output << "EndOfMeasurement;";
    output << std::endl;
  }
  if ( parsedArgs.count("description") )
  {
    output << parsedArgs["description"].ToString() << ";";
  }
  for (std::size_t i = 0; i < stats.size(); ++i)
  {
    output << stats[i].second << ";";
  }
  output << "EndOfMeasurement;";
  output << std::endl;
  output.close();

  return 0;
}

#endif
