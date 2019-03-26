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

#include <mitkCoreObjectFactory.h>
#include "mitkImage.h"
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.h"
#include <mitkLexicalCast.h>
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itkDiffusionMultiShellQballReconstructionImageFilter.h>
#include <itkDwiGradientLengthCorrectionFilter.h>

template<int L>
void TemplatedMultishellQBallReconstruction(float lambda, mitk::Image::Pointer dwi, bool output_sampled, int threshold, std::string outfilename)
{
  typedef itk::DiffusionMultiShellQballReconstructionImageFilter<short,short,float,L,ODF_SAMPLING_SIZE> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  auto bMap = mitk::DiffusionPropertyHelper::GetBValueMap(dwi);
  auto it1 = bMap.rbegin();
  auto it2 = bMap.rbegin();
  ++it2;

  // Get average distance
  int avdistance = 0;
  for(; it2 != bMap.rend(); ++it1, ++it2)
    avdistance += static_cast<int>(it1->first - it2->first);
  avdistance /= bMap.size()-1;

  // Check if all shells are using the same averae distance
  it1 = bMap.rbegin();
  it2 = bMap.rbegin();
  ++it2;
  for(; it2 != bMap.rend(); ++it1,++it2)
  {
    if(avdistance != static_cast<int>(it1->first - it2->first))
    {
      mitkThrow() << "Shells are not equidistant.";
    }
  }

  auto itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi);
  filter->SetBValueMap(bMap);
  filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), itkVectorImagePointer, mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi));
  filter->SetThreshold(static_cast<short>(threshold));
  filter->SetLambda(static_cast<double>(lambda));
  filter->Update();

  mitk::OdfImage::Pointer image = mitk::OdfImage::New();
  mitk::Image::Pointer coeffsImage = dynamic_cast<mitk::Image*>(mitk::ShImage::New().GetPointer());
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );
  coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
  coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );

  std::string coeffout = outfilename;
  coeffout += ".nii.gz";
  mitk::IOUtil::Save(coeffsImage, "SH_IMAGE", coeffout);

  outfilename += ".odf";
  if (output_sampled)
    mitk::IOUtil::Save(image, outfilename);
}

template<int L>
void TemplatedCsaQBallReconstruction(float lambda, mitk::Image::Pointer dwi, bool output_sampled, int threshold, std::string outfilename)
{
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,ODF_SAMPLING_SIZE> FilterType;
  auto itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi);

  FilterType::Pointer filter = FilterType::New();
  filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi));
  filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), itkVectorImagePointer );
  filter->SetThreshold(static_cast<short>(threshold));
  filter->SetLambda(static_cast<double>(lambda));
//  filter->SetUseMrtrixBasis(mrTrix);
  filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
  filter->Update();

  mitk::OdfImage::Pointer image = mitk::OdfImage::New();
  mitk::Image::Pointer coeffsImage = dynamic_cast<mitk::Image*>(mitk::ShImage::New().GetPointer());
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );
  coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
  coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );

  std::string coeffout = outfilename;
  coeffout += ".nii.gz";
  mitk::IOUtil::Save(coeffsImage, "SH_IMAGE", coeffout);

  outfilename += ".odf";
  if (output_sampled)
    mitk::IOUtil::Save(image, outfilename);
}


/**
 * Perform Q-ball reconstruction using a spherical harmonics basis
 */
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input image", "input raw dwi (.dwi or .nii/.nii.gz)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output image", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("sh_order", "", mitkCommandLineParser::Int, "Spherical harmonics order", "spherical harmonics order", 4);
  parser.addArgument("b0_threshold", "", mitkCommandLineParser::Int, "b0 threshold", "baseline image intensity threshold", 0);
  parser.addArgument("round_bvalues", "", mitkCommandLineParser::Int, "Round b-values", "round to specified integer", 0);
  parser.addArgument("lambda", "", mitkCommandLineParser::Float, "Lambda", "ragularization factor lambda", 0.006);
  parser.addArgument("output_sampled", "", mitkCommandLineParser::Bool, "Output sampled ODFs", "output file containing the sampled ODFs");

  parser.setCategory("Signal Modelling");
  parser.setTitle("Qball Reconstruction");
  parser.setDescription("");
  parser.setContributor("MIC");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outfilename = us::any_cast<std::string>(parsedArgs["o"]);
  if (itksys::SystemTools::GetFilenamePath(outfilename).size()>0)
    outfilename = itksys::SystemTools::GetFilenamePath(outfilename)+"/"+itksys::SystemTools::GetFilenameWithoutExtension(outfilename);
  else
    outfilename = itksys::SystemTools::GetFilenameWithoutExtension(outfilename);

  int threshold = 0;
  if (parsedArgs.count("b0_threshold"))
    threshold = us::any_cast<int>(parsedArgs["b0_threshold"]);

  int round_bvalues = 0;
  if (parsedArgs.count("round_bvalues"))
    round_bvalues = us::any_cast<int>(parsedArgs["round_bvalues"]);

  int shOrder = 4;
  if (parsedArgs.count("sh_order"))
    shOrder = us::any_cast<int>(parsedArgs["sh_order"]);

  float lambda = 0.006f;
  if (parsedArgs.count("lambda"))
    lambda = us::any_cast<float>(parsedArgs["lambda"]);

  bool outCoeffs = false;
  if (parsedArgs.count("output_coeffs"))
    outCoeffs = us::any_cast<bool>(parsedArgs["output_coeffs"]);

