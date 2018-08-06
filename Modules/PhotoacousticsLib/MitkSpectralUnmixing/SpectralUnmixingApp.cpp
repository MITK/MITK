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

struct InputParameters
{
  std::string inputPath;
  std::string savePath;
  unsigned int numberOfInputs;
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
                     "s",
                     mitkCommandLineParser::InputDirectory,
                     "Input folder (directory)",
                     "input folder",
                     us::Any(),
                     false);
  parser.addArgument("savePath",
                     "s",
                     mitkCommandLineParser::InputDirectory,
                     "Input save folder (directory)",
                     "input save folder",
                     us::Any(),
                     false);
  parser.addArgument("numberOfInputs",
                     "s",
                     mitkCommandLineParser::InputDirectory,
                     "Number of Input files",
                     "number of inputs",
                     us::Any(),
                     false);
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    exit(-1);

  if (parsedArgs.count("inputPath"))
  {
    input.inputPath = us::any_cast<std::string>(parsedArgs["inputPath"]);
  }
  else
  {
    mitkThrow() << "Error: No inputPath";
  }
  if (parsedArgs.count("savePath"))
  {
    input.savePath = us::any_cast<std::string>(parsedArgs["savePath"]);
  }
  else
  {
    mitkThrow() << "Error: No savePath";
  }

  MITK_INFO << "Parsing arguments...[Done]";
  return input;
}


mitk::pa::SpectralUnmixingFilterBase::Pointer GetFilterInstance(std::string algorithm)
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

    std::vector<int> weigthVec = {39, 45, 47};

    for (int i = 0; i < 3; ++i)
    {
      dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(spectralUnmixingFilter.GetPointer())
        ->AddWeight(weigthVec[i]);
    }
  }
  return spectralUnmixingFilter;
}

int main(int argc, char *argv[])
{
  auto params = parseInput(argc, argv);

  auto inputPathname = params.inputPath;
  auto outputPath = params.savePath;
  auto numberOfInputs = params.numberOfInputs;

  std::vector<std::string> stringvec = { inputPathname };
  // boost::filesystem::path p(inputPathname);
  // boost::filesystem::directory_iterator start(p);
  // boost::filesystem::directory_iterator end;
  // std::transform(start, end, std::back_inserter(stringvec), path_leaf_string());

  std::vector<std::string> algorithms = {"QR", "LU", "SVD", "NNLS", "WLS"};

  numberOfInputs = 1;

  for (unsigned int filename = 0; filename < numberOfInputs; ++filename)
  {
    // m_inputImage = mitk::IOUtil::Load<mitk::Image>(stringvec[filename]);
    auto m_inputImage = mitk::IOUtil::Load<mitk::Image>(inputPathname);
    for (unsigned alg = 0; alg < 1; ++alg)
    {
      mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter = GetFilterInstance(algorithms[alg]);

      m_SpectralUnmixingFilter->SetInput(m_inputImage);
      m_SpectralUnmixingFilter->AddOutputs(2);
      m_SpectralUnmixingFilter->Verbose(false);
      m_SpectralUnmixingFilter->RelativeError(false);
      m_SpectralUnmixingFilter->AddWavelength(757);
      m_SpectralUnmixingFilter->AddWavelength(797);
      m_SpectralUnmixingFilter->AddWavelength(847);
      m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
      m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

      m_SpectralUnmixingFilter->Update();

      auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
      m_sO2->Verbose(false);
      auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
      auto output2 = m_SpectralUnmixingFilter->GetOutput(1);

      std::string unmixingOutputHbO2 = outputPath + "/SUOutput/" + algorithms[alg] + "/HbO2_" + stringvec[filename];
      std::string unmixingOutputHb = outputPath + "/SUOutput/" + algorithms[alg] + "/Hb_" + stringvec[filename];

      mitk::IOUtil::Save(output1, unmixingOutputHbO2);
      mitk::IOUtil::Save(output2, unmixingOutputHb);

      m_sO2->SetInput(0, output1);
      m_sO2->SetInput(1, output2);

      m_sO2->Update();

      mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
      sO2->SetSpacing(output1->GetGeometry()->GetSpacing());

      std::string outputSo2 = outputPath + "/So2/" + algorithms[alg] + "/So2_" + stringvec[filename];
      mitk::IOUtil::Save(sO2, outputSo2);
    }
  }
  MITK_INFO << "Spectral Unmixing DONE";
}
