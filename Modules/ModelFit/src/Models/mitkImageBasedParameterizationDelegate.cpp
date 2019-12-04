/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageBasedParameterizationDelegate.h"

#include "mitkImagePixelReadAccessor.h"
#include "mitkPixelTypeMultiplex.h"

mitk::ImageBasedParameterizationDelegate::ImageBasedParameterizationDelegate()
{
}

mitk::ImageBasedParameterizationDelegate::~ImageBasedParameterizationDelegate()
{
}

mitk::ImageBasedParameterizationDelegate::ParametersType mitk::ImageBasedParameterizationDelegate::GetInitialParameterization() const
{
    itk::Index<3> index = {{0, 0, 0}};
    return GetInitialParameterization(index);
}

template <typename TPixel>
static void AccessPixel(mitk::PixelType /*ptype*/, mitk::Image::ConstPointer im, const mitk::ImageBasedParameterizationDelegate::IndexType &currentPosition, mitk::ImageBasedParameterizationDelegate::ParametersType::ValueType &val)
{
  mitk::ImagePixelReadAccessor<TPixel, 3> access(im);
  val = access.GetPixelByIndex(currentPosition);
}

mitk::ImageBasedParameterizationDelegate::ParametersType mitk::ImageBasedParameterizationDelegate::GetInitialParameterization(const IndexType &currentPosition) const
{
    ParametersType initialParameters = this->m_Parameterization;

    for (const auto& pos : this->m_ParameterImageMap)
    {
      ParametersType::ValueType value = 0.0;
      mitkPixelTypeMultiplex3(AccessPixel, pos.second->GetChannelDescriptor().GetPixelType(), pos.second, currentPosition, value);

      initialParameters[pos.first] = value;
    }

    return initialParameters;
}

void
mitk::ImageBasedParameterizationDelegate::AddInitialParameterImage(const mitk::Image* image, ParametersType::size_type paramIndex)
{
  if (paramIndex >= this->m_Parameterization.Size())
  {
    mitkThrow() << "Initial value image is set for invalid parameter index. Number of parameter: " << this->m_Parameterization.Size() << "; invalid index: " << paramIndex;
  }

  m_ParameterImageMap.insert(std::make_pair(paramIndex, image));
};
