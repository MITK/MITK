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

#include <mitkImageCast.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include "ctkCommandLineParser.h"
#include <boost/algorithm/string.hpp>
#include <DiffusionWeightedImages/mitkDiffusionImage.h>
#include <itkNonLocalMeansDenoisingFilter.h>
#include <mitkNrrdDiffusionImageWriter.h>
#include <itkImage.h>

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
    MITK_INFO << "File " << filename << " could not be read!";
    return NULL;
  }

  mitk::BaseData::Pointer baseData = infile.at(0);
  return baseData;
}

/**
 * Calculate indices derived from Qball or tensor images
 */
int DwiDenoising(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", ctkCommandLineParser::String, "input image (DWI)", us::Any(), false);
  parser.addArgument("mask", "m", ctkCommandLineParser::String, "brainmask for input image", us::Any(), false);
  parser.addArgument("search", "s", ctkCommandLineParser::Int, "search radius", us::Any(), false);
  parser.addArgument("compare", "c", ctkCommandLineParser::Int, "compare radius", us::Any(), false);
  parser.addArgument("channels", "ch", ctkCommandLineParser::Int, "radius of used channels", us::Any(), true);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  string inFileName = us::any_cast<string>(parsedArgs["input"]);
  string maskName = us::any_cast<string>(parsedArgs["mask"]);
  string outFileName = inFileName;
  boost::algorithm::erase_all(outFileName, ".dwi");
  int search = us::any_cast<int>(parsedArgs["search"]);
  int compare = us::any_cast<int>(parsedArgs["compare"]);
  int channels = 0;
  if (parsedArgs.count("channels"))
    channels = us::any_cast<int>(parsedArgs["channels"]);

  try
  {
    RegisterDiffusionCoreObjectFactory();


    if( boost::algorithm::ends_with(inFileName, ".dwi"))
    {

      DiffusionImageType::Pointer dwi = dynamic_cast<DiffusionImageType*>(LoadFile(inFileName).GetPointer());
      mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(LoadFile(maskName).GetPointer());
      ImageType::Pointer itkMask = ImageType::New();
      mitk::CastToItkImage(mask, itkMask);

      itk::NonLocalMeansDenoisingFilter<short>::Pointer filter = itk::NonLocalMeansDenoisingFilter<short>::New();
      filter->SetNumberOfThreads(12);
      filter->SetInputImage(dwi->GetVectorImage());
      filter->SetInputMask(itkMask);

      if (channels == 0)
      {
        MITK_INFO << "Denoising with: s = " << search << "; c = " << compare;

        filter->SetUseJointInformation(false);
        filter->SetSearchRadius(search);
        filter->SetComparisonRadius(compare);
        filter->Update();

        DiffusionImageType::Pointer output = DiffusionImageType::New();
        output->SetVectorImage(filter->GetOutput());
        output->SetB_Value(dwi->GetB_Value());
        output->SetDirections(dwi->GetDirections());
        output->InitializeFromVectorImage();

        std::stringstream name;
        name << outFileName << "_NLMr_" << search << "-" << compare << ".dwi";

        MITK_INFO << "Writing: " << name.str();

        mitk::NrrdDiffusionImageWriter<short>::Pointer writer = mitk::NrrdDiffusionImageWriter<short>::New();
        writer->SetInput(output);
        writer->SetFileName(name.str());
        writer->Update();
      }

      else
      {
        MITK_INFO << "Denoising with: s = " << search << "; c = " << compare << "; ch = " << channels;
        filter->SetUseJointInformation(true);
        filter->SetSearchRadius(search);
        filter->SetComparisonRadius(compare);
        filter->SetChannelRadius(channels);
        filter->Update();

        DiffusionImageType::Pointer output = DiffusionImageType::New();
        output->SetVectorImage(filter->GetOutput());
        output->SetB_Value(dwi->GetB_Value());
        output->SetDirections(dwi->GetDirections());
        output->InitializeFromVectorImage();


        std::stringstream name;
        name << outFileName << "_NLMv_" << search << "-" << compare << "-" << channels << ".dwi";

        MITK_INFO << "Writing " << name.str();

        mitk::NrrdDiffusionImageWriter<short>::Pointer writer = mitk::NrrdDiffusionImageWriter<short>::New();
        writer->SetInput(output);
        writer->SetFileName(name.str());
        writer->Update();
      }

      MITK_INFO << "Finish!";
    }
    else
    {
      MITK_INFO << "Only supported for .dwi!";
    }
  }
  catch (itk::ExceptionObject e)
  {
      MITK_INFO << e;
      return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
      MITK_INFO << e.what();
      return EXIT_FAILURE;
  }
  catch (...)
  {
      MITK_INFO << "ERROR!?!";
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}



RegisterDiffusionMiniApp(DwiDenoising);
