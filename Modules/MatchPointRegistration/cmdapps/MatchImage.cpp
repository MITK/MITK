/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkMAPAlgorithmHelper.h>
#include <mitkPointSet.h>
#include <mitkImageTimeSelector.h>

#include <itkStdStreamLogOutput.h>

// MatchPoint
#include <mapRegistrationAlgorithmInterface.h>
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapPointSetRegistrationAlgorithmInterface.h>
#include <mapMaskedRegistrationAlgorithmInterface.h>
#include <mapMetaPropertyAlgorithmInterface.h>
#include <mapMetaProperty.h>
#include <mapConvert.h>
#include <mapDeploymentDLLAccess.h>
#include <mapDeploymentDLLHandle.h>
#include <mapRegistrationBase.h>

#include <nlohmann/json.hpp>

struct Settings
{
  std::string movingFileName = "";
  std::string targetFileName = "";
  std::string outFileName = "";
  std::string algFileName = "";
  std::string parameters = "";
};

void SetupParser(mitkCommandLineParser& parser)
{
  parser.setTitle("Match Image");
  parser.setCategory("Registration Tools");
  parser.setDescription("");
  parser.setContributor("MIC, German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.beginGroup("Required I/O parameters");
  parser.addArgument(
    "moving", "m",
    mitkCommandLineParser::File,
    "Moving image files", "Path to the data that should be registred into the target space.",
    us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "target", "t",
    mitkCommandLineParser::File,
    "Tareget image files", "Path to the data that should be the target data on which the moving data should be registered.",
    us::Any(), false, false, false, mitkCommandLineParser::Input);

  parser.addArgument(
    "algorithm", "a",
    mitkCommandLineParser::File,
    "Registration algorithm", "Path to the registration algorithm that should be used for registration.",
    us::Any(), false, false, false, mitkCommandLineParser::Input);

  parser.addArgument("output",
    "o",
    mitkCommandLineParser::File,
    "Output file path",
    "Path to the generated registration.",
    us::Any(),
    false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "parameters", "p", mitkCommandLineParser::String, "Parameters", "Json string containing a json object that contains the parameters that should be passed to the algorithm as key value pairs.");
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.endGroup();
}

bool ConfigureApplicationSettings(std::map<std::string, us::Any> parsedArgs, Settings& settings)
{
  try
  {
    if (parsedArgs.size() == 0)
      return false;

    settings.movingFileName = us::any_cast<std::string>(parsedArgs["moving"]);
    settings.targetFileName = us::any_cast<std::string>(parsedArgs["target"]);
    settings.outFileName = us::any_cast<std::string>(parsedArgs["output"]);
    settings.algFileName = us::any_cast<std::string>(parsedArgs["algorithm"]);
    if (parsedArgs.count("parameters") > 0)
    {
      settings.parameters = us::any_cast<std::string>(parsedArgs["parameters"]);
    }
  }
  catch (...)
  {
    return false;
  }

  return true;
}

map::deployment::RegistrationAlgorithmBasePointer loadAlgorithm(const Settings& settings)
{
  map::deployment::RegistrationAlgorithmBasePointer spAlgorithmBase = nullptr;

  std::cout << std::endl << "Load registration algorithm..." << std::endl;

  map::deployment::DLLHandle::Pointer spHandle = nullptr;

  spHandle = map::deployment::openDeploymentDLL(settings.algFileName);

  if (spHandle.IsNull())
  {
    mapDefaultExceptionStaticMacro(<<
      "Cannot open deployed registration algorithm file.");
  }

  std::cout << "... libary opened..." << std::endl;

  std::cout << "Algorithm information: " << std::endl;
  spHandle->getAlgorithmUID().Print(std::cout, 2);
  std::cout << std::endl;

  //Now load the algorthm from DLL
  spAlgorithmBase = map::deployment::getRegistrationAlgorithm(spHandle);


  if (spAlgorithmBase.IsNotNull())
  {
    std::cout << "... done" << std::endl << std::endl;
  }
  else
  {
    mapDefaultExceptionStaticMacro(<< "Cannot create algorithm instance");
  }
  return spAlgorithmBase;
};

mitk::Image::Pointer ExtractFirstFrame(const mitk::Image* dynamicImage)
{
  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(dynamicImage);
  imageTimeSelector->SetTimeNr(0);
  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

template <typename TValueType>
map::core::MetaPropertyBase::Pointer
CheckCastAndSetProp(const nlohmann::json& value)
{
  map::core::MetaPropertyBase::Pointer prop;

  try
  {
    const auto castedValue = value.get<TValueType>();
    prop = map::core::MetaProperty<TValueType>::New(castedValue).GetPointer();
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Cannot convert value \"" << value << "\" into type: " << typeid(TValueType).name() << ". Details: " << e.what();
  }
  catch (...)
  {
    MITK_ERROR << "Unkown error. Cannot convert value \"" << value << "\" into type: " << typeid(TValueType).name();
  }

  return prop;
};

template <typename TValueType>
map::core::MetaPropertyBase::Pointer
CheckCastAndSetItkArrayProp(const nlohmann::json& valueSequence)
{
  using ArrayType = ::itk::Array<TValueType>;
  ArrayType castedValue;
  map::core::MetaPropertyBase::Pointer prop;

  try
  {
    castedValue.SetSize(valueSequence.size());

    typename ::itk::Array<TValueType>::SizeValueType index = 0;
    for (const auto& element : valueSequence)
    {
      const auto castedElement = element.template get<TValueType>();
      castedValue[index] = castedElement;
    }
    prop = map::core::MetaProperty<::itk::Array<TValueType>>::New(castedValue).GetPointer();
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Cannot convert value \"" << valueSequence << "\" into type: " << typeid(ArrayType).name() << ". Details: " << e.what();
  }
  catch (...)
  {
    MITK_ERROR << "Unkown error. Cannot convert value \"" << valueSequence << "\" into type: " << typeid(ArrayType).name();
  }

  return prop;
};

::map::core::MetaPropertyBase::Pointer
WrapIntoMetaProperty(const ::map::algorithm::MetaPropertyInfo* pInfo, const nlohmann::json& value)
{
  map::core::MetaPropertyBase::Pointer metaProp;

  if (pInfo == nullptr)
  {
    return metaProp;
  }

  if (pInfo->getTypeInfo() == typeid(int)) {
    metaProp = CheckCastAndSetProp<int>(value);
  }
  else if (pInfo->getTypeInfo() == typeid(unsigned int)) {
    metaProp = CheckCastAndSetProp<unsigned int>(value);
  }
  else if (pInfo->getTypeInfo() == typeid(long)) {
    metaProp = CheckCastAndSetProp<long>(value);
  }
  else if (pInfo->getTypeInfo() == typeid(unsigned long)) {
    metaProp = CheckCastAndSetProp<unsigned long>(value);
  }
  else if (pInfo->getTypeInfo() == typeid(float)) {
    metaProp = CheckCastAndSetProp<float>(value);
  }
  else if (pInfo->getTypeInfo() == typeid(double)) {
    metaProp = CheckCastAndSetProp<double>(value);
  }
  else if (pInfo->getTypeInfo() == typeid(::itk::Array<double>)) {
    metaProp = CheckCastAndSetItkArrayProp< double >(value);
  }
  else if (pInfo->getTypeInfo() == typeid(bool)) {
    metaProp = CheckCastAndSetProp< bool >(value);
  }
  else if (pInfo->getTypeInfo() == typeid(::map::core::String))
  {
    metaProp = map::core::MetaProperty<map::core::String>::New(value).GetPointer();
  }

  return metaProp;
};

void OnMapAlgorithmEvent(::itk::Object*, const itk::EventObject& event, void*)
{
  const map::events::AlgorithmEvent* pAlgEvent = dynamic_cast<const map::events::AlgorithmEvent*>(&event);
  const map::events::AlgorithmWrapperEvent* pWrapEvent =
    dynamic_cast<const map::events::AlgorithmWrapperEvent*>(&event);
  const map::events::InitializingAlgorithmEvent* pInitEvent =
    dynamic_cast<const map::events::InitializingAlgorithmEvent*>(&event);
  const map::events::StartingAlgorithmEvent* pStartEvent =
    dynamic_cast<const map::events::StartingAlgorithmEvent*>(&event);
  const map::events::StoppingAlgorithmEvent* pStoppingEvent =
    dynamic_cast<const map::events::StoppingAlgorithmEvent*>(&event);
  const map::events::StoppedAlgorithmEvent* pStoppedEvent =
    dynamic_cast<const map::events::StoppedAlgorithmEvent*>(&event);
  const map::events::FinalizingAlgorithmEvent* pFinalizingEvent =
    dynamic_cast<const map::events::FinalizingAlgorithmEvent*>(&event);
  const map::events::FinalizedAlgorithmEvent* pFinalizedEvent =
    dynamic_cast<const map::events::FinalizedAlgorithmEvent*>(&event);

  if (pInitEvent)
  {
    std::cout <<"Initializing algorithm ..." << std::endl;
  }
  else if (pStartEvent)
  {
    std::cout <<"Starting algorithm ..." << std::endl;
  }
  else if (pStoppingEvent)
  {
    std::cout <<"Stopping algorithm ..." << std::endl;
  }
  else if (pStoppedEvent)
  {
    std::cout <<"Stopped algorithm ..." << std::endl;

    if (!pStoppedEvent->getComment().empty())
    {
      std::cout <<"Stopping condition: " << pStoppedEvent->getComment() << std::endl;
    }
  }
  else if (pFinalizingEvent)
  {
    std::cout <<"Finalizing algorithm and results ..." << std::endl;
  }
  else if (pFinalizedEvent)
  {
    std::cout <<"Finalized algorithm ..." << std::endl;
  }
  else if (pAlgEvent && !pWrapEvent)
  {
    std::cout << pAlgEvent->getComment() << std::endl;
  }
}

int main(int argc, char* argv[])
{
  std::cout << "MitkMatchImage - Generic light weight image registration tool based on MatchPoint." << std::endl;

  Settings settings;
  mitkCommandLineParser parser;
  SetupParser(parser);

  const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);
  if (!ConfigureApplicationSettings(parsedArgs, settings))
  {
    MITK_ERROR << "Command line arguments are invalid. To see the correct usage please call with -h or --help to show the help information.";
    return EXIT_FAILURE;
  };

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::cout << std::endl << "*******************************************" << std::endl;
  std::cout << "Moving file:        " << settings.movingFileName << std::endl;
  std::cout << "Target file:        " << settings.targetFileName << std::endl;
  std::cout << "Output file:        " << settings.outFileName << std::endl;
  std::cout << "Algorithm location: " << settings.algFileName << std::endl;

  try
  {
    auto algorithm = loadAlgorithm(settings);

    auto command = ::itk::CStyleCommand::New();
    command->SetCallback(OnMapAlgorithmEvent);
    algorithm->AddObserver(::map::events::AlgorithmEvent(), command);

    auto metaPropInterface = dynamic_cast<map::algorithm::facet::MetaPropertyAlgorithmInterface*>(algorithm.GetPointer());

    if (!settings.parameters.empty())
    {
      if (nullptr == metaPropInterface)
      {
        MITK_WARN << "loaded algorithm does not support custom parameterization. Passed user parameters are ignored.";
      }
      else
      {
        nlohmann::json paramMap;

        std::string parseError = "";
        try
        {
          paramMap = nlohmann::json::parse(settings.parameters);
        }
        catch (const std::exception& e)
        {
          parseError = e.what();
        }
        if (!parseError.empty())
        {
          mitkThrow() << "Cannot parametrize algorithm. Passed JSON parameter string seems to be invalid. Passed string: \"" << settings.parameters << "\". Error details: " << parseError;
        }

        std::cout << "Configuring algorithm with user specified parameters ..." << std::endl;

        for (const auto& [key, val] : paramMap.items())
        {
          const auto info = metaPropInterface->getPropertyInfo(key);

          if (info.IsNotNull())
          {
            if (info->isWritable())
            {
              std::cout << "Set meta property: " << key << " = " << val << std::endl;
              ::map::core::MetaPropertyBase::Pointer prop = WrapIntoMetaProperty(info, val);
              if (prop.IsNull())
              {
                mitkThrow() << "Error. Cannot set specified meta property. Type conversion is not supported or value cannot be converted into type. Property name: " << info->getName() << "; property type: " << info->getTypeName();
              }
              else
              {
                metaPropInterface->setProperty(key, prop);
              }
            }
            else
            {
              mitkThrow() << "Cannot parametrize algorithm. A passed parameter is not writable for the algorithm. Violating parameter: \"" << key << "\".";
            }
          }
          else
          {
            auto knownProps = metaPropInterface->getPropertyInfos();
            std::ostringstream knownPropsNameString;
            for (const auto& knownProp : knownProps)
            {
              knownPropsNameString << knownProp->getName() << "; ";
            }
            mitkThrow() << "Cannot parametrize algorithm. A parameter is unkown to algorithm. Unkown passed parameter: \"" << key << "\". Known parameters: " << knownPropsNameString.str();
          }
        }
      }
    }

    std::cout << "Load moving data..." << std::endl;
    auto movingImage = mitk::IOUtil::Load<mitk::Image>(settings.movingFileName);

    if (movingImage.IsNull())
    {
      MITK_ERROR << "Cannot load moving image.";
      return EXIT_FAILURE;
    }

    if (movingImage->GetTimeSteps() > 1)
    {
      movingImage = mitk::SelectImageByTimeStep(movingImage, 0)->Clone(); //we have to clone because SelectImageByTimeStep 
                                                                          //only generates as new view of the data and we
                                                                          //are overwriting the only smartpointer to the source.
      std::cout << "Moving image has multiple time steps. Use first time step for registartion." << std::endl;
    }

    std::cout << "Load target data..." << std::endl;
    auto targetImage = mitk::IOUtil::Load<mitk::Image>(settings.targetFileName);

    if (targetImage.IsNull())
    {
      MITK_ERROR << "Cannot load target image.";
      return EXIT_FAILURE;
    }

    if (targetImage->GetTimeSteps() > 1)
    {
      targetImage = mitk::SelectImageByTimeStep(targetImage, 0)->Clone(); //we have to clone because SelectImageByTimeStep 
                                                                          //only generates as new view of the data and we
                                                                          //are overwriting the only smartpointer to the source.
      std::cout << "Target image has multiple time steps. Use first time step for registartion." << std::endl;
    }

    std::cout << "Start registration...." << std::endl;
    mitk::MAPAlgorithmHelper helper(algorithm);
    helper.SetData(movingImage, targetImage);

    ::itk::StdStreamLogOutput::Pointer spStreamLogOutput = ::itk::StdStreamLogOutput::New();
    spStreamLogOutput->SetStream(std::cout);
    map::core::Logbook::addAdditionalLogOutput(spStreamLogOutput);

    auto registration = helper.GetRegistration();

    // wrap the registration in a data node
    if (registration.IsNull())
    {
      MITK_ERROR << "No valid registration generated";
      return EXIT_FAILURE;
    }

    auto regWrapper = mitk::MAPRegistrationWrapper::New(registration);
    std::cout << "Store registration...." << std::endl;
    mitk::IOUtil::Save(regWrapper, settings.outFileName);
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

  return EXIT_SUCCESS;
}
