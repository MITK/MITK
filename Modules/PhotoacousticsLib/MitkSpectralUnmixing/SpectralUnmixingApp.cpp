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

#include "mitkPASpectralUnmixingFilterBase.h"
#include "mitkPALinearSpectralUnmixingFilter.h"
#include "mitkPASpectralUnmixingSO2.h"
#include "mitkPASpectralUnmixingFilterVigra.h"

#include <mitkIOUtil.h>
#include <mitkCommandLineParser.h>
#include <mitkUIDGenerator.h>
#include <mitkException.h>

#include <itksys/SystemTools.hxx>

using namespace mitk::pa;

TissueGeneratorParameters::Pointer CreatePhantom_04_04_18_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(12);

  return returnParameters;
}

struct InputParameters
{
  std::string saveFolderPath;
  std::string identifyer;
  std::string exePath;
  std::string probePath;
  bool empty;
  bool verbose;
};

InputParameters parseInput(int argc, char* argv[])
{
  MITK_INFO << "Parsing arguments...";
  mitkCommandLineParser parser;

  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Tissue Batch Generator");
  parser.setDescription("Creates in silico tissue in batch processing and automatically calculates fluence values for the central slice of the volume.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required parameters");
  parser.addArgument(
    "savePath", "s", mitkCommandLineParser::InputDirectory,
    "Input save folder (directory)", "input save folder",
    us::Any(), false);
  parser.addArgument(
    "mitkMcxyz", "m", mitkCommandLineParser::OutputFile,
    "MitkMcxyz binary (file)", "path to the MitkMcxyz binary",
    us::Any(), false);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "probe", "p", mitkCommandLineParser::OutputFile,
    "xml probe file (file)", "file to the definition of the used probe (*.xml)",
    us::Any());
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool,
    "Verbose Output", "Whether to produce verbose, or rather debug output");
  parser.addArgument(
    "identifyer", "i", mitkCommandLineParser::String,
    "Generator identifyer (string)", "A unique identifyer for the calculation instance");
  parser.addArgument(
    "empty-volume", "e", mitkCommandLineParser::Bool,
    "omit vessel structures (boolean flag)", "Whether to create an empty volume with no structures inside.");
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    exit(-1);

  if (parsedArgs.count("empty-volume"))
  {
    input.empty = us::any_cast<bool>(parsedArgs["empty-volume"]);
  }
  else
  {
    input.empty = false;
  }

  if (parsedArgs.count("verbose"))
  {
    input.verbose = us::any_cast<bool>(parsedArgs["verbose"]);
  }
  else
  {
    input.verbose = false;
  }

  if (parsedArgs.count("savePath"))
  {
    input.saveFolderPath = us::any_cast<std::string>(parsedArgs["savePath"]);
  }

  if (parsedArgs.count("mitkMcxyz"))
  {
    input.exePath = us::any_cast<std::string>(parsedArgs["mitkMcxyz"]);
  }

  if (parsedArgs.count("probe"))
  {
    input.probePath = us::any_cast<std::string>(parsedArgs["probe"]);
  }

  if (parsedArgs.count("identifyer"))
  {
    input.identifyer = us::any_cast<std::string>(parsedArgs["identifyer"]);
  }
  else
  {
    auto uid = mitk::UIDGenerator("", 8);
    input.identifyer = uid.GetUID();
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
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);
  }

  else if (algorithm == "SVD")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::JACOBISVD);
  }

  else if (algorithm == "LU")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVLU);
  }

  else if (algorithm == "NNLS")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS);
  }

  else if (algorithm == "WLS")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED);
	  
	std::vector<int> weigthVec = {39, 45, 47}
	for (int i; i < 3; ++i)
	{
	3dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
          ->AddWeight(weigthVec[i]);
    }
  }
    return spectralUnmixingFilter;
}


