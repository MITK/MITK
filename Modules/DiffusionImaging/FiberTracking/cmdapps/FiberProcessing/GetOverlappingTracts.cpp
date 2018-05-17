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
  parser.addArgument("input", "i", mitkCommandLineParser::StringList, "Input:", "input tractograms (.fib/.trk/.tck/.dcm)", us::Any(), false);
  parser.addArgument("reference", "r", mitkCommandLineParser::StringList, "Reference:", "reference tractograms or mask images", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output Folder:", "move input tracts that do/don't overlap here", us::Any(), false);

  parser.addArgument("overlap_fraction", "", mitkCommandLineParser::Float, "Overlap fraction:", "", 0.9);
  parser.addArgument("use_any_overlap", "", mitkCommandLineParser::Bool, "Use any overlap:", "Don't find maximum overlap but use first overlap larger threshold");
  parser.addArgument("dont_save_tracts", "", mitkCommandLineParser::Bool, "Don't save tracts:", "if true, only text files documenting the overlaps are saved and no tract files are copied");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType input = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input"]);
  mitkCommandLineParser::StringContainerType reference = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference"]);

  std::string out_folder = us::any_cast<std::string>(parsedArgs["out"]);

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
    itk::TractDensityImageFilter< ItkFloatImgType >::Pointer filter = itk::TractDensityImageFilter< ItkFloatImgType >::New();
    filter->SetDoFiberResampling(true);
    filter->SetUpsamplingFactor(0.25);
    filter->SetBinaryOutput(true);
    MITK_INFO << "Loading references";
    std::vector< ItkFloatImgType::Pointer > masks;
    for (auto f : reference)
    {
      MITK_INFO << f;
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      if (fib.IsNotNull())
      {
        filter->SetFiberBundle(fib);
        filter->Update();
        masks.push_back(filter->GetOutput());
      }
      else
      {
        mitk::Image::Pointer m = mitk::IOUtil::Load<mitk::Image>(f);
        ItkFloatImgType::Pointer itkImage = ItkFloatImgType::New();
        CastToItkImage(m, itkImage);
        masks.push_back(itkImage);
      }
      std::cout.rdbuf (old);              // <-- restore
    }

    MITK_INFO << "Finding overlaps";
    ofstream logfile;
    logfile.open (out_folder + "Overlaps.txt");

    ofstream logfile2;
    logfile2.open (out_folder + "AllOverlaps.txt");

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

      float overlap = 0;
      float max_overlap = 0;
      std::string max_ref = "-";
      int i = 0;
      std::string overlap_string = ist::GetFilenameWithoutExtension(f);
      for (auto m : masks)
      {
        overlap = fib->GetOverlap(m, false);
        if (overlap>max_overlap)
        {
          max_overlap = overlap;
          max_ref = ist::GetFilenameWithoutExtension(reference.at(i));
        }
        if (use_any_overlap && overlap>=overlap_threshold)
          break;
        overlap_string += " " + ist::GetFilenameWithoutExtension(reference.at(i)) + " " + boost::lexical_cast<std::string>(overlap);
        ++i;
      }
      if (overlap>=overlap_threshold)
        is_overlapping = true;

      logfile << ist::GetFilenameWithoutExtension(f) << " - " << max_ref << ": " << boost::lexical_cast<std::string>(max_overlap) << "\n";
      logfile2 << overlap_string << "\n";

      if (!dont_save_tracts && is_overlapping)
        ist::CopyAFile(f, out_folder + ist::GetFilenameName(f));
      std::cout.rdbuf (old);              // <-- restore
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
