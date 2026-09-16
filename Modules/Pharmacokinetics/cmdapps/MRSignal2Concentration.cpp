/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// std includes
#include <string>

// itk includes
#include <itksys/SystemTools.hxx>

// CTK includes
#include <mitkCommandLineParser.h>

// MITK includes
#include <mitkIOUtil.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

#include <mitkConcentrationCurveGenerator.h>

std::string inFilename;
std::string outFileName;

mitk::Image::Pointer image;

bool verbose(false);

bool t1_absolute(false);
bool t1_relative(false);
bool t2(false);

float k(0);
float te(0);

void setupParser(mitkCommandLineParser& parser)
{
    // set general information about the app
    parser.setCategory("Dynamic Data Analysis Tools");
    parser.setTitle("MR Signal to Concentration Converter");
    parser.setDescription("Converts a T1 or T2 signal image into a concentration image for perfusion analysis.");
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
        "Output file",
        "where to save the output concentration image.",
        us::Any(),
        false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("Conversion parameters");
    parser.addArgument(
      "t1-absolute", "", mitkCommandLineParser::Bool, "T1 absolute signal enhancement", "Activate conversion for T1 absolute signal enhancement.");
    parser.addArgument(
      "t1-relative", "", mitkCommandLineParser::Bool, "T1 relative signal enhancement", "Activate conversion for T1 relative signal enhancement.");
    parser.addArgument(
      "t2", "", mitkCommandLineParser::Bool, "T2 signal conversion", "Activate conversion for T2 signal enhancement to concentration.");

    parser.addArgument(
      "k", "k", mitkCommandLineParser::Float, "Conversion factor k", "Required for all conversion modes (T1-absolute, T1-relative, T2).", us::Any(), false);
    parser.addArgument(
      "te", "", mitkCommandLineParser::Float, "Echo time TE", "Required for the T2 conversion mode.", us::Any());
    parser.endGroup();

    parser.beginGroup("Optional parameters");
    parser.addArgument(
        "verbose", "v", mitkCommandLineParser::Bool, "Verbose Output", "Whether to produce verbose output");
    parser.endGroup();
    //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
    if (parsedArgs.size() == 0)
        return false;

    inFilename = us::any_cast<std::string>(parsedArgs["input"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    verbose = false;
    if (parsedArgs.count("verbose"))
    {
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);
    }

    t1_absolute = false;
    if (parsedArgs.count("t1-absolute"))
    {
      t1_absolute = us::any_cast<bool>(parsedArgs["t1-absolute"]);
    }

    t1_relative = false;
    if (parsedArgs.count("t1-relative"))
    {
      t1_relative = us::any_cast<bool>(parsedArgs["t1-relative"]);
    }


    t2 = false;
    if (parsedArgs.count("t2"))
    {
      t2 = us::any_cast<bool>(parsedArgs["t2"]);
    }

    k = 0.0;
    const bool kIsSet = parsedArgs.count("k") > 0;
    if (kIsSet)
    {
      k = us::any_cast<float>(parsedArgs["k"]);
    }

    te = 0.0;
    const bool teIsSet = parsedArgs.count("te") > 0;
    if (teIsSet)
    {
      te = us::any_cast<float>(parsedArgs["te"]);
    }

    //consistency checks
    int modeCount = 0;
    if (t1_absolute) ++modeCount;
    if (t1_relative) ++modeCount;
    if (t2) ++modeCount;

    if (modeCount==0)
    {
      mitkThrow() << "Invalid program call. Please select the type of conversion.";
    }

    if (modeCount >1)
    {
      mitkThrow() << "Invalid program call. Please select only ONE type of conversion.";
    }

    // Test presence, not the value: testing the number itself reported an
    // explicitly supplied 0 as a missing argument.
    if (!kIsSet)
    {
      mitkThrow() << "Invalid program call. Please set 'k', if you use t1-absolute, t1-relative or t2.";
    }

    if (k == 0.0f)
    {
      mitkThrow() << "Invalid program call. 'k' must not be 0.";
    }

    if (t2 && !teIsSet)
    {
      mitkThrow() << "Invalid program call. Please set 'te', if you use t2 mode.";
    }

    if (t2 && te == 0.0f)
    {
      mitkThrow() << "Invalid program call. 'te' must not be 0.";
    }


    return true;
}

void doConversion()
{
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(image);

    concentrationGen->SetAbsoluteSignalEnhancement(t1_absolute);
    concentrationGen->SetRelativeSignalEnhancement(t1_relative);

    concentrationGen->SetisT2weightedImage(t2);

    if (t2)
    {
      concentrationGen->SetT2Factor(k);
      concentrationGen->SetT2EchoTime(te);
    }
    else
    {
      concentrationGen->SetFactor(k);
    }

    mitk::Image::Pointer concentrationImage = concentrationGen->GetConvertedImage();

    mitk::IOUtil::Save(concentrationImage, outFileName);

    std::cout << "Store result: " << outFileName << std::endl;
}

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    setupParser(parser);
    const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);

    try
    {
      if (!configureApplicationSettings(parsedArgs))
      {
        return EXIT_FAILURE;
      }
    }
    catch (const itk::ExceptionObject& e)
    {
      MITK_ERROR << e.what();
      return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << e.what();
      return EXIT_FAILURE;
    }
    catch (...)
    {
      MITK_ERROR << "Unexpected error encountered when parsing the CLI arguments.";
      return EXIT_FAILURE;
    }

    mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (autoselect)" }, { "" });

    //! [do processing]
    try
    {
        image = mitk::IOUtil::Load<mitk::Image>(inFilename, &readerFilterFunctor);
        std::cout << "Input: " << inFilename << std::endl;

        doConversion();

        std::cout << "Processing finished." << std::endl;

        return EXIT_SUCCESS;
    }
    catch (const itk::ExceptionObject& e)
    {
        MITK_ERROR << e.what();
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
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
