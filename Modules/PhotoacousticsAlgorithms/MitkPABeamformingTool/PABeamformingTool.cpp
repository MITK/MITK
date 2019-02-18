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
  int above;
  int below;
  int right;
  int left;
  int zStart;
  int zEnd;
};

struct BModeSettings
{
  mitk::PhotoacousticFilterService::BModeMethod method;
  bool UseLogFilter;
};

struct ProcessSettings
{
  bool DoBeamforming;
  bool DoCropping;
  bool DoBmode;
};

InputParameters parseInput(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Photoacoustics Beamforming Tool");
  parser.setDescription("Reads a nrrd file as an input and applies a beamforming method as set with the parameters defined in an additionally provided xml file.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required parameters");
  parser.addArgument(
    "inputImage", "i", mitkCommandLineParser::InputImage,
    "Input image (mitk::Image)", "input image (.nrrd file)",
    us::Any(), false);
  parser.addArgument(
    "output", "o", mitkCommandLineParser::OutputFile,
    "Output filename", "output image (.nrrd file)",
    us::Any(), false);
  parser.addArgument(
    "settings", "s", mitkCommandLineParser::String,
    "settings file", "file containing beamforming and other specifications(.xml file)",
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

  return input;
}

void ParseXML(std::string xmlFile, InputParameters input, mitk::BeamformingSettings::Pointer *bfSet, CropSettings cropSet, BModeSettings bmodeSet, ProcessSettings processSet)
{
  TiXmlDocument doc(xmlFile);
  if (!doc.LoadFile())
    mitkThrow() << "Failed to load settings file " << xmlFile << "succeeded";

  TiXmlElement* root = doc.FirstChildElement();
  if (root == NULL)
  {
    mitkThrow() << "Failed to load file: No root element.";
    doc.Clear();
  }
  for (TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
  {
    std::string elemName = elem->Value();
    if (elemName == "Beamforming")
    {
      float PitchInMeters = std::stof(elem->Attribute("pitchInMeters"));
      float SpeedOfSound = std::stof(elem->Attribute("speedOfSound"));
      float TimeSpacing = std::stof(elem->Attribute("timeSpacing"));
      float Angle = std::stof(elem->Attribute("angle"));
      bool IsPhotoacousticImage = std::stoi(elem->Attribute("isPhotoacousticImage"));
      unsigned int SamplesPerLine = std::stoi(elem->Attribute("samplesPerLine"));
      unsigned int ReconstructionLines = std::stoi(elem->Attribute("reconstructionLines"));
      float ReconstructionDepth = std::stof(elem->Attribute("reconstructionDepth"));
      bool UseGPU = std::stoi(elem->Attribute("useGPU"));
      unsigned int GPUBatchSize = std::stoi(elem->Attribute("GPUBatchSize"));

      std::string apodizationStr = elem->Attribute("apodization");
      mitk::BeamformingSettings::Apodization Apodization = mitk::BeamformingSettings::Apodization::Box;
      if (apodizationStr == "Box")
        Apodization = mitk::BeamformingSettings::Apodization::Box;
      else if (apodizationStr == "Hann")
        Apodization = mitk::BeamformingSettings::Apodization::Hann;
      else if (apodizationStr == "Hamm")
        Apodization = mitk::BeamformingSettings::Apodization::Hamm;
      else
        mitkThrow() << "Apodization incorrectly defined in settings";

      unsigned int ApodizationArraySize = std::stoi(elem->Attribute("apodizationArraySize"));

      std::string algorithmStr = elem->Attribute("algorithm");
      mitk::BeamformingSettings::BeamformingAlgorithm Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
      if (algorithmStr == "DAS")
        Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
      else if (algorithmStr == "DMAS")
        Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DMAS;
      else if (algorithmStr == "sDMAS")
        Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::sDMAS;
      else
      {
        mitkThrow() << "Beamforming algorithm incorrectly defined in settings";
      }

      *bfSet = mitk::BeamformingSettings::New(
        (float)(input.inputImage->GetGeometry()->GetSpacing()[0] / 1000),
        SpeedOfSound,
        (float)(input.inputImage->GetGeometry()->GetSpacing()[1] / 1000000),
        Angle,
        IsPhotoacousticImage,
        SamplesPerLine,
        ReconstructionLines,
        input.inputImage->GetDimensions(),
        ReconstructionDepth,
        UseGPU,
        GPUBatchSize,
        mitk::BeamformingSettings::DelayCalc::Spherical,
        Apodization,
        ApodizationArraySize,
        Algorithm
      );
      processSet.DoBeamforming = std::stoi(elem->Attribute("do"));
    }
    if (elemName == "Cropping") 
    {
      cropSet.above = std::stoi(elem->Attribute("cutAbove"));
      cropSet.below = std::stoi(elem->Attribute("cutBelow"));
      cropSet.right = std::stoi(elem->Attribute("cutRight"));
      cropSet.left = std::stoi(elem->Attribute("cutLeft"));
      cropSet.zStart = std::stoi(elem->Attribute("firstSlice"));
      cropSet.zEnd = std::stoi(elem->Attribute("cutSlices"));
      processSet.DoCropping = std::stoi(elem->Attribute("do"));
    }
    if (elemName == "BMode")
    {
      if (elem->Attribute("method") == "EnvelopeDetection")
        bmodeSet.method = mitk::PhotoacousticFilterService::BModeMethod::EnvelopeDetection;
      else if (elem->Attribute("method") == "Abs")
        bmodeSet.method = mitk::PhotoacousticFilterService::BModeMethod::Abs;
      else
        mitkThrow() << "Beamforming method incorrectly set in configuration file";
      bmodeSet.UseLogFilter = (bool)std::stoi(elem->Attribute("useLogFilter"));
      processSet.DoBmode = std::stoi(elem->Attribute("do"));
    }
  }
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);

  mitk::BeamformingSettings::Pointer bfSettings;
  BModeSettings bmodeSettings{ mitk::PhotoacousticFilterService::BModeMethod::EnvelopeDetection, false };
  CropSettings cropSettings{ 0,0,0,0,0,0 };
  ProcessSettings processSettings{ true, false, false };

  ParseXML(input.settingsFile, input, &bfSettings, cropSettings, bmodeSettings, processSettings);

  MITK_INFO(input.verbose) << "Beamforming input image...";
  mitk::Image::Pointer inputImage = input.inputImage;
  if (!(inputImage->GetPixelType().GetTypeAsString() == "scalar (float)" || inputImage->GetPixelType().GetTypeAsString() == " (float)"))
  {
    // we need a float image, so cast it here
    MITK_INFO(input.verbose) << "Casting input image to float...";
    mitk::CastToFloatImageFilter::Pointer castFilter = mitk::CastToFloatImageFilter::New();
    castFilter->SetInput(inputImage);
    castFilter->Update();
    inputImage = castFilter->GetOutput();
    MITK_INFO(input.verbose) << "Casting input image to float...[Done]";
  }

  mitk::PhotoacousticFilterService::Pointer m_FilterService = mitk::PhotoacousticFilterService::New();

  mitk::Image::Pointer output;
  if (processSettings.DoBeamforming)
  {
    MITK_INFO(input.verbose) << "Beamforming input image...";
    output = m_FilterService->ApplyBeamforming(inputImage, bfSettings);
    MITK_INFO(input.verbose) << "Beamforming input image...[Done]";
  }
  if (processSettings.DoCropping)
  {
    int err;
    MITK_INFO(input.verbose) << "Applying Crop filter to image...";
    output = m_FilterService->ApplyCropping(output, 
      cropSettings.above, cropSettings.below, cropSettings.right, cropSettings.left, cropSettings.zStart, cropSettings.zEnd, &err);
    MITK_INFO(input.verbose) << "Applying Crop filter to image...[Done]";
  }
  if (processSettings.DoBmode)
  {
    MITK_INFO(input.verbose) << "Applying BModeFilter to image...";
    output = m_FilterService->ApplyBmodeFilter(output, bmodeSettings.method, bmodeSettings.UseLogFilter);
    MITK_INFO(input.verbose) << "Applying BModeFilter to image...[Done]";
  }

  MITK_INFO(input.verbose) << "Saving image...";
  mitk::IOUtil::Save(output, input.outputFilename);
  MITK_INFO(input.verbose) << "Saving image...[Done]";

  MITK_INFO(input.verbose) << "Beamforming input image...[Done]";
}
