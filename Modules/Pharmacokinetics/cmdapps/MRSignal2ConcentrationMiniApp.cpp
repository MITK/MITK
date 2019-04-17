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
#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkModelFitUIDHelper.h>

#include <mitkConcentrationCurveGenerator.h>

std::string inFilename;
std::string outFileName;

mitk::Image::Pointer image;

bool verbose(false);

bool t1_absolute(false);
bool t1_relative(false);
bool t1_flash(false);
bool t2(false);

float k(1.0);
float te(0);
float rec_time(0);
float relaxivity(0);
float rel_time(0);

void setupParser(mitkCommandLineParser& parser)
{
    // set general information about your MiniApp
    parser.setCategory("Dynamic Data Analysis Tools");
    parser.setTitle("MR Signal to Concentration Converter");
    parser.setDescription("MiniApp that allows to convert a T1 or T2 signal image into a concentration image for perfusion analysis.");
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
      "t1-flash", "", mitkCommandLineParser::Bool, "T1 turbo flash", "Activate specific conversion for T1 turbo flash sequences.");
    parser.addArgument(
      "t2", "", mitkCommandLineParser::Bool, "T2 signal conversion", "Activate conversion for T2 signal enhancement to concentration.");

    parser.addArgument(
      "k", "k", mitkCommandLineParser::Float, "Conversion factor k", "Needed for the following conversion modes: T1-absolute, T1-relative, T2. Default value is 1.", us::Any(1));
    parser.addArgument(
      "recovery-time", "", mitkCommandLineParser::Float, "Recovery time", "Needed for the following conversion modes: T1-flash.");
    parser.addArgument(
      "relaxivity", "", mitkCommandLineParser::Float, "Relaxivity", "Needed for the following conversion modes: T1-flash.");
    parser.addArgument(
      "relaxation-time", "", mitkCommandLineParser::Float, "Relaxation time", "Needed for the following conversion modes: T1-flash.");
    parser.addArgument(
      "te", "", mitkCommandLineParser::Float, "Echo time TE", "Needed for the following conversion modes: T2.", us::Any(1));

    parser.beginGroup("Optional parameters");
    parser.addArgument(
        "verbose", "v", mitkCommandLineParser::Bool, "Verbose Output", "Whether to produce verbose output");
    parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
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

    t1_flash = false;
    if (parsedArgs.count("t1-flash"))
    {
      t1_flash = us::any_cast<bool>(parsedArgs["t1-flash"]);
    }

    t2 = false;
    if (parsedArgs.count("t2"))
    {
      t2 = us::any_cast<bool>(parsedArgs["t2"]);
    }

    k = 0.0;
    if (parsedArgs.count("k"))
    {
      k = us::any_cast<float>(parsedArgs["k"]);
    }

    relaxivity = 0.0;
    if (parsedArgs.count("relaxivity"))
    {
      relaxivity = us::any_cast<float>(parsedArgs["relaxivity"]);
    }

    rec_time = 0.0;
    if (parsedArgs.count("recovery-time"))
    {
      rec_time = us::any_cast<float>(parsedArgs["recovery-time"]);
    }

    rel_time = 0.0;
    if (parsedArgs.count("relaxation-time"))
    {
      rel_time = us::any_cast<float>(parsedArgs["relaxation-time"]);
    }

    te = 0.0;
    if (parsedArgs.count("te"))
    {
      te = us::any_cast<float>(parsedArgs["te"]);
    }

    //consistency checks
    int modeCount = 0;
    if (t1_absolute) ++modeCount;
    if (t1_flash) ++modeCount;
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

    if (!k && (t2 || t1_absolute || t1_relative))
    {
      mitkThrow() << "Invalid program call. Please set 'k', if you use t1-absolute, t1-relative or t2.";
    }

    if (!te && t2)
    {
      mitkThrow() << "Invalid program call. Please set 'te', if you use t2 mode.";
    }

    if ((!rec_time||!rel_time||!relaxivity) && t1_flash)
    {
      mitkThrow() << "Invalid program call. Please set 'recovery-time', 'relaxation-time' and 'relaxivity', if you use t1-flash mode.";
    }

    return true;
}

void doConversion()
{
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(image);

    concentrationGen->SetisTurboFlashSequence(t1_flash);
    concentrationGen->SetAbsoluteSignalEnhancement(t1_absolute);
    concentrationGen->SetRelativeSignalEnhancement(t1_relative);

    concentrationGen->SetisT2weightedImage(t2);

    if (t1_flash)
    {
      concentrationGen->SetRecoveryTime(rec_time);
      concentrationGen->SetRelaxationTime(rel_time);
      concentrationGen->SetRelaxivity(relaxivity);
    }
    else if (t2)
    {
      concentrationGen->SetT2Factor(k);
      concentrationGen->SetT2EchoTime(te);
    }
    else
    {
      concentrationGen->SetFactor(k);
    }

    mitk::Image::Pointer concentrationImage = concentrationGen->GetConvertedImage();

    mitk::EnsureModelFitUID(concentrationImage);
    mitk::IOUtil::Save(concentrationImage, outFileName);

    std::cout << "Store result: " << outFileName << std::endl;
}

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    setupParser(parser);
    const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);
    if (!configureApplicationSettings(parsedArgs))
    {
        return EXIT_FAILURE;
    };

    mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (classic config)" }, { "MITK DICOM Reader" });

    // Show a help message
    if (parsedArgs.count("help") || parsedArgs.count("h"))
    {
        std::cout << parser.helpText();
        return EXIT_SUCCESS;
    }

    //! [do processing]
    try
    {
        image = mitk::IOUtil::Load<mitk::Image>(inFilename, &readerFilterFunctor);
        std::cout << "Input: " << inFilename << std::endl;

        doConversion();

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
