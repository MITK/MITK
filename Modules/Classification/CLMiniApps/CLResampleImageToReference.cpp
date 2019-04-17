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
#ifndef mitkCLResampleImageToReference_cpp
#define mitkCLResampleImageToReference_cpp

#include "mitkCommandLineParser.h"
#include <mitkImageAccessByItk.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageTimeSelector.h>

// ITK
#include "itkLinearInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"


template<typename TPixel, unsigned int VImageDimension>
void
ResampleImageToReferenceFunction(itk::Image<TPixel, VImageDimension>* itkReference, mitk::Image::Pointer moving, std::string ergPath)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;

  // Identify Transform
  typedef itk::IdentityTransform<double, VImageDimension> T_Transform;
  typename T_Transform::Pointer _pTransform = T_Transform::New();
  _pTransform->SetIdentity();

  typedef itk::WindowedSincInterpolateImageFunction< InputImageType, VImageDimension> WindowedSincInterpolatorType;
  typename WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

  typedef itk::LinearInterpolateImageFunction< InputImageType> LinearInterpolateImageFunctionType;
  typename LinearInterpolateImageFunctionType::Pointer lin_interpolator = LinearInterpolateImageFunctionType::New();

  typedef itk::NearestNeighborInterpolateImageFunction< InputImageType> NearestNeighborInterpolateImageFunctionType;
  typename NearestNeighborInterpolateImageFunctionType::Pointer nn_interpolator = NearestNeighborInterpolateImageFunctionType::New();

  typename InputImageType::Pointer itkMoving = InputImageType::New();
  mitk::CastToItkImage(moving,itkMoving);
  typedef itk::ResampleImageFilter<InputImageType, InputImageType>  ResampleFilterType;

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(itkMoving);
  resampler->SetReferenceImage( itkReference );
  resampler->UseReferenceImageOn();
  resampler->SetTransform(_pTransform);
  //if ( sincInterpol)
  //  resampler->SetInterpolator(sinc_interpolator);
  //else
    resampler->SetInterpolator(lin_interpolator);

  resampler->Update();

  // Convert back to mitk
  mitk::Image::Pointer result = mitk::Image::New();
  result->InitializeByItk(resampler->GetOutput());
  GrabItkImageMemory(resampler->GetOutput(), result);
  MITK_INFO << "writing result to: " << ergPath;
  mitk::IOUtil::Save(result, ergPath);
  //return result;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("fix", "f", mitkCommandLineParser::Image, "Input Image", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("moving", "m", mitkCommandLineParser::File, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Extension", "File extension. Default is .nii.gz", us::Any(), false, false, false, mitkCommandLineParser::Output);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Resample Image To Reference");
  parser.setDescription("Resamples an image (moving) to an given image (fix) without additional registration.");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
  {
    return EXIT_FAILURE;
  }
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  mitk::Image::Pointer fix = mitk::IOUtil::Load<mitk::Image>(parsedArgs["fix"].ToString());
  mitk::Image::Pointer moving = mitk::IOUtil::Load<mitk::Image>(parsedArgs["moving"].ToString());
  mitk::Image::Pointer erg = mitk::Image::New();

  AccessByItk_2(fix, ResampleImageToReferenceFunction, moving, parsedArgs["output"].ToString());

}



#endif
