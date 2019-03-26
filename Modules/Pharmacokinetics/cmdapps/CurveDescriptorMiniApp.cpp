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

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

#include <mitkModelFitCmdAppsHelper.h>
#include <mitkExtractTimeGrid.h>
#include <mitkPixelBasedDescriptionParameterImageGenerator.h>

#include <mitkAreaUnderTheCurveDescriptionParameter.h>
#include <mitkAreaUnderFirstMomentDescriptionParameter.h>
#include <mitkMeanResidenceTimeDescriptionParameter.h>
#include <mitkTimeToPeakCurveDescriptionParameter.h>


std::string inFilename;
std::string outFileName;
std::string maskFileName;
bool verbose(false);
bool preview(false);
mitk::Image::Pointer image;
mitk::Image::Pointer mask;


void onFitEvent(::itk::Object* caller, const itk::EventObject & event, void* /*data*/)
{
    itk::ProgressEvent progressEvent;

    if (progressEvent.CheckEvent(&event))
    {
        mitk::PixelBasedDescriptionParameterImageGenerator* castedReporter = dynamic_cast<mitk::PixelBasedDescriptionParameterImageGenerator*>(caller);
        std::cout <<castedReporter->GetProgress()*100 << "% ";
    }
}


void setupParser(mitkCommandLineParser& parser)
{
    // set general information about your MiniApp
    parser.setCategory("Dynamic Data Analysis Tools");
    parser.setTitle("Curve Descriptor");
    parser.setDescription("MiniApp that allows to generate curve descriptor maps for dynamic image.");
    parser.setContributor("DKFZ MIC");
    //! [create parser]

    //! [add arguments]
    // how should arguments be prefixed
    parser.setArgumentPrefix("--", "-");
    // add each argument, unless specified otherwise each argument is optional
    // see mitkCommandLineParser::addArgument for more information
    parser.beginGroup("Required I/O parameters");
    parser.addArgument(
        "input", "i", mitkCommandLineParser::File, "Input file", "input 3D+t image file", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("output",
        "o",
        mitkCommandLineParser::File,
        "Output file template",
        "where to save the output parameter images. The specified path will be used as template to determine the format (via extension) and the name \"root\". For each parameter a suffix will be added to the name.",
        us::Any(),
        false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("Optional parameters");
    parser.addArgument(
        "mask", "m", mitkCommandLineParser::File, "Mask file", "Mask that defines the spatial image region that should be fitted. Must have the same geometry as the input image!", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument(
        "verbose", "v", mitkCommandLineParser::Bool, "Verbose Output", "Whether to produce verbose output");
    parser.addArgument(
      "preview", "p", mitkCommandLineParser::Bool, "Preview outputs", "The application previews the outputs (filename, type) it would produce with the current settings.");
    parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
    parser.endGroup();
    //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
    if (parsedArgs.size() == 0)
        return false;

    verbose = false;
    if (parsedArgs.count("verbose"))
    {
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);
    }

    if (parsedArgs.count("mask"))
    {
        maskFileName = us::any_cast<std::string>(parsedArgs["mask"]);
    }

    preview = false;
    if (parsedArgs.count("preview"))
    {
      preview = us::any_cast<bool>(parsedArgs["preview"]);
    }

    inFilename = us::any_cast<std::string>(parsedArgs["input"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    return true;
}

void ConfigureFunctor(mitk::CurveParameterFunctor* functor)
{
    mitk::CurveDescriptionParameterBase::Pointer parameterFunction = mitk::AreaUnderTheCurveDescriptionParameter::New().GetPointer();
    functor->RegisterDescriptionParameter("AUC", parameterFunction);

    parameterFunction = mitk::AreaUnderFirstMomentDescriptionParameter::New().GetPointer();
    functor->RegisterDescriptionParameter("AUMC", parameterFunction);

    parameterFunction = mitk::MeanResidenceTimeDescriptionParameter::New().GetPointer();
    functor->RegisterDescriptionParameter("MRT", parameterFunction);

    parameterFunction = mitk::TimeToPeakCurveDescriptionParameter::New().GetPointer();
    functor->RegisterDescriptionParameter("TimeToPeak", parameterFunction);
};

void doDescription()
{
    mitk::PixelBasedDescriptionParameterImageGenerator::Pointer generator =
      mitk::PixelBasedDescriptionParameterImageGenerator::New();
    mitk::CurveParameterFunctor::Pointer functor = mitk::CurveParameterFunctor::New();
    ConfigureFunctor(functor);
    functor->SetGrid(mitk::ExtractTimeGrid(image));

    generator->SetFunctor(functor);
    generator->SetDynamicImage(image);
    generator->SetMask(mask);

    ::itk::CStyleCommand::Pointer command = ::itk::CStyleCommand::New();
    command->SetCallback(onFitEvent);

    std::cout << "Started curve descriptor computation process..." << std::endl;
    generator->AddObserver(::itk::AnyEvent(), command);
    generator->Generate();
    std::cout << std::endl << "Finished computation process" << std::endl;

    for (auto imageIterator : generator->GetParameterImages())
    {
        mitk::storeParameterResultImage(outFileName, imageIterator.first, imageIterator.second);
    }
}

void doPreview()
{
  mitk::CurveParameterFunctor::Pointer functor = mitk::CurveParameterFunctor::New();
  ConfigureFunctor(functor);

  auto pNames = functor->GetDescriptionParameterNames();

  for (auto aName : pNames)
  { 
    auto fullPath = mitk::generateModelFitResultImagePath(outFileName, aName);
    std::cout << "Store result parameter: " << aName << " -> " << fullPath << std::endl;
  }
}

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    setupParser(parser);

	mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (classic config)" }, { "MITK DICOM Reader" });

    const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);
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

    //! [do processing]
    try
    {
      if (preview)
      {
        doPreview();
      }
      else
      {
        image = mitk::IOUtil::Load<mitk::Image>(inFilename, &readerFilterFunctor);
        std::cout << "Input: " << inFilename << std::endl;

        if (!maskFileName.empty())
        {
          mask = mitk::IOUtil::Load<mitk::Image>(maskFileName, &readerFilterFunctor);
          std::cout << "Mask:  " << maskFileName << std::endl;
        }
        else
        {
          std::cout << "Mask:  none" << std::endl;
        }

        doDescription();
      }

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
