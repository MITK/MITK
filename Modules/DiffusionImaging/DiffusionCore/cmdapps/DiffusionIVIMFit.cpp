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

#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <mitkDiffusionPropertyHelper.h>

#include <mitkImageCaster.h>
#include <itkDiffusionKurtosisReconstructionImageFilter.h>

//vnl_includes
#include "vnl/vnl_math.h"
#include "vnl/vnl_cost_function.h"
#include "vnl/vnl_least_squares_function.h"
#include "vnl/algo/vnl_lbfgsb.h"
#include "vnl/algo/vnl_lbfgs.h"

#include "vnl/algo/vnl_levenberg_marquardt.h"

typedef mitk::DiffusionPropertyHelper DPH;

#include <itkGaussianBlurImageFunction.h>
#include <itkUnaryFunctorImageFilter.h>

#include <itkImageFileWriter.h>

#include <itkVectorIndexSelectionCastImageFilter.h>
#include <itkComposeImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>

DPH::ImageType::Pointer GetBlurredVectorImage( DPH::ImageType::Pointer vectorImage, double sigma)
{
  typedef itk::DiscreteGaussianImageFilter< itk::Image<DPH::DiffusionPixelType, 3 >, itk::Image<DPH::DiffusionPixelType, 3 > > GaussianFilterType;

  typedef itk::VectorIndexSelectionCastImageFilter< DPH::ImageType, itk::Image<DPH::DiffusionPixelType, 3 > > IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetInput( vectorImage );

  typedef itk::ComposeImageFilter< itk::Image<DPH::DiffusionPixelType, 3>, DPH::ImageType > ComposeFilterType;
  ComposeFilterType::Pointer vec_composer = ComposeFilterType::New();

  for( unsigned int i=0; i<vectorImage->GetVectorLength(); ++i)
  {
    GaussianFilterType::Pointer gaussian_filter = GaussianFilterType::New();

    indexSelectionFilter->SetIndex( i );

    gaussian_filter->SetInput( indexSelectionFilter->GetOutput() );
    gaussian_filter->SetVariance( sigma );

    vec_composer->SetInput(i, gaussian_filter->GetOutput() );

    gaussian_filter->Update();
  }

  try
  {
    vec_composer->Update();
  }
  catch(const itk::ExceptionObject &e)
  {
    mitkThrow() << "[VectorImage.GaussianSmoothing] !! Failed with ITK Exception: " << e.what();
  }

  DPH::ImageType::Pointer smoothed_vector = vec_composer->GetOutput();
/*
  itk::ImageFileWriter< DPH::ImageType >::Pointer writer =
      itk::ImageFileWriter< DPH::ImageType >::New();

  writer->SetInput( smoothed_vector );
  writer->SetFileName( "/tmp/itk_smoothed_vector.nrrd");
  writer->Update();*/

  return smoothed_vector;


}

void KurtosisMapComputation( mitk::Image::Pointer input, std::string output_prefix )
{
  DPH::ImageType::Pointer vectorImage = DPH::ImageType::New();
  mitk::CastToItkImage( input, vectorImage );



  typedef itk::DiffusionKurtosisReconstructionImageFilter< short, double > KurtosisFilterType;
  KurtosisFilterType::Pointer kurtosis_filter = KurtosisFilterType::New();

  kurtosis_filter->SetInput( GetBlurredVectorImage( vectorImage, 1.5 ) );
  kurtosis_filter->SetReferenceBValue( DPH::GetReferenceBValue( input.GetPointer() ) );
  kurtosis_filter->SetGradientDirections( DPH::GetGradientContainer( input.GetPointer() ) );
  kurtosis_filter->SetNumberOfThreads(1);

  KurtosisFilterType::OutputImageRegionType o_region;
  KurtosisFilterType::OutputImageRegionType::SizeType o_size;
  KurtosisFilterType::OutputImageRegionType::IndexType o_index;

  o_index.Fill(0); o_size.Fill(0);
  o_index[0] = 48; o_index[1] = 18; o_index[2] = 12;
  o_size[0] = 16; o_size[1] = 24; o_size[2] = 1;

  o_region.SetSize( o_size );
  o_region.SetIndex( o_index );
  kurtosis_filter->SetMapOutputRegion( o_region );

  try
  {
    kurtosis_filter->Update();
  }
  catch( const itk::ExceptionObject& e)
  {
    mitkThrow() << "Kurtosis fit failed with an ITK Exception: " << e.what();
  }

  mitk::Image::Pointer d_image = mitk::Image::New();
  d_image->InitializeByItk( kurtosis_filter->GetOutput(0) );
  d_image->SetVolume( kurtosis_filter->GetOutput(0)->GetBufferPointer() );

  mitk::Image::Pointer k_image = mitk::Image::New();
  k_image->InitializeByItk( kurtosis_filter->GetOutput(1) );
  k_image->SetVolume( kurtosis_filter->GetOutput(1)->GetBufferPointer() );

  std::string outputD_FileName = output_prefix + "_ADC_map.nrrd";
  std::string outputK_FileName = output_prefix + "_AKC_map.nrrd";

  try
  {
    mitk::IOUtil::Save(  d_image, outputD_FileName );
    mitk::IOUtil::Save(  k_image, outputK_FileName );
  }
  catch( const itk::ExceptionObject& e)
  {
    mitkThrow() << "Failed to save the KurtosisFit Results due to exception: " << e.what();
  }

}

int main( int argc, char* argv[] )
{


  mitkCommandLineParser parser;

  parser.setTitle("Diffusion IVIM (Kurtosis) Fit");
  parser.setCategory("Signal Reconstruction");
  parser.setContributor("MIC");
  parser.setDescription("Fitting of IVIM / Kurtosis");

  parser.setArgumentPrefix("--","-");
  // mandatory arguments
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input: ", "input image (DWI)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::String, "Output Preifx: ", "Prefix for the output images, will append _f, _K, _D accordingly ", us::Any(), false);
  parser.addArgument("fit", "f", mitkCommandLineParser::String, "Input: ", "input image (DWI)", us::Any(), false);

  // optional arguments
  parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Masking Image: ", "ROI (segmentation)", us::Any(), true);


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
  std::string out_prefix = us::any_cast<std::string>(parsedArgs["output"]);
  std::string fit_name = us::any_cast<std::string>(parsedArgs["fit"]);

  mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inFileName);
  if( !DPH::IsDiffusionWeightedImage( inputImage ) )
  {
    MITK_ERROR("DiffusionIVIMFit.Input") << "No valid diffusion-weighted image provided, failed to load " << inFileName << " as DW Image. Aborting...";
    return EXIT_FAILURE;
  }

  if( fit_name == "Kurtosis" )
  {
    MITK_INFO("DiffusionIVIMFit.Main") << "-----[ Kurtosis Fit ]-----";

    KurtosisMapComputation( inputImage, out_prefix );

  }
  else if (fit_name == "IVIM" )
  {
    MITK_INFO("DiffusionIVIMFit.Main") << "IVIM Fit not fully implemented yet. Aborting...";
    return EXIT_FAILURE;
  }
  else
  {
    MITK_ERROR("DiffusionIVIMFit.Main") << "Unrecognized option: " << fit_name << ". Valid values [\"IVIM\", \"Kurtosis\"] \n Aborting... \n";
    return EXIT_FAILURE;

  }

}
