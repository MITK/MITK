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

#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>

#include <mitkPixelBasedParameterFitImageGenerator.h>
#include <mitkROIBasedParameterFitImageGenerator.h>
#include <mitkModelFitInfo.h>
#include <mitkModelFitCmdAppsHelper.h>

#include <mitkMaskedDynamicImageStatisticsGenerator.h>
#include <mitkLevenbergMarquardtModelFitFunctor.h>
#include <mitkNormalizedSumOfSquaredDifferencesFitCostFunction.h>
#include <mitkExtractTimeGrid.h>
#include <mitkAterialInputFunctionGenerator.h>
#include <mitkModelFitResultHelper.h>
#include <mitkDescriptivePharmacokineticBrixModelParameterizer.h>
#include <mitkDescriptivePharmacokineticBrixModelValueBasedParameterizer.h>
#include <mitkDescriptivePharmacokineticBrixModelFactory.h>
#include <mitkExtendedToftsModelParameterizer.h>
#include <mitkExtendedToftsModelFactory.h>
#include <mitkTwoCompartmentExchangeModelParameterizer.h>
#include <mitkTwoCompartmentExchangeModelFactory.h>
#include <mitkThreeStepLinearModelParameterizer.h>
#include <mitkThreeStepLinearModelFactory.h>
#include <mitkModelFactoryBase.h>

std::string inFilename;
std::string outFileName;
std::string maskFileName;
std::string aifMaskFileName;
std::string aifImageFileName;

mitk::Image::Pointer image;
mitk::Image::Pointer mask;
mitk::Image::Pointer aifImage;
mitk::Image::Pointer aifMask;

bool useConstraints(false);
bool verbose(false);
bool roibased(false);
bool preview(false);

std::string modelName;

float aifHematocritLevel(0);
float brixInjectionTime(0);

