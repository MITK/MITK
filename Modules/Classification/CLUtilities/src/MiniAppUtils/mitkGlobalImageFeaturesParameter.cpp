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

#include <mitkGlobalImageFeaturesParameter.h>


#include <fstream>
#include <itkFileTools.h>
#include <itksys/SystemTools.hxx>


static bool fileExists(const std::string& filename)
{
  std::ifstream infile(filename.c_str());
  bool isGood = infile.good();
  infile.close();
  return isGood;
}


void mitk::cl::GlobalImageFeaturesParameter::AddParameter(mitkCommandLineParser &parser)
{
  // Required Parameter
  parser.addArgument("image",   "i", mitkCommandLineParser::Image, "Input Image", "Path to the input image file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::Image, "Input Mask", "Path to the mask Image that specifies the area over for the statistic (Values = 1)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("morph-mask", "morph", mitkCommandLineParser::Image, "Morphological Image Mask", "Path to the mask Image that specifies the area over for the statistic (Values = 1)", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output",  "o", mitkCommandLineParser::File, "Output text file", "Path to output file. The output statistic is appended to this file.", us::Any(), false, false, false, mitkCommandLineParser::Output);

  // Optional Parameter
  parser.addArgument("logfile",    "log",         mitkCommandLineParser::File, "Text Logfile", "Path to the location of the target log file. ", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("save-image", "save-image",  mitkCommandLineParser::File, "Output Image", "If spezified, the image that is used for the analysis is saved to this location.", us::Any(), true, false, false, mitkCommandLineParser::Output);
  parser.addArgument("save-mask", "save-mask",    mitkCommandLineParser::File, "Output Image", "If spezified, the mask that is used for the analysis is saved to this location. ", us::Any(), true, false, false, mitkCommandLineParser::Output);
  parser.addArgument("save-image-screenshots", "save-screenshot", mitkCommandLineParser::File, "Output Image", "If spezified,  a screenshot of each slice is saved. Specify an EXISTING folder with prefix (for example ~/demo/ or ~/demo/image-", us::Any(), true, false, false, mitkCommandLineParser::Output);

  parser.addArgument("header",            "head",    mitkCommandLineParser::Bool, "Add Header (Labels) to output", "", us::Any());
  parser.addArgument("first-line-header", "fl-head", mitkCommandLineParser::Bool, "Add Header (Labels) to first line of output", "", us::Any());
  parser.addArgument("decimal-point", "decimal", mitkCommandLineParser::String, "Decima Point that is used in Conversion", "", us::Any());

  parser.addArgument("resample-mask",   "rm", mitkCommandLineParser::Bool,  "Bool",  "Resamples the mask to the resolution of the input image ", us::Any());
  parser.addArgument("same-space",      "sp", mitkCommandLineParser::Bool,  "Bool",  "Set the spacing of all images to equal. Otherwise an error will be thrown. ", us::Any());
  parser.addArgument("fixed-isotropic", "fi", mitkCommandLineParser::Float, "Float", "Input image resampled to fixed isotropic resolution given in mm. Should be used with resample-mask ", us::Any());

  parser.addArgument("minimum-intensity", "minimum", mitkCommandLineParser::Float, "Float", "Minimum intensity. If set, it is overwritten by more specific intensity minima", us::Any());
  parser.addArgument("maximum-intensity", "maximum", mitkCommandLineParser::Float, "Float", "Maximum intensity. If set, it is overwritten by more specific intensity maxima", us::Any());
  parser.addArgument("bins", "bins", mitkCommandLineParser::Int, "Int", "Number of bins if bins are used. If set, it is overwritten by more specific bin count", us::Any());
  parser.addArgument("binsize", "binsize", mitkCommandLineParser::Float, "Int", "Size of bins that is used. If set, it is overwritten by more specific bin count", us::Any());
  parser.addArgument("ignore-mask-for-histogram", "ignore-mask", mitkCommandLineParser::Bool, "Bool", "If the whole image is used to calculate the histogram. ", us::Any());
  parser.addArgument("encode-parameter-in-name", "encode-parameter", mitkCommandLineParser::Bool, "Bool", "If true, the parameters used for each feature is encoded in its name. ", us::Any());
}

void mitk::cl::GlobalImageFeaturesParameter::ParseParameter(std::map<std::string, us::Any> parsedArgs)
{
  ParseFileLocations(parsedArgs);
  ParseAdditionalOutputs(parsedArgs);
  ParseHeaderInformation(parsedArgs);
  ParseMaskAdaptation(parsedArgs);
  ParseGlobalFeatureParameter(parsedArgs);
}

void mitk::cl::GlobalImageFeaturesParameter::ParseFileLocations(std::map<std::string, us::Any> &parsedArgs)
{

  //
  // Read input and output file informations
  //
  imagePath = parsedArgs["image"].ToString();
  maskPath = parsedArgs["mask"].ToString();
  outputPath = parsedArgs["output"].ToString();

  imageFolder = itksys::SystemTools::GetFilenamePath(imagePath);
  imageName = itksys::SystemTools::GetFilenameName(imagePath);
  maskFolder = itksys::SystemTools::GetFilenamePath(maskPath);
  maskName = itksys::SystemTools::GetFilenameName(maskPath);

  useMorphMask = false;
  if (parsedArgs.count("morph-mask"))
  {
    useMorphMask = true;
    morphPath = parsedArgs["morph-mask"].ToString();
    morphName = itksys::SystemTools::GetFilenameName(morphPath);
  }

}

void mitk::cl::GlobalImageFeaturesParameter::ParseAdditionalOutputs(std::map<std::string, us::Any> &parsedArgs)
{

  //
  // Read input and output file informations
  //
  useLogfile = false;
  if (parsedArgs.count("logfile"))
  {
    useLogfile = true;
    logfilePath = us::any_cast<std::string>(parsedArgs["logfile"]);
  }
  writeAnalysisImage = false;
  if (parsedArgs.count("save-image"))
  {
    writeAnalysisImage = true;
    anaylsisImagePath = us::any_cast<std::string>(parsedArgs["save-image"]);
  }
  writeAnalysisMask = false;
  if (parsedArgs.count("save-mask"))
  {
    writeAnalysisMask = true;
    analysisMaskPath = us::any_cast<std::string>(parsedArgs["save-mask"]);
  }
  writePNGScreenshots = false;
  if (parsedArgs.count("save-image-screenshots"))
  {
    writePNGScreenshots = true;
    pngScreenshotsPath = us::any_cast<std::string>(parsedArgs["save-image-screenshots"]);
    std::string pngScrenshotFolderPath = itksys::SystemTools::GetFilenamePath(pngScreenshotsPath);
    if (pngScreenshotsPath.back() == '/' || pngScreenshotsPath.back() == '\\')
    {
      pngScrenshotFolderPath = pngScreenshotsPath;
    }
    itk::FileTools::CreateDirectory(pngScrenshotFolderPath.c_str());
  }
  useDecimalPoint = false;
  if (parsedArgs.count("decimal-point"))
  {
    auto tmpDecimalPoint = us::any_cast<std::string>(parsedArgs["decimal-point"]);
    if (tmpDecimalPoint.length() > 0)
    {
      useDecimalPoint = true;
      decimalPoint = tmpDecimalPoint.at(0);
    }
  }

}

void mitk::cl::GlobalImageFeaturesParameter::ParseHeaderInformation(std::map<std::string, us::Any> &parsedArgs)
{
  //
  // Check if an header is required or not. Consider also first line header option.
  //
  useHeader = false;
  useHeaderForFirstLineOnly = false;
  if (parsedArgs.count("header"))
  {
    useHeader = us::any_cast<bool>(parsedArgs["header"]);
  }
  if (parsedArgs.count("first-line-header"))
  {
    useHeaderForFirstLineOnly = us::any_cast<bool>(parsedArgs["first-line-header"]);
  }
  if (useHeaderForFirstLineOnly)
  {
    useHeader = !fileExists(outputPath);
  }
}

void mitk::cl::GlobalImageFeaturesParameter::ParseMaskAdaptation(std::map<std::string, us::Any> &parsedArgs)
{
  //
  // Parse parameters that control how the input mask is adapted to the input image
  //
  resampleMask = false;
  ensureSameSpace = false;
  resampleToFixIsotropic = false;
  resampleResolution = 1.0;
  if (parsedArgs.count("resample-mask"))
  {
    resampleMask = us::any_cast<bool>(parsedArgs["resample-mask"]);
  }
  if (parsedArgs.count("same-space"))
  {
    ensureSameSpace = us::any_cast<bool>(parsedArgs["same-space"]);
  }
  if (parsedArgs.count("fixed-isotropic"))
  {
    resampleToFixIsotropic = true;
    resampleResolution = us::any_cast<float>(parsedArgs["fixed-isotropic"]);
  }
}

void mitk::cl::GlobalImageFeaturesParameter::ParseGlobalFeatureParameter(std::map<std::string, us::Any> &parsedArgs)
{
  //
  // Parse parameters that control how the input mask is adapted to the input image
  //
  defineGlobalMinimumIntensity = false;
  defineGlobalMaximumIntensity = false;
  defineGlobalNumberOfBins = false;
  encodeParameter = false;
  if (parsedArgs.count("minimum-intensity"))
  {
    defineGlobalMinimumIntensity = true;
    globalMinimumIntensity = us::any_cast<float>(parsedArgs["minimum-intensity"]);
  }
  if (parsedArgs.count("maximum-intensity"))
  {
    defineGlobalMaximumIntensity = true;
    globalMaximumIntensity = us::any_cast<float>(parsedArgs["maximum-intensity"]);
  }
  if (parsedArgs.count("bins"))
  {
    defineGlobalNumberOfBins = true;
    globalNumberOfBins = us::any_cast<int>(parsedArgs["bins"]);
  }
  if (parsedArgs.count("encode-parameter-in-name"))
  {
    encodeParameter = true;
  }
}
