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

//#include <boost>
#include <chrono>
#include <mitkCommon.h>

#include "mitkPALinearSpectralUnmixingFilter.h"
#include "mitkPASpectralUnmixingFilterBase.h"
#include "mitkPASpectralUnmixingFilterVigra.h"
#include "mitkPASpectralUnmixingSO2.h"

#include <mitkCommandLineParser.h>
#include <mitkException.h>
#include <mitkIOUtil.h>
#include <mitkUIDGenerator.h>

#include <itksys/SystemTools.hxx>


#include "mitkPreferenceListReaderOptionsFunctor.h"


struct InputParameters
{
  std::string inputPath;
  std::string outputPath;
  std::string inputWavelengths;
  std::string inputAlg;
  std::string inputWeights;
  mitkCommandLineParser::StringContainerType test;
};


InputParameters parseInput(int argc, char *argv[])
{
  MITK_INFO << "Parsing arguments...";
  mitkCommandLineParser parser;

  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Spectral Unmixing App");
  parser.setDescription("Batch processing for spectral unmixing.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required parameters");
  parser.addArgument("inputPath",
                     "i",
                     mitkCommandLineParser::InputDirectory,
                     "Input folder (directory)",
                     "input folder",
                     us::Any(),
                     false);
  parser.addArgument("outputPath",
                     "o",
                     mitkCommandLineParser::OutputDirectory,
                     "Input save folder (directory)",
                     "input save folder",
                     us::Any(),
                     false);
  parser.addArgument("inputWavelengths",
                     "l",
                     mitkCommandLineParser::String,
                     "Input wavelengths (string)",
                     "input wavelengths",
                     us::Any(),
                     false);
  parser.addArgument("inputAlg",
                     "a",
                     mitkCommandLineParser::String,
                     "Input algorithm (string)",
                     "input algorithm",
                     us::Any(),
                     false);
  parser.addArgument("inputWeights",
                     "w",
                     mitkCommandLineParser::String,
                     "Input weights (string)",
                     "input weights",
                     us::Any(),
                     true);
  parser.addArgument("test",
                     "t",
                     mitkCommandLineParser::StringList,
                     "Input tes (string)",
                     "input tes",
                     us::Any(),
                     true);
  parser.endGroup();


  InputParameters input;


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (argc == 0)
    exit(-1);

  for (int i = 0; i < argc; ++i)
  {
    MITK_INFO << argv[i];
  }

  if (parsedArgs.count("inputPath"))
  { 
    input.inputPath = us::any_cast<std::string>(parsedArgs["inputPath"]);
  }
  else
  {
    MITK_ERROR << "Error: No inputPath";
    mitkThrow() << "Error: No inputPath";
  }

  if (parsedArgs.count("outputPath"))
  { 
    input.outputPath = us::any_cast<std::string>(parsedArgs["outputPath"]);
  }
  else
  {
    MITK_ERROR << "Error: No outputPath";
    mitkThrow() << "Error: No outputPath";
  }

  if (parsedArgs.count("inputWavelengths"))
  {
    input.inputWavelengths = us::any_cast<std::string>(parsedArgs["inputWavelengths"]);
  }
  else
  {
    MITK_ERROR << "Error: No wavelengths";
    mitkThrow() << "Error: No wavelengths";
  }
  if (parsedArgs.count("inputAlg"))
  {
    input.inputAlg = us::any_cast<std::string>(parsedArgs["inputAlg"]);
  }
  else
  {
    MITK_ERROR << "Error: No algorithm";
    mitkThrow() << "Error: No algorithm";
  }

  if (parsedArgs.count("inputWeights"))
  {
    input.inputWeights = us::any_cast<std::string>(parsedArgs["inputWeights"]);
  }

  if (parsedArgs.count("test"))
  {
    input.test = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["test"]);
  }

  MITK_INFO << "Parsing arguments...[Done]";
  return input;
}