const std::string MODEL_NAME_3SL = "3SL";
const std::string MODEL_NAME_descriptive = "descriptive";
const std::string MODEL_NAME_tofts = "tofts";
const std::string MODEL_NAME_2CX = "2CX";

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
    parser.setTitle("MR Perfusion");
    parser.setDescription("MiniApp that allows to fit MRI perfusion models and generates the according parameter maps. IMPORTANT!!!: The app assumes that the input images (signal and AIF) are concentration images. If your images do not hold this assumption, convert the image date before using this app (e.g. by using the signal-to-concentration-converter mini app.");
    parser.setContributor("DKFZ MIC");
    //! [create parser]

    //! [add arguments]
    // how should arguments be prefixed
    parser.setArgumentPrefix("--", "-");
    // add each argument, unless specified otherwise each argument is optional
    // see mitkCommandLineParser::addArgument for more information
    parser.beginGroup("Model parameters");
    parser.addArgument(
        "model", "l", mitkCommandLineParser::String, "Model function", "Model that should be used to fit the concentration signal. Options are: \""+MODEL_NAME_descriptive+"\" (descriptive pharmacokinetic Brix model),\""+MODEL_NAME_3SL+"\" (three step linear model), \""+MODEL_NAME_tofts+"\" (extended tofts model) or \""+MODEL_NAME_2CX+"\" (two compartment exchange model).", us::Any(std::string(MODEL_NAME_tofts)));
    parser.addArgument(
        "injectiontime", "j", mitkCommandLineParser::Float, "Injection time [min]", "Injection time of the bolus. This information is needed for the descriptive pharmacokinetic Brix model.", us::Any());
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

    parser.beginGroup("AIF parameters");
    parser.addArgument(
      "aifmask", "n", mitkCommandLineParser::File, "AIF mask file", "Mask that defines the spatial image region that should be used as AIF for models that need one. Must have the same geometry as the AIF input image!", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument(
      "aifimage", "a", mitkCommandLineParser::File, "AIF image file", "3D+t image that defines the image that containes the AIF signal. If this flag is not set and the model needs a AIF, the CLI will assume that the AIF is encoded in the normal image. Must have the same geometry as the AIF mask!", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument(
      "hematocrit", "h", mitkCommandLineParser::Float, "Hematocrit Level", "Value needed for correct AIF computation. Only needed if model needs an AIF. Default value is 0.45.", us::Any(0.45));
    parser.endGroup();

    parser.beginGroup("Optional parameters");
    parser.addArgument(
        "mask", "m", mitkCommandLineParser::File, "Mask file", "Mask that defines the spatial image region that should be fitted. Must have the same geometry as the input image!", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument(
        "verbose", "v", mitkCommandLineParser::Bool, "Verbose Output", "Whether to produce verbose output");
    parser.addArgument(
        "roibased", "r", mitkCommandLineParser::Bool, "Roi based fitting", "Will compute a mean intesity signal over the ROI before fitting it. If this mode is used a mask must be specified.");
    parser.addArgument(
      "constraints", "c", mitkCommandLineParser::Bool, "Constraints", "Indicates if constraints should be used for the fitting (if flag is set the default contraints will be used.).", us::Any(false));
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

    // parse, cast and set required arguments
    modelName = MODEL_NAME_tofts;
    if (parsedArgs.count("model"))
    {
      modelName = us::any_cast<std::string>(parsedArgs["model"]);
    }

    inFilename = us::any_cast<std::string>(parsedArgs["input"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    if (parsedArgs.count("mask"))
    {
      maskFileName = us::any_cast<std::string>(parsedArgs["mask"]);
    }

    if (parsedArgs.count("aifimage"))
    {
      aifImageFileName = us::any_cast<std::string>(parsedArgs["aifimage"]);
    }

    if (parsedArgs.count("aifmask"))
    {
      aifMaskFileName = us::any_cast<std::string>(parsedArgs["aifmask"]);
    }

    verbose = false;
    if (parsedArgs.count("verbose"))
    {
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);
    }

    preview = false;
    if (parsedArgs.count("preview"))
    {
      preview = us::any_cast<bool>(parsedArgs["preview"]);
    }

    roibased = false;
    if (parsedArgs.count("roibased"))
    {
        roibased = us::any_cast<bool>(parsedArgs["roibased"]);
    }

    useConstraints = false;
    if (parsedArgs.count("constraints"))
    {
      useConstraints = us::any_cast<bool>(parsedArgs["constraints"]);
    }

    aifHematocritLevel = 0.45;
    if (parsedArgs.count("hematocrit"))
    {
      aifHematocritLevel = us::any_cast<float>(parsedArgs["hematocrit"]);
    }

    brixInjectionTime = 0.0;
    if (parsedArgs.count("injectiontime"))
    {
      brixInjectionTime = us::any_cast<float>(parsedArgs["injectiontime"]);
    }
    return true;
}

mitk::ModelFitFunctorBase::Pointer createDefaultFitFunctor(
    const mitk::ModelParameterizerBase* parameterizer, const mitk::ModelFactoryBase* modelFactory)
{
    mitk::LevenbergMarquardtModelFitFunctor::Pointer fitFunctor =
        mitk::LevenbergMarquardtModelFitFunctor::New();

    mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::Pointer chi2 =
        mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::New();
    fitFunctor->RegisterEvaluationParameter("Chi^2", chi2);

    if (useConstraints)
    {
      fitFunctor->SetConstraintChecker(modelFactory->CreateDefaultConstraints().GetPointer());
    }

    mitk::ModelBase::Pointer refModel = parameterizer->GenerateParameterizedModel();

    ::itk::LevenbergMarquardtOptimizer::ScalesType scales;
    scales.SetSize(refModel->GetNumberOfParameters());
    scales.Fill(1.0);
    fitFunctor->SetScales(scales);

    fitFunctor->SetDebugParameterMaps(true);

    return fitFunctor.GetPointer();
}

/**Helper that ensures that the mask (if it exists) is always 3D image. If the mask is originally an 4D image, the first
time step will be used.*/
mitk::Image::Pointer getMask3D()
{
  mitk::Image::Pointer result;

  if (mask.IsNotNull())
  {
    result = mask;

    //mask settings
    if (mask->GetTimeSteps() > 1)
    {
      MITK_INFO << "Selected mask has multiple timesteps. Only use first timestep to mask model fit.";
      mitk::ImageTimeSelector::Pointer maskedImageTimeSelector = mitk::ImageTimeSelector::New();
      maskedImageTimeSelector->SetInput(mask);
      maskedImageTimeSelector->SetTimeNr(0);
      maskedImageTimeSelector->UpdateLargestPossibleRegion();
      result = maskedImageTimeSelector->GetOutput();
    }
  }

  return result;
}


void getAIF(mitk::AIFBasedModelBase::AterialInputFunctionType& aif,
  mitk::AIFBasedModelBase::AterialInputFunctionType& aifTimeGrid)
{
  if (aifMask.IsNotNull())
  {
    aif.clear();
    aifTimeGrid.clear();

    mitk::AterialInputFunctionGenerator::Pointer aifGenerator =
      mitk::AterialInputFunctionGenerator::New();

    //Hematocrit level
    aifGenerator->SetHCL(aifHematocritLevel);
    std::cout << "AIF hematocrit level: " << aifHematocritLevel << std::endl;

    mitk::Image::Pointer selectedAIFMask = aifMask;

    //mask settings
    if (aifMask->GetTimeSteps() > 1)
    {
      MITK_INFO << "Selected AIF mask has multiple timesteps. Only use first timestep to mask model fit.";
      mitk::ImageTimeSelector::Pointer maskedImageTimeSelector = mitk::ImageTimeSelector::New();
      maskedImageTimeSelector->SetInput(aifMask);
      maskedImageTimeSelector->SetTimeNr(0);
      maskedImageTimeSelector->UpdateLargestPossibleRegion();
      aifMask = maskedImageTimeSelector->GetOutput();
    }

    aifGenerator->SetMask(aifMask);

    mitk::Image::Pointer selectedAIFImage = image;
    //image settings
    if (aifImage.IsNotNull())
    {
      selectedAIFImage = aifImage;
    }

    aifGenerator->SetDynamicImage(selectedAIFImage);

    aif = aifGenerator->GetAterialInputFunction();
    aifTimeGrid = aifGenerator->GetAterialInputFunctionTimeGrid();
  }
  else
  {
    mitkThrow() << "Cannot generate AIF. AIF mask was not specified or correctly loaded.";
  }
}


void generateDescriptiveBrixModel_PixelBased(mitk::modelFit::ModelFitInfo::Pointer&
  modelFitInfo, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::PixelBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::PixelBasedParameterFitImageGenerator::New();

  mitk::DescriptivePharmacokineticBrixModelParameterizer::Pointer modelParameterizer =
    mitk::DescriptivePharmacokineticBrixModelParameterizer::New();

  mitk::Image::Pointer mask3D = getMask3D();

  //Model configuration (static parameters) can be done now
  modelParameterizer->SetTau(brixInjectionTime);
  std::cout << "Injection time [min]: " << brixInjectionTime << std::endl;

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(0);
  imageTimeSelector->UpdateLargestPossibleRegion();

  mitk::DescriptivePharmacokineticBrixModelParameterizer::BaseImageType::Pointer baseImage;
  mitk::CastToItkImage(imageTimeSelector->GetOutput(), baseImage);

  modelParameterizer->SetBaseImage(baseImage);

  //Specify fitting strategy and criterion parameters
  mitk::ModelFactoryBase::Pointer factory = mitk::DescriptivePharmacokineticBrixModelFactory::New().GetPointer();
  mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer, factory);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  std::string roiUID = "";

  if (mask3D.IsNotNull())
  {
    fitGenerator->SetMask(mask3D);
    roiUID = mitk::EnsureModelFitUID(mask);
  }

  fitGenerator->SetDynamicImage(image);
  fitGenerator->SetFitFunctor(fitFunctor);

  generator = fitGenerator.GetPointer();

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    image, mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED(), roiUID);
}

void generateDescriptiveBrixModel_ROIBased(mitk::modelFit::ModelFitInfo::Pointer&
  modelFitInfo, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::Image::Pointer mask3D = getMask3D();

  if (mask3D.IsNull())
  {
    return;
  }

  mitk::ROIBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::ROIBasedParameterFitImageGenerator::New();

  mitk::DescriptivePharmacokineticBrixModelValueBasedParameterizer::Pointer modelParameterizer =
    mitk::DescriptivePharmacokineticBrixModelValueBasedParameterizer::New();

  //Compute ROI signal
  mitk::MaskedDynamicImageStatisticsGenerator::Pointer signalGenerator =
    mitk::MaskedDynamicImageStatisticsGenerator::New();
  signalGenerator->SetMask(mask3D);
  signalGenerator->SetDynamicImage(image);
  signalGenerator->Generate();

  mitk::MaskedDynamicImageStatisticsGenerator::ResultType roiSignal = signalGenerator->GetMean();

  //Model configuration (static parameters) can be done now
  modelParameterizer->SetTau(brixInjectionTime);
  std::cout << "Injection time [min]: " << brixInjectionTime << std::endl;
  modelParameterizer->SetBaseValue(roiSignal[0]);

  //Specify fitting strategy and criterion parameters
  mitk::ModelFactoryBase::Pointer factory = mitk::DescriptivePharmacokineticBrixModelFactory::New().GetPointer();
  mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer, factory);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  fitGenerator->SetMask(mask3D);
  fitGenerator->SetFitFunctor(fitFunctor);
  fitGenerator->SetSignal(roiSignal);
  fitGenerator->SetTimeGrid(mitk::ExtractTimeGrid(image));

  generator = fitGenerator.GetPointer();

  std::string roiUID = mitk::EnsureModelFitUID(mask);

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    image, mitk::ModelFitConstants::FIT_TYPE_VALUE_ROIBASED(), roiUID);
  mitk::ScalarListLookupTable::ValueType infoSignal;

  for (mitk::MaskedDynamicImageStatisticsGenerator::ResultType::const_iterator pos =
    roiSignal.begin(); pos != roiSignal.end(); ++pos)
  {
    infoSignal.push_back(*pos);
  }

  modelFitInfo->inputData.SetTableValue("ROI", infoSignal);
}


