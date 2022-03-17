/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageStitchingHelper.h"

#include <itkInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkWindowedSincInterpolateImageFunction.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkGeometry3D.h>
#include <mitkImageToItk.h>
#include <mitkMAPAlgorithmHelper.h>

#include "mapRegistration.h"

#include "mitkRegistrationHelper.h"

template <typename TImage >
typename ::itk::InterpolateImageFunction< TImage >::Pointer generateInterpolator(mitk::ImageMappingInterpolator::Type interpolatorType)
{
  typedef ::itk::InterpolateImageFunction< TImage > BaseInterpolatorType;
  typename BaseInterpolatorType::Pointer result;

  switch (interpolatorType)
  {
  case mitk::ImageMappingInterpolator::NearestNeighbor:
    {
      result = ::itk::NearestNeighborInterpolateImageFunction<TImage>::New();
      break;
    }
  case mitk::ImageMappingInterpolator::BSpline_3:
    {
      typename ::itk::BSplineInterpolateImageFunction<TImage>::Pointer spInterpolator = ::itk::BSplineInterpolateImageFunction<TImage>::New();
      spInterpolator->SetSplineOrder(3);
      result = spInterpolator;
      break;
    }
  case mitk::ImageMappingInterpolator::WSinc_Hamming:
    {
      result = ::itk::WindowedSincInterpolateImageFunction<TImage,4>::New();
      break;
    }
  case mitk::ImageMappingInterpolator::WSinc_Welch:
    {
      result = ::itk::WindowedSincInterpolateImageFunction<TImage,4,::itk::Function::WelchWindowFunction<4> >::New();
      break;
    }
  default:
    {
      result = ::itk::LinearInterpolateImageFunction<TImage>::New();
      break;
    }

  }

  return result;
};

template <typename TPixelType, unsigned int VImageDimension >
void doMITKStitching(const ::itk::Image<TPixelType,VImageDimension>* /*input1*/,
  mitk::Image::Pointer& result,
  std::vector<mitk::Image::ConstPointer> inputs,
  std::vector<::map::core::RegistrationBase::ConstPointer> registrations,
  const mitk::BaseGeometry* resultGeometry,
  const double& paddingValue, itk::StitchStrategy stitchStrategy, mitk::ImageMappingInterpolator::Type interpolatorType)
{
  using ConcreteRegistrationType = ::map::core::Registration<VImageDimension, VImageDimension>;
  using ItkImageType = itk::Image<TPixelType, VImageDimension>;

  using StitchingFilterType = ::itk::StitchImageFilter<ItkImageType, ItkImageType>;
  auto stitcher = StitchingFilterType::New();
  
  stitcher->SetDefaultPixelValue(paddingValue);

  stitcher->SetOutputOrigin(resultGeometry->GetOrigin());

  const auto spacing = resultGeometry->GetSpacing();
  stitcher->SetOutputSpacing(spacing);

  typename StitchingFilterType::DirectionType itkDirection;
  const auto mitkDirection = resultGeometry->GetIndexToWorldTransform()->GetMatrix();
  for (unsigned int i = 0; i < VImageDimension; ++i)
  {
    for (unsigned int j = 0; j < VImageDimension; ++j)
    {
      itkDirection[i][j] = mitkDirection[i][j] / spacing[j];
    }
  }
  stitcher->SetOutputDirection(itkDirection);

  typename ItkImageType::SizeType size;
  size[0] = resultGeometry->GetExtent(0);
  size[1] = resultGeometry->GetExtent(1);
  size[2] = resultGeometry->GetExtent(2);
  stitcher->SetSize(size);
  stitcher->SetNumberOfWorkUnits(1);
  stitcher->SetStitchStrategy(stitchStrategy);

  auto inputIter = inputs.begin();
  auto regIter = registrations.begin();
  unsigned int index = 0;

  while (inputIter != inputs.end())
  {
    auto itkInput = mitk::ImageToItkImage<TPixelType, VImageDimension>(*inputIter);

    auto castedReg = dynamic_cast<const ConcreteRegistrationType*>(regIter->GetPointer());

    auto kernel = dynamic_cast<const ::map::core::RegistrationKernel<VImageDimension, VImageDimension>* >(&(castedReg->getInverseMapping()));
    if (nullptr == kernel)
    {
      mitkThrow() << "Cannot stitch images. At least passed registration object #"<<index<<" doesn't have a valid inverse mapping registration kernel.";
    }

    stitcher->SetInput(index, itkInput, kernel->getTransformModel(), generateInterpolator< ::itk::Image<TPixelType, VImageDimension> >(interpolatorType));
    ++inputIter;
    ++regIter;
    ++index;
  }

  stitcher->Update();
  mitk::CastToMitkImage<>(stitcher->GetOutput(),result);
}

