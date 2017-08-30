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

#include <itkInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkWindowedSincInterpolateImageFunction.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkGeometry3D.h>
#include <mitkImageToItk.h>
#include <mitkImageTimeSelector.h>

#include "mapRegistration.h"

#include "mitkImageMappingHelper.h"
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
void doMITKMap(const ::itk::Image<TPixelType,VImageDimension>* input, mitk::ImageMappingHelper::ResultImageType::Pointer& result, const mitk::ImageMappingHelper::RegistrationType*& registration,
  bool throwOnOutOfInputAreaError, const double& paddingValue, const mitk::ImageMappingHelper::ResultImageGeometryType*& resultGeometry,
  bool throwOnMappingError, const double& errorValue, mitk::ImageMappingInterpolator::Type interpolatorType)
{
  typedef ::map::core::Registration<VImageDimension,VImageDimension> ConcreteRegistrationType;
  typedef ::map::core::ImageMappingTask<ConcreteRegistrationType, ::itk::Image<TPixelType,VImageDimension>, ::itk::Image<TPixelType,VImageDimension> > MappingTaskType;
  typename MappingTaskType::Pointer spTask = MappingTaskType::New();

  typedef typename MappingTaskType::ResultImageDescriptorType ResultImageDescriptorType;
  typename ResultImageDescriptorType::Pointer resultDescriptor;

  //check if image and result geometry fits the passed registration
  /////////////////////////////////////////////////////////////////
  if (registration->getMovingDimensions()!=VImageDimension)
  {
    map::core::OStringStream str;
    str << "Dimension of MITK image ("<<VImageDimension<<") does not equal the moving dimension of the registration object ("<<registration->getMovingDimensions()<<").";
    throw mitk::AccessByItkException(str.str());
  }

  if (registration->getTargetDimensions()!=VImageDimension)
  {
    map::core::OStringStream str;
    str << "Dimension of MITK image ("<<VImageDimension<<") does not equal the target dimension of the registration object ("<<registration->getTargetDimensions()<<").";
    throw mitk::AccessByItkException(str.str());
  }

  const ConcreteRegistrationType* castedReg = dynamic_cast<const ConcreteRegistrationType*>(registration);

  if (registration->getTargetDimensions()==2 && resultGeometry)
  {
    mitk::ImageMappingHelper::ResultImageGeometryType::BoundsArrayType bounds = resultGeometry->GetBounds();

    if (bounds[4]!=0 || bounds[5]!=0)
    {
      //array "bounds" is constructed as [min Dim1, max Dim1, min Dim2, max Dim2, min Dim3, max Dim3]
      //therfore [4] and [5] must be 0

      map::core::OStringStream str;
      str << "Dimension of defined result geometry does not equal the target dimension of the registration object ("<<registration->getTargetDimensions()<<").";
      throw mitk::AccessByItkException(str.str());
    }
  }

  //check/create resultDescriptor
  /////////////////////////
  if (resultGeometry)
  {
    resultDescriptor = ResultImageDescriptorType::New();

    typename ResultImageDescriptorType::PointType origin;
    typename ResultImageDescriptorType::SizeType size;
    typename ResultImageDescriptorType::SpacingType fieldSpacing;
    typename ResultImageDescriptorType::DirectionType matrix;

    mitk::ImageMappingHelper::ResultImageGeometryType::BoundsArrayType geoBounds = resultGeometry->GetBounds();
    mitk::Vector3D geoSpacing = resultGeometry->GetSpacing();
    mitk::Point3D geoOrigin = resultGeometry->GetOrigin();
    mitk::AffineTransform3D::MatrixType geoMatrix = resultGeometry->GetIndexToWorldTransform()->GetMatrix();

    for (unsigned int i = 0; i<VImageDimension; ++i)
    {
      origin[i] = static_cast<typename ResultImageDescriptorType::PointType::ValueType>(geoOrigin[i]);
      fieldSpacing[i] = static_cast<typename ResultImageDescriptorType::SpacingType::ValueType>(geoSpacing[i]);
      size[i] = static_cast<typename ResultImageDescriptorType::SizeType::SizeValueType>(geoBounds[(2*i)+1]-geoBounds[2*i])*fieldSpacing[i];
    }

    //Matrix extraction
    matrix.SetIdentity();
    unsigned int i;
    unsigned int j;

    /// \warning 2D MITK images could have a 3D rotation, since they have a 3x3 geometry matrix.
    /// If it is only a rotation around the transversal plane normal, it can be express with a 2x2 matrix.
    /// In this case, the ITK image conservs this information and is identical to the MITK image!
    /// If the MITK image contains any other rotation, the ITK image will have no rotation at all.
    /// Spacing is of course conserved in both cases.

    // the following loop devides by spacing now to normalize columns.
    // counterpart of InitializeByItk in mitkImage.h line 372 of revision 15092.

    // Check if information is lost
    if (  VImageDimension == 2)
    {
      if (  ( geoMatrix[0][2] != 0) ||
        ( geoMatrix[1][2] != 0) ||
        ( geoMatrix[2][0] != 0) ||
        ( geoMatrix[2][1] != 0) ||
        (( geoMatrix[2][2] != 1) &&  ( geoMatrix[2][2] != -1) ))
      {
        // The 2D MITK image contains 3D rotation information.
        // This cannot be expressed in a 2D ITK image, so the ITK image will have no rotation
      }
      else
      {
        // The 2D MITK image can be converted to an 2D ITK image without information loss!
        for ( i=0; i < 2; ++i)
        {
          for( j=0; j < 2; ++j )
          {
            matrix[i][j] = geoMatrix[i][j]/fieldSpacing[j];
          }
        }
      }
    }
    else if (VImageDimension == 3)
    {
      // Normal 3D image. Conversion possible without problem!
      for ( i=0; i < 3; ++i)
      {
        for( j=0; j < 3; ++j )
        {
          matrix[i][j] = geoMatrix[i][j]/fieldSpacing[j];
        }
      }
    }
    else
    {
      assert(0);
      throw mitk::AccessByItkException("Usage of resultGeometry for 2D images is not yet implemented.");
      /**@TODO Implement extraction of 2D-Rotation-Matrix out of 3D-Rotation-Matrix
      * to cover this case as well.
      * matrix = extract2DRotationMatrix(resultGeometry)*/
    }

    resultDescriptor->setOrigin(origin);
    resultDescriptor->setSize(size);
    resultDescriptor->setSpacing(fieldSpacing);
    resultDescriptor->setDirection(matrix);
  }

  //do the mapping
  /////////////////////////
  typedef ::itk::InterpolateImageFunction< ::itk::Image<TPixelType,VImageDimension> > BaseInterpolatorType;
  typename BaseInterpolatorType::Pointer interpolator = generateInterpolator< ::itk::Image<TPixelType,VImageDimension> >(interpolatorType);
  assert(interpolator.IsNotNull());
  spTask->setImageInterpolator(interpolator);
  spTask->setInputImage(input);
  spTask->setRegistration(castedReg);
  spTask->setResultImageDescriptor(resultDescriptor);
  spTask->setThrowOnMappingError(throwOnMappingError);
  spTask->setErrorValue(errorValue);
  spTask->setThrowOnPaddingError(throwOnOutOfInputAreaError);
  spTask->setPaddingValue(paddingValue);

  spTask->execute();
  mitk::CastToMitkImage<>(spTask->getResultImage(),result);
}