void Generate3StepLinearModelFit_PixelBased(mitk::modelFit::ModelFitInfo::Pointer&
  modelFitInfo, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::PixelBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::PixelBasedParameterFitImageGenerator::New();

  mitk::ThreeStepLinearModelParameterizer::Pointer modelParameterizer =
    mitk::ThreeStepLinearModelParameterizer::New();

  mitk::Image::Pointer mask3D = getMask3D();

  //Specify fitting strategy and criterion parameters
  mitk::ModelFactoryBase::Pointer factory = mitk::ThreeStepLinearModelFactory::New().GetPointer();
  mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer, factory);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  std::string roiUID = "";

  if (mask3D.IsNotNull())
  {
    fitGenerator->SetMask(mask3D);
    roiUID = mitk::EnsureModelFitUID(mask);
  }

  fitGenerator->SetDynamicImage(image);
  fitGenerator->SetFitFunctor(fitFunctor);

  generator = fitGenerator.GetPointer();

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    image, mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED(), roiUID);
}

void Generate3StepLinearModelFit_ROIBased(mitk::modelFit::ModelFitInfo::Pointer&
  modelFitInfo, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::Image::Pointer mask3D = getMask3D();

  if (mask3D.IsNull())
  {
    return;
  }

  mitk::ROIBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::ROIBasedParameterFitImageGenerator::New();

  mitk::ThreeStepLinearModelParameterizer::Pointer modelParameterizer =
    mitk::ThreeStepLinearModelParameterizer::New();

  //Compute ROI signal
  mitk::MaskedDynamicImageStatisticsGenerator::Pointer signalGenerator =
    mitk::MaskedDynamicImageStatisticsGenerator::New();
  signalGenerator->SetMask(mask3D);
  signalGenerator->SetDynamicImage(image);
  signalGenerator->Generate();

  mitk::MaskedDynamicImageStatisticsGenerator::ResultType roiSignal = signalGenerator->GetMean();

  //Specify fitting strategy and criterion parameters
  mitk::ModelFactoryBase::Pointer factory = mitk::ThreeStepLinearModelFactory::New().GetPointer();
  mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer, factory);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  fitGenerator->SetMask(mask3D);
  fitGenerator->SetFitFunctor(fitFunctor);
  fitGenerator->SetSignal(roiSignal);
  fitGenerator->SetTimeGrid(mitk::ExtractTimeGrid(image));

  generator = fitGenerator.GetPointer();

  std::string roiUID = mitk::EnsureModelFitUID(mask);

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    image, mitk::ModelFitConstants::FIT_TYPE_VALUE_ROIBASED(), roiUID);
  mitk::ScalarListLookupTable::ValueType infoSignal;

  for (mitk::MaskedDynamicImageStatisticsGenerator::ResultType::const_iterator pos =
    roiSignal.begin(); pos != roiSignal.end(); ++pos)
  {
    infoSignal.push_back(*pos);
  }

  modelFitInfo->inputData.SetTableValue("ROI", infoSignal);
}