//  bool mrTrix = false;
//  if (parsedArgs.count("mrtrix"))
//    mrTrix = us::any_cast<bool>(parsedArgs["mrtrix"]);

  try
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
    std::vector< mitk::BaseData::Pointer > infile = mitk::IOUtil::Load(inFileName, &functor);

    mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(infile.at(0).GetPointer());

    if (round_bvalues>0)
    {
      MITK_INFO << "Rounding b-values";
      typedef itk::DwiGradientLengthCorrectionFilter FilterType;
      FilterType::Pointer filter = FilterType::New();
      filter->SetRoundingValue(round_bvalues);
      filter->SetReferenceBValue(static_cast<double>(mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi)));
      filter->SetReferenceGradientDirectionContainer(mitk::DiffusionPropertyHelper::GetGradientContainer(dwi));
      filter->Update();
      mitk::DiffusionPropertyHelper::SetReferenceBValue(dwi, static_cast<float>(filter->GetNewBValue()));
      mitk::DiffusionPropertyHelper::CopyProperties(dwi, dwi, true);
      mitk::DiffusionPropertyHelper::SetGradientContainer(dwi, filter->GetOutputGradientDirectionContainer());
      mitk::DiffusionPropertyHelper::InitializeImage(dwi);
    }

    auto bMap = mitk::DiffusionPropertyHelper::GetBValueMap(dwi);
    if(bMap.size()!=4 && bMap.size()!=2)
      mitkThrow() << "Only three equidistant shells or a single shell are supported. Found " << bMap.size();

    MITK_INFO << "Averaging redundant gradients";
    mitk::DiffusionPropertyHelper::AverageRedundantGradients(dwi, 0.001);

    MITK_INFO << "SH order: " << shOrder;
    MITK_INFO << "lambda: " << lambda;
    MITK_INFO << "B0 threshold: " << threshold;
    MITK_INFO << "Round bvalues: " << round_bvalues;
    switch ( shOrder )
    {
    case 4:
    {
      if(bMap.size()==2)
        TemplatedCsaQBallReconstruction<4>(lambda, dwi, outCoeffs, threshold, outfilename);
      else if(bMap.size()==4)
        TemplatedMultishellQBallReconstruction<4>(lambda, dwi, outCoeffs, threshold, outfilename);
      break;
    }
    case 6:
    {
      if(bMap.size()==2)
        TemplatedCsaQBallReconstruction<6>(lambda, dwi, outCoeffs, threshold, outfilename);
      else if(bMap.size()==4)
        TemplatedMultishellQBallReconstruction<6>(lambda, dwi, outCoeffs, threshold, outfilename);
      break;
    }
    case 8:
    {
      if(bMap.size()==2)
        TemplatedCsaQBallReconstruction<8>(lambda, dwi, outCoeffs, threshold, outfilename);
      else if(bMap.size()==4)
        TemplatedMultishellQBallReconstruction<8>(lambda, dwi, outCoeffs, threshold, outfilename);
      break;
    }
    case 10:
    {
      if(bMap.size()==2)
        TemplatedCsaQBallReconstruction<10>(lambda, dwi, outCoeffs, threshold, outfilename);
      else if(bMap.size()==4)
        TemplatedMultishellQBallReconstruction<10>(lambda, dwi, outCoeffs, threshold, outfilename);
      break;
    }
    case 12:
    {
      if(bMap.size()==2)
        TemplatedCsaQBallReconstruction<12>(lambda, dwi, outCoeffs, threshold, outfilename);
      else if(bMap.size()==4)
        TemplatedMultishellQBallReconstruction<12>(lambda, dwi, outCoeffs, threshold, outfilename);
      break;
    }
    default:
    {
      mitkThrow() << "SH order not supported";
    }
    }
  }
  catch ( itk::ExceptionObject &err)
  {
    std::cout << "Exception: " << err;
  }
  catch ( std::exception err)
  {
    std::cout << "Exception: " << err.what();
  }
  catch ( ... )
  {
    std::cout << "Exception!";
  }
  return EXIT_SUCCESS;
}
