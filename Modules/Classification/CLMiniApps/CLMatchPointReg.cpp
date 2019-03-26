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

#include "mitkProperties.h"

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"

#include "mitkPreferenceListReaderOptionsFunctor.h"


// MatchPoint
#include <mapRegistrationAlgorithmInterface.h>
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapDeploymentDLLDirectoryBrowser.h>
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapPointSetRegistrationAlgorithmInterface.h>
#include <mapMaskedRegistrationAlgorithmInterface.h>
#include <mapConvert.h>
#include <mapDeploymentDLLAccess.h>
#include <mapRegistrationBase.h>

#include <mitkAlgorithmHelper.h>
// Qt
#include <QDir>
#include <QFileInfo>
#include <mitkStandaloneDataStorage.h>
//#include <QApplication>
#include <QStringList>
#include <QCoreApplication>
#include <QmitkRegistrationJob.h>
#include <mitkImageMappingHelper.h>


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Dicom Loader");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("moving", "m", mitkCommandLineParser::Directory, "Input folder:", "Input folder", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("fixed", "f", mitkCommandLineParser::Directory, "Input folder:", "Input folder", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("reader", "r", mitkCommandLineParser::Int, "Reader ID", "Reader Name", us::Any(), false);
  parser.addArgument("interpolation", "interp", mitkCommandLineParser::Int, "Reader ID", "Reader Name", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  QFileInfo fi(argv[0]);
  map::deployment::DLLDirectoryBrowser::Pointer browser = map::deployment::DLLDirectoryBrowser::New();
  browser->addDLLSearchLocation(QDir::homePath().toStdString());
  browser->addDLLSearchLocation(QDir::currentPath().toStdString());
  browser->addDLLSearchLocation(fi.canonicalPath().toStdString());
  browser->update();
  auto dllList = browser->getLibraryInfos();

  int id = 0;
  std::cout << std::endl << " --- Algorithm List --- " << std::endl;
  for (auto info : dllList)
  {
    std::cout << "Algorithm ID " << id << ": " << info->getAlgorithmUID().getName() << std::endl;
    ++id;
  }
  std::cout << std::endl << " --- Interpolation List --- " << std::endl;
  std::cout << "Interpolation ID 0: Linear Interpolation " << std::endl;
  std::cout << "Interpolation ID 1: Nearest Neighbour" << std::endl;
  std::cout << "Interpolation ID 2: BSpline 3D" << std::endl << std::endl;

  mitk::ImageMappingInterpolator::Type interpolationMode = mitk::ImageMappingInterpolator::Linear;

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }


  std::string movingFile = us::any_cast<std::string>(parsedArgs["moving"]);
  std::string fixedFile = us::any_cast<std::string>(parsedArgs["fixed"]);
  int selectedAlgorithm = us::any_cast<int>(parsedArgs["reader"]);
  std::string outputPath = us::any_cast<std::string>(parsedArgs["output"]);

  if (parsedArgs.count("interpolation"))
  {
    switch (us::any_cast<int>(parsedArgs["interpolation"]))
    {
    case 0:
      interpolationMode = mitk::ImageMappingInterpolator::Linear;
      break;
    case 1:
      interpolationMode = mitk::ImageMappingInterpolator::NearestNeighbor;
      break;
    case 2:
      interpolationMode = mitk::ImageMappingInterpolator::BSpline_3;
      break;
    default:
      interpolationMode = mitk::ImageMappingInterpolator::Linear;
    }
  }


  mitk::Image::Pointer movingImage = mitk::IOUtil::Load<mitk::Image>(movingFile);
  mitk::Image::Pointer fixedImage = mitk::IOUtil::Load<mitk::Image>(fixedFile);

  auto dllInfo = dllList[selectedAlgorithm];

  if (!dllInfo)
  {
    MITK_ERROR << "No valid algorithm is selected. Cannot load algorithm. ABORTING.";
    return -1;
  }

  ::map::deployment::DLLHandle::Pointer tempDLLHandle = ::map::deployment::openDeploymentDLL(
    dllInfo->getLibraryFilePath());
  ::map::algorithm::RegistrationAlgorithmBase::Pointer tempAlgorithm
    = ::map::deployment::getRegistrationAlgorithm(tempDLLHandle);
  MITK_INFO << "Well....";
  if (tempAlgorithm.IsNull())
  {
    MITK_ERROR << "Error. Cannot load selected algorithm.";
    return -2;
  }

  mitk::MITKAlgorithmHelper helper(tempAlgorithm);
  helper.SetData(movingImage, fixedImage);
  auto registration = helper.GetRegistration();
  MITK_INFO << "Well....";

  mitk::Image::Pointer spResultData= mitk::ImageMappingHelper::map(movingImage,
                                                    registration,
                                                    false, // Use all Pixels
                                                    0.0,  // Padding Value
                                                    fixedImage->GetGeometry()->Clone().GetPointer(), // Ref. Geometry
                                                    false, //!(this->m_allowUnregPixels),
                                                    0, // Error Value
                                                    interpolationMode // Interpolator Type
                            );

  MITK_INFO << "Well....";
  mitk::IOUtil::Save(spResultData, outputPath);

  return EXIT_SUCCESS;
}