mitk::Image::Pointer
mitk::StitchImages(std::vector<Image::ConstPointer> inputs,
  std::vector<::map::core::RegistrationBase::ConstPointer> registrations,
  const BaseGeometry* resultGeometry,
  const double& paddingValue, itk::StitchStrategy stitchStrategy,
  mitk::ImageMappingInterpolator::Type interpolatorType)
{
  if (inputs.size() != registrations.size())
  {
    mitkThrow() << "Cannot stitch images. Passed inputs vector and registrations vector have different sizes.";
  }

  if (inputs.empty())
  {
    mitkThrow() << "Cannot stitch images. No input images are defined.";
  }

  auto inputDim = inputs.front()->GetDimension();
  auto inputPixelType = inputs.front()->GetPixelType();

  for (const auto& input : inputs)
  {
    if (input->GetDimension() != inputDim)
    {
      mitkThrow() << "Cannot stitch images. Images have different dimensions. Dimeonsion of first input: " << inputDim << "; wrong dimension: " << input->GetDimension();
    }
    if (input->GetPixelType() != inputPixelType)
    {
      mitkThrow() << "Cannot stitch images. Input images have different pixeltypes. The current implementation does only support stitching of images with same pixel type. Dimeonsion of first input: " << inputPixelType.GetTypeAsString() << "; wrong dimension: " << input->GetPixelType().GetTypeAsString();
    }
    if (input->GetTimeSteps() > 1)
    {
      mitkThrow() << "Cannot stitch dynamic images. At least one input image has multiple time steps.";
    }
  }

  for (const auto& reg : registrations)
  {
    if (reg->getMovingDimensions() != inputDim)
    {
      mitkThrow() << "Cannot stitch images. At least one registration has a different moving dimension then the inputs. Dimeonsion of inputs: " << inputDim << "; wrong dimension: " << reg->getMovingDimensions();
    }
    if (reg->getTargetDimensions() != inputDim)
    {
      mitkThrow() << "Cannot stitch images. At least one registration has a different target dimension then the inputs. Dimeonsion of inputs: " << inputDim << "; wrong dimension: " << reg->getTargetDimensions();
    }
  }

  Image::Pointer result;

  AccessFixedDimensionByItk_n(inputs.front(), doMITKStitching, 3, (result, inputs, registrations, resultGeometry, paddingValue, stitchStrategy, interpolatorType));

  return result;
}

mitk::Image::Pointer
mitk::StitchImages(std::vector<Image::ConstPointer> inputs,
  std::vector<MAPRegistrationWrapper::ConstPointer> registrations,
  const BaseGeometry* resultGeometry,
  const double& paddingValue, itk::StitchStrategy stitchStrategy,
  mitk::ImageMappingInterpolator::Type interpolatorType)
{

  std::vector<::map::core::RegistrationBase::ConstPointer> unwrappedRegs;
  for (const auto& reg : registrations)
  {
    if (!reg)
    {
      mitkThrow() << "Cannot stitch images. At least one passed registration wrapper pointer is nullptr.";
    }
    unwrappedRegs.push_back(reg->GetRegistration());
  }

  Image::Pointer result = StitchImages(inputs, unwrappedRegs, resultGeometry, paddingValue, stitchStrategy, interpolatorType);
  return result;
}

mitk::Image::Pointer
mitk::StitchImages(std::vector<Image::ConstPointer> inputs,
  const BaseGeometry* resultGeometry,
  const double& paddingValue, itk::StitchStrategy stitchStrategy,
  mitk::ImageMappingInterpolator::Type interpolatorType)
{
  auto defaultReg = GenerateIdentityRegistration3D();
  std::vector<::map::core::RegistrationBase::ConstPointer> defaultRegs;
  defaultRegs.resize(inputs.size());
  std::fill(defaultRegs.begin(), defaultRegs.end(), defaultReg->GetRegistration());
  
  Image::Pointer result = StitchImages(inputs, defaultRegs, resultGeometry, paddingValue, stitchStrategy, interpolatorType);
  return result;
}

