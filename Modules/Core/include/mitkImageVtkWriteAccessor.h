/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkWriteAccessor_h
#define mitkImageVtkWriteAccessor_h

#include "mitkImageAccessorBase.h"

class vtkImageData;

namespace mitk
{
  class Image;
  class ImageDataItem;

  /**
   * @brief ImageVtkWriteAccessor class provides any image write access which is required by Vtk methods
   * @ingroup Data
   */
  class MITKCORE_EXPORT ImageVtkWriteAccessor : public ImageAccessorBase
  {
  public:
    typedef itk::SmartPointer<Image> ImagePointer;

    /** \brief Creates an ImageVtkWriteAccessor for a whole Image
     *  \param iP specifies the associated Image
     *  \param iDI
     *  \param imageDataVtk
     */
    ImageVtkWriteAccessor(ImagePointer iP, const ImageDataItem *iDI, vtkImageData *imageDataVtk);

    ~ImageVtkWriteAccessor() override;

    vtkImageData *GetVtkImageData() const;

  protected:
    const Image *GetImage() const override;

  private:
    // due to smart pointer issues, the image is only kept as a weak pointer.
    Image *m_Image;
    vtkImageData *m_ImageDataVtk;
  };
}

#endif