mitk::ImageMappingHelper::ResultImageType::Pointer
  mitk::ImageMappingHelper::map(const InputImageType* input, const RegistrationType* registration,
  bool throwOnOutOfInputAreaError, const double& paddingValue, const ResultImageGeometryType* resultGeometry,
  bool throwOnMappingError, const double& errorValue, mitk::ImageMappingInterpolator::Type interpolatorType)
{
  if (!registration)
  {
    mitkThrow() << "Cannot map image. Passed registration wrapper pointer is nullptr.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot map image. Passed image pointer is nullptr.";
  }

  ResultImageType::Pointer result;

  if(input->GetTimeSteps()==1)
  { //map the image and done
    AccessByItk_n(input, doMITKMap, (result, registration, throwOnOutOfInputAreaError, paddingValue, resultGeometry, throwOnMappingError, errorValue, interpolatorType));
  }
  else
  { //map every time step and compose

    mitk::TimeGeometry::ConstPointer timeGeometry = input->GetTimeGeometry();
    mitk::TimeGeometry::Pointer mappedTimeGeometry = timeGeometry->Clone();

    for (unsigned int i = 0; i<input->GetTimeSteps(); ++i)
    {
      ResultImageGeometryType::Pointer mappedGeometry = resultGeometry->Clone();
      mappedTimeGeometry->SetTimeStepGeometry(mappedGeometry,i);
    }

    result = mitk::Image::New();
    result->Initialize(input->GetPixelType(),*mappedTimeGeometry, 1, input->GetTimeSteps());

    for (unsigned int i = 0; i<input->GetTimeSteps(); ++i)
    {
      mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
      imageTimeSelector->SetInput(input);
      imageTimeSelector->SetTimeNr(i);
      imageTimeSelector->UpdateLargestPossibleRegion();

      InputImageType::Pointer timeStepInput = imageTimeSelector->GetOutput();
      ResultImageType::Pointer timeStepResult;
      AccessByItk_n(timeStepInput, doMITKMap, (timeStepResult, registration, throwOnOutOfInputAreaError, paddingValue, resultGeometry, throwOnMappingError, errorValue, interpolatorType));
      mitk::ImageReadAccessor readAccess(timeStepResult);
      result->SetVolume(readAccess.GetData(),i);
    }
  }

  return result;
}