mitk::pa::SpectralUnmixingFilterBase::Pointer GetFilterInstance(std::string algorithm, std::vector<int> weights = std::vector<int>())
{
  mitk::pa::SpectralUnmixingFilterBase::Pointer spectralUnmixingFilter;

  if (algorithm == "QR")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter *>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);
  }

  else if (algorithm == "SVD")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter *>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::JACOBISVD);
  }

  else if (algorithm == "LU")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter *>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVLU);
  }

  else if (algorithm == "NNLS")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS);
  }

  else if (algorithm == "WLS")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED);

    std::vector<int> weigthVec = weights;

    for (int i = 0; i < 3; ++i)
    {
      dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(spectralUnmixingFilter.GetPointer())
        ->AddWeight(weigthVec[i]/100);
    }
  }
  return spectralUnmixingFilter;
}

std::vector<int> inputWavelengthGenerator(std::string input)
{
  std::vector<int> output;

  
  for (int i = 0; i < input.size(); ++i)
  {
    if (input[i] == ',')
    {
      std::string foo;
      foo.push_back(input[i-3]);
      foo.push_back(input[i-2]);
      foo.push_back(input[i-1]);
      output.push_back(std::stoi(foo));
    }
  }
  std::string foo;
  foo.push_back(input[input.size() - 3]);
  foo.push_back(input[input.size() - 2]);
  foo.push_back(input[input.size()-1]);
  output.push_back(std::stoi(foo));
  
  return output;
}

int main(int argc, char *argv[])
{ 
  auto input = parseInput(argc, argv);


  auto tst = input.test;

  for (int s = 0; s < tst.size(); ++s)
  {
    MITK_INFO << " s:" << s <<  "trs: " << tst[s] << "\n";
    int sdh = std::stoi(tst[s]);
    MITK_INFO << "int: " << sdh << "\n";
  }


  std::string inputImage = input.inputPath;
  std::string outputDir = input.outputPath;
  std::string inputWls = input.inputWavelengths;
  std::vector<int> wavelengths = inputWavelengthGenerator(inputWls);
  std::string algo = input.inputAlg;

 
   
 auto m_inputImage = mitk::IOUtil::Load<mitk::Image>(inputImage);

 mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;

 if (algo == "WLS")
 {
   std::vector<int> Weights = inputWavelengthGenerator(input.inputWeights);
   m_SpectralUnmixingFilter = GetFilterInstance(algo, Weights);
 }
 else
 {
   m_SpectralUnmixingFilter = GetFilterInstance(algo);
 }

 m_SpectralUnmixingFilter->SetInput(m_inputImage);
      
 m_SpectralUnmixingFilter->Verbose(false);
 m_SpectralUnmixingFilter->RelativeError(false);
 m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
 m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);
 m_SpectralUnmixingFilter->AddOutputs(2);
 
 for (int wIdx = 0; wIdx < wavelengths.size(); ++wIdx)
 {
   m_SpectralUnmixingFilter->AddWavelength(wavelengths[wIdx]);
   MITK_INFO << wavelengths[wIdx];
 }
 
 m_SpectralUnmixingFilter->Update();
 
 auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
 auto output2 = m_SpectralUnmixingFilter->GetOutput(1);
 output1->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());
 output2->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());

 std::string unmixingOutputHbO2 = outputDir + "_HbO2_SU_.nrrd";
 std::string unmixingOutputHb = outputDir + "_Hb_SU_.nrrd";
 mitk::IOUtil::Save(output1, unmixingOutputHbO2);
 mitk::IOUtil::Save(output2, unmixingOutputHb);

 auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
 m_sO2->Verbose(false);
   
 m_sO2->SetInput(0, output1);
 m_sO2->SetInput(1, output2);

 m_sO2->Update();

 mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
 sO2->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());

 std::string outputSo2 = outputDir + "_sO2_.nrrd";
 mitk::IOUtil::Save(sO2, outputSo2);

 m_sO2 = nullptr;
 m_SpectralUnmixingFilter = nullptr;

 MITK_INFO << "Spectral Unmixing DONE";
}
