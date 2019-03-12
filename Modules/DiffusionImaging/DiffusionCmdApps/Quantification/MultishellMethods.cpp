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
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExceptionObject.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <itkResampleImageFilter.h>

#include <mitkImage.h>
#include <mitkOdfImage.h>
#include <mitkBaseData.h>
#include "mitkCommandLineParser.h"
#include <mitkLexicalCast.h>

#include <itkRadialMultishellToSingleshellImageFilter.h>
#include <itkADCAverageFunctor.h>
#include <itkBiExpFitFunctor.h>
#include <itkKurtosisFitFunctor.h>
#include <itkDwiGradientLengthCorrectionFilter.h>
#include <mitkIOUtil.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkProperties.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Multishell Methods");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("adc", "D", mitkCommandLineParser::Bool, "ADC:", "ADC Average", us::Any(), false);
  parser.addArgument("akc", "K", mitkCommandLineParser::Bool, "Kurtosis fit:", "Kurtosis Fit", us::Any(), false);
  parser.addArgument("biexp", "B", mitkCommandLineParser::Bool, "BiExp fit:", "BiExp fit", us::Any(), false);
  parser.addArgument("targetbvalue", "b", mitkCommandLineParser::String, "b Value:", "target bValue (mean, min, max)", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string inName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outName = us::any_cast<std::string>(parsedArgs["o"]);
  bool applyADC = us::any_cast<bool>(parsedArgs["adc"]);
  bool applyAKC = us::any_cast<bool>(parsedArgs["akc"]);
  bool applyBiExp = us::any_cast<bool>(parsedArgs["biexp"]);
  std::string targetType = us::any_cast<std::string>(parsedArgs["targetbvalue"]);

  try
  {
    std::cout << "Loading " << inName;

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
    mitk::Image::Pointer dwi = mitk::IOUtil::Load<mitk::Image>(inName, &functor);

    if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dwi ) )
    {
      typedef itk::RadialMultishellToSingleshellImageFilter<short, short> FilterType;

      typedef itk::DwiGradientLengthCorrectionFilter  CorrectionFilterType;

      CorrectionFilterType::Pointer roundfilter = CorrectionFilterType::New();
      roundfilter->SetRoundingValue( 1000 );
      roundfilter->SetReferenceBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue( dwi ));
      roundfilter->SetReferenceGradientDirectionContainer(mitk::DiffusionPropertyHelper::GetGradientContainer(dwi));
      roundfilter->Update();

      mitk::DiffusionPropertyHelper::SetReferenceBValue(dwi, roundfilter->GetNewBValue());
      mitk::DiffusionPropertyHelper::SetGradientContainer(dwi, roundfilter->GetOutputGradientDirectionContainer());

      // filter input parameter
      const mitk::DiffusionPropertyHelper::BValueMapType
        &originalShellMap  = mitk::DiffusionPropertyHelper::GetBValueMap(dwi);

      mitk::DiffusionPropertyHelper::ImageType::Pointer vectorImage = mitk::DiffusionPropertyHelper::ImageType::New();
      mitk::CastToItkImage(dwi, vectorImage);

      const mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer
          gradientContainer = mitk::DiffusionPropertyHelper::GetGradientContainer(dwi);

      const unsigned int
          &bValue            = mitk::DiffusionPropertyHelper::GetReferenceBValue( dwi );

      // filter call


      vnl_vector<double> bValueList(originalShellMap.size()-1);
      double targetBValue = bValueList.mean();

      mitk::DiffusionPropertyHelper::BValueMapType::const_iterator it = originalShellMap.begin();
      ++it; int i = 0 ;
      for(; it != originalShellMap.end(); ++it)
        bValueList.put(i++,it->first);

      if( targetType == "mean" )
        targetBValue = bValueList.mean();
      else if( targetType == "min" )
        targetBValue = bValueList.min_value();
      else if( targetType == "max" )
        targetBValue = bValueList.max_value();

      if(applyADC)
      {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(vectorImage);
        filter->SetOriginalGradientDirections(gradientContainer);
        filter->SetOriginalBValueMap(originalShellMap);
        filter->SetOriginalBValue(bValue);

        itk::ADCAverageFunctor::Pointer functor = itk::ADCAverageFunctor::New();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);

        filter->SetFunctor(functor);
        filter->Update();
        // create new DWI image
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );

        mitk::DiffusionPropertyHelper::SetReferenceBValue(outImage, targetBValue);
        mitk::DiffusionPropertyHelper::SetGradientContainer(outImage, filter->GetTargetGradientDirections());
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );

        mitk::IOUtil::Save(outImage, (outName + "_ADC.dwi").c_str());
      }
      if(applyAKC)
      {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(vectorImage);
        filter->SetOriginalGradientDirections(gradientContainer);
        filter->SetOriginalBValueMap(originalShellMap);
        filter->SetOriginalBValue(bValue);

        itk::KurtosisFitFunctor::Pointer functor = itk::KurtosisFitFunctor::New();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);

        filter->SetFunctor(functor);
        filter->Update();
        // create new DWI image
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );

        mitk::DiffusionPropertyHelper::SetReferenceBValue(outImage, targetBValue);
        mitk::DiffusionPropertyHelper::SetGradientContainer(outImage, filter->GetTargetGradientDirections());
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );

        mitk::IOUtil::Save(outImage, (std::string(outName) + "_AKC.dwi").c_str());
      }
      if(applyBiExp)
      {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(vectorImage);
        filter->SetOriginalGradientDirections(gradientContainer);
        filter->SetOriginalBValueMap(originalShellMap);
        filter->SetOriginalBValue(bValue);

        itk::BiExpFitFunctor::Pointer functor = itk::BiExpFitFunctor::New();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);

        filter->SetFunctor(functor);
        filter->Update();
        // create new DWI image
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );

        mitk::DiffusionPropertyHelper::SetReferenceBValue(outImage, targetBValue);
        mitk::DiffusionPropertyHelper::SetGradientContainer(outImage, filter->GetTargetGradientDirections());
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );

        mitk::IOUtil::Save(outImage, (std::string(outName) + "_BiExp.dwi").c_str());
      }
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