template <typename TParameterizer, typename TFactory>
void generateAIFbasedModelFit_PixelBased(mitk::modelFit::ModelFitInfo::Pointer&
  modelFitInfo, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::PixelBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::PixelBasedParameterFitImageGenerator::New();

  typename TParameterizer::Pointer modelParameterizer =
    TParameterizer::New();

  mitk::AIFBasedModelBase::AterialInputFunctionType aif;
  mitk::AIFBasedModelBase::AterialInputFunctionType aifTimeGrid;
  getAIF(aif, aifTimeGrid);

  modelParameterizer->SetAIF(aif);
  modelParameterizer->SetAIFTimeGrid(aifTimeGrid);

  mitk::Image::Pointer mask3D = getMask3D();

  //Specify fitting strategy and criterion parameters
  mitk::ModelFactoryBase::Pointer factory = TFactory::New().GetPointer();
  mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer, factory);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  std::string roiUID = "";

  if (mask3D.IsNotNull())
  {
    fitGenerator->SetMask(mask3D);
    roiUID = mitk::EnsureModelFitUID(mask);
  }

  fitGenerator->SetDynamicImage(image);
  fitGenerator->SetFitFunctor(fitFunctor);

  generator = fitGenerator.GetPointer();

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    image, mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED(),
    roiUID);

  mitk::ScalarListLookupTable::ValueType infoSignal;

  for (mitk::AIFBasedModelBase::AterialInputFunctionType::const_iterator pos =
    aif.begin(); pos != aif.end(); ++pos)
  {
    infoSignal.push_back(*pos);
  }

  modelFitInfo->inputData.SetTableValue("AIF", infoSignal);
}