mitk::ImageMappingHelper::ResultImageType::Pointer
  mitk::ImageMappingHelper::map(const InputImageType* input, const MITKRegistrationType* registration,
  bool throwOnOutOfInputAreaError, const double& paddingValue, const ResultImageGeometryType* resultGeometry,
  bool throwOnMappingError, const double& errorValue, mitk::ImageMappingInterpolator::Type)
{
  if (!registration)
  {
    mitkThrow() << "Cannot map image. Passed registration wrapper pointer is nullptr.";
  }
  if (!registration->GetRegistration())
  {
    mitkThrow() << "Cannot map image. Passed registration wrapper containes no registration.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot map image. Passed image pointer is nullptr.";
  }

  ResultImageType::Pointer result = map(input, registration->GetRegistration(), throwOnOutOfInputAreaError, paddingValue, resultGeometry, throwOnMappingError, errorValue);
  return result;
}


mitk::ImageMappingHelper::ResultImageType::Pointer
  mitk::ImageMappingHelper::
  refineGeometry(const InputImageType* input, const RegistrationType* registration,
  bool throwOnError)
{
  mitk::ImageMappingHelper::ResultImageType::Pointer result = nullptr;

  if (!registration)
  {
    mitkThrow() << "Cannot refine image geometry. Passed registration pointer is nullptr.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot refine image geometry. Passed image pointer is nullptr.";
  }

  mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer spTransform = mitk::MITKRegistrationHelper::getAffineMatrix(registration,false);
  if(spTransform.IsNull() && throwOnError)
  {
    mitkThrow() << "Cannot refine image geometry. Registration does not contain a suitable direct mapping kernel (3D affine transformation or compatible required).";
  }

  if(spTransform.IsNotNull())
  {
    //copy input image
    result = input->Clone();

    //refine geometries
    for(unsigned int i = 0; i < result->GetTimeSteps(); ++i)
    { //refine every time step
      result->GetGeometry(i)->Compose(spTransform);
    }
    result->GetTimeGeometry()->Update();
  }

  return result;
}

mitk::ImageMappingHelper::ResultImageType::Pointer
  mitk::ImageMappingHelper::
  refineGeometry(const InputImageType* input, const MITKRegistrationType* registration,
  bool throwOnError)
{
  if (!registration)
  {
    mitkThrow() << "Cannot refine image geometry. Passed registration wrapper pointer is nullptr.";
  }
  if (!registration->GetRegistration())
  {
    mitkThrow() << "Cannot refine image geometry. Passed registration wrapper containes no registration.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot refine image geometry. Passed image pointer is nullptr.";
  }

  ResultImageType::Pointer result = refineGeometry(input, registration->GetRegistration(), throwOnError);
  return result;
}

bool
  mitk::ImageMappingHelper::
  canRefineGeometry(const RegistrationType* registration)
{
  bool result = true;

  if (!registration)
  {
    mitkThrow() << "Cannot check refine capability of registration. Passed registration pointer is nullptr.";
  }

  //if the helper does not return null, we can refine the geometry.
  result = mitk::MITKRegistrationHelper::getAffineMatrix(registration,false).IsNotNull();

  return result;
}

bool
  mitk::ImageMappingHelper::
  canRefineGeometry(const MITKRegistrationType* registration)
{
  if (!registration)
  {
    mitkThrow() << "Cannot check refine capability of registration. Passed registration wrapper pointer is nullptr.";
  }
  if (!registration->GetRegistration())
  {
    mitkThrow() << "Cannot check refine capability of registration. Passed registration wrapper containes no registration.";
  }

  return canRefineGeometry(registration->GetRegistration());
}

