/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMaskGenerator.h>

namespace mitk
{

MaskGenerator::MaskGenerator():
    m_TimeStep(0)
{
    m_inputImage = nullptr;
}

void MaskGenerator::SetTimeStep(unsigned int timeStep)
{
    if (timeStep != m_TimeStep)
    {
        m_TimeStep = timeStep;
    }
}

void MaskGenerator::SetInputImage(mitk::Image::ConstPointer inputImg)
{
    if (inputImg != m_inputImage)
    {
        m_inputImage = inputImg;
        this->Modified();
    }
}

mitk::Image::ConstPointer MaskGenerator::GetReferenceImage()
{
    return m_inputImage;
}
}
