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

struct InputParameters
{
  mitk::Image::Pointer inputImage;
  std::string outputFilename;
  bool verbose;
  float speedOfSound;
  unsigned int cutoff;
  float angle;
  unsigned int samples;
  mitk::BeamformingSettings::BeamformingAlgorithm algorithm;
};

InputParameters parseInput(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Photoacoustics Beamforming Tool");
  parser.setDescription("Reads a nrrd file as an input and applies a beamforming method as set with the parameters.");
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
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool,
    "Verbose Output", "Whether to produce verbose, or rather debug output. (default: false)");
  parser.addArgument(
    "speed-of-sound", "sos", mitkCommandLineParser::Float,
    "Speed of Sound [m/s]", "The average speed of sound as assumed for the reconstruction in [m/s]. (default: 1500)");
  parser.addArgument(
    "cutoff", "co", mitkCommandLineParser::Int,
    "cutoff margin on the top of the image [pixels]", "The number of pixels to be ignored for this filter in [pixels] (default: 0).");
  parser.addArgument(
    "angle", "a", mitkCommandLineParser::Float,
    "field of view of the transducer elements [degrees]", "The field of view of each individual transducer element [degrees] (default: 27).");
  parser.addArgument(
    "samples", "s", mitkCommandLineParser::Int,
    "samples per reconstruction line [pixels]", "The pixels along the y axis in the beamformed image [pixels] (default: 2048).");
  parser.addArgument(
    "algorithm", "alg", mitkCommandLineParser::String,
    "one of [\"DAS\", \"DMAS\", \"sDMAS\"]", "The beamforming algorithm to be used for reconstruction (default: DAS).");
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    exit(-1);

  if (parsedArgs.count("verbose"))
  {
    input.verbose = true;
  }
  else
  {
    input.verbose = false;
  }
  MITK_INFO(input.verbose) << "### VERBOSE OUTPUT ENABLED ###";

  if (parsedArgs.count("inputImage"))
  {
    MITK_INFO(input.verbose) << "Reading input image...";
    input.inputImage = mitk::IOUtil::Load<mitk::Image>(us::any_cast<std::string>(parsedArgs["inputImage"]));
    MITK_INFO(input.verbose) << "Reading input image...[Done]";
  }
  else
  {
    mitkThrow() << "No input image given.";
  }

  if (parsedArgs.count("output"))
  {
    input.outputFilename = us::any_cast<std::string>(parsedArgs["output"]);
  }
  else
  {
    mitkThrow() << "No output image path given..";
  }

  if (parsedArgs.count("speed-of-sound"))
  {
    input.speedOfSound = us::any_cast<int>(parsedArgs["speed-of-sound"]);
  }
  else
  {
    input.speedOfSound = 1500;
  }

  if (parsedArgs.count("cutoff"))
  {
    input.cutoff = us::any_cast<int>(parsedArgs["cutoff"]);
  }
  else
  {
    input.cutoff = 0;
  }

  if (parsedArgs.count("angle"))
  {
    input.angle = us::any_cast<float>(parsedArgs["angle"]);
  }
  else
  {
    input.angle = 27;
  }

  if (parsedArgs.count("samples"))
  {
    input.samples = us::any_cast<int>(parsedArgs["samples"]);
  }
  else
  {
    input.samples = 2048;
  }

  if (parsedArgs.count("algorithm"))
  {
    std::string algorithm = us::any_cast<std::string>(parsedArgs["algorithm"]);
    MITK_INFO(input.verbose) << "Parsing algorithm: " << algorithm;
    if (algorithm == "DAS")
      input.algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
    else if (algorithm == "DMAS")
      input.algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DMAS;
    else if (algorithm == "sDMAS")
      input.algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::sDMAS;
    else
    {
      MITK_INFO(input.verbose) << "Not a valid beamforming algorithm: " << algorithm << " Reverting to DAS";
      input.algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
    }

    MITK_INFO(input.verbose) << "Sucessfully set algorithm: " << algorithm;
  }
  else
  {
    input.algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
    MITK_INFO(input.verbose) << "No matching algorithm found. Using DAS.";
  }

  return input;
}

mitk::BeamformingSettings::Pointer ParseSettings(InputParameters &input)
{
  mitk::BeamformingSettings::Pointer outputSettings = mitk::BeamformingSettings::New(
    (float)(input.inputImage->GetGeometry()->GetSpacing()[0] / 1000),
    (float)(input.speedOfSound),
    (float)(input.inputImage->GetGeometry()->GetSpacing()[1] / 1000000),
    input.angle,
    true,
    input.inputImage->GetDimension(1),
    input.inputImage->GetDimension(0),
    input.inputImage->GetDimensions(),
    0.04,
    false,
    16,
    mitk::BeamformingSettings::DelayCalc::Spherical,
    mitk::BeamformingSettings::Apodization::Box,
    input.inputImage->GetDimension(0),
    input.algorithm
    );

  return outputSettings;
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);

  MITK_INFO(input.verbose) << "Beamforming input image...";

  mitk::PhotoacousticFilterService::Pointer m_BeamformingService = mitk::PhotoacousticFilterService::New();

  mitk::BeamformingSettings::Pointer settings = ParseSettings(input);

  mitk::CastToFloatImageFilter::Pointer castFilter = mitk::CastToFloatImageFilter::New();
  castFilter->SetInput(input.inputImage);
  castFilter->Update();
  auto floatImage = castFilter->GetOutput();

  auto output = m_BeamformingService->ApplyBeamforming(floatImage, settings);
  MITK_INFO(input.verbose) << "Applying BModeFilter to image...";
  auto output2 = m_BeamformingService->ApplyBmodeFilter(output, mitk::PhotoacousticFilterService::EnvelopeDetection, false);
  MITK_INFO(input.verbose) << "Applying BModeFilter to image...[Done]";

  MITK_INFO(input.verbose) << "Saving image...";
  mitk::IOUtil::Save(output2, input.outputFilename);
  MITK_INFO(input.verbose) << "Saving image...[Done]";

  MITK_INFO(input.verbose) << "Beamforming input image...[Done]";
}
