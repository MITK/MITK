/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIMAGEVTKREADACCESSOR_H
#define MITKIMAGEVTKREADACCESSOR_H

#include "mitkImageAccessorBase.h"

class vtkImageData;

namespace mitk
{
  class Image;
  class ImageDataItem;

  /**
   * @brief ImageVtkReadAccessor class provides any image read access which is required by Vtk methods
   * @ingroup Data
   */
  class MITKCORE_EXPORT ImageVtkReadAccessor : public ImageAccessorBase
  {
  public:
    /** \brief Creates an ImageVtkWriteAccessor for a whole Image
     *  \param iP specifies the associated Image
     *  \param iDI
     *  \param imageDataVtk
     */
    ImageVtkReadAccessor(ImageConstPointer iP, const ImageDataItem *iDI, const vtkImageData *imageDataVtk);

    ~ImageVtkReadAccessor() override;

    const vtkImageData *GetVtkImageData() const;

    // vtkTypeMacro(ImageVtkWriteAccessor,vtkDataSet)

  protected:
    const Image *GetImage() const override;

  private:
    // due to smart pointer issues, the image is only kept as a weak pointer.
    const Image *m_Image;
    const vtkImageData *m_ImageDataVtk;
  };
}

#endif // MITKIMAGEVTKREADACCESSOR_H
