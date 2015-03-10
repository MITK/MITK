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

#include <mitkImage.h>
#include <mitkQBallImage.h>
#include <mitkBaseData.h>
#include <mitkFiberBundle.h>
#include "mitkCommandLineParser.h"
#include <boost/lexical_cast.hpp>

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

    mitk::Image::Pointer dwi = mitk::IOUtil::LoadImage(inName);

    if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dwi ) )
    {
      typedef itk::RadialMultishellToSingleshellImageFilter<short, short> FilterType;

      typedef itk::DwiGradientLengthCorrectionFilter  CorrectionFilterType;

      CorrectionFilterType::Pointer roundfilter = CorrectionFilterType::New();
      roundfilter->SetRoundingValue( 1000 );
      roundfilter->SetReferenceBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue( dwi ));
      roundfilter->SetReferenceGradientDirectionContainer(mitk::DiffusionPropertyHelper::GetGradientContainer(dwi));
      roundfilter->Update();

      dwi->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( roundfilter->GetNewBValue()  ) );
      dwi->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( roundfilter->GetOutputGradientDirectionContainer() ) );

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
        outImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( targetBValue  ) );
        outImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetTargetGradientDirections() ) );
        mitk::DiffusionPropertyHelper propertyHelper( outImage );
        propertyHelper.InitializeImage();

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
        outImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( targetBValue  ) );
        outImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetTargetGradientDirections() ) );
        mitk::DiffusionPropertyHelper propertyHelper( outImage );
        propertyHelper.InitializeImage();

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
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );
        outImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( targetBValue  ) );
        outImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetTargetGradientDirections() ) );
        mitk::DiffusionPropertyHelper propertyHelper( outImage );
        propertyHelper.InitializeImage();

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
