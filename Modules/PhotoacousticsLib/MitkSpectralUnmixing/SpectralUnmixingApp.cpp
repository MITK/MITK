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


/* \brief The spectral unmixing mini app (SUMA) is designed to enable batch processing
  for spectral unmixing. For detailed documentation look into the header files of the 
  included spectral unmixing filters.*/

struct InputParameters
{
  std::string inputFilename;
  std::string outputFileStruct; // "E:/mydata/awesome_exp_unmixed/a" will be saved as "a_HbO2_SU_.nrrd", "a_Hb_SU_.nrrd" and "a_sO2_.nrrd";
  std::string inputAlg;
  std::string outputFileNumber;
  mitkCommandLineParser::StringContainerType inputWavelengths;
  mitkCommandLineParser::StringContainerType inputWeights;
};

InputParameters parseInput(int argc, char *argv[])
{
  //MITK_INFO << "Parsing arguments...";
  mitkCommandLineParser parser;

  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Spectral Unmixing App");
  parser.setDescription("Batch processing for spectral unmixing.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required parameters");
  parser.addArgument("inputFilename",
                     "i",
                     mitkCommandLineParser::Directory,
                     "Input Filename (NAME.nrrd)",
                     "input filename",
                     us::Any(),
                     false);
  parser.addArgument("outputFileStruct",
                     "o",
                     mitkCommandLineParser::Directory,
                     "Output save name (name without ending!)",
                     "Output save name",
                     us::Any(),
                     false);
  parser.addArgument("outputFileNumber",
                     "n",
                     mitkCommandLineParser::String,
                     "Output file number",
                     "Output save number",
                     us::Any(),
                     false);
  parser.addArgument("inputWavelengths",
                     "l",
                     mitkCommandLineParser::StringList,
                     "Input wavelengths (123 124 125 ... int blank int blank)",
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
                     mitkCommandLineParser::StringList,
                     "Input weights (123 124 125 ... int in % blank int in % blank)",
                     "input weights",
                     us::Any(),
                     true);
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (argc == 0)
    exit(-1);

  //for (int i = 0; i < argc; ++i)
  //{
  //  MITK_INFO << argv[i];
  //}


  if (parsedArgs.count("inputFilename"))
  { 

    input.inputFilename = us::any_cast<std::string>(parsedArgs["inputFilename"]);
  }
  else
  {
    MITK_ERROR << "Error: No input file";
    mitkThrow() << "Error: No input file";
  }

  if (parsedArgs.count("outputFileStruct"))
  { 
    input.outputFileStruct = us::any_cast<std::string>(parsedArgs["outputFileStruct"]);
  }
  else
  {
    MITK_ERROR << "Error: No output";
    mitkThrow() << "Error: No output";
  }

  if (parsedArgs.count("outputFileNumber"))
  {
    input.outputFileNumber = us::any_cast<std::string>(parsedArgs["outputFileNumber"]);
  }
  else
  {
    MITK_ERROR << "Error: No output number";
    mitkThrow() << "Error: No output number";
  }

  if (parsedArgs.count("inputWavelengths"))
  {
    input.inputWavelengths = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["inputWavelengths"]);
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
    input.inputWeights = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["inputWeights"]);
  }

  //MITK_INFO << "Parsing arguments...[Done]";
  return input;
}

// Class takes string and sets algorithm for spectral unmixing in the corresponding filter class
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

    std::vector<int> weightVec = weights;

    for (unsigned int i = 0; i < weightVec.size(); ++i)
    {
      dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(spectralUnmixingFilter.GetPointer())
        ->AddWeight(weightVec[i]);
    }
  }
  return spectralUnmixingFilter;
}

int main(int argc, char *argv[])
{ 
  auto input = parseInput(argc, argv);

  std::string algo = input.inputAlg;
  std::string outputDir = input.outputFileStruct;
  std::string outputNumber = input.outputFileNumber;

  auto inputWls = input.inputWavelengths;

  std::vector<int> wavelengths;
  for (unsigned int s = 0; s < inputWls.size(); ++s)
  {
    int wl = std::stoi(inputWls[s]);
    wavelengths.push_back(wl);
    //MITK_INFO << "Wavelength: " << wl << "\n";
  }

  mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;

  if (algo == "WLS")
  {
    auto inputW = input.inputWeights;

    std::vector<int> Weights;
    for (unsigned int s = 0; s < inputW.size(); ++s)
    {
      int w = std::stoi(inputW[s]);
      Weights.push_back(w);
      //MITK_INFO << "Weights: " << w << "\n";
    }

    m_SpectralUnmixingFilter = GetFilterInstance(algo, Weights);
  }
  else
  {
    m_SpectralUnmixingFilter = GetFilterInstance(algo);
  }

  m_SpectralUnmixingFilter->Verbose(false);
  m_SpectralUnmixingFilter->RelativeError(false);
  m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
  m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);
  m_SpectralUnmixingFilter->AddOutputs(2);
 
  for (unsigned int wIdx = 0; wIdx < wavelengths.size(); ++wIdx)
  {
    m_SpectralUnmixingFilter->AddWavelength(wavelengths[wIdx]);
    //MITK_INFO << wavelengths[wIdx];
  }

  //to add a batch processing: loop for a dir start here; don't forget to set a counter to the three output savenames!!!
  std::string inputImage = input.inputFilename; 
  auto m_inputImage = mitk::IOUtil::Load<mitk::Image>(inputImage);

  m_SpectralUnmixingFilter->SetInput(m_inputImage);

  m_SpectralUnmixingFilter->Update();
 
  auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
  auto output2 = m_SpectralUnmixingFilter->GetOutput(1);
  output1->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());
  output2->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());

  std::string unmixingOutputHbO2 = outputDir + "HbO2." + outputNumber + ".nrrd";
  std::string unmixingOutputHb = outputDir + "Hb." + outputNumber + ".nrrd";
  //mitk::IOUtil::Save(output1, unmixingOutputHbO2);
  //mitk::IOUtil::Save(output2, unmixingOutputHb);

  auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
  m_sO2->Verbose(false);
   
  m_sO2->SetInput(0, output1);
  m_sO2->SetInput(1, output2);

  m_sO2->Update();

  mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
  mitk::Image::Pointer tHb = m_sO2->GetOutput(1);
  sO2->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());
  tHb->SetSpacing(m_inputImage->GetGeometry()->GetSpacing());

  std::string outputSo2 = outputDir + "sO2." + outputNumber + ".nrrd";
  mitk::IOUtil::Save(sO2, outputSo2);

  std::string outputTHb = outputDir + "tHb." + outputNumber + ".nrrd";
  mitk::IOUtil::Save(tHb, outputTHb);

  m_sO2 = nullptr;
  m_SpectralUnmixingFilter = nullptr;
  //to add a batch processing: loop for a dir end here 
  MITK_INFO << "Spectral Unmixing DONE";
}
