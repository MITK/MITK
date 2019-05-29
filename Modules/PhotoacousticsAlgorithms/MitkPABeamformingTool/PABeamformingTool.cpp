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
  std::string imageType;
};

struct BandpassSettings
{
  float highPass;
  float lowPass;
  float alphaLow;
  float alphaHigh;
  float speedOfSound;
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

struct ResampleSettings
{
  double spacing;
  int dimX;
};

struct BModeSettings
{
  mitk::PhotoacousticFilterService::BModeMethod method;
  bool UseLogFilter;
};

struct ProcessSettings
{
  bool DoBeamforming;
  bool DoBandpass;
  bool DoCropping;
  bool DoResampling;
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
    "inputImage", "i", mitkCommandLineParser::Image,
    "Input image (mitk::Image)", "input image (.nrrd file)",
    us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "output", "o", mitkCommandLineParser::File,
    "Output filename", "output image (.nrrd file)",
    us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument(
    "settings", "s", mitkCommandLineParser::String,
    "settings file", "file containing beamforming and other specifications(.xml file)",
    us::Any(), false);
  parser.addArgument(
    "type", "t", mitkCommandLineParser::String,
    "image type", "Specifies whether to use the PA or US subsection of the xml file must be 'PA' or 'US'",
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

  if (parsedArgs.count("type"))
    input.imageType = us::any_cast<std::string>(parsedArgs["type"]);
  else
    mitkThrow() << "No settings image type given..";

  return input;
}

TiXmlElement* getRootChild(TiXmlElement* root, std::string childName)
{
  for (TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
  {
    if (elem->Value() == childName)
      return elem;
  }
  return nullptr;
}

void ParseXML(std::string xmlFile, InputParameters input, mitk::BeamformingSettings::Pointer *bfSet, BandpassSettings& bandpassSet, CropSettings& cropSet, ResampleSettings& resSet, BModeSettings& bmodeSet, ProcessSettings& processSet)
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

  TiXmlElement* elemtop = getRootChild(root, input.imageType);
  if(elemtop == nullptr)
      mitkThrow() << "The xml file is wrongly formatted, no element \"" << input.imageType << "\" found";

  for (TiXmlElement* elem = elemtop->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
  {
    std::string elemName = elem->Value();
    if (elemName == "Beamforming")
    {
      processSet.DoBeamforming = std::stoi(elem->Attribute("do"));
      if (!processSet.DoBeamforming)
        continue;

      float PitchInMeters = std::stof(elem->Attribute("pitchInMeters"));
      float SpeedOfSound = std::stof(elem->Attribute("speedOfSound"));
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
    }
    if (elemName == "Bandpass")
    {
      processSet.DoBandpass = std::stoi(elem->Attribute("do"));
      if (!processSet.DoBandpass)
        continue;

      bandpassSet.highPass = std::stof(elem->Attribute("highPass"));
      bandpassSet.lowPass = std::stof(elem->Attribute("lowPass"));
      bandpassSet.alphaLow = std::stof(elem->Attribute("alphaLow"));
      bandpassSet.alphaHigh = std::stof(elem->Attribute("alphaHigh"));
      bandpassSet.speedOfSound = std::stof(elem->Attribute("speedOfSound"));
    }
    if (elemName == "Cropping")
    {
      processSet.DoCropping = std::stoi(elem->Attribute("do"));
      if (!processSet.DoCropping)
        continue;

      cropSet.above = std::stoi(elem->Attribute("cutAbove"));
      cropSet.below = std::stoi(elem->Attribute("cutBelow"));
      cropSet.right = std::stoi(elem->Attribute("cutRight"));
      cropSet.left = std::stoi(elem->Attribute("cutLeft"));
      cropSet.zStart = std::stoi(elem->Attribute("firstSlice"));
      cropSet.zEnd = std::stoi(elem->Attribute("cutSlices"));
    }
    if (elemName == "Resampling")
    {
      processSet.DoResampling = std::stoi(elem->Attribute("do"));
      if (!processSet.DoResampling)
        continue;

      resSet.spacing = std::stod(elem->Attribute("spacing"));
      resSet.dimX = std::stoi(elem->Attribute("dimX"));
    }
    if (elemName == "BMode")
    {
      processSet.DoBmode = std::stoi(elem->Attribute("do"));
      if (!processSet.DoBmode)
        continue;

      std::string methodStr = elem->Attribute("method");
      if (methodStr == "EnvelopeDetection")
        bmodeSet.method = mitk::PhotoacousticFilterService::BModeMethod::EnvelopeDetection;
      else if (elem->Attribute("method") == "Abs")
        bmodeSet.method = mitk::PhotoacousticFilterService::BModeMethod::Abs;
      else
        mitkThrow() << "BMode method incorrectly set in configuration file";
      bmodeSet.UseLogFilter = (bool)std::stoi(elem->Attribute("useLogFilter"));
    }
  }
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);

  mitk::BeamformingSettings::Pointer bfSettings;
  BandpassSettings bandpassSettings{5,10,1,1,1540};
  BModeSettings bmodeSettings{ mitk::PhotoacousticFilterService::BModeMethod::EnvelopeDetection, false };
  CropSettings cropSettings{ 0,0,0,0,0,0 };
  ResampleSettings resSettings{ 0.15 };
  ProcessSettings processSettings{ true, false, false };

  MITK_INFO << "Parsing settings XML...";
  try
  {
    ParseXML(input.settingsFile, input, &bfSettings, bandpassSettings, cropSettings, resSettings, bmodeSettings, processSettings);
  }
  catch (mitk::Exception e)
  {
    MITK_INFO << e;
    return -1;
  }

  MITK_INFO << "Parsing settings XML...[Done]";

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
    MITK_INFO << inputImage->GetPixelType().GetPixelTypeAsString();
    MITK_INFO(input.verbose) << "Casting input image to float...[Done]";
  }