void CalculateSO2(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec)
{
  MITK_INFO(PluginVerbose) << "CALCULATE OXYGEN SATURATION ...";

  if (!boolVec[0])
    mitkThrow() << "SELECT CHROMOPHORE DEOXYHEMOGLOBIN!";
  if (!boolVec[1])
    mitkThrow() << "SELECT CHROMOPHORE OXYHEMOGLOBIN!";
  auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
  m_sO2->Verbose(PluginVerbose);
  SetSO2Settings(m_sO2);

  // Initialize pipeline from SU filter class to SO2 class
  auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
  auto output2 = m_SpectralUnmixingFilter->GetOutput(1);
  m_sO2->SetInput(0, output1);
  m_sO2->SetInput(1, output2);

  m_sO2->Update();

  mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
  sO2->SetSpacing(output1->GetGeometry()->GetSpacing());
  WriteOutputToDataStorage(sO2, "sO2");
  MITK_INFO(PluginVerbose) << "[DONE]";
}


int main(int argc, char * argv[])
{
	std::vector<std::string> algorithms = {'QR', 'LU', 'SVD', 'NNLS', 'WLS'},
	
	
	
	for (int i; i < 5; ++i)
	{
		mitk::pa::LinearSpectralUnmixingFilter::Pointer m_SpectralUnmixingFilter;
		m_SpectralUnmixingFilter = GetFilterInstance(algorithms[i]);
		
		m_SpectralUnmixingFilter->SetAlgorithm(algorithms[i]);
		m_SpectralUnmixingFilter->SetInput(inputImage);
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
		
		# save outputs! 
		
		
		m_sO2->SetInput(0, output1);
		m_sO2->SetInput(1, output2);
  
		m_sO2->Update();

		mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
		sO2->SetSpacing(output1->GetGeometry()->GetSpacing());
		WriteOutputToDataStorage(sO2, "sO2");
		
		# save output!!
		
	}
	
	
	
	
	
	
	
	



  auto input = parseInput(argc, argv);
  auto parameters = CreatePhantom_04_04_18_Parameters();
  if (input.empty)
  {
    parameters->SetMaxNumberOfVessels(0);
    parameters->SetMinNumberOfVessels(0);
  }
  MITK_INFO(input.verbose) << "Generating tissue..";
  auto resultTissue = InSilicoTissueGenerator::GenerateInSilicoData(parameters);
  MITK_INFO(input.verbose) << "Generating tissue..[Done]";

  auto inputfolder = std::string(input.saveFolderPath + "input/");
  auto outputfolder = std::string(input.saveFolderPath + "output/");
  if (!itksys::SystemTools::FileIsDirectory(inputfolder))
  {
    itksys::SystemTools::MakeDirectory(inputfolder);
  }
  if (!itksys::SystemTools::FileIsDirectory(outputfolder))
  {
    itksys::SystemTools::MakeDirectory(outputfolder);
  }

  std::string savePath = input.saveFolderPath + "input/Phantom_" + input.identifyer +
    ".nrrd";
  mitk::IOUtil::Save(resultTissue->ConvertToMitkImage(), savePath);
  std::string outputPath = input.saveFolderPath + "output/Phantom_" + input.identifyer +
    "/";

  resultTissue = nullptr;

  if (!itksys::SystemTools::FileIsDirectory(outputPath))
  {
    itksys::SystemTools::MakeDirectory(outputPath);
  }

  outputPath = outputPath + "Fluence_Phantom_" + input.identifyer;

  MITK_INFO(input.verbose) << "Simulating fluence..";

  int result = -4;

  std::string cmdString = std::string(input.exePath + " -i " + savePath + " -o " +
    (outputPath + ".nrrd") +
    " -yo " + "0" + " -p " + input.probePath +
    " -n 10000000");

  MITK_INFO << "Executing: " << cmdString;

  result = std::system(cmdString.c_str());

  MITK_INFO << result;
  MITK_INFO(input.verbose) << "Simulating fluence..[Done]";
}
