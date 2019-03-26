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
#include <mitkPixelBasedParameterFitImageGenerator.h>
#include <mitkROIBasedParameterFitImageGenerator.h>
#include <mitkLinearModelParameterizer.h>
#include <mitkGenericParamModelParameterizer.h>
#include <mitkModelFitInfo.h>
#include <mitkMaskedDynamicImageStatisticsGenerator.h>
#include <mitkLevenbergMarquardtModelFitFunctor.h>
#include <mitkNormalizedSumOfSquaredDifferencesFitCostFunction.h>
#include <mitkExtractTimeGrid.h>
#include <mitkModelFitCmdAppsHelper.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

std::string inFilename;
std::string outFileName;
std::string maskFileName;
bool verbose(false);
bool roibased(false);
std::string functionName;
std::string formular;
mitk::Image::Pointer image;
mitk::Image::Pointer mask;


void onFitEvent(::itk::Object* caller, const itk::EventObject & event, void* /*data*/)
{
    itk::ProgressEvent progressEvent;

    if (progressEvent.CheckEvent(&event))
    {
        mitk::ParameterFitImageGeneratorBase* castedReporter = dynamic_cast<mitk::ParameterFitImageGeneratorBase*>(caller);
        std::cout <<castedReporter->GetProgress()*100 << "% ";
    }
}


void setupParser(mitkCommandLineParser& parser)
{
    // set general information about your MiniApp
    parser.setCategory("Dynamic Data Analysis Tools");
    parser.setTitle("Generic Fitting");
    parser.setDescription("MiniApp that allows to make a pixel based fitting on the intensity signal over time for a given model function.");
    parser.setContributor("DKFZ MIC");
    //! [create parser]

    //! [add arguments]
    // how should arguments be prefixed
    parser.setArgumentPrefix("--", "-");
    // add each argument, unless specified otherwise each argument is optional
    // see mitkCommandLineParser::addArgument for more information
    parser.beginGroup("Model parameters");
    parser.addArgument(
        "function", "f", mitkCommandLineParser::String, "Model function", "Function that should be used to fit the intensity signals. Options are: \"Linear\" or \"<Parameter Number>\" (for generic formulas).", us::Any(std::string("Linear")));
    parser.addArgument(
        "formular", "y", mitkCommandLineParser::String, "Generic model function formular", "Formular of a generic model (if selected) that will be parsed and fitted.", us::Any());
    parser.endGroup();
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
        "roibased", "r", mitkCommandLineParser::Bool, "Roi based fitting", "Will compute a mean intesity signal over the ROI before fitting it. If this mode is used a mask must be specified.");
    parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
    parser.endGroup();
    //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
    if (parsedArgs.size() == 0)
        return false;

    // parse, cast and set required arguments
    functionName = "Linear";
    if (parsedArgs.count("function"))
    {
        functionName = us::any_cast<std::string>(parsedArgs["function"]);
    }
    if (parsedArgs.count("formular"))
    {
        formular = us::any_cast<std::string>(parsedArgs["formular"]);
    }
    inFilename = us::any_cast<std::string>(parsedArgs["input"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);


    verbose = false;
    if (parsedArgs.count("verbose"))
    {
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);
    }

    roibased = false;
    if (parsedArgs.count("roibased"))
    {
        roibased = us::any_cast<bool>(parsedArgs["roibased"]);
    }

    if (parsedArgs.count("mask"))
    {
        maskFileName = us::any_cast<std::string>(parsedArgs["mask"]);
    }

    return true;
}

void configureInitialParametersOfParameterizer(mitk::ModelParameterizerBase*
    parameterizer)
{
    mitk::GenericParamModelParameterizer* genericParameterizer =
        dynamic_cast<mitk::GenericParamModelParameterizer*>(parameterizer);

    if (genericParameterizer)
    {
        genericParameterizer->SetFunctionString(formular);
    }
}

mitk::ModelFitFunctorBase::Pointer createDefaultFitFunctor(
    const mitk::ModelParameterizerBase* parameterizer)
{
    mitk::LevenbergMarquardtModelFitFunctor::Pointer fitFunctor =
        mitk::LevenbergMarquardtModelFitFunctor::New();

    mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::Pointer chi2 =
        mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::New();
    fitFunctor->RegisterEvaluationParameter("Chi^2", chi2);

    mitk::ModelBase::Pointer refModel = parameterizer->GenerateParameterizedModel();

    ::itk::LevenbergMarquardtOptimizer::ScalesType scales;
    scales.SetSize(refModel->GetNumberOfParameters());
    scales.Fill(1.0);
    fitFunctor->SetScales(scales);

    fitFunctor->SetDebugParameterMaps(true);

    return fitFunctor.GetPointer();
}