  mitk::PhotoacousticFilterService::Pointer m_FilterService = mitk::PhotoacousticFilterService::New();

  mitk::Image::Pointer output = inputImage;
  if (processSettings.DoBeamforming)
  {
    MITK_INFO(input.verbose) << "Beamforming input image...";
    output = m_FilterService->ApplyBeamforming(output, bfSettings);
    MITK_INFO(input.verbose) << "Beamforming input image...[Done]";
  }
  if (processSettings.DoBandpass)
  {
    MITK_INFO(input.verbose) << "Bandpassing input image...";
    output = m_FilterService->ApplyBandpassFilter(output, bandpassSettings.highPass*1e6, bandpassSettings.lowPass*1e6, bandpassSettings.alphaHigh, bandpassSettings.alphaLow, 1, bandpassSettings.speedOfSound, true);
    MITK_INFO(input.verbose) << "Bandpassing input image...[Done]";
  }
  if (processSettings.DoCropping)
  {
    int err;
    MITK_INFO(input.verbose) << "Applying Crop filter to image...";
    output = m_FilterService->ApplyCropping(output, 
      cropSettings.above, cropSettings.below, cropSettings.right, cropSettings.left, cropSettings.zStart, cropSettings.zEnd, &err);
    MITK_INFO(input.verbose) << "Applying Crop filter to image...[Done]";
  }
  if (processSettings.DoResampling)
  {
    double spacing[3] = {output->GetGeometry()->GetSpacing()[0]*output->GetDimension(0)/resSettings.dimX, resSettings.spacing, output->GetGeometry()->GetSpacing()[2]};
    MITK_INFO(input.verbose) << "Applying Resample filter to image...";
    output = m_FilterService->ApplyResampling(output, spacing);
    if (output->GetDimension(0) != resSettings.dimX)
    {
      double dim[3] = {(double)resSettings.dimX, (double)output->GetDimension(1), (double)output->GetDimension(2)};
      output = m_FilterService->ApplyResamplingToDim(output, dim);
    }
    MITK_INFO(input.verbose) << "Applying Resample filter to image...[Done]";
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
