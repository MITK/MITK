/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageVtkReadView.h>

#include <mitkExceptionMacro.h>

#include <vtkImageData.h>

namespace
{
  mitk::Image::ImageDataItemPointer GetVolume(const mitk::Image* image, mitk::TimeStepType timeStep)
  {
    if (nullptr == image)
      mitkThrow() << "Cannot create a VTK view of an image. Passed image is nullptr.";

    if (!image->GetTimeGeometry()->IsValidTimeStep(timeStep))
      mitkThrow() << "Cannot create a VTK view of an image. The image does not have time step " << timeStep << ".";

    return image->GetVolumeData(static_cast<int>(timeStep));
  }
}

mitk::ImageVtkReadView::ImageVtkReadView(const Image* image, TimeStepType timeStep)
  : m_Volume(GetVolume(image, timeStep)),
    m_Accessor(image, m_Volume.GetPointer()),
    m_VtkImageData(m_Volume->CreateVtkImageDataView(image))
{
  if (m_VtkImageData == nullptr)
    mitkThrow() << "Cannot create a VTK view of an image. Its pixel type or dimension has no VTK equivalent.";
}

vtkImageData* mitk::ImageVtkReadView::GetVtkImageData() const
{
  return m_VtkImageData;
}
