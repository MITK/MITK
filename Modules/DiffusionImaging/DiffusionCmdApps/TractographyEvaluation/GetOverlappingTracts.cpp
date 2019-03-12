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
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <itksys/SystemTools.hxx>
#include <itkTractDensityImageFilter.h>
#include <mitkDiffusionDataIOHelper.h>

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

  parser.setTitle("Get Overlapping Tracts");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Find tracts that overlap with the reference masks or tracts");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::StringList, "Input:", "input tractograms (.fib/.trk/.tck/.dcm)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output Folder:", "move input tracts that do/don't overlap here", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("reference", "", mitkCommandLineParser::StringList, "Reference:", "reference tractograms or mask images", us::Any(), false, false, false, mitkCommandLineParser::Input);

  parser.addArgument("overlap_fraction", "", mitkCommandLineParser::Float, "Overlap fraction:", "", 0.9);
  parser.addArgument("use_any_overlap", "", mitkCommandLineParser::Bool, "Use any overlap:", "Don't find maximum overlap but use first overlap larger threshold");
  parser.addArgument("dont_save_tracts", "", mitkCommandLineParser::Bool, "Don't save tracts:", "if true, only text files documenting the overlaps are saved and no tract files are copied");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType input = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i"]);
  mitkCommandLineParser::StringContainerType reference = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference"]);

  std::string out_folder = us::any_cast<std::string>(parsedArgs["o"]);

  bool use_any_overlap = false;
  if (parsedArgs.count("use_any_overlap"))
    use_any_overlap = us::any_cast<bool>(parsedArgs["use_any_overlap"]);

  bool dont_save_tracts = false;
  if (parsedArgs.count("dont_save_tracts"))
    dont_save_tracts = us::any_cast<bool>(parsedArgs["dont_save_tracts"]);

  float overlap_threshold = 0.9;
  if (parsedArgs.count("overlap_fraction"))
    overlap_threshold = us::any_cast<float>(parsedArgs["overlap_fraction"]);

  try
  {
    MITK_INFO << "Loading references";
    std::vector< std::string > reference_names;
    auto masks = mitk::DiffusionDataIOHelper::load_itk_images<ItkFloatImgType>(reference, &reference_names);
    auto reference_fibs = mitk::DiffusionDataIOHelper::load_fibs(reference, &reference_names);

    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect
    itk::TractDensityImageFilter< ItkFloatImgType >::Pointer filter = itk::TractDensityImageFilter< ItkFloatImgType >::New();
    filter->SetUpsamplingFactor(0.25);
    filter->SetBinaryOutput(true);
    for (auto fib : reference_fibs)
    {
        filter->SetFiberBundle(fib);
        filter->Update();
        masks.push_back(filter->GetOutput());
    }
    std::cout.rdbuf (old);              // <-- restore

    MITK_INFO << "Loading input tractograms";
    std::vector< std::string > input_names;
    auto input_fibs = mitk::DiffusionDataIOHelper::load_fibs(input, &input_names);

    MITK_INFO << "Finding overlaps";
    ofstream logfile;
    logfile.open (out_folder + "Overlaps.txt");

    ofstream logfile2;
    logfile2.open (out_folder + "AllOverlaps.txt");

    boost::progress_display disp(input.size());
    unsigned int c = 0;
    for (auto fib : input_fibs)
    {
      ++disp;
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      bool is_overlapping = false;
      float overlap = 0;
      float max_overlap = 0;
      std::string max_ref = "-";
      int i = 0;
      std::string overlap_string = ist::GetFilenameWithoutExtension(input_names.at(c));
      for (auto m : masks)
      {
        overlap = fib->GetOverlap(m);
        if (overlap>max_overlap)
        {
          max_overlap = overlap;
          max_ref = ist::GetFilenameWithoutExtension(reference_names.at(i));
        }
        if (use_any_overlap && overlap>=overlap_threshold)
          break;
        overlap_string += " " + ist::GetFilenameWithoutExtension(reference_names.at(i)) + " " + boost::lexical_cast<std::string>(overlap);
        ++i;
      }
      if (overlap>=overlap_threshold)
        is_overlapping = true;

      logfile << ist::GetFilenameWithoutExtension(input_names.at(c)) << " - " << max_ref << ": " << boost::lexical_cast<std::string>(max_overlap) << "\n";
      logfile2 << overlap_string << "\n";

      if (!dont_save_tracts && is_overlapping)
        ist::CopyAFile(input_names.at(c), out_folder + ist::GetFilenameName(input_names.at(c)));
      std::cout.rdbuf (old);              // <-- restore
      ++c;
    }
    logfile.close();
    logfile2.close();
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
