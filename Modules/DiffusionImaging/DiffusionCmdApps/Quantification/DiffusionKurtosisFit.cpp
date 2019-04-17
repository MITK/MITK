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

#include "mitkImage.h"
#include <iostream>
#include <usAny.h>
#include <fstream>
#include "mitkIOUtil.h"

#include <itkFileTools.h>
#include <itksys/SystemTools.hxx>


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
#include <mitkPreferenceListReaderOptionsFunctor.h>


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

void KurtosisMapComputation( mitk::Image::Pointer input,
                             std::string output_prefix ,
                             std::string output_type,
                             std::string maskPath,
                             bool omitBZero,
                             double lower,
                             double upper )
{
  DPH::ImageType::Pointer vectorImage = DPH::ImageType::New();
  mitk::CastToItkImage( input, vectorImage );

  typedef itk::DiffusionKurtosisReconstructionImageFilter< short, double > KurtosisFilterType;
  KurtosisFilterType::Pointer kurtosis_filter = KurtosisFilterType::New();
  kurtosis_filter->SetInput( GetBlurredVectorImage( vectorImage, 1.5 ) );
  kurtosis_filter->SetReferenceBValue( DPH::GetReferenceBValue( input.GetPointer() ) );
  kurtosis_filter->SetGradientDirections( DPH::GetGradientContainer( input.GetPointer() ) );
//  kurtosis_filter->SetNumberOfThreads(1);
  kurtosis_filter->SetOmitUnweightedValue(omitBZero);
  kurtosis_filter->SetBoundariesForKurtosis(-lower,upper);
//  kurtosis_filter->SetInitialSolution(const vnl_vector<double>& x0 );


  if(maskPath != "")
  {
    mitk::Image::Pointer segmentation;
    segmentation = mitk::IOUtil::Load<mitk::Image>(maskPath);
    typedef itk::Image< short , 3>            MaskImageType;
    MaskImageType::Pointer vectorSeg = MaskImageType::New() ;
    mitk::CastToItkImage( segmentation, vectorSeg );
    kurtosis_filter->SetImageMask(vectorSeg) ;
  }

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

  std::string outputD_FileName = output_prefix + "_ADC_map." + output_type;
  std::string outputK_FileName = output_prefix + "_AKC_map." + output_type;

  try
  {
    mitk::IOUtil::Save(  d_image,  outputD_FileName );
    mitk::IOUtil::Save(  k_image,  outputK_FileName );
  }
  catch( const itk::ExceptionObject& e)
  {
    mitkThrow() << "Failed to save the KurtosisFit Results due to exception: " << e.what();
  }

}

int main( int argc, char* argv[] )
{

  mitkCommandLineParser parser;

  parser.setTitle("Diffusion Kurtosis Fit");
  parser.setCategory("Diffusion Related Measures");
  parser.setContributor("MIC");
  parser.setDescription("Fitting Kurtosis");
  parser.setArgumentPrefix("--","-");

  // mandatory arguments
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input: ", "input image (DWI)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output Preifx: ", "Prefix for the output images, will append _f, _K, _D accordingly ", us::Any(), false);
  parser.addArgument("output_type", "", mitkCommandLineParser::String, "Output Type: ", "choose data type of output image, e.g. '.nii' or '.nrrd' ", us::Any(), false);

  // optional arguments
  parser.addArgument("mask", "m", mitkCommandLineParser::String, "Masking Image: ", "ROI (segmentation)", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help", "Show this help text");
  parser.addArgument("omitbzero", "om", mitkCommandLineParser::Bool, "Omit b0:", "Omit b0 value during fit (default = false)", us::Any());
  parser.addArgument("lowerkbound", "kl", mitkCommandLineParser::Float, "lower Kbound:", "Set (unsigned) lower boundary for Kurtosis parameter (default = -1000)", us::Any());
  parser.addArgument("upperkbound", "ku", mitkCommandLineParser::Float, "upper Kbound:", "Set upper boundary for Kurtosis parameter (default = 1000)", us::Any());


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h")){
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  // mandatory arguments
  std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
  std::string out_prefix = us::any_cast<std::string>(parsedArgs["output"]);
  std::string maskPath = "";

  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
  mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>(inFileName, &functor);

  bool omitBZero = false;
  double lower = -1000;
  double upper = 1000;
  std::string  out_type = "nrrd";

  if (parsedArgs.count("mask") || parsedArgs.count("m"))
  {
    maskPath = us::any_cast<std::string>(parsedArgs["mask"]);
  }

  if (parsedArgs.count("output_type") || parsedArgs.count("ot"))
  {
    out_type = us::any_cast<std::string>(parsedArgs["output_type"]);
  }

  if (parsedArgs.count("omitbzero") || parsedArgs.count("om"))
  {
    omitBZero = us::any_cast<bool>(parsedArgs["omitbzero"]);
  }

  if (parsedArgs.count("lowerkbound") || parsedArgs.count("kl"))
  {
    lower = us::any_cast<float>(parsedArgs["lowerkbound"]);
  }

  if (parsedArgs.count("upperkbound") || parsedArgs.count("ku"))
  {
    upper = us::any_cast<float>(parsedArgs["upperkbound"]);
  }

  if( !DPH::IsDiffusionWeightedImage( inputImage ) )
  {
    MITK_ERROR("DiffusionIVIMFit.Input") << "No valid diffusion-weighted image provided, failed to load " << inFileName << " as DW Image. Aborting...";
    return EXIT_FAILURE;
  }



KurtosisMapComputation( inputImage,
                        out_prefix ,
                        out_type,
                        maskPath,
                        omitBZero,
                        lower,
                        upper);

}
