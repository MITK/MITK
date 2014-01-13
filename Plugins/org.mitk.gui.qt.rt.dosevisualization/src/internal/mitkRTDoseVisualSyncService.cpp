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


#include "m4cITKImageMapperImpl_p.h"

  m4cITKImageMapperImpl::m4cITKImageMapperImpl()
  {
  };

  m4cITKImage2DMapper::ResultFloatImageType::Pointer
    m4cITKImageMapperImpl::
    map(const m4cITKImage2DMapper::InputFloatImageType* input, const m4cITKImage2DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const float& paddingValue, const m4cITKImage2DMapper::ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const float& errorValue) const
  {
    return m4cITKImage2DMapper::doMap(input, registration, throwOnOutOfInputAreaError, paddingValue, resultDescriptor, throwOnMappingError, errorValue);
  };

  m4cITKImage2DMapper::ResultUShortImageType::Pointer
    m4cITKImageMapperImpl::
    map(const m4cITKImage2DMapper::InputUShortImageType* input, const m4cITKImage2DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const unsigned short& paddingValue, const m4cITKImage2DMapper::ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const unsigned short& errorValue) const
  {
    return m4cITKImage2DMapper::doMap(input, registration, throwOnOutOfInputAreaError, paddingValue, resultDescriptor, throwOnMappingError, errorValue);
  };

  m4cITKImage2DMapper::ResultUCharImageType::Pointer
    m4cITKImageMapperImpl::
    map(const m4cITKImage2DMapper::InputUCharImageType* input, const m4cITKImage2DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const unsigned char& paddingValue, const m4cITKImage2DMapper::ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const unsigned char& errorValue) const
  {
    return m4cITKImage2DMapper::doMap(input, registration, throwOnOutOfInputAreaError, paddingValue, resultDescriptor, throwOnMappingError, errorValue);
  };

  m4cITKImage3DMapper::ResultFloatImageType::Pointer
    m4cITKImageMapperImpl::
    map(const m4cITKImage3DMapper::InputFloatImageType* input, const m4cITKImage3DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const float& paddingValue, const m4cITKImage3DMapper::ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const float& errorValue) const
  {
    return m4cITKImage3DMapper::doMap(input, registration, throwOnOutOfInputAreaError, paddingValue, resultDescriptor, throwOnMappingError, errorValue);
  };

  m4cITKImage3DMapper::ResultUShortImageType::Pointer
    m4cITKImageMapperImpl::
    map(const m4cITKImage3DMapper::InputUShortImageType* input, const m4cITKImage3DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const unsigned short& paddingValue, const m4cITKImage3DMapper::ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const unsigned short& errorValue) const
  {
    return m4cITKImage3DMapper::doMap(input, registration, throwOnOutOfInputAreaError, paddingValue, resultDescriptor, throwOnMappingError, errorValue);
  };

  m4cITKImage3DMapper::ResultUCharImageType::Pointer
    m4cITKImageMapperImpl::
    map(const m4cITKImage3DMapper::InputUCharImageType* input, const m4cITKImage3DMapper::RegistrationType* registration,
    bool throwOnOutOfInputAreaError, const unsigned char& paddingValue, const m4cITKImage3DMapper::ResultImageDescriptorType* resultDescriptor,
    bool throwOnMappingError, const unsigned char& errorValue) const
  {
    return m4cITKImage3DMapper::doMap(input, registration, throwOnOutOfInputAreaError, paddingValue, resultDescriptor, throwOnMappingError, errorValue);
  };
