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


#ifndef M4C_ITK_IMAGE_MAPPER_IMPL_P_H
#define M4C_ITK_IMAGE_MAPPER_IMPL_P_H

#include "m4cConfigure.h"
#include "m4cITKImageMapperExport.h"
#include "m4cITKImageMapper.h"

#include <QObject>

class m4cITKImageMapperImpl : public QObject, public m4cITKImage2DMapper, public m4cITKImage3DMapper
{

  Q_OBJECT
  Q_INTERFACES(m4cITKImage2DMapper m4cITKImage3DMapper)

public:
  virtual m4cITKImage2DMapper::ResultFloatImageType::Pointer map(const m4cITKImage2DMapper::InputFloatImageType* input, const m4cITKImage2DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const float& paddingValue = 0, const m4cITKImage2DMapper::ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const float& errorValue = 0) const;

  virtual m4cITKImage2DMapper::ResultUShortImageType::Pointer map(const m4cITKImage2DMapper::InputUShortImageType* input, const m4cITKImage2DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const unsigned short& paddingValue = 0, const m4cITKImage2DMapper::ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const unsigned short& errorValue = 0)const;

  virtual m4cITKImage2DMapper::ResultUCharImageType::Pointer map(const m4cITKImage2DMapper::InputUCharImageType* input, const m4cITKImage2DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const unsigned char& paddingValue = 0, const m4cITKImage2DMapper::ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const unsigned char& errorValue = 0) const;

  virtual m4cITKImage3DMapper::ResultFloatImageType::Pointer map(const m4cITKImage3DMapper::InputFloatImageType* input, const m4cITKImage3DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const float& paddingValue = 0, const m4cITKImage3DMapper::ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const float& errorValue = 0) const;

  virtual m4cITKImage3DMapper::ResultUShortImageType::Pointer map(const m4cITKImage3DMapper::InputUShortImageType* input, const m4cITKImage3DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const unsigned short& paddingValue = 0, const m4cITKImage3DMapper::ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const unsigned short& errorValue = 0) const;

  virtual m4cITKImage3DMapper::ResultUCharImageType::Pointer map(const m4cITKImage3DMapper::InputUCharImageType* input, const m4cITKImage3DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError = false, const unsigned char& paddingValue = 0, const m4cITKImage3DMapper::ResultImageDescriptorType* resultDescriptor = NULL,
    bool throwOnMappingError = true, const unsigned char& errorValue = 0) const;

  m4cITKImageMapperImpl();
};

#endif // M4C_IMAGE_MAPPER_P_H