template <typename TParameterizer>
void generateModelFit_PixelBased(mitk::modelFit::ModelFitInfo::Pointer&
    /*modelFitInfo*/, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
    mitk::PixelBasedParameterFitImageGenerator::Pointer fitGenerator =
        mitk::PixelBasedParameterFitImageGenerator::New();

    typename TParameterizer::Pointer modelParameterizer =
        TParameterizer::New();

    configureInitialParametersOfParameterizer(modelParameterizer);

    //Specify fitting strategy and criterion parameters
    mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer);

    //Parametrize fit generator
    fitGenerator->SetModelParameterizer(modelParameterizer);

    fitGenerator->SetMask(mask);

    fitGenerator->SetDynamicImage(image);
    fitGenerator->SetFitFunctor(fitFunctor);

    generator = fitGenerator.GetPointer();
}

template <typename TParameterizer>
void generateModelFit_ROIBased(
    mitk::modelFit::ModelFitInfo::Pointer& /*modelFitInfo*/,
    mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
    mitk::ROIBasedParameterFitImageGenerator::Pointer fitGenerator =
        mitk::ROIBasedParameterFitImageGenerator::New();

    typename TParameterizer::Pointer modelParameterizer =
        TParameterizer::New();

    configureInitialParametersOfParameterizer(modelParameterizer);

    //Compute ROI signal
    mitk::MaskedDynamicImageStatisticsGenerator::Pointer signalGenerator =
        mitk::MaskedDynamicImageStatisticsGenerator::New();
    signalGenerator->SetMask(mask);
    signalGenerator->SetDynamicImage(image);
    signalGenerator->Generate();

    mitk::MaskedDynamicImageStatisticsGenerator::ResultType roiSignal = signalGenerator->GetMean();

    //Specify fitting strategy and criterion parameters
    mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer);

    //Parametrize fit generator
    fitGenerator->SetModelParameterizer(modelParameterizer);
    fitGenerator->SetMask(mask);
    fitGenerator->SetFitFunctor(fitFunctor);
    fitGenerator->SetSignal(roiSignal);
    fitGenerator->SetTimeGrid(mitk::ExtractTimeGrid(image));

    generator = fitGenerator.GetPointer();
}

void doFitting()
{
        mitk::ParameterFitImageGeneratorBase::Pointer generator = NULL;
        mitk::modelFit::ModelFitInfo::Pointer fitSession = NULL;

        ::itk::CStyleCommand::Pointer command = ::itk::CStyleCommand::New();
        command->SetCallback(onFitEvent);

        bool isLinearFactory = functionName == "Linear";

        if (isLinearFactory)
        {
            std::cout << "Model:  linear" << std::endl;
            if (!roibased)
            {
                generateModelFit_PixelBased<mitk::LinearModelParameterizer>(fitSession, generator);
            }
            else
            {
                generateModelFit_ROIBased<mitk::LinearModelParameterizer>(fitSession, generator);
            }
        }
        else
        {
            std::cout << "Model:  generic (2 parameter)" << std::endl;
            if (!roibased)
            {
                generateModelFit_PixelBased<mitk::GenericParamModelParameterizer>(fitSession, generator);
            }
            else
            {
                generateModelFit_ROIBased<mitk::GenericParamModelParameterizer>(fitSession, generator);
            }
        }

        if (generator.IsNotNull() )
        {
            std::cout << "Started fitting process..." << std::endl;
            generator->AddObserver(::itk::AnyEvent(), command);
            generator->Generate();
            std::cout << std::endl << "Finished fitting process" << std::endl;

            mitk::storeModelFitGeneratorResults(outFileName, generator, fitSession);
        }
        else
        {
            mitkThrow() << "Fitting error! Could not initalize fitting job.";
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

        if (roibased && mask.IsNull())
        {
            mitkThrow() << "Error. Cannot fit. Please specify mask if you select roi based fitting.";
        }

        std::cout << "Style: ";
        if (roibased)
        {
            std::cout << "ROI based";
        }
        else
        {
            std::cout << "pixel based";
        }
        std::cout << std::endl;

        doFitting();

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
