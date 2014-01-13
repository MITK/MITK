/*=============================================================================

  MAP4CTK - DKFZ MatchPoint for CommonTK
  (c) Copyright, German Cancer Research Center, Heidelberg, Germany

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#ifndef M4C_ITK_IMAGE_MAPPER_H
#define M4C_ITK_IMAGE_MAPPER_H

#include "mapImageMappingTask.h"
#include "mapRegistration.h"
#include "itkImage.h"
#include <QObject>

template <unsigned int VMovingDimensions, unsigned int VTargetDimensions = VMovingDimensions > 
class m4cITKImageMapper
{
public: 
  typedef ::map::core::Registration<VMovingDimensions,VTargetDimensions> RegistrationType;
  typedef ::map::core::FieldRepresentationDescriptor<VTargetDimensions> ResultImageDescriptorType;

  typedef ::itk::Image<float, VMovingDimensions> InputFloatImageType;
  typedef ::itk::Image<float, VTargetDimensions> ResultFloatImageType;
  typedef ::itk::Image<unsigned short, VMovingDimensions> InputUShortImageType;
  typedef ::itk::Image<unsigned short, VTargetDimensions> ResultUShortImageType;
  typedef ::itk::Image<unsigned char, VMovingDimensions> InputUCharImageType;
  typedef ::itk::Image<unsigned char, VTargetDimensions> ResultUCharImageType;

  virtual typename ResultFloatImageType::Pointer map(const InputFloatImageType* input, const RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const float& paddingValue = 0, const ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const float& errorValue = 0) const = 0;

  virtual typename ResultUShortImageType::Pointer map(const InputUShortImageType* input, const RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const unsigned short& paddingValue = 0, const ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const unsigned short& errorValue = 0) const = 0;

  virtual typename ResultUCharImageType::Pointer map(const InputUCharImageType* input, const RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const unsigned char& paddingValue = 0, const ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const unsigned char& errorValue = 0) const = 0;

protected:

  template <typename TPixelType> typename ::itk::Image<TPixelType,VTargetDimensions>::Pointer doMap(const ::itk::Image<TPixelType,VMovingDimensions>* input, const RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const TPixelType& paddingValue, const ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const TPixelType& errorValue) const
  {
    typedef ::map::core::ImageMappingTask<RegistrationType, ::itk::Image<TPixelType,VMovingDimensions>, ::itk::Image<TPixelType,VTargetDimensions> > MappingTaskType;
    typename MappingTaskType::Pointer spTask = MappingTaskType::New();

    spTask->setInputImage(input);
    spTask->setRegistration(registration);
    spTask->setResultImageDescriptor(resultDescriptor);
    spTask->setThrowOnMappingError(throwOnMappingError);
    spTask->setErrorValue(errorValue);
    spTask->setThrowOnPaddingError(throwOnOutOfInputAreaError);
    spTask->setPaddingValue(paddingValue);

    spTask->execute();
    return spTask->getResultImage();
  };

  m4cITKImageMapper() {};
  virtual ~m4cITKImageMapper() {};
};

typedef m4cITKImageMapper<2,2> m4cITKImage2DMapper;
typedef m4cITKImageMapper<3,3> m4cITKImage3DMapper;

Q_DECLARE_INTERFACE(m4cITKImage2DMapper, "de.dkfz.matchpoint.mapper.image.itk.2D")
Q_DECLARE_INTERFACE(m4cITKImage3DMapper, "de.dkfz.matchpoint.mapper.image.itk.3D")

#endif // M4C_MAPPER_P_H
