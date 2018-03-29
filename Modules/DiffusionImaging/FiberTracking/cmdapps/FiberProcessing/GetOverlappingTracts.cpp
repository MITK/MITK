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
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <itksys/SystemTools.hxx>
#include <itkTractDensityImageFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkFloatImgType;

/*!
\brief Extract fibers from a tractogram using binary image ROIs
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Move Overlapping Tracts");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Move tracts that overlap with one of the reference tracts");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::StringList, "Input:", "input tractograms (.fib/.trk/.tck/.dcm)", us::Any(), false);
  parser.addArgument("reference", "r", mitkCommandLineParser::StringList, "Reference:", "reference tractograms (.fib/.trk/.tck/.dcm)", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output Folder:", "move input tracts that do/don't overlap here", us::Any(), false);

  parser.addArgument("overlap_fraction", "", mitkCommandLineParser::Float, "Overlap fraction:", "", 0.9);
  parser.addArgument("move_overlapping", "", mitkCommandLineParser::Bool, ":", "");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType input = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input"]);
  mitkCommandLineParser::StringContainerType reference = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference"]);

  std::string out_folder = us::any_cast<std::string>(parsedArgs["out"]);

  bool move_overlapping = false;
  if (parsedArgs.count("move_overlapping"))
    move_overlapping = us::any_cast<bool>(parsedArgs["move_overlapping"]);

  float overlap_threshold = 0.9;
  if (parsedArgs.count("overlap_fraction"))
    overlap_threshold = us::any_cast<float>(parsedArgs["overlap_fraction"]);

  try
  {
    itk::TractDensityImageFilter< ItkFloatImgType >::Pointer filter = itk::TractDensityImageFilter< ItkFloatImgType >::New();
    filter->SetDoFiberResampling(true);
    filter->SetUpsamplingFactor(0.25);
    filter->SetBinaryOutput(true);
    std::vector< ItkFloatImgType::Pointer > masks;
    for (auto f : reference)
    {
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      filter->SetFiberBundle(fib);

      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      filter->Update();
      masks.push_back(filter->GetOutput());
      std::cout.rdbuf (old);              // <-- restore
    }

    boost::progress_display disp(input.size());
    for (auto f : input)
    {
      ++disp;
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      bool is_overlapping = false;
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      fib->ResampleLinear(2);
      for (auto m : masks)
      {
        float overlap = fib->GetOverlap(m, false);

        if (overlap>=overlap_threshold)
        {
          is_overlapping = true;
          break;
        }
      }
      if (move_overlapping && is_overlapping)
        ist::CopyAFile(f, out_folder + ist::GetFilenameName(f));
      else if (!move_overlapping && !is_overlapping)
        ist::CopyAFile(f, out_folder + ist::GetFilenameName(f));
      std::cout.rdbuf (old);              // <-- restore
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
