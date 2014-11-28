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
#include <string>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <itkResampleImageFilter.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionImage.h>
#include <mitkQBallImage.h>
#include <mitkBaseData.h>
#include <mitkFiberBundleX.h>
#include "mitkCommandLineParser.h"
#include <boost/lexical_cast.hpp>

#include <itkRadialMultishellToSingleshellImageFilter.h>
#include <itkADCAverageFunctor.h>
#include <itkBiExpFitFunctor.h>
#include <itkKurtosisFitFunctor.h>
#include <itkDwiGradientLengthCorrectionFilter.h>
#include <mitkIOUtil.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Multishell Methods");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input:", "input file", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output:", "output file", us::Any(), false);
  parser.addArgument("adc", "D", mitkCommandLineParser::Bool, "ADC:", "ADC Average", us::Any(), false);
  parser.addArgument("akc", "K", mitkCommandLineParser::Bool, "Kurtosis fit:", "Kurtosis Fit", us::Any(), false);
  parser.addArgument("biexp", "B", mitkCommandLineParser::Bool, "BiExp fit:", "BiExp fit", us::Any(), false);
  parser.addArgument("targetbvalue", "b", mitkCommandLineParser::String, "b Value:", "target bValue (mean, min, max)", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  string inName = us::any_cast<string>(parsedArgs["in"]);
  string outName = us::any_cast<string>(parsedArgs["out"]);
  bool applyADC = us::any_cast<bool>(parsedArgs["adc"]);
  bool applyAKC = us::any_cast<bool>(parsedArgs["akc"]);
  bool applyBiExp = us::any_cast<bool>(parsedArgs["biexp"]);
  string targetType = us::any_cast<string>(parsedArgs["targetbvalue"]);

  try
  {
    std::cout << "Loading " << inName;
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( inName, s1, s2, false );
    mitk::BaseData::Pointer baseData = infile.at(0);

    if ( dynamic_cast<mitk::DiffusionImage<short>*>(baseData.GetPointer()) )
    {
      mitk::DiffusionImage<short>::Pointer dwi = dynamic_cast<mitk::DiffusionImage<short>*>(baseData.GetPointer());
      typedef itk::RadialMultishellToSingleshellImageFilter<short, short> FilterType;

      typedef itk::DwiGradientLengthCorrectionFilter  CorrectionFilterType;

      CorrectionFilterType::Pointer roundfilter = CorrectionFilterType::New();
      roundfilter->SetRoundingValue( 1000 );
      roundfilter->SetReferenceBValue(dwi->GetReferenceBValue());
      roundfilter->SetReferenceGradientDirectionContainer(dwi->GetDirections());
      roundfilter->Update();

      dwi->SetReferenceBValue( roundfilter->GetNewBValue() );
      dwi->SetDirections( roundfilter->GetOutputGradientDirectionContainer());

      // filter input parameter
      const mitk::DiffusionImage<short>::BValueMap
          &originalShellMap  = dwi->GetBValueMap();

      const mitk::DiffusionImage<short>::ImageType
          *vectorImage       = dwi->GetVectorImage();

      const mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer
          gradientContainer = dwi->GetDirections();

      const unsigned int
          &bValue            = dwi->GetReferenceBValue();

      // filter call


      vnl_vector<double> bValueList(originalShellMap.size()-1);
      double targetBValue = bValueList.mean();

      mitk::DiffusionImage<short>::BValueMap::const_iterator it = originalShellMap.begin();
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
        mitk::DiffusionImage<short>::Pointer outImage = mitk::DiffusionImage<short>::New();
        outImage->SetVectorImage( filter->GetOutput() );
        outImage->SetReferenceBValue( targetBValue );
        outImage->SetDirections( filter->GetTargetGradientDirections() );
        outImage->InitializeFromVectorImage();

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
        mitk::DiffusionImage<short>::Pointer outImage = mitk::DiffusionImage<short>::New();
        outImage->SetVectorImage( filter->GetOutput() );
        outImage->SetReferenceBValue( targetBValue );
        outImage->SetDirections( filter->GetTargetGradientDirections() );
        outImage->InitializeFromVectorImage();

        mitk::IOUtil::Save(outImage, (string(outName) + "_AKC.dwi").c_str());
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
        mitk::DiffusionImage<short>::Pointer outImage = mitk::DiffusionImage<short>::New();
        outImage->SetVectorImage( filter->GetOutput() );
        outImage->SetReferenceBValue( targetBValue );
        outImage->SetDirections( filter->GetTargetGradientDirections() );
        outImage->InitializeFromVectorImage();

        mitk::IOUtil::Save(outImage, (string(outName) + "_BiExp.dwi").c_str());
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
