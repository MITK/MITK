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
#include <boost/lexical_cast.hpp>
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
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;

ItkUcharImgType::Pointer LoadItkMaskImage(const std::string& filename)
{
  mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(filename)[0].GetPointer());
  ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
  mitk::CastToItkImage(img, itkMask);
  return itkMask;
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
  parser.addArgument("rois", "", mitkCommandLineParser::StringList, "ROI images:", "Images with binary ROIs", us::Any(), false);

  parser.addArgument("both_ends", "", mitkCommandLineParser::Bool, "Both ends:", "Fibers are extracted if both endpoints are located in the ROI.", false);
  parser.addArgument("overlap_fraction", "", mitkCommandLineParser::Float, "Overlap fraction:", "Extract by overlap, not by endpoints. Extract fibers that overlap to at least the provided factor (0-1) with the ROI.", -1);
  parser.addArgument("invert", "", mitkCommandLineParser::Bool, "Invert:", "get streamlines not positive for any of the mask images", false);

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

  float overlap_fraction = -1;
  if (parsedArgs.count("overlap_fraction"))
    overlap_fraction = us::any_cast<float>(parsedArgs["overlap_fraction"]);

  bool any_point = false;
  if (overlap_fraction>=0)
    any_point = true;

  try
  {
    // load fiber bundle
    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(inFib)[0].GetPointer());

    std::vector< ItkUcharImgType::Pointer > mask_images;
    for (std::size_t i=0; i<roi_files.size(); ++i)
      mask_images.push_back(LoadItkMaskImage(roi_files.at(i)));

    itk::FiberExtractionFilter::Pointer extractor = itk::FiberExtractionFilter::New();
    extractor->SetInputFiberBundle(inputTractogram);
    extractor->SetMasks(mask_images);
    extractor->SetOverlapFraction(overlap_fraction);
    extractor->SetBothEnds(both_ends);
    if (invert)
      extractor->SetNoPositives(true);
    else
      extractor->SetNoNegatives(true);
    if (!any_point)
      extractor->SetMode(itk::FiberExtractionFilter::MODE::ENDPOINTS);
    extractor->Update();

    mitk::FiberBundle::Pointer newFib;
    if (invert)
      mitk::IOUtil::Save(extractor->GetNegatives().at(0), outFib);
    else
    {
      newFib->AddBundles(extractor->GetPositives());
      mitk::IOUtil::Save(newFib, outFib);
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