template <typename TParameterizer, typename TFactory>
void generateAIFbasedModelFit_ROIBased(
  mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
  mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  mitk::Image::Pointer mask3D = getMask3D();

  if (mask3D.IsNull())
  {
    return;
  }

  mitk::ROIBasedParameterFitImageGenerator::Pointer fitGenerator =
    mitk::ROIBasedParameterFitImageGenerator::New();

  typename TParameterizer::Pointer modelParameterizer =
    TParameterizer::New();

  mitk::AIFBasedModelBase::AterialInputFunctionType aif;
  mitk::AIFBasedModelBase::AterialInputFunctionType aifTimeGrid;
  getAIF(aif, aifTimeGrid);

  modelParameterizer->SetAIF(aif);
  modelParameterizer->SetAIFTimeGrid(aifTimeGrid);

  //Compute ROI signal
  mitk::MaskedDynamicImageStatisticsGenerator::Pointer signalGenerator =
    mitk::MaskedDynamicImageStatisticsGenerator::New();
  signalGenerator->SetMask(mask3D);
  signalGenerator->SetDynamicImage(image);
  signalGenerator->Generate();

  mitk::MaskedDynamicImageStatisticsGenerator::ResultType roiSignal = signalGenerator->GetMean();

  //Specify fitting strategy and criterion parameters
  mitk::ModelFactoryBase::Pointer factory = TFactory::New().GetPointer();
  mitk::ModelFitFunctorBase::Pointer fitFunctor = createDefaultFitFunctor(modelParameterizer, factory);

  //Parametrize fit generator
  fitGenerator->SetModelParameterizer(modelParameterizer);
  fitGenerator->SetMask(mask3D);
  fitGenerator->SetFitFunctor(fitFunctor);
  fitGenerator->SetSignal(roiSignal);
  fitGenerator->SetTimeGrid(mitk::ExtractTimeGrid(image));

  generator = fitGenerator.GetPointer();

  std::string roiUID = mitk::EnsureModelFitUID(mask);

  //Create model info
  modelFitInfo = mitk::modelFit::CreateFitInfoFromModelParameterizer(modelParameterizer,
    image, mitk::ModelFitConstants::FIT_TYPE_VALUE_ROIBASED(),
    roiUID);

  mitk::ScalarListLookupTable::ValueType infoSignal;

  for (mitk::MaskedDynamicImageStatisticsGenerator::ResultType::const_iterator pos =
    roiSignal.begin(); pos != roiSignal.end(); ++pos)
  {
    infoSignal.push_back(*pos);
  }

  modelFitInfo->inputData.SetTableValue("ROI", infoSignal);

  infoSignal.clear();

  for (mitk::AIFBasedModelBase::AterialInputFunctionType::const_iterator pos =
    aif.begin(); pos != aif.end(); ++pos)
  {
    infoSignal.push_back(*pos);
  }

  modelFitInfo->inputData.SetTableValue("AIF", infoSignal);
}


