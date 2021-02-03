/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
ResampleImageToReferenceFunction(itk::Image<TPixel, VImageDimension>* itkReference, mitk::Image::Pointer moving, std::string ergPath, int interp=0)
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
  
  switch(interp)
  {
  case 0:
  {
	resampler->SetInterpolator(lin_interpolator);
	break;
  }
  case 1:
  {
	resampler->SetInterpolator(nn_interpolator);
	break;
  }
  case 2:
  {
	resampler->SetInterpolator(sinc_interpolator);
	break;
  }
  default:
    resampler->SetInterpolator(lin_interpolator);
  }

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
  parser.addArgument("fix", "f", mitkCommandLineParser::Image, "Fixed Image", "fixed image file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("moving", "m", mitkCommandLineParser::File, "Moving Image", "moving image file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output Image", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("interpolator", "", mitkCommandLineParser::Int, "Interpolator:", "interpolator type: 0=linear (default), 1=nearest neighbor, 2=sinc", 0);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Resample Image To Reference");
  parser.setDescription("Resamples an image (moving) to an given image (fix) without additional registration.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

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
  
  int interpolator = 0;
  if (parsedArgs.count("interpolator"))
    interpolator = us::any_cast<int>(parsedArgs["interpolator"]);
	
  AccessByItk_3(fix, ResampleImageToReferenceFunction, moving, parsedArgs["output"].ToString(), interpolator);

}



#endif
