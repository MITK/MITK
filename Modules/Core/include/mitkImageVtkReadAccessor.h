/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkReadAccessor_h
#define mitkImageVtkReadAccessor_h

#include <mitkImageAccessorBase.h>

class vtkImageData;

namespace mitk
{
  class Image;
  class ImageDataItem;

  /**
   * \brief Provides read access to image data in the form required by VTK methods.
   *
   * This class locks the image data for reading and provides access to the
   * underlying vtkImageData representation.
   *
   * \ingroup Data
   */
  class MITKCORE_EXPORT ImageVtkReadAccessor : public ImageAccessorBase
  {
  public:
    /** \brief Creates an ImageVtkReadAccessor for a whole Image.
     *  \param iP The associated Image (const smart pointer).
     *  \param iDI The image data item describing the allocated image part.
     *  \param imageDataVtk The VTK image data representation to provide access to.
     */
    ImageVtkReadAccessor(ImageConstPointer iP, const ImageDataItem *iDI, const vtkImageData *imageDataVtk);

    /** \brief Destructor unregisters this accessor from the image. */
    ~ImageVtkReadAccessor() override;

    /** \brief Returns the VTK image data for read access. */
    const vtkImageData *GetVtkImageData() const;

  protected:
    /** \brief Returns a const pointer to the associated Image. */
    const Image *GetImage() const override;

  private:
    // due to smart pointer issues, the image is only kept as a weak pointer.
    const Image *m_Image;
    const vtkImageData *m_ImageDataVtk;
  };
}

#endif
