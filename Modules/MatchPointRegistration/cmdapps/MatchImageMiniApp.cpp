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
#include <mapConvert.h>
#include <mapDeploymentDLLAccess.h>
#include <mapDeploymentDLLHandle.h>
#include <mapRegistrationBase.h>

struct Settings
{
  std::string movingFileName = "";
  std::string targetFileName = "";
  std::string outFileName = "";
  std::string algFileName = "";
};

void SetupParser(mitkCommandLineParser& parser)
{
  parser.setTitle("Image Matcher");
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
  std::cout << "MitkRegistrationMiniApp - Generic light weight image registration tool based on MatchPoint." << std::endl;

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

  //load algorithm
  try
  {
    auto algorithm = loadAlgorithm(settings);

    auto command = ::itk::CStyleCommand::New();
    command->SetCallback(OnMapAlgorithmEvent);
    algorithm->AddObserver(::map::events::AlgorithmEvent(), command);

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
