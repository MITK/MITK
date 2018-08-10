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

#define _USE_MATH_DEFINES
#include <math.h>

typedef itk::Image< unsigned char, 3 >  ItkUcharImageType;

/*!
\brief Calculate angular error between two sets of directions stored in multiple 3D vector images where each pixel corresponds to a vector (itk::Image< itk::Vector< float, 3>, 3 >)
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("test", "", mitkCommandLineParser::StringList, "Test images", "test direction images", us::Any(), false);
  parser.addArgument("reference", "", mitkCommandLineParser::StringList, "Reference images", "reference direction images", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output directory", "output root", us::Any(), false);
  parser.addArgument("masks", "", mitkCommandLineParser::StringList, "Mask(s)", "mask image(s)");
  parser.addArgument("verbose", "", mitkCommandLineParser::Bool, "Verbose", "output optional and intermediate calculation results");
  parser.addArgument("ignore", "", mitkCommandLineParser::Bool, "Ignore", "don't increase error for missing or too many directions");

  parser.setCategory("Fiber Tracking Evaluation");
  parser.setTitle("Peaks Angular Error");
  parser.setDescription("Calculate angular error between two sets of directions stored in multiple 3D vector images where each pixel corresponds to a vector (itk::Image< itk::Vector< float, 3>, 3 >)");
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

  bool ignore = false;
  if (parsedArgs.count("ignore"))
    ignore = us::any_cast<bool>(parsedArgs["ignore"]);

  try
  {
    typedef itk::ComparePeakImagesFilter< float > EvaluationFilterType;

    auto directionImageContainer = mitk::DiffusionDataIOHelper::load_itk_images<EvaluationFilterType::PeakImageType>(testImages);

    // load reference directions
    auto referenceImageContainer = mitk::DiffusionDataIOHelper::load_itk_images<EvaluationFilterType::PeakImageType>(referenceImages);

    // load/create mask image
    auto itkMaskImages = mitk::DiffusionDataIOHelper::load_itk_images<ItkUcharImageType>(maskImages);

    if (directionImageContainer.size()!=referenceImageContainer.size())
      mitkThrow() << "Matching number of test and reference image required!";

    for (unsigned int i=0; i<directionImageContainer.size(); ++i)
    {
      // evaluate directions
      EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
      evaluationFilter->SetTestImage(directionImageContainer.at(i));
      evaluationFilter->SetReferenceImage(referenceImageContainer.at(i));
      if (i<maskImages.size())
        evaluationFilter->SetMaskImage(itkMaskImages.at(i));
      evaluationFilter->SetIgnoreMissingDirections(ignore);
      evaluationFilter->Update();

      if (verbose)
      {
        mitk::LocaleSwitch localeSwitch("C");
        EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
        typedef itk::ImageFileWriter< EvaluationFilterType::OutputImageType > WriterType;
        WriterType::Pointer writer = WriterType::New();

        std::string outfilename = outRoot;
        outfilename.append("_ERROR_IMAGE.nrrd");

        writer->SetFileName(outfilename.c_str());
        writer->SetInput(angularErrorImage);
        writer->Update();
      }

      std::string logFile = outRoot;
      logFile.append("_ANGULAR_ERROR.csv");

      ofstream file;
      file.open (logFile.c_str());

      std::string sens = "Mean:";
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMeanAngularError()));
      sens.append(";\n");

      sens.append("Median:");
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMedianAngularError()));
      sens.append(";\n");

      sens.append("Maximum:");
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMaxAngularError()));
      sens.append(";\n");

      sens.append("Minimum:");
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(evaluationFilter->GetMinAngularError()));
      sens.append(";\n");

      sens.append("STDEV:");
      sens.append(",");
      sens.append(boost::lexical_cast<std::string>(std::sqrt(evaluationFilter->GetVarAngularError())));
      sens.append(";\n");

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
