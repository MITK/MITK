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

#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <mitkIOUtil.h>
#include <mitkLexicalCast.h>
#include <mitkCoreObjectFactory.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkStandaloneDataStorage.h>
#include <itksys/SystemTools.hxx>
#include <itkFiberExtractionFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

typedef itksys::SystemTools ist;
typedef itk::Image<float, 3>    ItkFloatImgType;

ItkFloatImgType::Pointer LoadItkImage(const std::string& filename)
{
  mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(filename);
  ItkFloatImgType::Pointer itk_image = ItkFloatImgType::New();
  mitk::CastToItkImage(img, itk_image);
  return itk_image;
}

/*!
\brief Extract fibers from a tractogram using binary image ROIs
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fiber Extraction With ROI Image");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Extract fibers from a tractogram using binary image ROIs");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:", "input tractogram (.fib/.trk/.tck/.dcm)", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);
  parser.addArgument("rois", "", mitkCommandLineParser::StringList, "ROI images:", "ROI images", us::Any(), false);

  parser.addArgument("both_ends", "", mitkCommandLineParser::Bool, "Both ends:", "Fibers are extracted if both endpoints are located in the ROI.", false);
  parser.addArgument("overlap_fraction", "", mitkCommandLineParser::Float, "Overlap fraction:", "Extract by overlap, not by endpoints. Extract fibers that overlap to at least the provided factor (0-1) with the ROI.", -1);
  parser.addArgument("invert", "", mitkCommandLineParser::Bool, "Invert:", "get streamlines not positive for any of the ROI images", false);
  parser.addArgument("interpolate", "", mitkCommandLineParser::Bool, "Interpolate:", "interpolate ROI images", false);
  parser.addArgument("threshold", "", mitkCommandLineParser::Float, "Threshold:", "positive means ROI image value threshold", 0.5);
  parser.addArgument("labels", "", mitkCommandLineParser::StringList, "Labels:", "positive means roi image value in labels vector", false);
  parser.addArgument("split_labels", "", mitkCommandLineParser::Bool, "Split labels:", "output a separate tractogram for each label-->label tract", false);
  parser.addArgument("skip_self_connections", "", mitkCommandLineParser::Bool, "Skip self connections:", "ignore streamlines between two identical labels", false);
  parser.addArgument("min_fibers", "", mitkCommandLineParser::Int, "Min. num. fibers:", "discard positive tracts with less fibers", 0);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFib = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outFib = us::any_cast<std::string>(parsedArgs["out"]);
  mitkCommandLineParser::StringContainerType roi_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["rois"]);

  bool both_ends = false;
  if (parsedArgs.count("both_ends"))
    both_ends = us::any_cast<bool>(parsedArgs["both_ends"]);

  bool invert = false;
  if (parsedArgs.count("invert"))
    invert = us::any_cast<bool>(parsedArgs["invert"]);

  unsigned int min_fibers = 0;
  if (parsedArgs.count("min_fibers"))
    min_fibers = us::any_cast<int>(parsedArgs["min_fibers"]);

  bool split_labels = false;
  if (parsedArgs.count("split_labels"))
    split_labels = us::any_cast<bool>(parsedArgs["split_labels"]);

  bool skip_self_connections = false;
  if (parsedArgs.count("skip_self_connections"))
    skip_self_connections = us::any_cast<bool>(parsedArgs["skip_self_connections"]);

  float overlap_fraction = -1;
  if (parsedArgs.count("overlap_fraction"))
    overlap_fraction = us::any_cast<float>(parsedArgs["overlap_fraction"]);

  bool any_point = false;
  if (overlap_fraction>=0)
    any_point = true;

  bool interpolate = false;
  if (parsedArgs.count("interpolate"))
    interpolate = us::any_cast<bool>(parsedArgs["interpolate"]);

  float threshold = 0.5;
  if (parsedArgs.count("threshold"))
    threshold = us::any_cast<float>(parsedArgs["threshold"]);

  mitkCommandLineParser::StringContainerType labels;
  if (parsedArgs.count("labels"))
    labels = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["labels"]);

  try
  {
    // load fiber bundle
    mitk::FiberBundle::Pointer inputTractogram = mitk::IOUtil::Load<mitk::FiberBundle>(inFib);

    std::vector< ItkFloatImgType::Pointer > roi_images;
    for (std::size_t i=0; i<roi_files.size(); ++i)
      roi_images.push_back( LoadItkImage(roi_files.at(i)) );

    std::vector< ItkFloatImgType* > roi_images2;
    for (auto roi : roi_images)
      roi_images2.push_back(roi);

    std::vector< unsigned short > short_labels;
    for (auto l : labels)
      short_labels.push_back(boost::lexical_cast<unsigned short>(l));

    itk::FiberExtractionFilter<float>::Pointer extractor = itk::FiberExtractionFilter<float>::New();
    extractor->SetInputFiberBundle(inputTractogram);
    extractor->SetRoiImages(roi_images2);
    extractor->SetOverlapFraction(overlap_fraction);
    extractor->SetBothEnds(both_ends);
    extractor->SetInterpolate(interpolate);
    extractor->SetThreshold(threshold);
    extractor->SetLabels(short_labels);
    extractor->SetSplitLabels(split_labels);
    extractor->SetMinFibersPerTract(min_fibers);
    extractor->SetSkipSelfConnections(skip_self_connections);
    if (invert)
      extractor->SetNoPositives(true);
    else
      extractor->SetNoNegatives(true);
    if (!any_point)
      extractor->SetMode(itk::FiberExtractionFilter<float>::MODE::ENDPOINTS);
    if (short_labels.size()>0)
      extractor->SetInputType(itk::FiberExtractionFilter<float>::INPUT::LABEL_MAP);
    extractor->Update();

    mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(nullptr);
    if (invert)
      mitk::IOUtil::Save(extractor->GetNegatives().at(0), outFib);
    else
    {
      if (!split_labels)
      {
        newFib = newFib->AddBundles(extractor->GetPositives());
        mitk::IOUtil::Save(newFib, outFib);
      }
      else
      {
        int c = 0;
        std::vector< std::pair< unsigned int, unsigned int > > positive_labels = extractor->GetPositiveLabels();
        for (auto fib : extractor->GetPositives())
        {
          std::pair< unsigned int, unsigned int > l = positive_labels.at(c);
          mitk::IOUtil::Save(fib, outFib + "_" + boost::lexical_cast<std::string>(l.first) + "-" + boost::lexical_cast<std::string>(l.second) + ".trk");
          ++c;
        }
      }
    }
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
