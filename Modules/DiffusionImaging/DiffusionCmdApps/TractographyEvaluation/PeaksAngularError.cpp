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

#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <itkEvaluateDirectionImagesFilter.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <itkTractsToVectorImageFilter.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <mitkLexicalCast.h>
#include <iostream>
#include <fstream>
#include <mitkPeakImage.h>
#include <mitkDiffusionDataIOHelper.h>
#include <mitkLocaleSwitch.h>

typedef itk::Image< unsigned char, 3 >  ItkUcharImageType;

/*!
\brief Calculate angular error between two sets of directions stored in multiple 3D vector images where each pixel corresponds to a vector (itk::Image< itk::Vector< float, 3>, 3 >)
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("test", "", mitkCommandLineParser::StringList, "Test images", "test direction images", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("reference", "", mitkCommandLineParser::StringList, "Reference images", "reference direction images", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output folder", "output folder", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("masks", "", mitkCommandLineParser::StringList, "Mask(s)", "mask image(s)", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("verbose", "", mitkCommandLineParser::Bool, "Verbose", "output error images");
  parser.addArgument("ignore_test", "", mitkCommandLineParser::Bool, "Ignore missing test", "don't increase error if no test directions are found");
  parser.addArgument("ignore_ref", "", mitkCommandLineParser::Bool, "Ignore ignore missing ref", "don't increase error if no ref directions are found");

  parser.setCategory("Fiber Tracking Evaluation");
  parser.setTitle("Peaks Angular Error");
  parser.setDescription("Calculate angular error between two sets of peak images (1-1 correspondence)");
  parser.setContributor("MIC");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType testImages = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["test"]);
  mitkCommandLineParser::StringContainerType referenceImages = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference"]);

  mitkCommandLineParser::StringContainerType maskImages;
  if (parsedArgs.count("masks"))
    maskImages = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["masks"]);

  std::string outRoot = us::any_cast<std::string>(parsedArgs["o"]);

  bool verbose = false;
  if (parsedArgs.count("verbose"))
    verbose = us::any_cast<bool>(parsedArgs["verbose"]);

  bool ignore_test = false;
  if (parsedArgs.count("ignore_test"))
    ignore_test = us::any_cast<bool>(parsedArgs["ignore_test"]);

  bool ignore_ref = false;
  if (parsedArgs.count("ignore_ref"))
    ignore_ref = us::any_cast<bool>(parsedArgs["ignore_ref"]);

  try
  {
    typedef itk::ComparePeakImagesFilter< float > EvaluationFilterType;

    std::vector<std::string> test_names;
    auto test_images = mitk::DiffusionDataIOHelper::load_itk_images<EvaluationFilterType::PeakImageType>(testImages, &test_names);

    // load reference directions
    std::vector<std::string> ref_names;
    auto ref_images = mitk::DiffusionDataIOHelper::load_itk_images<EvaluationFilterType::PeakImageType>(referenceImages, &ref_names);

    // load/create mask image
    auto itkMaskImages = mitk::DiffusionDataIOHelper::load_itk_images<ItkUcharImageType>(maskImages);

    if (test_images.size()!=ref_images.size())
      mitkThrow() << "Matching number of test and reference image required!";

    for (unsigned int i=0; i<test_images.size(); ++i)
    {
      // evaluate directions
      EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
      evaluationFilter->SetTestImage(test_images.at(i));
      evaluationFilter->SetReferenceImage(ref_images.at(i));
      if (i<maskImages.size())
        evaluationFilter->SetMaskImage(itkMaskImages.at(i));
      evaluationFilter->SetIgnoreMissingTestDirections(ignore_test);
      evaluationFilter->SetIgnoreMissingRefDirections(ignore_ref);
      evaluationFilter->Update();

      std::string ref_name = ist::GetFilenameWithoutExtension(ref_names.at(i));
      std::string test_name = ist::GetFilenameWithoutExtension(test_names.at(i));

      if (verbose)
      {
        mitk::LocaleSwitch localeSwitch("C");
        EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
        EvaluationFilterType::OutputImageType::Pointer lengthErrorImage = evaluationFilter->GetOutput(1);
        typedef itk::ImageFileWriter< EvaluationFilterType::OutputImageType > WriterType;

        {
          WriterType::Pointer writer = WriterType::New();
          std::string outfilename = outRoot;
          outfilename.append(ref_name + "_" + test_name + "_AngularError.nii.gz");
          writer->SetFileName(outfilename.c_str());
          writer->SetInput(angularErrorImage);
          writer->Update();
        }

        {
          WriterType::Pointer writer = WriterType::New();
          std::string outfilename = outRoot;
          outfilename.append(ref_name + "_" + test_name + "_LengthError.nii.gz");
          writer->SetFileName(outfilename.c_str());
          writer->SetInput(lengthErrorImage);
          writer->Update();
        }
      }

      std::string logFile = outRoot;
      logFile.append("AngularErrors.csv");

      bool add_header = true;
      if (ist::FileExists(logFile, true))
        add_header = false;

      ofstream file;
      file.open (logFile.c_str(), std::fstream::app);

      std::string sens;
      if (add_header)
        sens.append("Test,Reference,Mean,Median,Maximum,Minimum,Stdev\n");

      sens.append(test_name);
      sens.append(",");
      sens.append(ref_name);
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMeanAngularError()));
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMedianAngularError()));
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMaxAngularError()));
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMinAngularError()));
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(std::sqrt(evaluationFilter->GetVarAngularError())));
      sens.append("\n");

      std::cout << sens;

      file << sens;
      file.close();
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
