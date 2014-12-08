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

#include "MiniAppManager.h"

#include "ctkCommandLineParser.h"
#include "mitkImage.h"
#include "mitkImageStatisticsCalculator.h"
#include "mitkIOUtil.h"
#include <iostream>
#include <usAny.h>
#include <fstream>

int ExtractImageStatistics(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Extract Image Statistics");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("help", "h", mitkCommandLineParser::String, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(),false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask:", "mask image / roi image denotin area on which statistics are calculated", us::Any(),false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output", "output file (default: filenameOfRoi.nrrd_statistics.txt)", us::Any());

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << "\n\n MiniApp Description: \nCalculates statistics on the supplied image using given mask." << endl;
    std::cout << "Output is written to the designated output file in this order:" << endl;
    std::cout << "Mean, Standard Deviation, RMS, Max, Min, Number of Voxels, Volume [mm3]" << endl;
    std::cout << "\n\n Parameters:"<< endl;
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  // Parameters:
  bool ignoreZeroValues = false;
  unsigned int timeStep = 0;

  std::string inputImageFile = us::any_cast<string>(parsedArgs["input"]);
  std::string maskImageFile = us::any_cast<string>(parsedArgs["mask"]);

  std::string outFile;
  if (parsedArgs.count("out") || parsedArgs.count("o") )
    outFile = us::any_cast<string>(parsedArgs["out"]);
  else
    outFile = inputImageFile + "_statistics.txt";

  // Load image and mask
  mitk::Image::Pointer maskImage = mitk::IOUtil::LoadImage(maskImageFile);
  mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputImageFile);

  // Calculate statistics
  mitk::ImageStatisticsCalculator::Statistics statisticsStruct;
  mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();
  try
  {
    calculator->SetImage(inputImage);
    calculator->SetImageMask(maskImage);
    calculator->SetMaskingModeToImage();
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Statistic Calculation Failed - ITK Exception:" << e.what();
    return -1;
  }


  calculator->SetDoIgnorePixelValue(ignoreZeroValues);
  calculator->SetIgnorePixelValue(0);
  try
  {
    calculator->ComputeStatistics(timeStep);
  }
  catch ( mitk::Exception& e)
  {
    MITK_ERROR<< "MITK Exception: " << e.what();
    return -1;
  }


  statisticsStruct = calculator->GetStatistics(timeStep);


  // Calculate Volume
  double volume = 0;
  const mitk::BaseGeometry *geometry = inputImage->GetGeometry();
  if ( geometry != NULL )
  {
    const mitk::Vector3D &spacing = inputImage->GetGeometry()->GetSpacing();
    volume = spacing[0] * spacing[1] * spacing[2] * (double) statisticsStruct.GetN();
  }

  // Write Results to file
  std::ofstream output;
  output.open(outFile.c_str());
  output << statisticsStruct.GetMean() << " , ";
  output << statisticsStruct.GetSigma() << " , ";
  output << statisticsStruct.GetRMS() << " , ";
  output << statisticsStruct.GetMax() << " , ";
  output << statisticsStruct.GetMin() << " , ";
  output << statisticsStruct.GetN() << " , ";
  output << volume << "\n";

  output.flush();
  output.close();
  return 0;
}

RegisterDiffusionMiniApp(ExtractImageStatistics);