void storeResultImage(const std::string& name, mitk::Image* image, mitk::modelFit::Parameter::Type nodeType, const mitk::modelFit::ModelFitInfo* modelFitInfo)
{
    mitk::modelFit::SetModelFitDataProperties(image, name, nodeType, modelFitInfo);

    std::string ext = ::itksys::SystemTools::GetFilenameLastExtension(outFileName);

    std::string dir = itksys::SystemTools::GetFilenamePath(outFileName);
    dir = itksys::SystemTools::ConvertToOutputPath(dir);

    std::string rootName = itksys::SystemTools::GetFilenameWithoutLastExtension(outFileName);
    
    std::string fileName = rootName + "_" + name + ext;

    std::vector<std::string> pathElements;
    pathElements.push_back(dir);
    pathElements.push_back(fileName);

    std::string fullOutPath = itksys::SystemTools::ConvertToOutputPath(dir + "/" + fileName);

    mitk::IOUtil::Save(image, fullOutPath);

    std::cout << "Store result (parameter: "<<name<<"): " << fullOutPath << std::endl;
}


void createFitGenerator(mitk::modelFit::ModelFitInfo::Pointer& fitSession, mitk::ParameterFitImageGeneratorBase::Pointer& generator)
{
  bool isDescBrixFactory = modelName == MODEL_NAME_descriptive;
  bool isToftsFactory = modelName == MODEL_NAME_tofts;
  bool is2CXMFactory = modelName == MODEL_NAME_2CX;
  bool is3SLFactory = modelName == MODEL_NAME_3SL;

  if (isDescBrixFactory)
  {
    std::cout << "Model:  descriptive pharmacokinetic brix model" << std::endl;
    if (!roibased)
    {
      generateDescriptiveBrixModel_PixelBased(fitSession, generator);
    }
    else
    {
      generateDescriptiveBrixModel_ROIBased(fitSession, generator);
    }
  }
  else if (is3SLFactory)
  {
    std::cout << "Model:  three step linear model" << std::endl;
    if (!roibased)
    {
      Generate3StepLinearModelFit_PixelBased(fitSession, generator);
    }
    else
    {
      Generate3StepLinearModelFit_ROIBased(fitSession, generator);
    }
  }
  else if (isToftsFactory)
  {
    std::cout << "Model:  extended tofts model" << std::endl;
    if (!roibased)
    {
      generateAIFbasedModelFit_PixelBased<mitk::ExtendedToftsModelParameterizer, mitk::ExtendedToftsModelFactory>(fitSession, generator);
    }
    else
    {
      generateAIFbasedModelFit_ROIBased<mitk::ExtendedToftsModelParameterizer, mitk::ExtendedToftsModelFactory>(fitSession, generator);
    }
  }
  else if (is2CXMFactory)
  {
    std::cout << "Model:  two compartment exchange model" << std::endl;
    if (!roibased)
    {
      generateAIFbasedModelFit_PixelBased<mitk::TwoCompartmentExchangeModelParameterizer, mitk::TwoCompartmentExchangeModelFactory>(fitSession, generator);
    }
    else
    {
      generateAIFbasedModelFit_ROIBased<mitk::TwoCompartmentExchangeModelParameterizer, mitk::TwoCompartmentExchangeModelFactory>(fitSession, generator);
    }
  }
  else
  {
    std::cerr << "ERROR. Model flag is unknown. Given flag: " << modelName << std::endl;
  }
}

