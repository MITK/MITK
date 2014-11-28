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

#include <mitkImageCast.h>
#include <mitkBaseDataIOFactory.h>
#include "mitkCommandLineParser.h"
#include <boost/algorithm/string.hpp>
#include <DiffusionWeightedImages/mitkDiffusionImage.h>
#include <itkNonLocalMeansDenoisingFilter.h>
#include <itkImage.h>
#include <mitkIOUtil.h>

typedef mitk::DiffusionImage<short> DiffusionImageType;
typedef itk::Image<short, 3> ImageType;

mitk::BaseData::Pointer LoadFile(std::string filename)
{
  if( filename.empty() )
    return NULL;

  const std::string s1="", s2="";
  std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( filename, s1, s2, false );
  if( infile.empty() )
  {
    std::cout << "File " << filename << " could not be read!";
    return NULL;
  }

  mitk::BaseData::Pointer baseData = infile.at(0);
  return baseData;
}

/**
 * Denoises DWI using the Nonlocal - Means algorithm
 */
int main(int argc, char* argv[])
{
    std::cout << "DwiDenoising";
  mitkCommandLineParser parser;

  parser.setTitle("DWI Denoising");
  parser.setCategory("Preprocessing Tools");
  parser.setContributor("MBI");
  parser.setDescription("Denoising for diffusion weighted images using a non-local means algorithm.");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input image (DWI)", us::Any(), false);
  parser.addArgument("variance", "v", mitkCommandLineParser::Float, "Variance:", "noise variance", us::Any(), false);

  parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask:", "brainmask for input image", us::Any(), true);
  parser.addArgument("search", "s", mitkCommandLineParser::Int, "Search radius:", "search radius", us::Any(), true);
  parser.addArgument("compare", "c", mitkCommandLineParser::Int, "Comparison radius:", "comparison radius", us::Any(), true);
  parser.addArgument("joint", "j", mitkCommandLineParser::Bool, "Joint information:", "use joint information");
  parser.addArgument("rician", "r", mitkCommandLineParser::Bool, "Rician adaption:", "use rician adaption");

  parser.changeParameterGroup("Output", "Output of this miniapp");

  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output:", "output image (DWI)", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  string inFileName = us::any_cast<string>(parsedArgs["input"]);
  double variance = static_cast<double>(us::any_cast<float>(parsedArgs["variance"]));
  string maskName;
  if (parsedArgs.count("mask"))
    maskName = us::any_cast<string>(parsedArgs["mask"]);
  string outFileName = us::any_cast<string>(parsedArgs["output"]);
//  boost::algorithm::erase_all(outFileName, ".dwi");
  int search = 4;
  if (parsedArgs.count("search"))
    search = us::any_cast<int>(parsedArgs["search"]);
  int compare = 1;
  if (parsedArgs.count("compare"))
    compare = us::any_cast<int>(parsedArgs["compare"]);
  bool joint = false;
  if (parsedArgs.count("joint"))
    joint = true;
  bool rician = false;
  if (parsedArgs.count("rician"))
    rician = true;

  try
  {

    if( boost::algorithm::ends_with(inFileName, ".dwi"))
    {

      DiffusionImageType::Pointer dwi = dynamic_cast<DiffusionImageType*>(LoadFile(inFileName).GetPointer());

      itk::NonLocalMeansDenoisingFilter<short>::Pointer filter = itk::NonLocalMeansDenoisingFilter<short>::New();
      filter->SetNumberOfThreads(12);
      filter->SetInputImage(dwi->GetVectorImage());

      if (!maskName.empty())
      {
        mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(LoadFile(maskName).GetPointer());
        ImageType::Pointer itkMask = ImageType::New();
        mitk::CastToItkImage(mask, itkMask);
        filter->SetInputMask(itkMask);
      }

      filter->SetUseJointInformation(joint);
      filter->SetUseRicianAdaption(rician);
      filter->SetSearchRadius(search);
      filter->SetComparisonRadius(compare);
      filter->SetVariance(variance);
      filter->Update();

      DiffusionImageType::Pointer output = DiffusionImageType::New();
      output->SetVectorImage(filter->GetOutput());
      output->SetReferenceBValue(dwi->GetReferenceBValue());
      output->SetDirections(dwi->GetDirections());
      output->InitializeFromVectorImage();

//      std::stringstream name;
//      name << outFileName << "_NLM_" << search << "-" << compare << "-" << variance << ".dwi";

      mitk::IOUtil::Save(output, outFileName.c_str());
    }
    else
    {
      std::cout << "Only supported for .dwi!";
    }
  }
  catch (itk::ExceptionObject e)
  {
      std::cout << e;
      return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
      std::cout << e.what();
      return EXIT_FAILURE;
  }
  catch (...)
  {
      std::cout << "ERROR!?!";
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
