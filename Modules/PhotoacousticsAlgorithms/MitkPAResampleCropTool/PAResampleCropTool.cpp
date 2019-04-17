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

#include <mitkCommon.h>
#include <chrono>
#include <mitkIOUtil.h>
#include <mitkCommandLineParser.h>
#include <mitkException.h>

#include <mitkPhotoacousticFilterService.h>
#include <mitkBeamformingSettings.h>
#include <mitkCastToFloatImageFilter.h>

#include <itksys/SystemTools.hxx>
#include <tinyxml\tinyxml.h>

struct InputParameters
{
  mitk::Image::Pointer inputImage;
  std::string outputFilename;
  bool verbose;
  std::string settingsFile;
};

struct CropSettings
{
  unsigned int dimX;
  unsigned int dimY;
  unsigned int cutAbove;
};

struct ResamplingSettings
{
  double spacing[3];
};


InputParameters parseInput(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Photoacoustics Resample and Crop Tool");
  parser.setDescription("Reads a nrrd file as an input and crops and resamples it as set with the parameters defined in an additionally provided xml file.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required parameters");
  parser.addArgument(
    "inputImage", "i", mitkCommandLineParser::Image,
    "Input image (mitk::Image)", "input image (.nrrd file)",
    us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "output", "o", mitkCommandLineParser::File,
    "Output filename", "output image (.nrrd file)",
    us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument(
    "settings", "s", mitkCommandLineParser::String,
    "settings file", "file containing specifications (.xml file)",
    us::Any(), false);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool,
    "Verbose Output", "Whether to produce verbose, or rather debug output. (default: false)");
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    exit(-1);

  input.verbose = (bool)parsedArgs.count("verbose");
  MITK_INFO(input.verbose) << "### VERBOSE OUTPUT ENABLED ###";

  if (parsedArgs.count("inputImage"))
  {
    MITK_INFO(input.verbose) << "Reading input image...";
    input.inputImage = mitk::IOUtil::Load<mitk::Image>(us::any_cast<std::string>(parsedArgs["inputImage"]));
    MITK_INFO(input.verbose) << "Reading input image...[Done]";
  }
  else
    mitkThrow() << "No input image given.";

  if (parsedArgs.count("output"))
    input.outputFilename = us::any_cast<std::string>(parsedArgs["output"]);
  else
    mitkThrow() << "No output image path given..";

  if (parsedArgs.count("settings"))
    input.settingsFile = us::any_cast<std::string>(parsedArgs["settings"]);
  else
    mitkThrow() << "No settings image path given..";

  return input;
}

void ParseXML(std::string xmlFile, InputParameters input, CropSettings& cropSet, ResamplingSettings& resSet)
{
  MITK_INFO << "Loading configuration File \"" << xmlFile << "\"";
  TiXmlDocument doc(xmlFile);
  if (!doc.LoadFile())
    mitkThrow() << "Failed to load settings file \"" << xmlFile << "\" Error: " << doc.ErrorDesc();

  TiXmlElement* root = doc.FirstChildElement();
  if (root == NULL)
  {
    mitkThrow() << "Failed to load file: No root element.";
    doc.Clear();
  }
  for (TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
  {
    std::string elemName = elem->Value();
    if (elemName == "ResampleCrop") 
    {
      cropSet.dimX = std::stoi(elem->Attribute("dimX"));
      cropSet.dimY = std::stoi(elem->Attribute("dimY"));
      cropSet.cutAbove = std::stoi(elem->Attribute("cutAbove"));
      resSet.spacing[0] = std::stod(elem->Attribute("spacingX"));
      resSet.spacing[1] = std::stod(elem->Attribute("spacingY"));
      resSet.spacing[2] = std::stod(elem->Attribute("spacingZ"));
    }
  }
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);

  CropSettings cropSettings{ 0,0,0 };
  ResamplingSettings resSettings{ 0 };

  MITK_INFO << "Parsing settings XML...";
  try
  {
    ParseXML(input.settingsFile, input, cropSettings, resSettings);
  }
  catch (mitk::Exception e)
  {
    MITK_INFO << e;
    return -1;
  }

  MITK_INFO << "Parsing settings XML...[Done]";

  MITK_INFO(input.verbose) << "Processing input image...";

  mitk::PhotoacousticFilterService::Pointer m_FilterService = mitk::PhotoacousticFilterService::New();

  mitk::Image::Pointer output = input.inputImage;
  MITK_INFO(input.verbose) << "Resampling input image...";
  MITK_INFO << resSettings.spacing[0];
  output = m_FilterService->ApplyResampling(output, resSettings.spacing);
  MITK_INFO(input.verbose) << "Resampling input image...[Done]";

  if (output->GetDimension(0) != cropSettings.dimX)
  {
    double outputDim[] = {(double)cropSettings.dimX, (double)output->GetDimension(1), (double)output->GetDimension(2)};
    output = m_FilterService->ApplyResamplingToDim(output, outputDim);
  }

  int err = 0;
  int below = output->GetDimension(1) - cropSettings.dimY - cropSettings.cutAbove;
  if (below < 0)
  {
    MITK_INFO(input.verbose) << "Extending input image...";
    output = m_FilterService->ExtendImage(output, 0, cropSettings.dimY);
    MITK_INFO(input.verbose) << "Extending input image...[Done]";
  }
  else
  {
    MITK_INFO(input.verbose) << "Cropping input image...";
    output = m_FilterService->ApplyCropping(output, cropSettings.cutAbove, below, 0, 0, 0, 0, &err);
    MITK_INFO(input.verbose) << "Cropping input image...[Done]";
  }

  
  MITK_INFO(input.verbose) << "Saving image...";
  mitk::IOUtil::Save(output, input.outputFilename);
  MITK_INFO(input.verbose) << "Saving image...[Done]";

  MITK_INFO(input.verbose) << "Processing input image...[Done]";
}