void doFitting()
{
        mitk::ParameterFitImageGeneratorBase::Pointer generator = nullptr;
        mitk::modelFit::ModelFitInfo::Pointer fitSession = nullptr;

        ::itk::CStyleCommand::Pointer command = ::itk::CStyleCommand::New();
        command->SetCallback(onFitEvent);

        createFitGenerator(fitSession, generator);

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
            mitkThrow() << "Fitting error! Could not initialize fitting job.";
        }
}

void doPreview()
{
  mitk::ParameterFitImageGeneratorBase::Pointer generator = nullptr;
  mitk::modelFit::ModelFitInfo::Pointer fitSession = nullptr;

  createFitGenerator(fitSession, generator);

  if (generator.IsNotNull())
  {
    mitk::previewModelFitGeneratorResults(outFileName, generator);
  }
  else
  {
    mitkThrow() << "Fitting error! Could not initialize fitting job.";
  }
}

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    setupParser(parser);
    const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);

    mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (classic config)" }, { "MITK DICOM Reader" });

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

        if (modelName != MODEL_NAME_descriptive && modelName != MODEL_NAME_3SL)
        {
          if (!aifMaskFileName.empty())
          {
            aifMask = mitk::IOUtil::Load<mitk::Image>(aifMaskFileName, &readerFilterFunctor);
            std::cout << "AIF mask:  " << aifMaskFileName << std::endl;
          }
          else
          {
            mitkThrow() << "Error. Cannot fit. Choosen model needs an AIF. Please specify AIF mask (--aifmask).";
          }
          if (!aifImageFileName.empty())
          {
   	        aifImage = mitk::IOUtil::Load<mitk::Image>(aifImageFileName, &readerFilterFunctor);
            std::cout << "AIF image:  " << aifImageFileName << std::endl;
          }
          else
          {
            std::cout << "AIF image: none (using signal image)" << std::endl;
          }
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

        if (preview)
        {
          doPreview();
        }
        else
        {
          doFitting();
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
