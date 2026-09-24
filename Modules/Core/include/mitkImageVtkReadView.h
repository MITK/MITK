/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkReadView_h
#define mitkImageVtkReadView_h

#include <MitkCoreExports.h>
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>

#include <vtkSmartPointer.h>

class vtkImageData;

namespace mitk
{
  /**
   * \brief Read access to a volume of an image through a vtkImageData of its own.
   *
   * Image::GetVtkImageData() builds the VTK representation of an image on first access and
   * hands the same vtkImageData to every caller, the mappers included; feeding it into a VTK
   * pipeline modifies it as well. Off the thread that owns the data storage, both race with
   * rendering. A view wraps the same pixel buffer without copying it, in a vtkImageData that
   * belongs to the view alone, so it can be used on any thread. The volume stays read-locked
   * for as long as the view exists.
   *
   * \sa Image::GetVtkImageData(), BaseData::PrebuildVtkRepresentation()
   */
  class MITKCORE_EXPORT ImageVtkReadView
  {
  public:
    /**
     * \param[in] image Image to read.
     * \param[in] timeStep Time step of the volume to read.
     * \throw mitk::Exception if image is nullptr, does not have the time step, or has a pixel
     * type or dimension without VTK equivalent.
     */
    ImageVtkReadView(const Image* image, TimeStepType timeStep);

    ImageVtkReadView(const ImageVtkReadView&) = delete;
    ImageVtkReadView& operator=(const ImageVtkReadView&) = delete;

    /** \brief The vtkImageData of this view. It is valid as long as the view exists. */
    vtkImageData* GetVtkImageData() const;

  private:
    Image::ImageDataItemPointer m_Volume;
    ImageReadAccessor m_Accessor;
    vtkSmartPointer<vtkImageData> m_VtkImageData;
  };
}

#endif
