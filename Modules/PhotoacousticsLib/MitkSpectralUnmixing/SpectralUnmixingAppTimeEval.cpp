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
  int numberOfInputs;
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
                     mitkCommandLineParser::Directory,
                     "Input folder (directory)",
                     "input folder",
                     us::Any(),
                     false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("outputPath",
                     "o",
                     mitkCommandLineParser::Directory,
                     "Input save folder (directory)",
                     "input save folder",
                     us::Any(),
                     false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("numberOfInputs",
                     "n",
                     mitkCommandLineParser::Int,
                     "Number of Input files",
                     "number of inputs",
                     us::Any(),
                     false);
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
  if (parsedArgs.count("numberOfInputs"))
  {
    input.numberOfInputs = us::any_cast<int>(parsedArgs["numberOfInputs"]);
  }
  else
  {
    MITK_ERROR << "Error: No number of Inputs";
    mitkThrow() << "Error: No number of Inputs";
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

    /*std::vector<int> weigthVec = {39, 45, 47};

    for (int i = 0; i < 3; ++i)
    {
      dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(spectralUnmixingFilter.GetPointer())
        ->AddWeight(weigthVec[i]);
    }*/
  }
  return spectralUnmixingFilter;
}

void add_weight(int weights, mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter)
{
  std::vector<int> weigthVec = { 30, 32, 33, 35, 37, 38, 40, 41, 43, 44, 45, 46, 47, 47,
    47, 47, 47, 46, 46, 45, 44, 44, 43, 42, 42, 41 };

  for (int i = 0; i < weights; ++i)
  {
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra *>(m_SpectralUnmixingFilter.GetPointer())
      ->AddWeight(weigthVec[i]);
  }
}



int main(int argc, char *argv[])
{ 
  auto input = parseInput(argc, argv);

  std::string inputDir = input.inputPath;
  std::string outputDir = input.outputPath;
  unsigned int N = input.numberOfInputs;

/*
  //maybee try with "itk system tools"

  //auto test = itksys::SystemTools::GetFilenameName(argv[0]).c_str();
  
  //MITK_INFO << "test: " << test;
  

  /  +++ temporary solution BEGIN +++
  std::vector<std::string> files;
  std::string file;
  for (int i = 1; i < 34; ++i)
  {

    if (i < 10)
    {
      file = "E:/NHDATA/sdmas_beamformed/merged/static-oxy_sdmas_00" + std::to_string(i) + "_merged.nrrd";
    }
    else
    {
      file = "E:/NHDATA/sdmas_beamformed/merged/static-oxy_sdmas_0" + std::to_string(i) + "_merged.nrrd";
    }
    files.push_back(file);
  }
  /  +++ temporary solution END +++

  std::vector<std::string> files;
  std::string file;
  for (int i = 0; i < 7; ++i)
  {
    file = "E:/NHCAMI/cami-experimental/PAI/spectralUnmixing/inSilico/paImages/selection/noiselevel1_rep1000_wavelength_selction_data_" +
      std::to_string(i) + ".nrrd";
    files.push_back(file);
  }
  std::vector<std::string> files;
  std::string file;
  file = "E:/NHCAMI/cami-experimental/PAI/spectralUnmixing/inSilico/paImages/selection/noiselevel1_rep1000_wavelength_selction_data.nrrd";
  files.push_back(file);*/
  
  std::vector<std::string> algorithms = { "QR", "LU", "SVD", "NNLS", "WLS" };
  int repetition = 6000;

  for (unsigned alg = 0; alg < 5; ++alg)
  {
    ofstream myerrorfile;
    myerrorfile.open("E:/NHDATA/time/time_evaluation_" + std::to_string(repetition)+"_" + algorithms[alg] + "_new02.txt");

    int ctr = 0;
    for(int i = 2; i < 27; ++i)
    {
      myerrorfile << std::to_string(i) + "\t";
      std::string file;
      if (i < 10)
        file = "E:/NHDATA/time/input/time_0" + std::to_string(i) + ".nrrd";
      else
        file = "E:/NHDATA/time/input/time_" + std::to_string(i) + ".nrrd";
      
      auto m_inputImage = mitk::IOUtil::Load<mitk::Image>(file);
    
      MITK_INFO << "File: " << i;

      for (int j = 0; j < repetition; ++j)
        {
           std::chrono::steady_clock::time_point _start;
           _start = std::chrono::steady_clock::now(); 

          mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter = GetFilterInstance(algorithms[alg]);
          m_SpectralUnmixingFilter->SetInput(m_inputImage);
          m_SpectralUnmixingFilter->AddOutputs(2);
          m_SpectralUnmixingFilter->Verbose(false);
          m_SpectralUnmixingFilter->RelativeError(false);
          m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
          m_SpectralUnmixingFilter->AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);
          
          for (int wl = 0; wl < i; ++wl)
          {
            m_SpectralUnmixingFilter->AddWavelength(700 + wl * 10);
          }    
          
          if (alg == 4)
          {         
            add_weight(i, m_SpectralUnmixingFilter);
          }


          m_SpectralUnmixingFilter->Update();

          auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
          auto output2 = m_SpectralUnmixingFilter->GetOutput(1);

          m_SpectralUnmixingFilter = nullptr;

          std::chrono::steady_clock::time_point _end(std::chrono::steady_clock::now());
          myerrorfile << std::chrono::duration_cast<std::chrono::duration<double>>(_end - _start).count() << "\t";

        /*std::string unmixingOutputHbO2 = "E:/NHDATA/time/output/time_" + std::to_string(i) + ".nrrd";
        std::string unmixingOutputHb = "E:/NHDATA/time/output/time_" + std::to_string(i) + ".nrrd";
        mitk::IOUtil::Save(output1, unmixingOutputHbO2);
        mitk::IOUtil::Save(output2, unmixingOutputHb);/*
/*
        //auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
        //m_sO2->Verbose(false);
        //auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
        //auto output2 = m_SpectralUnmixingFilter->GetOutput(1);


        //std::string unmixingOutputHbO2 ="E:/NHDATA/time/input/time_" + std::to_string(i) + ".nrrd";
        //std::string unmixingOutputHb = outputDir + "/SUOutput/" + "Hb_" + algorithms[alg] + "_" + str_ctr + ".nrrd";
        //mitk::IOUtil::Save(output1, unmixingOutputHbO2);
        //mitk::IOUtil::Save(output2, unmixingOutputHb);

        //m_sO2->SetInput(0, output1);
        //m_sO2->SetInput(1, output2);

        //m_sO2->Update();

        //mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
        //sO2->SetSpacing(output1->GetGeometry()->GetSpacing());

        //std::string outputSo2 = outputDir + "/So2/" + algorithms[alg] + "/So2_" + algorithms[alg] + "_" + str_ctr + ".nrrd";
        //std::string outputSo2 = outputDir + "/" + algorithms[alg] + "_sel_" + str_ctr + ".nrrd";
        //std::string outputSo2 = outputDir + "/" + algorithms[alg] + "_sel.nrrd";
        //mitk::IOUtil::Save(sO2, outputSo2);

        //std::string outputSo2 = "E:/NHDATA/time/output/time_" + std::to_string(i) + algorithms[alg] + ".nrrd";
        //mitk::IOUtil::Save(sO2, outputSo2);*/
        }
      myerrorfile << "\n";
    }
  myerrorfile.close();
  }
  MITK_INFO << "Spectral Unmixing DONE";
}
